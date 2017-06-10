/*===========================================================================

                        QCMAP_WEB_CLIENT.CPP

DESCRIPTION

  Web CLIENT which will service Web server requests/responses
  and bridges with QCMAP Manager.

EXTERNALIZED FUNCTIONS


  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/
/*===========================================================================

when        who    what, where, why
--------    ---    ----------------------------------------------------------
01/08/14    rk     Webserver paswd encryption.
11/22/13    rk     Webserver security flaws fixed.
30/08/13    at     Added support for UPnP DLNA notify interval.
17/07/13    at     Fix for WWAN statistics display.
30/04/13    at     Added support for Firewall and IPV6 page.
20/02/13    rk     Created module.
===========================================================================*/

/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/
#include "QCMAP_Web_CLIENT.h"
#include "QCMAP_Client.h"

#ifdef __cplusplus
  extern "C" {
#endif

/*==========================================================================
                                DATA TYPES
===========================================================================*/
QCMAP_Client *QcMapClient =NULL;
/* Server sockets */
uint32_t qcmap_webclient_sockfd;
//FD set used to hold sockets for select.
fd_set initial_sockfd;
qmi_error_type_v01 qmi_error;
boolean mobile_ap_enable_flag = 0;
const char *QMI_Error_String[] =
{
  "SUCCESS",
  "MALFORMED MSG",
  "NO MEMORY",
  "INTERNAL ERROR",
  "ABORTED PROCESSING",
  "CLIENT IDS EXHAUSTED",
  "UNABORTABLE TRANSACTION",
  "INVALID CLIENT ID",
  "NO THRESHOLDS",
  "INVALID HANDLE"  ,
  "INVALID PROFILE"  ,
  "INVALID PINID"  ,
  "INCORRECT PIN"  ,
  "NO NETWORK FOUND"  ,
  "CALL FAILED"  ,
  "OUT OF CALL"  ,
  "NOT PROVISIONED"  ,
  "MISSING ARG"  ,
  "UNDEFINED",
  "ARG TOO LONG"  ,
  "UNDEFINED",
  "UNDEFINED",
  "INVALID TX ID"  ,
  "DEVICE IN USE"  ,
  "OP NETWORK UNSUPPORTED"  ,
  "OP DEVICE UNSUPPORTED"  ,
  "NO EFFECT"  ,
  "NO FREE PROFILE"  ,
  "INVALID PDP TYPE"  ,
  "INVALID TECH PREF"  ,
  "INVALID PROFILE TYPE"  ,
  "INVALID SERVICE TYPE"  ,
  "INVALID REGISTER ACTION"  ,
  "INVALID PS ATTACH ACTION"  ,
  "AUTHENTICATION FAILED"  ,
  "PIN BLOCKED"  ,
  "PIN PERM BLOCKED"  ,
  "SIM NOT INITIALIZED"  ,
  "MAX QOS REQUESTS IN USE"  ,
  "INCORRECT FLOW FILTER"  ,
  "NETWORK QOS UNAWARE"  ,
  "INVALID ID"  ,
  "REQUESTED NUM UNSUPPORTED"  ,
  "INTERFACE NOT FOUND"  ,
  "FLOW SUSPENDED"  ,
  "INVALID DATA FORMAT"  ,
  "GENERAL"  ,
  "UNKNOWN"  ,
  "INVALID ARG"  ,
  "INVALID INDEX"  ,
  "NO ENTRY"  ,
  "DEVICE STORAGE FULL"  ,
  "DEVICE NOT READY"  ,
  "NETWORK NOT READY"  ,
  "CAUSE CODE"  ,
  "MESSAGE NOT SENT"  ,
  "MESSAGE DELIVERY FAILURE"  ,
  "INVALID MESSAGE ID"  ,
  "ENCODING"  ,
  "AUTHENTICATION LOCK"  ,
  "INVALID TRANSITION"  ,
  "NOT A MCAST IFACE"  ,
  "MAX MCAST REQUESTS IN USE"  ,
  "INVALID MCAST HANDLE"  ,
  "INVALID IP FAMILY PREF"  ,
  "SESSION INACTIVE"  ,
  "SESSION INVALID"  ,
  "SESSION OWNERSHIP"  ,
  "INSUFFICIENT RESOURCES"  ,
  "DISABLED"  ,
  "INVALID OPERATION"  ,
  "INVALID QMI CMD"  ,
  "TPDU TYPE"  ,
  "SMSC ADDR"  ,
  "INFO UNAVAILABLE"  ,
  "SEGMENT TOO LONG"  ,
  "SEGMENT ORDER"  ,
  "BUNDLING NOT SUPPORTED"  ,
  "OP PARTIAL FAILURE"  ,
  "POLICY MISMATCH"  ,
  "SIM FILE NOT FOUND"  ,
  "EXTENDED INTERNAL"  ,
  "ACCESS DENIED"  ,
  "HARDWARE RESTRICTED"  ,
  "ACK NOT SENT"  ,
  "INJECT TIMEOUT"  ,
  "UNDEFINED",
  "UNDEFINED",
  "UNDEFINED",
  "UNDEFINED",
  "INCOMPATIBLE STATE"  ,
  "FDN RESTRICT"  ,
  "SUPS FAILURE CAUSE"  ,
  "NO RADIO"  ,
  "NOT SUPPORTED"  ,
  "NO SUBSCRIPTION"  ,
  "CARD CALL CONTROL FAILED"  ,
  "NETWORK ABORTED"  ,
  "MSG BLOCKED"  ,
  "UNDEFINED",
  "INVALID SESSION TYPE"  ,
  "INVALID PB TYPE"  ,
  "NO SIM"  ,
  "PB NOT READY"  ,
  "PIN RESTRICTION"  ,
  "PIN2 RESTRICTION"  ,
  "PUK RESTRICTION"  ,
  "PUK2 RESTRICTION"  ,
  "PB ACCESS RESTRICTED"  ,
  "PB DELETE IN PROG"  ,
  "PB TEXT TOO LONG"  ,
  "PB NUMBER TOO LONG"  ,
  "PB HIDDEN KEY RESTRICTION"
};

const char *firewall_string[] =
{
  "handle",
  "IPversion",
 "nxthdrprotovld",
  "nxthdrproto",
  "tcpudpsrcvld",
  "tcpudpsrcport",
  "tcpudpsrcrange",
  "tcpudpdstvld",
  "tcpudpdstport",
  "tcpudpdstrange",
  "icmptypvld",
  "icmptyp",
  "icmpcodevld",
  "icmpcode",
  "espspivld",
  "espspi",
  "ipsrcaddrvld",
  "ipsrcaddr",
  "ipsrcsubnet",
  "iptosvld",
  "iptosvlv",
  "iptosmsk",
  "ip6srcvld",
  "ip6srcaddr",
  "ip6srclen",
  "ip6trsclfvld",
  "ip6trsclfvlv",
  "ip6trsclfmsk"
};

const char *nat_settings_str[]=
{
"Page",
"mask",
"nattype",
"nattype_result",
"dmzip",
"dmzip_result",
"ipsec",
"ipsec_result",
"pptp",
"pptp_result",
"l2tp",
"l2tp_result",
"gen_timeout",
"gen_timeout_result",
"icmp_timeout",
"icmp_timeout_result",
"tcp_timeout",
"tcp_timeout_result",
"udp_timeout",
"udp_timeout_result",
"wwan_access",
"wwan_access_result"
};

const char *lan_config_str[]=
{
"Page",
"Mask",
"lan_config_result",
"wlan_status_result",
"wlan_config_result",
"lan_gw_addrs",
"lan_sub",
"lan_dhcp",
"lan_dhcp_start",
"lan_dhcp_end",
"lan_dhcp_lease",
"wlan_status",
"wlan_mode",
"wlan_access",
"ap_sta_dhcp",
"ap_sta_ip",
"ap_sta_gw_addr",
"ap_sta_dns",
"ap_sta_sub"
};
const char *WWAN_v6_string[]=
{
  "Page",
  "mask",
  "Enable",
  "Enable_result",
  "backhaul",
  "backhaul_result",
  "Policy_result"
};

const char *WWAN_statistic_string[]=
{
"Bytes_Rx",
"Bytes_Tx",
"Packets_Rx",
"Packets_Tx",
"Packet_Drop_Rx",
"Packet_Drop_Tx"
};

const char *WWAN_Status_String[]=
{
"error",
// for V4
"connecting ",
"connecting_fail",
"connected",
"disconnecting",
"disconnecting_fail",
"disconnected",
// for v6
"connecting ",
"connecting_fail",
"connected",
"disconnecting",
"disconnecting_fail",
"disconnected"
};
const char *STA_String[]=
{
"notused",
"connected",
"disconnected"
};

const char *get_wwan_stats_string[]=
{
"Page",
"reset",
"ip_family",
"Result",
"Bytes_Rx",
"Bytes_Tx",
"Packets_Rx",
"Packets_Tx",
"Packet_Drop_Rx",
"Packet_Drop_Tx",
"Config_error",
"Public_IP",
"Primary_DNS",
"Secondary_DNS",
"backhaul_result",
"backhaul",
"station_result",
"station_mode"
};
const char *get_data_rate_string[]=
{
"Page",
"Result",
"Bytes_Tx",
"Bytes_Tx",
"Bytes_Tx_max",
"Bytes_Rx_max"
};

const char *ipv6_state_string[]=
{
"Page",
"enable_error",
"enable",
"V6status_result",
"V6status",
"policy_result",
"Tech_pref",
"profile_id_3gpp",
"profile_id_3gpp2"
};

const char *mobileap_str[]=
{
"Page",
"mask",
"autoconnect",
"autoconnect_result",
"roaming",
"roaming_result"
};
const char *get_media_dir_str[]=
{
"Page",
"Result",
"Dir"
};

const char *set_media_dir_str[]=
{
"Page",
"Operation",
"Dir",
"Result"
};

const char *upnp_dlna_str[]=
{
"Page",
"mask",
"upnp_result",
"upnp_enable",
"dlna_result",
"dlna_enable",
"mdns_result",
"mdns_enable"
};

const char *wwan_ipv4_str[]=
{
"Page",
"mask",
"enable",
"enable_result",
"backhaul",
"backhaul_result",
"tech_result",
"tech_pref",
"profile_id_3gpp",
"profile_id_3gpp2"
};

const char *upnp_dlna_notify_str[]=
{
"Page",
"mask",
"upnp_interval_result",
"upnp_interval",
"dlna_interval_result",
"dlna_interval"
};

/*===========================================================================
  FUNCTION send_response_webclient_socket
===========================================================================*/
/*!
@brief
  Send response back to WEB CGI

@input
  qcmap_webcli_buffer - buffer to send.
  nLength             - lenght of the buffer.
  their_addr          - socket address to which data needs to sent.
  addr_len            - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
 /*=========================================================================*/
static int send_response_webclient_socket
(
  char* qcmap_webcli_buffer,
  int nLength,
  sockaddr_un their_addr,
  socklen_t addr_len
)
{
  int numBytes=0;
  socklen_t len;
  int test_mode = 0, nbytes=0;

  /* Send the response to Webserver  socket. */
  if ((numBytes = sendto(qcmap_webclient_sockfd, (void *) qcmap_webcli_buffer,
                          nLength, 0,
     (struct sockaddr *)&their_addr, addr_len))  <= 0 )
  {
    LOG_MSG_ERROR("Send Failed from webcli interface context numBytes %d %d \
                   %d\n ", numBytes, 0, 0);
    return QCMAP_CM_ERROR;
  }
  return QCMAP_CM_SUCCESS;
}
/*===========================================================================
  FUNCTION ResetWWANStats
===========================================================================*/
/*!
@brief
  WWAN statistics are resetted in MobileAP.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int ResetWWANStats
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  int mask_flag=0;
  int result;
  char tmp[MAX_BUF_LEN] ={0};
  qcmap_msgr_wwan_status_enum_v01 v4status;
  qcmap_msgr_wwan_status_enum_v01 v6status;
  qcmap_nw_params_t qcmap_nw_params;
  char str[INET6_ADDRSTRLEN];
  qcmap_msgr_ip_family_enum_v01 ip_family;
  qcmap_msgr_station_mode_status_enum_v01 sta_status;

  ip_family =
  (qcmap_msgr_ip_family_enum_v01)atoi(values[WWAN_STATS_PAGE_IP_FAMILY]);
  memset(&qcmap_nw_params,0,sizeof(qcmap_nw_params_t));
  qmi_error=QMI_ERR_NONE_V01;
  /*Reset wwan statistics */
  if (!QcMapClient->ResetWWANStatistics(ip_family, &qmi_error))
    LOG_MSG_ERROR(" ResetWWANStats Fail: ip family %d \n",ip_family,0,0);

  snprintf(values[WWAN_STATS_PAGE_RESULT], strlen(QMI_Error_String[qmi_error])
                                       +1,"%s",QMI_Error_String[qmi_error]) ;
  snprintf(values[WWAN_STATS_PAGE_BYTES_RX],
                                   ZEROS_BUFF_SIZE,"%s","00") ;
  snprintf(values[WWAN_STATS_PAGE_BYTES_TX],
                                   ZEROS_BUFF_SIZE,"%s","00") ;
  snprintf(values[WWAN_STATS_PAGE_PACKETS_RX],
                                     ZEROS_BUFF_SIZE,"%s","00") ;
  snprintf(values[WWAN_STATS_PAGE_PACKETS_TX],
                                     ZEROS_BUFF_SIZE,"%s","00") ;
  snprintf(values[WWAN_STATS_PAGE_PACKET_DROPPED_RX],
                                     ZEROS_BUFF_SIZE,"%s","00") ;
  snprintf(values[WWAN_STATS_PAGE_PACKET_DROPPED_TX],
                                     ZEROS_BUFF_SIZE,"%s","00") ;

  qmi_error = QMI_ERR_NONE_V01;
  /* Retrieve WWAN Config*/
  if(!QcMapClient->GetNetworkConfiguration(
                    ip_family,
                    &qcmap_nw_params,
                    &qmi_error))
    LOG_MSG_ERROR(" get network configuration Fail:family%d \n",ip_family,0,0);

  snprintf(values[WWAN_STATS_PAGE_CONFIG_ERROR],
       strlen(QMI_Error_String[qmi_error])+1,"%s",QMI_Error_String[qmi_error]);
  if (qmi_error != QMI_ERR_NONE_V01 )
  {
    snprintf(values[WWAN_STATS_PAGE_PUBLIC_IP],sizeof("0.0.0.0"),"%s","0.0.0.0");
    snprintf(values[WWAN_STATS_PAGE_DNS_PRI],sizeof("0.0.0.0"),"%s","0.0.0.0");
    snprintf(values[WWAN_STATS_PAGE_DNS_SEC],sizeof("0.0.0.0"),"%s","0.0.0.0");
  }
  else if(ip_family == IPV6)
  {
    system_ip_to_readable_ip(AF_INET6,(uint32 *)&qcmap_nw_params.v6_conf.\
                                                   public_ip_v6,(char *)&str);
    snprintf(values[WWAN_STATS_PAGE_PUBLIC_IP],sizeof(str),"%s",str) ;
    system_ip_to_readable_ip(AF_INET6,(uint32 *)&qcmap_nw_params.v6_conf.\
                                                 primary_dns_v6,(char *)&str);
    snprintf(values[WWAN_STATS_PAGE_DNS_PRI],sizeof(str),"%s",str) ;
    system_ip_to_readable_ip(AF_INET6,(uint32*)&qcmap_nw_params.v6_conf.\
                                               secondary_dns_v6,(char *)&str);
    snprintf(values[WWAN_STATS_PAGE_DNS_SEC],sizeof(str),"%s",str) ;
  }
  else
  {
    system_ip_to_readable_ip(AF_INET,(uint32 *)&qcmap_nw_params.v4_conf.\
                                               public_ip.s_addr,(char *)&str);
    snprintf(values[WWAN_STATS_PAGE_PUBLIC_IP],sizeof(str),"%s",str) ;
    system_ip_to_readable_ip(AF_INET,(uint32 *)&qcmap_nw_params.v4_conf.\
                                             primary_dns.s_addr,(char *)&str);
    snprintf(values[WWAN_STATS_PAGE_DNS_PRI],sizeof(str),"%s",str) ;
    system_ip_to_readable_ip(AF_INET,(uint32 *)&qcmap_nw_params.v4_conf.\
                                           secondary_dns.s_addr,(char *)&str);
    snprintf(values[WWAN_STATS_PAGE_DNS_SEC],sizeof(str),"%s",str) ;
  }

  qmi_error = QMI_ERR_NONE_V01;
  if(!QcMapClient->GetWWANStatus(&v4status,&v6status,&qmi_error))
    LOG_MSG_ERROR(" get WWAN status Fail:  \n", 0, 0, 0);

  snprintf(values[WWAN_STATS_PAGE_STATUS_ERROR],
           strlen(QMI_Error_String[qmi_error])+1,"%s",
           QMI_Error_String[qmi_error]) ;

  if(ip_family == IPV6)
  {

  snprintf(values[WWAN_STATS_PAGE_STATUS],
           strlen(WWAN_Status_String[v6status])+1,"%s",
           WWAN_Status_String[v6status]);
  }
  else
  {

  snprintf(values[WWAN_STATS_PAGE_STATUS],
  strlen(WWAN_Status_String[v4status])+1,"%s",WWAN_Status_String[v4status]) ;

  }
  qmi_error = QMI_ERR_NONE_V01;
  if (QcMapClient->GetStationModeStatus(&sta_status,&qmi_error))
  {
    snprintf(values[WWAN_STATS_PAGE_STA_STATUS],
             strlen(STA_String[sta_status])+1,"%s",STA_String[sta_status]);
    snprintf(values[WWAN_STATS_PAGE_STA_RESULT],
             strlen(QMI_Error_String[qmi_error])+1,"%s",QMI_Error_String[qmi_error]);
  }
  else
  {
    snprintf(values[WWAN_STATS_PAGE_STA_RESULT],
    strlen(QMI_Error_String[qmi_error])+1, "%s",QMI_Error_String[qmi_error]);
    LOG_MSG_ERROR("Get wwan STA status failed!! qmi error number:%d",
                   qmi_error, 0, 0);
  }
  create_response_buf(values,buf,WWAN_STATS_PAGE_MAX,get_wwan_stats_string,0,0);

  #ifdef DEBUG_PRINT
  printf("Response message: %s\n",buf);
  #endif
  /* send response back to CGI */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response  for Reset wwan stats result is failure : %d \n",
        result,0,0);
  }
}

/*===========================================================================
  FUNCTION GetDatarate
===========================================================================*/
/*!
@brief
  Data rates are retrieved from MobileAP.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int GetDatarate
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  int result,temp_size=0;
  char tmp[MAX_BUF_LEN] ={0};
  qcmap_msgr_data_bitrate_v01 data_rate;
  int stats=0;
  memset(&data_rate, 0, sizeof(qcmap_msgr_data_bitrate_v01));
  qmi_error=QMI_ERR_NONE_V01;
  if(!QcMapClient->GetDataRate(&data_rate, &qmi_error))
    LOG_MSG_ERROR(" get Data rate Fail:  \n", 0, 0, 0);
  snprintf(values[DATA_RATE_PAGE_RESULT],
            strlen(QMI_Error_String[qmi_error])+1,"%s",
            QMI_Error_String[qmi_error]) ;
  if ( qmi_error == QMI_ERR_NONE_V01)
  {
    stats=data_rate.tx_rate;
    temp_size = log10(stats)+ PADDING;
    snprintf(values[DATA_RATE_PAGE_BYTES_TX],temp_size,"%d",stats) ;
    stats=data_rate.rx_rate;
    temp_size = log10(stats)+ PADDING;
    snprintf(values[DATA_RATE_PAGE_BYTES_RX],temp_size,"%d",stats) ;
    stats=data_rate.max_tx_rate;
    temp_size = log10(stats)+ PADDING;
    snprintf(values[DATA_RATE_PAGE_BYTES_TX_MAX],temp_size,"%d",stats) ;
    stats=data_rate.max_rx_rate;
    temp_size = log10(stats)+ PADDING;
    snprintf(values[DATA_RATE_PAGE_BYTES_RX_MAX],temp_size,"%d",stats) ;
  }
  else
  {
    snprintf(values[DATA_RATE_PAGE_BYTES_TX],ZEROS_BUFF_SIZE,"%s","00") ;
    snprintf(values[DATA_RATE_PAGE_BYTES_RX],ZEROS_BUFF_SIZE,"%s","00") ;
    snprintf(values[DATA_RATE_PAGE_BYTES_TX_MAX],ZEROS_BUFF_SIZE,"%s","00") ;
    snprintf(values[DATA_RATE_PAGE_BYTES_RX_MAX],ZEROS_BUFF_SIZE,"%s","00") ;
  }
  create_response_buf(values,buf,DATA_RATE_PAGE_MAX,get_data_rate_string,0,0);

  #ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
  #endif
  /* send response to CGI */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response Get Data rate result is failure: %d \n",
        result, 0, 0);
  }
}

/*===========================================================================
  FUNCTION GetWWANStats
===========================================================================*/
/*!
@brief
  WWAN statics are retrieved from MobileAP.

@input
  values - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int GetWWANStats
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  int result;
  char tmp[MAX_BUF_LEN] ={0};
  qcmap_msgr_wwan_status_enum_v01 v4status;
  qcmap_msgr_wwan_status_enum_v01 v6status;
  qcmap_nw_params_t qcmap_nw_params;
  char str[INET6_ADDRSTRLEN];
  qcmap_msgr_ip_family_enum_v01 ip_family;
  int stats=0, tmp_size=0;
  qcmap_msgr_station_mode_status_enum_v01 sta_status=0x02;
  qcmap_msgr_wwan_statistics_type_v01 wwan_stats;
  ip_family =
  (qcmap_msgr_ip_family_enum_v01)atoi(values[WWAN_STATS_PAGE_IP_FAMILY]);

  memset((void *)&wwan_stats, 0, sizeof(qcmap_msgr_wwan_statistics_type_v01));
  memset(&qcmap_nw_params,0,sizeof(qcmap_nw_params_t));

  qmi_error=QMI_ERR_NONE_V01;

  if(!QcMapClient->GetWWANStatistics(ip_family, &wwan_stats, &qmi_error))
    LOG_MSG_ERROR(" get WWAN status Fail:  \n", 0, 0, 0);

  snprintf(values[WWAN_STATS_PAGE_RESULT],
           strlen(QMI_Error_String[qmi_error])+1,"%s",
           QMI_Error_String[qmi_error]) ;
  if ( qmi_error == QMI_ERR_NONE_V01)
  {
    stats=wwan_stats.bytes_rx;
    tmp_size = log10(stats) + PADDING;
    snprintf(values[WWAN_STATS_PAGE_BYTES_RX], tmp_size,"%d",stats) ;
    stats=wwan_stats.bytes_tx;
    tmp_size = log10(stats) + PADDING;
    snprintf(values[WWAN_STATS_PAGE_BYTES_TX], tmp_size,"%d",stats) ;
    tmp_size = log10(wwan_stats.pkts_rx) + PADDING;
    snprintf(values[WWAN_STATS_PAGE_PACKETS_RX], tmp_size,
             "%d",wwan_stats.pkts_rx);
    tmp_size = log10(wwan_stats.pkts_tx) + PADDING;
    snprintf(values[WWAN_STATS_PAGE_PACKETS_TX], tmp_size,
             "%d",wwan_stats.pkts_tx);
    tmp_size = log10(wwan_stats.pkts_dropped_rx) + PADDING;
    snprintf(values[WWAN_STATS_PAGE_PACKET_DROPPED_RX],
             tmp_size,"%d",wwan_stats.pkts_dropped_rx);
    tmp_size = log10(wwan_stats.pkts_dropped_tx) + PADDING;
    snprintf(values[WWAN_STATS_PAGE_PACKET_DROPPED_TX],
             tmp_size,"%d",wwan_stats.pkts_dropped_tx);
  }
  else
  {
    snprintf(values[WWAN_STATS_PAGE_BYTES_RX],
             ZEROS_BUFF_SIZE,"%s","00") ;
    snprintf(values[WWAN_STATS_PAGE_BYTES_TX],
             ZEROS_BUFF_SIZE,"%s","00") ;
    snprintf(values[WWAN_STATS_PAGE_PACKETS_RX],
             ZEROS_BUFF_SIZE,"%s","00") ;
    snprintf(values[WWAN_STATS_PAGE_PACKETS_TX],
             ZEROS_BUFF_SIZE,"%s","00") ;
    snprintf(values[WWAN_STATS_PAGE_PACKET_DROPPED_RX],
             ZEROS_BUFF_SIZE,"%s","00") ;
    snprintf(values[WWAN_STATS_PAGE_PACKET_DROPPED_TX],
             ZEROS_BUFF_SIZE,"%s","00") ;

  }

  qmi_error = QMI_ERR_NONE_V01;
  if(!QcMapClient->GetNetworkConfiguration(
                    ip_family,
                    &qcmap_nw_params,
                    &qmi_error))
    LOG_MSG_ERROR(" get network configuration Fail:family%d \n",ip_family,0,0);

   /* Retrieve WWAN statistics */
  snprintf(values[WWAN_STATS_PAGE_CONFIG_ERROR],
           strlen(QMI_Error_String[qmi_error])+1,"%s",QMI_Error_String[qmi_error]);
  if (qmi_error != QMI_ERR_NONE_V01 )
  {
    snprintf(values[WWAN_STATS_PAGE_PUBLIC_IP],sizeof("0.0.0.0"),"%s","0.0.0.0");
    snprintf(values[WWAN_STATS_PAGE_DNS_PRI],sizeof("0.0.0.0"),"%s","0.0.0.0");
    snprintf(values[WWAN_STATS_PAGE_DNS_SEC],sizeof("0.0.0.0"),"%s","0.0.0.0");
  }
  else if(ip_family == IPV6)
  {
    system_ip_to_readable_ip(AF_INET6,(uint32 *)&qcmap_nw_params.v6_conf.\
                                                   public_ip_v6,(char *)&str);
    snprintf(values[WWAN_STATS_PAGE_PUBLIC_IP],sizeof(str),"%s",str) ;
             system_ip_to_readable_ip(AF_INET6,(uint32 *)&qcmap_nw_params.v6_conf.\
                                                 primary_dns_v6,(char *)&str);
    snprintf(values[WWAN_STATS_PAGE_DNS_PRI],sizeof(str),"%s",str) ;
             system_ip_to_readable_ip(AF_INET6,(uint32*)&qcmap_nw_params.v6_conf.\
                                               secondary_dns_v6,(char *)&str);
    snprintf(values[WWAN_STATS_PAGE_DNS_SEC],sizeof(str),"%s",str) ;
  }
  else
  {
    system_ip_to_readable_ip(AF_INET,(uint32 *)&qcmap_nw_params.v4_conf.\
                                               public_ip.s_addr,(char *)&str);
    snprintf(values[WWAN_STATS_PAGE_PUBLIC_IP],sizeof(str),"%s",str) ;
             system_ip_to_readable_ip(AF_INET,(uint32 *)&qcmap_nw_params.v4_conf.\
                                             primary_dns.s_addr,(char *)&str);
    snprintf(values[WWAN_STATS_PAGE_DNS_PRI],sizeof(str),"%s",str) ;
    system_ip_to_readable_ip(AF_INET,(uint32 *)&qcmap_nw_params.v4_conf.\
                                           secondary_dns.s_addr,(char *)&str);
    snprintf(values[WWAN_STATS_PAGE_DNS_SEC],sizeof(str),"%s",str) ;
  }

  qmi_error = QMI_ERR_NONE_V01;
  if(!QcMapClient->GetWWANStatus(&v4status,&v6status,&qmi_error))
    LOG_MSG_ERROR(" get WWAN status Fails:  \n", 0, 0, 0);

  snprintf(values[WWAN_STATS_PAGE_STATUS_ERROR],
           strlen(QMI_Error_String[qmi_error])+1,"%s",QMI_Error_String[qmi_error]);

  if(ip_family == IPV6)
  {

    snprintf(values[WWAN_STATS_PAGE_STATUS],
             strlen(WWAN_Status_String[v6status])+1,"%s",
             WWAN_Status_String[v6status]);

  }
  else
  {
    snprintf(values[WWAN_STATS_PAGE_STATUS],
     strlen(WWAN_Status_String[v4status])+1,"%s",WWAN_Status_String[v4status]);
  }
  qmi_error = QMI_ERR_NONE_V01;
  if (QcMapClient->GetStationModeStatus(&sta_status,&qmi_error))
  {
    snprintf(values[WWAN_STATS_PAGE_STA_STATUS],
             strlen(STA_String[sta_status])+1,"%s",STA_String[sta_status]);
    snprintf(values[WWAN_STATS_PAGE_STA_RESULT],
             strlen(QMI_Error_String[qmi_error])+1,"%s",QMI_Error_String[qmi_error]);
  }
  else
  {
    snprintf(values[WWAN_STATS_PAGE_STA_RESULT],
    strlen(QMI_Error_String[qmi_error])+1, "%s",QMI_Error_String[qmi_error]);
    LOG_MSG_ERROR("Get wwan STA status failed!! qmi error number:%d",
                   qmi_error, 0, 0);
  }

  create_response_buf(values,buf,WWAN_STATS_PAGE_MAX,get_wwan_stats_string,0,0);

  #ifdef DEBUG_PRINT
  printf("Response message: %s\n",buf);
  #endif
  /* send response to CGI */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response  for Get wwan stats result is failure: %d \n",
    result, 0, 0);
  }
}
/*===========================================================================
  FUNCTION AddDHCPReservationRecord
===========================================================================*/
/*!
@brief
  Add/Delete/Edit DHCP Resrvation Record in MobileAP configuration .

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int AddDHCPReservationRecord
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  qmi_error=QMI_ERR_NONE_V01;
  int result;
  char tmp[MAX_BUF_LEN] ={0};
  qcmap_msgr_dhcp_reservation_v01 dhcp_reserv_record;
  uint32 client_reserved_ip = 0;
  memset(&dhcp_reserv_record, 0, sizeof(dhcp_reserv_record));

  /* check snat add/delete/modify */
  if(atoi(values[DHCP_RESERVATION_SETTINGS_PAGE_ADD_DELETE_MODIFY_SNAT]) == ADD)
  {
    dhcp_reserv_record.client_reserved_ip=readable_ip_to_system_ip(values[
                               DHCP_RESERVATION_SETTINGS_PAGE_RESERVED_IP]);
    //transalate special characters url encoded if any in the MAC
    unencode_string(values[DHCP_RESERVATION_SETTINGS_PAGE_MAC_ADDR],
                    strlen(values[DHCP_RESERVATION_SETTINGS_PAGE_MAC_ADDR]));
    ds_mac_addr_pton(values[DHCP_RESERVATION_SETTINGS_PAGE_MAC_ADDR],
                  dhcp_reserv_record.client_mac_addr);
    unencode_string(values[DHCP_RESERVATION_SETTINGS_PAGE_DEVICE_NAME],
                    strlen(values[DHCP_RESERVATION_SETTINGS_PAGE_DEVICE_NAME]));
    memcpy(dhcp_reserv_record.client_device_name,
           values[DHCP_RESERVATION_SETTINGS_PAGE_DEVICE_NAME],
           strlen(values[DHCP_RESERVATION_SETTINGS_PAGE_DEVICE_NAME]));
    dhcp_reserv_record.enable_reservation=
          atoi(values[DHCP_RESERVATION_SETTINGS_PAGE_ENABLE_RESERVATION]);
    /* Add static nat entry config into AP */
    if (!QcMapClient->AddDHCPReservRecord(&dhcp_reserv_record,&qmi_error))
      LOG_MSG_ERROR("DHCPReservRecord failed!!qmi error number : %d ",
                      qmi_error, 0, 0);
    }
  else if(atoi(values[DHCP_RESERVATION_SETTINGS_PAGE_ADD_DELETE_MODIFY_SNAT]) ==
          DELETE)
  {
    qmi_error=QMI_ERR_NONE_V01;
    client_reserved_ip=readable_ip_to_system_ip(values[
                               DHCP_RESERVATION_SETTINGS_PAGE_IP_ADDR]);
    /* Delete static nat entry from AP config */
    if (!QcMapClient->DeleteDHCPReservRecord(&client_reserved_ip,&qmi_error))
      LOG_MSG_ERROR("DHCPReservRecord deletion failed!!qmi error number : \
                     %d ", qmi_error, 0, 0);
  }
  else if(atoi(values[DHCP_RESERVATION_SETTINGS_PAGE_ADD_DELETE_MODIFY_SNAT]) ==
          MODIFY)
  {
    //first delete the snat entry
    qmi_error=QMI_ERR_NONE_V01;
    client_reserved_ip=readable_ip_to_system_ip(values[
                               DHCP_RESERVATION_SETTINGS_PAGE_IP_ADDR]);
    dhcp_reserv_record.client_reserved_ip=readable_ip_to_system_ip(values[
                               DHCP_RESERVATION_SETTINGS_PAGE_RESERVED_IP]);
    //transalate special characters url encoded if any in the MAC
    unencode_string(values[DHCP_RESERVATION_SETTINGS_PAGE_MAC_ADDR],
                    strlen(values[DHCP_RESERVATION_SETTINGS_PAGE_MAC_ADDR]));
    ds_mac_addr_pton(values[DHCP_RESERVATION_SETTINGS_PAGE_MAC_ADDR],
                  dhcp_reserv_record.client_mac_addr);
    unencode_string(values[DHCP_RESERVATION_SETTINGS_PAGE_DEVICE_NAME],
                    strlen(values[DHCP_RESERVATION_SETTINGS_PAGE_DEVICE_NAME]));
    memcpy(dhcp_reserv_record.client_device_name,
           values[DHCP_RESERVATION_SETTINGS_PAGE_DEVICE_NAME],
           strlen(values[DHCP_RESERVATION_SETTINGS_PAGE_DEVICE_NAME]));
    dhcp_reserv_record.enable_reservation=
          atoi(values[DHCP_RESERVATION_SETTINGS_PAGE_ENABLE_RESERVATION]);
    if (!QcMapClient->EditDHCPReservRecord(&client_reserved_ip, &dhcp_reserv_record, &qmi_error))
      LOG_MSG_ERROR("DHCPReservRecord Edit failed!!qmi error number : \
                     %d ", qmi_error, 0, 0);
  }
  /* build response msg */
  strncat(buf,"{",1);
  memset(tmp,0,MAX_BUF_LEN);
  snprintf(tmp, strlen(ADDDHCPRESERVATIONRECORD) + strlen(PAGE)+1, PAGE,ADDDHCPRESERVATIONRECORD);
  strncat(buf,tmp,strlen(tmp));
  memset(tmp,0,MAX_BUF_LEN);
  snprintf(tmp, strlen(QMI_Error_String[qmi_error]) + strlen(DHCP_RESULT)+1, DHCP_RESULT,
           QMI_Error_String[qmi_error]);
  strncat(buf,tmp,strlen(tmp));
  strncat(buf,"}",1);
  #ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
  #endif
  /* send response msg to CGI  */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response  for AddDHCPReservation result is failure: %d \n",
        result, 0, 0);
  }
}
/*===========================================================================
  FUNCTION GetDHCPReservationRecords
===========================================================================*/
/*!
@brief
  Retrieves DHCP Reservation Records from MobileAP configuration.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int GetDHCPReservationRecords
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  in_addr tmpIP;
  int i=0, num_entries=0;
  int result;
  char tmp[MAX_BUF_LEN] ={0};
  qcmap_msgr_dhcp_reservation_v01 dhcp_reserv_records[QCMAP_MSGR_MAX_DHCP_RESERVATION_ENTRIES_V01];
  uint8 mac_addr_int[QCMAP_MSGR_MAC_ADDR_LEN_V01]; /*byte array of mac address*/
  char mac_addr_str[QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01]; /*char array of mac address*/

  memset(dhcp_reserv_records, 0, QCMAP_MSGR_MAX_DHCP_RESERVATION_ENTRIES_V01*sizeof
        (qcmap_msgr_dhcp_reservation_v01));
  /* Retrieve SNAT entries */
  if (QcMapClient->GetDHCPReservRecords(dhcp_reserv_records, &num_entries, &qmi_error))
  {
    /* build snat entries reply msg to CGI */
    memset(tmp,0,MAX_BUF_LEN);
    strncat(buf,"{",1);
    snprintf(tmp, strlen(GETDHCPRESERVATIONRECORDS) + strlen(PAGE)+1,
             PAGE,GETDHCPRESERVATIONRECORDS);
    strncat(buf,tmp,strlen(tmp));
    memset(tmp,0,MAX_BUF_LEN);
    snprintf(tmp, sizeof(num_entries) + strlen(DHCP_RESERVATION_COUNT)+1,
             DHCP_RESERVATION_COUNT,num_entries);
    strncat(buf,tmp,strlen(tmp));
    memset(tmp,0,MAX_BUF_LEN);
    snprintf(tmp, strlen(ENTRIES)+1,ENTRIES);
    strncat(buf,tmp,strlen(tmp));
    /* Add DHCP Reservation Records into reply msg */
    for (i=0; i<num_entries; i++)
    {
      #ifdef DEBUG_PRINT
        printf("\n\nEntry %d:",i);
      #endif
      ds_mac_addr_ntop(dhcp_reserv_records[i].client_mac_addr, mac_addr_str);
      tmpIP.s_addr = ntohl(dhcp_reserv_records[i].client_reserved_ip);
      strncat(buf,"{",1);
      memset(tmp,0,MAX_BUF_LEN);
      snprintf(tmp, strlen(mac_addr_str) + strlen(MAC_ADDR)+1,MAC_ADDR,
               mac_addr_str);
      strncat(buf,tmp,strlen(tmp));
      memset(tmp,0,MAX_BUF_LEN);
      snprintf(tmp, strlen(inet_ntoa(tmpIP)) + strlen(RESERVED_IP)+1,RESERVED_IP,
               inet_ntoa(tmpIP));
      strncat(buf,tmp,strlen(tmp));
      memset(tmp,0,MAX_BUF_LEN);
      snprintf(tmp, strlen(dhcp_reserv_records[i].client_device_name) + strlen(DEV_NAME)+1,
               DEV_NAME, dhcp_reserv_records[i].client_device_name);
      strncat(buf,tmp,strlen(tmp));
      memset(tmp,0,MAX_BUF_LEN);
      snprintf(tmp,
                sizeof(dhcp_reserv_records[i].enable_reservation) + strlen(RESERVATION_ENABLE)+1,
                RESERVATION_ENABLE, dhcp_reserv_records[i].enable_reservation);
      strncat(buf,tmp,strlen(tmp));
      if(i == (num_entries - 1))
      {
        strncat(buf,"}",1);
      }
      else
      {
        strncat(buf,"},",2);
      }
    }
    strncat(buf,"]",1);
    memset(tmp,0,MAX_BUF_LEN);
    snprintf(tmp, strlen(RES_SUCCESS), RES_SUCCESS);
    strncat(buf,tmp,strlen(tmp));
    strncat(buf,"}",1);
    #ifdef DEBUG_PRINT
      printf("Response message: %s\n",buf);
    #endif
    /*send response to CGI */
    result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
    if(result == QCMAP_CM_ERROR)
    {
      LOG_MSG_ERROR(" Sent the response  for GetDHCPReservationRecords result is failure: %d \n",
          result, 0, 0);
    }
  }
  /* build failure to retreive snat entries msg to CGI */
  else
  {
    LOG_MSG_ERROR("GetDHCPReservationRecords failed!!qmi error number : %d ",
                    qmi_error, 0, 0);
    memset(tmp,0,MAX_BUF_LEN);
    strncat(buf,"{",1);
    snprintf(tmp,strlen(GETDHCPRESERVATIONRECORDS) + strlen(PAGE)+1, PAGE,
             GETDHCPRESERVATIONRECORDS);
    strncat(buf,tmp,strlen(tmp));
    memset(tmp,0,MAX_BUF_LEN);
    snprintf(tmp, strlen(QMI_Error_String[qmi_error]) + strlen(DHCP_RESULT),
             DHCP_RESULT,QMI_Error_String[qmi_error]);
    strncat(buf,tmp,strlen(tmp));
    strncat(buf,"}",1);
    #ifdef DEBUG_PRINT
      printf("Response message: %s\n",buf);
    #endif
    /* send response msg to CGI */
    result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
    if(result == QCMAP_CM_ERROR)
    {
      LOG_MSG_ERROR(" Sent the response for GetDHCPReservationRecords is failure: %d \n",
          result, 0, 0);
    }
  }
}


/*===========================================================================
  FUNCTION GetLanConfig
===========================================================================*/
/*!
@brief
  Retrieves Lan parameters from MobileAP configuration.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int GetLanConfig
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  int mask_flag=0;
  int flag=-1;
  qcmap_msgr_wlan_mode_enum_v01 wlan_status=-1;
  int result;
  int lan_config_status=-1;
  char tmp[MAX_BUF_LEN] ={0};
  int i=0;
  mask_flag=atoi(values[LAN_PAGE_MASK]);
  qcmap_msgr_wlan_mode_enum_v01 wlan_mode=-1;
  qcmap_msgr_access_profile_v01 gap_profile=-1;
  qcmap_msgr_lan_config_v01 lan_config;
  qcmap_msgr_station_mode_config_v01 station_config;
  in_addr addr;
  qmi_error=QMI_ERR_NONE_V01;
  memset(&lan_config,0,sizeof(qcmap_msgr_lan_config_v01));
  //now get lan configuration
  if (!QcMapClient->GetLANConfig(&lan_config, &qmi_error))
  {
    LOG_MSG_ERROR("Get Lan Config failed!!  error :%d \n", qmi_error, 0, 0);
  }
  i = 2; //values buffer already has pagename,mask in index 0,1
  snprintf(values[(i)++],
   strlen(QMI_Error_String[qmi_error])+ 1, "%s", QMI_Error_String[qmi_error]) ;

  qmi_error=QMI_ERR_NONE_V01;
  /* get wlan status */
  if (!QcMapClient->GetWLANStatus(&wlan_status, &qmi_error))
  {
    LOG_MSG_ERROR("Get Wlan status failed!!  error :%d \n", qmi_error, 0, 0);
  }
  snprintf(values[(i)++],
     strlen(QMI_Error_String[qmi_error])+1, "%s", QMI_Error_String[qmi_error]);
  //now get wlan configuration
  memset(&station_config,0,sizeof(qcmap_msgr_station_mode_config_v01));
  memset(&addr, 0 ,sizeof(in_addr));
  #ifdef DEBUG_PRINT
    printf("get lanconfig called !!\n");
  #endif
  qmi_error=QMI_ERR_NONE_V01;
  if (!QcMapClient->GetWLANConfig(&wlan_mode, &gap_profile,&station_config,&qmi_error))
  {
    LOG_MSG_ERROR("Get Wlan configuration failed!!  error :%d \n", qmi_error,
                   0, 0);
  }
  snprintf(values[(i)++],
   strlen(QMI_Error_String[qmi_error])+ 1, "%s", QMI_Error_String[qmi_error]) ;
  #ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
  #endif
  /* build response message to CGI */
  store_lanconfig_res(values,wlan_status,wlan_mode,gap_profile,&i,&lan_config,
                      &station_config);
  /* build response msg */
  create_response_buf( values, buf, i, lan_config_str, 0,0);

  /* send response message to CGI */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response  for Get LanConfig result is failure: %d \n",
        result, 0, 0);
  }
}
/*===========================================================================
  FUNCTION SetLanConfig
===========================================================================*/
/*!
@brief
  Lan parameters are configured.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int SetLanConfig
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  int mask_flag=0;
  int flag=-1;
  int result;
  char tmp[MAX_BUF_LEN] ={0};
  qcmap_msgr_lan_config_v01 ap_config;
  mask_flag=atoi(values[LAN_PAGE_MASK]);
  flag=atoi(values[LAN_PAGE_WLAN_ENABLE_DISABLE]);
  qmi_error=QMI_ERR_NONE_V01;
  memset(tmp,0,MAX_BUF_LEN);
  memset(&ap_config,0,sizeof(qcmap_msgr_lan_config_v01));

  strncat(buf,"{",1);
  snprintf(tmp, strlen(SETLAN_CONFIG) + strlen(PAGE)+1, PAGE,SETLAN_CONFIG);
  strncat(buf,tmp,strlen(tmp));
  /* check field mask whether we really to need to enable wlan*/
  if(  ( (mask_flag & FIELD_MASK_1)  == FIELD_MASK_1) )
  {
    ap_config.gw_ip=readable_ip_to_system_ip(values[LAN_PAGE_LAN_AP_GW_IP]);
    ap_config.netmask=readable_ip_to_system_ip(
                                        values[LAN_PAGE_LAN_AP_SUBNET_MASK]);
    ap_config.enable_dhcp=atoi(values[LAN_PAGE_LAN_AP_DHCP_MODE]);
    ap_config.dhcp_config.dhcp_start_ip=readable_ip_to_system_ip(
                                      values[LAN_PAGE_LAN_AP_DHCP_START_IP]);
    ap_config.dhcp_config.dhcp_end_ip=readable_ip_to_system_ip(
                                        values[LAN_PAGE_LAN_AP_DHCP_END_IP]);
    ap_config.dhcp_config.lease_time=atoi(values[LAN_PAGE_LAN_AP_DHCP_LEASE_TIME]);
    if (QcMapClient->SetLANConfig(ap_config, &qmi_error))
    {
      snprintf(values[LAN_PAGE_LAN_AP_GW_IP],
               strlen(QMI_Error_String[qmi_error])+1,
               "%s", QMI_Error_String[qmi_error]);
      qmi_error=QMI_ERR_NONE_V01;
      //Activate the LAN so that configuration changed will be reflected in A5.
      if (QcMapClient->ActivateLAN(&qmi_error))
      {
        snprintf(values[LAN_PAGE_LAN_AP_SUBNET_MASK],
                 strlen(QMI_Error_String[qmi_error])+1,
                 "%s", QMI_Error_String[qmi_error]);
      }
      else
      {
        snprintf(values[LAN_PAGE_LAN_AP_SUBNET_MASK],
            strlen(QMI_Error_String[qmi_error])+1,
                   "%s", QMI_Error_String[qmi_error]) ;
        LOG_MSG_ERROR("Lan Activate failed!! qmi error number : %d",
                       qmi_error, 0, 0);
      }
    }
    else
    {
      snprintf(values[LAN_PAGE_LAN_AP_GW_IP],
          strlen(QMI_Error_String[qmi_error])+1,
                 "%s", QMI_Error_String[qmi_error]) ;
      LOG_MSG_ERROR("Lan Enable config failed!! qmi error number : %d",
                      qmi_error, 0, 0);
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" no change for LAN Config :  and mask flag : %d \n",
                  mask_flag ,0, 0);
    snprintf(values[LAN_PAGE_LAN_AP_GW_IP],
             strlen(QMI_Error_String[qmi_error])+1, "%s",
             QMI_Error_String[qmi_error]) ;
    snprintf(values[LAN_PAGE_LAN_AP_SUBNET_MASK],
             strlen(QMI_Error_String[qmi_error])+1,"%s",
             QMI_Error_String[qmi_error]) ;
  }
  qmi_error=QMI_ERR_NONE_V01;
  if(  ( (mask_flag & FIELD_MASK_2)  == FIELD_MASK_2) )
  {
    if(flag==ENABLED)
    {
      if (QcMapClient->EnableWLAN(&qmi_error))
      {
        snprintf(values[LAN_PAGE_WLAN_ENABLE_DISABLE],
                 strlen(QMI_Error_String[qmi_error])+1,
                 "%s", QMI_Error_String[qmi_error]);
      }
      else
      {
        snprintf(values[LAN_PAGE_WLAN_ENABLE_DISABLE],
            strlen(QMI_Error_String[qmi_error])+1,
                   "%s", QMI_Error_String[qmi_error]) ;
        LOG_MSG_ERROR("Lan Enable config failed!! qmi error number : %d",
                       qmi_error, 0, 0);
      }
    }
    /* check field mask whether we really to need to disable wlan*/
    else if(flag==DISABLED)
    {
      if (QcMapClient->DisableWLAN(&qmi_error))
      {
        snprintf(values[LAN_PAGE_WLAN_ENABLE_DISABLE],
                 strlen(QMI_Error_String[qmi_error])+1,
                 "%s",QMI_Error_String[qmi_error]);
      }
      else
      {
        snprintf(values[LAN_PAGE_WLAN_ENABLE_DISABLE],
                 strlen(QMI_Error_String[qmi_error])+1,"%s",
                 QMI_Error_String[qmi_error]);
        LOG_MSG_ERROR("Lan Disable config failed!! qmi error number : %d",
                      qmi_error, 0, 0);
      }
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" LAN Enable/Disable:  %d and mask flag : %d \n", flag,
                    mask_flag , 0);
    snprintf(values[LAN_PAGE_WLAN_ENABLE_DISABLE],
             strlen(QMI_Error_String[qmi_error])+1,
             "%s", QMI_Error_String[qmi_error]) ;
  }
  qmi_error=QMI_ERR_NONE_V01;
  if(   (mask_flag >=FIELD_MASK_3 ) )
  {
    qcmap_msgr_wlan_mode_enum_v01 wlan_mode;
    qcmap_msgr_access_profile_v01 gap_profile;
    qcmap_msgr_station_mode_config_v01 station_config;
    memset(&station_config,0,sizeof(qcmap_msgr_station_mode_config_v01));
    wlan_mode=atoi(values[LAN_PAGE_WLAN_MODE]);
    gap_profile=atoi(values[LAN_PAGE_WLAN_PROFILE]);
    //want to enter AP-STA mode config?
    if(  ( (mask_flag & FIELD_MASK_5)  == FIELD_MASK_5) )
    {
      station_config.conn_type=atoi(values[LAN_PAGE_WLAN_AP_STA_MODE]);
      if(station_config.conn_type == QCMAP_MSGR_STA_CONNECTION_STATIC_V01)
      {
        station_config.static_ip_config.ip_addr=readable_ip_to_system_ip(
                                      values[LAN_PAGE_WLAN_AP_STA_STATIC_IP]);
        station_config.static_ip_config.gw_ip=readable_ip_to_system_ip(
                                          values[LAN_PAGE_WLAN_AP_STA_GW_IP]);
        station_config.static_ip_config.dns_addr=readable_ip_to_system_ip(
                                  values[LAN_PAGE_WLAN_AP_STA_STATIC_DNS_IP]);
        station_config.static_ip_config.netmask=readable_ip_to_system_ip(
                             values[LAN_PAGE_WLAN_AP_STA_STATIC_SUBNET_MASK]);
      }//otherwise do nothing.
    }
    else
    {
      //no change required, so just print information
      LOG_MSG_INFO1("  AP-STA: config not required to change  %d and mask flag \
                    : %d \n", wlan_mode , mask_flag , 0);
    }

    if (QcMapClient->SetWLANConfig(wlan_mode, gap_profile, station_config ,
                                   &qmi_error))
    {
      snprintf(values[LAN_PAGE_WLAN_MODE],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]) ;
      qmi_error=QMI_ERR_NONE_V01;
      if(QcMapClient->ActivateWLAN(&qmi_error))
      {
        snprintf(values[LAN_PAGE_WLAN_PROFILE],
                 strlen(QMI_Error_String[qmi_error])+1, "%s",
                 QMI_Error_String[qmi_error]) ;
      }
      else
      {
        LOG_MSG_ERROR("\nWLAN Activation fails, Error: 0x%x", qmi_error, 0, 0);
        snprintf(values[LAN_PAGE_WLAN_PROFILE],
                 strlen(QMI_Error_String[qmi_error])+1, "%s",
                 QMI_Error_String[qmi_error]) ;
      }
    }
    else
    {
      LOG_MSG_ERROR("\nWLAN Config set fails, Error: 0x%x", qmi_error, 0, 0);
      snprintf(values[LAN_PAGE_WLAN_MODE],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]) ;
    }

  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" WLAN Mode: %d and mask flag : %d \n", flag , mask_flag , 0);
    snprintf(values[LAN_PAGE_WLAN_MODE],
             strlen(QMI_Error_String[qmi_error])+1, "%s",
             QMI_Error_String[qmi_error]) ;
    snprintf(values[LAN_PAGE_WLAN_PROFILE],
             strlen(QMI_Error_String[qmi_error])+1, "%s",
             QMI_Error_String[qmi_error]) ;
  }
  memset(tmp,0,MAX_BUF_LEN);
  snprintf(tmp,strlen(values[LAN_PAGE_LAN_AP_GW_IP]) + strlen(SET_LAN_CONFIG)+1,
           SET_LAN_CONFIG,values[LAN_PAGE_LAN_AP_GW_IP]);
  strncat(buf,tmp,strlen(tmp));
  memset(tmp,0,MAX_BUF_LEN);
  snprintf(tmp,strlen(values[LAN_PAGE_WLAN_ENABLE_DISABLE]) + strlen(SET_WLAN_STATUS)+1,
           SET_WLAN_STATUS,values[LAN_PAGE_WLAN_ENABLE_DISABLE]);
  strncat(buf,tmp,strlen(tmp));
  memset(tmp,0,MAX_BUF_LEN);
  snprintf(tmp,strlen(values[LAN_PAGE_WLAN_MODE]) + strlen(SET_WLAN_CONFIG)+1,
           SET_WLAN_CONFIG,values[LAN_PAGE_WLAN_MODE]);
  strncat(buf,tmp,strlen(tmp));
  memset(tmp,0,MAX_BUF_LEN);
  snprintf(tmp,strlen(values[LAN_PAGE_LAN_AP_SUBNET_MASK]) + strlen(SET_LAN_ACTIVATE)+1,
           SET_LAN_ACTIVATE,values[LAN_PAGE_LAN_AP_SUBNET_MASK]);
  strncat(buf,tmp,strlen(tmp));
  memset(tmp,0,MAX_BUF_LEN);
  snprintf(tmp,strlen(values[LAN_PAGE_WLAN_PROFILE]) + strlen(SET_WLAN_ACTIVATE)+1,
           SET_WLAN_ACTIVATE,values[LAN_PAGE_WLAN_PROFILE]);
  strncat(buf,tmp,strlen(tmp));
  strncat(buf,"}",1);

  #ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
  #endif
  /* send response msg back to CGI */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response  for Set LanConfig stats result is failure: %d \n",
        result, 0, 0);
  }
}
/*===========================================================================
  FUNCTION GetNatSettings
===========================================================================*/
/*!
@brief
  NAT parameters are configured.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int GetNatSettings
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  boolean flag=0;
  uint32 dmz_ip=0;
  qcmap_msgr_nat_enum_v01 nat_type;
  qcmap_msgr_nat_timeout_enum_v01 timeout_type;
  uint32 timeout_value;
  char str[INET6_ADDRSTRLEN];
  int result, temp_size=0;
  int length=0;
  char tmp[MAX_BUF_LEN] ={0};
  in_addr tmpIP;

  qmi_error=QMI_ERR_NONE_V01;
  /* Get Nat type currently configured*/
  if (QcMapClient->GetNatType(&nat_type,&qmi_error))
  {
    snprintf(values[NAT_SETTINGS_PAGE_NAT_TYPE],sizeof(nat_type),"%d",nat_type);
  }
  else
  {
    LOG_MSG_ERROR("Get NatType  failed:  qmi error number : %d !!  \n",
                   qmi_error, 0, 0);
  }
  snprintf(values[NAT_SETTINGS_PAGE_NAT_TYPE_RESULT],
           strlen(QMI_Error_String[qmi_error])+1, "%s",
           QMI_Error_String[qmi_error]);
  qmi_error=QMI_ERR_NONE_V01;
  /* retrieve DMZ IP configured */
  if (QcMapClient->GetDMZ(&dmz_ip, &qmi_error))
  {
    if ( dmz_ip == 0 )
    {
      LOG_MSG_ERROR("Get DMZ IP  failed in translation!! : \n", 0, 0, 0);
      snprintf(values[NAT_SETTINGS_PAGE_DMZ_IP],sizeof("0.0.0.0"),
        "%s","0.0.0.0") ;
    }
    else
    {
      tmpIP.s_addr = ntohl(dmz_ip);
      length = strlen(inet_ntoa(tmpIP));
      #ifdef DEBUG_PRINT
        printf("Get DMZ IP %s \n",values[NAT_SETTINGS_PAGE_DMZ_IP]);
        printf("Get DMZ IP %s \n",inet_ntoa(tmpIP));
        printf("Get DMZ IP %d \n",dmz_ip);
      #endif
      snprintf(values[NAT_SETTINGS_PAGE_DMZ_IP],length+1,"%s",inet_ntoa(tmpIP));
    }
  }
  else
  {
    LOG_MSG_ERROR("Get DMZ IP  failed!!  error :%d \n", qmi_error, 0, 0);
  snprintf(values[NAT_SETTINGS_PAGE_DMZ_IP],sizeof("0.0.0.0"),"%s","0.0.0.0") ;
  }
  /* fill response with qmi error code as result*/
  snprintf(values[NAT_SETTINGS_PAGE_DMZ_IP_RESULT],
              strlen(QMI_Error_String[qmi_error])+1, "%s",
              QMI_Error_String[qmi_error]) ;
  qmi_error=QMI_ERR_NONE_V01;
  /*Get current config IPSECVpn passthrough */
  if (QcMapClient->GetIPSECVpnPassthrough(&flag, &qmi_error))
  {
    snprintf(values[NAT_SETTINGS_PAGE_IPSEC_VPN_PASSTHROUGH],sizeof(int),"%d",
             (int) flag) ;
  }
  else
  {
    LOG_MSG_ERROR("Get IPSEC vpn passthrough flag failed!!", 0, 0, 0);
  }
  snprintf(values[NAT_SETTINGS_PAGE_IPSEC_VPN_PASSTHROUGH_RESULT],
           strlen(QMI_Error_String[qmi_error])+1, "%s",
           QMI_Error_String[qmi_error]) ;
  qmi_error=QMI_ERR_NONE_V01;

  if (QcMapClient->GetPPTPVpnPassthrough((boolean*)&flag, &qmi_error))
  {
    snprintf(values[NAT_SETTINGS_PAGE_PPTP_VPN_PASSTHROUGH],sizeof(int),"%d",
             (int) flag) ;
  }
  else
  {
    LOG_MSG_ERROR("Get PPTP vpn passthrough flag failed!!", 0, 0, 0);
  }
  snprintf(values[NAT_SETTINGS_PAGE_PPTP_VPN_PASSTHROUGH_RESULT],
            strlen(QMI_Error_String[qmi_error])+1,
           "%s",QMI_Error_String[qmi_error]) ;
  qmi_error=QMI_ERR_NONE_V01;

  if (QcMapClient->GetL2TPVpnPassthrough(&flag, &qmi_error))
  {
    snprintf(values[NAT_SETTINGS_PAGE_L2TP_VPN_PASSTHROUGH],sizeof(int),"%d",
             (int) flag) ;
  }
  else
  {
    LOG_MSG_ERROR("Get L2TP vpn passthrough flag failed!!", 0, 0, 0);
  }
  snprintf(values[NAT_SETTINGS_PAGE_L2TP_VPN_PASSTHROUGH_RESULT],
            strlen(QMI_Error_String[qmi_error])+1, "%s",
           QMI_Error_String[qmi_error]) ;
  /* based on timeout type, retrieve timeout val */
  for(int i = 0, timeout_type = QCMAP_MSGR_NAT_TIMEOUT_GENERIC_V01;
         timeout_type <= QCMAP_MSGR_NAT_TIMEOUT_UDP_V01; timeout_type++)
  {
    qmi_error=QMI_ERR_NONE_V01;
    if (QcMapClient->GetNatTimeout(timeout_type,&timeout_value, &qmi_error))
    {
      temp_size = log10(timeout_value)+ PADDING;
      snprintf(values[NAT_SETTINGS_PAGE_NAT_GEN_TIMEOUT + i],temp_size,
               "%d",timeout_value) ;
    }
    else
    {
      LOG_MSG_ERROR("Get Nattimeout  failed for %d !!",
                      QCMAP_MSGR_NAT_TIMEOUT_GENERIC_V01, 0, 0);
    }
    /*Fill values buffer with qmi error code */
    snprintf(values[NAT_SETTINGS_PAGE_NAT_GEN_TIMEOUT_RESULT + i],
             strlen(QMI_Error_String[qmi_error])+1,"%s",
             QMI_Error_String[qmi_error]) ;
    i+=2;
    }

  /* Get Webserver WWAN Access Flag.
  */
  qmi_error=QMI_ERR_NONE_V01;
  if (QcMapClient->GetWebserverWWANAccess((boolean*)&flag, &qmi_error))
  {
    snprintf(values[NAT_SETTINGS_PAGE_WEBSERVER_WWAN_ACCESS],sizeof(int),"%d",
             (int) flag);
  }
  else
  {
    LOG_MSG_ERROR("Get Webserver WWAN Access failed!!", 0, 0, 0);
  }
  snprintf(values[NAT_SETTINGS_PAGE_WEBSERVER_WWAN_ACCESS_RESULT],
            strlen(QMI_Error_String[qmi_error])+1,
           "%s",QMI_Error_String[qmi_error]);

  /* build response msg */
  create_response_buf( values, buf, NAT_SETTINGS_PAGE_MAX, nat_settings_str,
                                          NAT_SETTINGS_PAGE_PAGE,0);
  #ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
  #endif
  /* send response msg back to CGI */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response  for Get NatSettings result is failure: %d \n",
        result, 0, 0);
  }
}
/*===========================================================================
  FUNCTION SetNatSettings
===========================================================================*/
/*!
@brief
  NAT parameters are configured.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int SetNatSettings
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  int mask_flag=0;
  /* store field mask */
  mask_flag=atoi(values[NAT_SETTINGS_PAGE_MASK]);
  int result;
  char tmp[MAX_BUF_LEN] ={0};
  uint32 dmz_ip=0;
  in_addr addr;
  qmi_error=QMI_ERR_NONE_V01;
  /*check field mask whether to call respective QcMapClient methods */
  if( (  (mask_flag & FIELD_MASK_1)  == FIELD_MASK_1) )
  {
    if (QcMapClient->SetNatType(atoi(values[NAT_SETTINGS_PAGE_NAT_TYPE]),
        &qmi_error))
    {
      snprintf(values[NAT_SETTINGS_PAGE_NAT_TYPE_RESULT],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]) ;
    }
    else
    {
      snprintf(values[NAT_SETTINGS_PAGE_NAT_TYPE_RESULT],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]);
      LOG_MSG_ERROR("Set Nat type failed!qmi error number:%d",qmi_error, 0, 0);
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" Nat type setting: %d and mask flag : %d \n", atoi(
                  values[NAT_SETTINGS_PAGE_NAT_TYPE]) , mask_flag , 0);
    snprintf(values[NAT_SETTINGS_PAGE_NAT_TYPE_RESULT],
             strlen(QMI_Error_String[qmi_error])+1, "%s",
             QMI_Error_String[qmi_error]) ;
  }
  qmi_error=QMI_ERR_NONE_V01;
  if( ( (mask_flag & FIELD_MASK_2)  == FIELD_MASK_2) )
  {
    #ifdef DEBUG_PRINT
      printf("Add DMZ IP %s \n",values[NAT_SETTINGS_PAGE_DMZ_IP]);
      printf("Add DMZ IP %d \n",readable_ip_to_system_ip(
              values[NAT_SETTINGS_PAGE_DMZ_IP]));
    #endif
    //first delete existing DMZ value
     if (QcMapClient->DeleteDMZ(&qmi_error))
     {
       snprintf(values[NAT_SETTINGS_PAGE_DMZ_IP_RESULT],
                strlen(QMI_Error_String[qmi_error])+1, "%s",
                QMI_Error_String[qmi_error]) ;
     }
     else
     {
       //due to backhaul down, xml entry might have been done!!
       if(qmi_error != QMI_ERR_INTERFACE_NOT_FOUND_V01)
       {
         LOG_MSG_ERROR("Delete old DMZ IP failed qmi error:%d", qmi_error, 0, 0);
       }
       else
       {
         qmi_error = QMI_ERR_NONE_V01;
       }
       snprintf(values[NAT_SETTINGS_PAGE_DMZ_IP_RESULT],
                strlen(QMI_Error_String[qmi_error])+1, "%s",
                QMI_Error_String[qmi_error]) ;
     }
     //now try add the new value,irrespective delete status
     qmi_error = QMI_ERR_NONE_V01;
     if ( (inet_aton(values[NAT_SETTINGS_PAGE_DMZ_IP], &addr))  <= 0 )
     {
       LOG_MSG_ERROR("inet_aton failed!!. err code %d \n",errno , 0, 0);
       qmi_error = QMI_ERR_INVALID_DATA_FORMAT_V01;
       snprintf(values[NAT_SETTINGS_PAGE_DMZ_IP_RESULT],
                strlen(QMI_Error_String[qmi_error])+1, "%s",
                QMI_Error_String[qmi_error]);
     }
     else
     {
         qmi_error=QMI_ERR_NONE_V01;
       dmz_ip = ntohl(addr.s_addr);
       if (QcMapClient->AddDMZ(dmz_ip,&qmi_error))
       {
         snprintf(values[NAT_SETTINGS_PAGE_DMZ_IP_RESULT],
                  strlen(QMI_Error_String[qmi_error])+1, "%s",
                  QMI_Error_String[qmi_error]) ;
       }
       else
       {
         //due to backhaul down, xml entry might have been done!!.
         if(qmi_error != QMI_ERR_INTERFACE_NOT_FOUND_V01)
         {
           LOG_MSG_ERROR("Add new DMZ IP failed!!qmi error:%d", qmi_error, 0, 0);
         }
         else
         {
            qmi_error=QMI_ERR_NONE_V01;
         }
         snprintf(values[NAT_SETTINGS_PAGE_DMZ_IP_RESULT],
                  strlen(QMI_Error_String[qmi_error])+1, "%s",
                  QMI_Error_String[qmi_error]) ;
       }
     }

  }
  else
  {
    //no change required, so just print information
    snprintf(values[NAT_SETTINGS_PAGE_DMZ_IP_RESULT],
    strlen(QMI_Error_String[qmi_error])+1, "%s", QMI_Error_String[qmi_error]) ;
  }
  boolean enable;
  enable=atoi(values[NAT_SETTINGS_PAGE_IPSEC_VPN_PASSTHROUGH]);
  qmi_error=QMI_ERR_NONE_V01;
  if( ( (mask_flag & FIELD_MASK_3)  == FIELD_MASK_3) )
  {
    if (QcMapClient->SetIPSECVpnPassthrough(enable,&qmi_error))
    {
      /*store qmi error code to send back response to CGI */
      snprintf(values[NAT_SETTINGS_PAGE_IPSEC_VPN_PASSTHROUGH_RESULT],
      strlen(QMI_Error_String[qmi_error])+1, "%s",QMI_Error_String[qmi_error]);
    }
    else
    {
      //due to backhaul down, xml entry might have been done!!.
      if(qmi_error != QMI_ERR_INTERFACE_NOT_FOUND_V01)
      {
        LOG_MSG_ERROR("IPSec pass through %d  failed!!qmi error number : %d ",
                       enable, qmi_error, 0);
      }
      else
      {
        qmi_error=QMI_ERR_NONE_V01;
      }

      /*store qmi error code to send back response to CGI */
      snprintf(values[NAT_SETTINGS_PAGE_IPSEC_VPN_PASSTHROUGH_RESULT],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]) ;
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" IPSec pass through( Enable 1/Disable 0): %d and mask flag \
                   : %d \n", enable , mask_flag , 0);
    snprintf(values[NAT_SETTINGS_PAGE_IPSEC_VPN_PASSTHROUGH_RESULT],
             strlen(QMI_Error_String[qmi_error])+1, "%s",
             QMI_Error_String[qmi_error]) ;
  }
  enable=atoi(values[NAT_SETTINGS_PAGE_PPTP_VPN_PASSTHROUGH]);
  qmi_error=QMI_ERR_NONE_V01;
  if( ( (mask_flag & FIELD_MASK_4)  == FIELD_MASK_4) )
  {
    if (QcMapClient->SetPPTPVpnPassthrough(enable,&qmi_error))
    {
      snprintf(values[NAT_SETTINGS_PAGE_PPTP_VPN_PASSTHROUGH_RESULT],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]) ;
    }
    else
    {
      //due to backhaul down, xml entry might have been done!!.
      if(qmi_error != QMI_ERR_INTERFACE_NOT_FOUND_V01)
      {
           LOG_MSG_ERROR("SetPPTPVpnPassthrough %d  failed!!qmi error number : %d ",\
                          enable, qmi_error, 0);
      }
      else
      {
         qmi_error=QMI_ERR_NONE_V01;
      }
       snprintf(values[NAT_SETTINGS_PAGE_PPTP_VPN_PASSTHROUGH_RESULT],
                strlen(QMI_Error_String[qmi_error])+1, "%s",
                QMI_Error_String[qmi_error]) ;
       LOG_MSG_ERROR("SetPPTPVpnPassthrough %d  failed!!qmi error number : %d ",\
                      enable, qmi_error, 0);
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" SetPPTPVpnPassthrough ( Enable 1/Disable 0 ): %d and mask \
                    flag : %d \n", enable , mask_flag , 0);
    snprintf(values[NAT_SETTINGS_PAGE_PPTP_VPN_PASSTHROUGH_RESULT],
             strlen(QMI_Error_String[qmi_error])+1, "%s",
             QMI_Error_String[qmi_error]) ;
  }
  enable=atoi(values[NAT_SETTINGS_PAGE_L2TP_VPN_PASSTHROUGH]);
  qmi_error=QMI_ERR_NONE_V01;
  if( ( (mask_flag & FIELD_MASK_5)  == FIELD_MASK_5) )
  {
    if (QcMapClient->SetL2TPVpnPassthrough(enable,&qmi_error))
    {
      snprintf(values[NAT_SETTINGS_PAGE_L2TP_VPN_PASSTHROUGH_RESULT],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]) ;
    }
    else
    {
      //due to backhaul down, xml entry might have been done!!.
      if(qmi_error != QMI_ERR_INTERFACE_NOT_FOUND_V01)
      {
        LOG_MSG_ERROR("SetL2TPVpnPassthrough %d  failed!!qmi error number : %d",
                       enable, qmi_error, 0);
      }
      else
      {
        qmi_error=QMI_ERR_NONE_V01;
      }
      snprintf(values[NAT_SETTINGS_PAGE_L2TP_VPN_PASSTHROUGH_RESULT],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]) ;
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" SetL2TPVpnPassthrough ( Enable 1/Disable 0 ): %d and mask \
                   flag : %d \n", enable , mask_flag , 0);
    snprintf(values[NAT_SETTINGS_PAGE_L2TP_VPN_PASSTHROUGH_RESULT],
             strlen(QMI_Error_String[qmi_error])+1, "%s",
             QMI_Error_String[qmi_error]) ;
  }
  int nat_timeout_value;
  int nat_timeout_type;
  qmi_error=QMI_ERR_NONE_V01;
  /*set nat type to Generic timeout */
  nat_timeout_type=QCMAP_MSGR_NAT_TIMEOUT_GENERIC_V01;
  nat_timeout_value=atoi(values[NAT_SETTINGS_PAGE_NAT_GEN_TIMEOUT]);
  if( ( (mask_flag & FIELD_MASK_6)  == FIELD_MASK_6) )
  {
    if (QcMapClient->SetNatTimeout(nat_timeout_type,nat_timeout_value,
                                   &qmi_error))
    {
      snprintf(values[NAT_SETTINGS_PAGE_NAT_GEN_TIMEOUT_RESULT],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]) ;
    }
    else
    {
      snprintf(values[NAT_SETTINGS_PAGE_NAT_GEN_TIMEOUT_RESULT],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]) ;
      LOG_MSG_ERROR("SetNatTimeout nat_timeout type %d value %d failed!!qmi \
                    error: %d",nat_timeout_type, nat_timeout_value, qmi_error);
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" SetNatTimeout : type %d  value %d and mask : %d \n",
                    nat_timeout_type , nat_timeout_value, mask_flag);
    snprintf(values[NAT_SETTINGS_PAGE_NAT_GEN_TIMEOUT_RESULT],
             strlen(QMI_Error_String[qmi_error])+1, "%s",
             QMI_Error_String[qmi_error]) ;
  }
  qmi_error=QMI_ERR_NONE_V01;
  /*set nat type to ICMP timeout */
  nat_timeout_type=QCMAP_MSGR_NAT_TIMEOUT_ICMP_V01;
  nat_timeout_value=atoi(values[NAT_SETTINGS_PAGE_NAT_ICMP_TIMEOUT]);
  if( ( (mask_flag & FIELD_MASK_7)  == FIELD_MASK_7) )
  {
    if (QcMapClient->SetNatTimeout(nat_timeout_type,nat_timeout_value,
                                   &qmi_error))
    {
      snprintf(values[NAT_SETTINGS_PAGE_NAT_ICMP_TIMEOUT_RESULT],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]) ;
    }
    else
    {
      snprintf(values[NAT_SETTINGS_PAGE_NAT_ICMP_TIMEOUT_RESULT],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]) ;
      LOG_MSG_ERROR("SetNatTimeout type %d value %d failed!!qmi error : %d ",
                     nat_timeout_type, nat_timeout_value, qmi_error);
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" SetNatTimeout : type %d value %d and mask : %d \n",
                    nat_timeout_type ,nat_timeout_value, mask_flag);
    snprintf(values[NAT_SETTINGS_PAGE_NAT_ICMP_TIMEOUT_RESULT],
             strlen(QMI_Error_String[qmi_error])+1, "%s",
             QMI_Error_String[qmi_error]) ;
  }
  qmi_error=QMI_ERR_NONE_V01;
  nat_timeout_value=atoi(values[NAT_SETTINGS_PAGE_NAT_TCP_TIMEOUT]);
  /*set nat type to TCP timeout */
  nat_timeout_type=QCMAP_MSGR_NAT_TIMEOUT_TCP_ESTABLISHED_V01;
  if( ( (mask_flag & FIELD_MASK_8)  == FIELD_MASK_8) )
  {
    if (QcMapClient->SetNatTimeout(nat_timeout_type,nat_timeout_value,
                                   &qmi_error))
    {
      snprintf(values[NAT_SETTINGS_PAGE_NAT_TCP_TIMEOUT_RESULT],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]) ;
    }
    else
    {
      snprintf(values[NAT_SETTINGS_PAGE_NAT_TCP_TIMEOUT_RESULT],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]) ;
      LOG_MSG_ERROR("SetNatTimeout type %d  value %d failed!!qmi error: %d ",
                      nat_timeout_type, nat_timeout_value, qmi_error);
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" SetNatTimeout : type %d  value %d and mask : %d \n",
                    nat_timeout_type , nat_timeout_value, mask_flag);
    snprintf(values[NAT_SETTINGS_PAGE_NAT_TCP_TIMEOUT_RESULT],
             strlen(QMI_Error_String[qmi_error])+1, "%s",
             QMI_Error_String[qmi_error]) ;
  }
  qmi_error=QMI_ERR_NONE_V01;
    /*set nat type to UDP timeout */
    nat_timeout_type=QCMAP_MSGR_NAT_TIMEOUT_UDP_V01;
    nat_timeout_value=atoi(values[NAT_SETTINGS_PAGE_NAT_UDP_TIMEOUT]);
  if( ( (mask_flag & FIELD_MASK_9)  == FIELD_MASK_9) )
  {
    if (QcMapClient->SetNatTimeout(nat_timeout_type,nat_timeout_value,
                                   &qmi_error))
    {
      snprintf(values[NAT_SETTINGS_PAGE_NAT_UDP_TIMEOUT_RESULT],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]) ;
    }
    else
    {
      snprintf(values[NAT_SETTINGS_PAGE_NAT_UDP_TIMEOUT_RESULT],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]) ;
      LOG_MSG_ERROR("SetNatTimeout type %d  value %d failed!!qmi error: %d ",
                      nat_timeout_type, nat_timeout_value, qmi_error);
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" SetNatTimeout : type %d  value %d and mask : %d \n",
                    nat_timeout_type , nat_timeout_value, mask_flag);
    snprintf(values[NAT_SETTINGS_PAGE_NAT_UDP_TIMEOUT_RESULT],
             strlen(QMI_Error_String[qmi_error])+1, "%s",
             QMI_Error_String[qmi_error]) ;
  }
  /* Set Webserver WWAN Access. */
  enable=atoi(values[NAT_SETTINGS_PAGE_WEBSERVER_WWAN_ACCESS]);
  qmi_error=QMI_ERR_NONE_V01;
  if( ( (mask_flag & FIELD_MASK_10)  == FIELD_MASK_10) )
  {
    if (QcMapClient->SetWebserverWWANAccess(enable,&qmi_error))
    {
      snprintf(values[NAT_SETTINGS_PAGE_WEBSERVER_WWAN_ACCESS_RESULT],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]) ;
    }
    else
    {
      //due to backhaul down, xml entry might have been done!!.
      if(qmi_error != QMI_ERR_INTERFACE_NOT_FOUND_V01)
      {
           LOG_MSG_ERROR("SetWebserverWWANAccess %d  failed!!qmi error number : %d ",\
                          enable, qmi_error, 0);
      }
      else
      {
         qmi_error=QMI_ERR_NONE_V01;
      }
       snprintf(values[NAT_SETTINGS_PAGE_WEBSERVER_WWAN_ACCESS_RESULT],
                strlen(QMI_Error_String[qmi_error])+1, "%s",
                QMI_Error_String[qmi_error]) ;
       LOG_MSG_ERROR("SetWebserverWWANAccess %d  failed!!qmi error number : %d ",\
                      enable, qmi_error, 0);
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" SetWebserverWWANAccess ( Enable 1/Disable 0 ): %d and mask \
                    flag : %d \n", enable , mask_flag , 0);
    snprintf(values[NAT_SETTINGS_PAGE_WEBSERVER_WWAN_ACCESS_RESULT],
             strlen(QMI_Error_String[qmi_error])+1, "%s",
             QMI_Error_String[qmi_error]) ;
  }
  /* build response msg */
  create_response_buf( values, buf, NAT_SETTINGS_PAGE_MAX, nat_settings_str,
                       NAT_SETTINGS_PAGE_PAGE,0);
  #ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
  #endif
  /* send response msg to CGI  */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response  for Set NatSettings result is failure: %d \n",
        result, 0, 0);
  }
}
/*===========================================================================
  FUNCTION SetMediaDir
===========================================================================*/
/*!
@brief
  Adds Media Dir list into MobileAP configuration .

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int SetMediaDir
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  qmi_error=QMI_ERR_NONE_V01;
  int result;
  char tmp[MAX_BUF_LEN] ={0};
  char media_dir_get[QCMAP_MSGR_MAX_DLNA_DIR_LEN_V01] = "";
  char media_dir_set[QCMAP_MSGR_MAX_DLNA_DIR_LEN_V01] = "";
  char *ptr=NULL;
  memset(media_dir_get,'\0',QCMAP_MSGR_MAX_DLNA_DIR_LEN_V01);
  memset(media_dir_set,'\0',QCMAP_MSGR_MAX_DLNA_DIR_LEN_V01);
  //transalate special characters url encoded
  unencode_string(values[SET_MEDIA_DIR_PAGE_DIR_LIST],
                  strlen(values[SET_MEDIA_DIR_PAGE_DIR_LIST]));
  #ifdef DEBUG_PRINT
    printf("unencode returned: %s \n",values[SET_MEDIA_DIR_PAGE_DIR_LIST]);
  #endif
  /* check media dir list add/modify */
  if(atoi(values[SET_MEDIA_DIR_PAGE_ADD_MODIFY]) == ADD)
  {
    if(QcMapClient->GetDLNAMediaDir( media_dir_get, &qmi_error))
    {
      #ifdef DEBUG_PRINT
        printf("Got following dir: Add operation %s \n",media_dir_get);
      #endif
      //need to check return value
      strlcpy(media_dir_set,values[SET_MEDIA_DIR_PAGE_DIR_LIST],
              sizeof(media_dir_set));
      strlcat(media_dir_set, ",", sizeof(media_dir_set));
      strlcat(media_dir_set, media_dir_get, sizeof(media_dir_set));
      //replace all newlines with ','
      ptr = strchr(media_dir_set, '\n');
      while (ptr)
      {
        media_dir_set[ptr-media_dir_set] = ',';
        ptr = strchr(ptr+1, '\n');
      }
    }
    //remvoe extra , at the end of string!!.
    result = strlen(media_dir_set);
    if(media_dir_set[result] == ',' )
      media_dir_set[result] = '\0';
    #ifdef DEBUG_PRINT
      printf("setting following dir add operation: %s \n",media_dir_set);
    #endif
    if(QcMapClient->SetDLNAMediaDir( media_dir_set, &qmi_error))
    {
      snprintf(values[SET_MEDIA_DIR_PAGE_DIR_LIST],strlen(media_dir_set )+1,
               "%s", media_dir_set) ;
    }
    else
    {
      if (qmi_error != QMI_ERR_NO_EFFECT_V01)
      {
        LOG_MSG_ERROR("SetDLNAMediaDir returns Error: 0x%x ",
                       qmi_error, 0, 0);
      }
      else
      {
        LOG_MSG_ERROR("SetDLNAMediaDir succeeds but restart failed " ,0, 0, 0);
      }
    }
    snprintf(values[SET_MEDIA_DIR_PAGE_RESULT],
    strlen(QMI_Error_String[qmi_error])+1, "%s", QMI_Error_String[qmi_error]) ;
  }
  else if(atoi(values[SET_MEDIA_DIR_PAGE_ADD_MODIFY])
               == MODIFY)
  {
    qmi_error=QMI_ERR_NONE_V01;
    strlcpy(media_dir_set,values[SET_MEDIA_DIR_PAGE_DIR_LIST],
            sizeof(media_dir_set));
    #ifdef DEBUG_PRINT
      printf("setting following dir: modify operation %s \n",media_dir_set);
    #endif
    if(!QcMapClient->SetDLNAMediaDir( media_dir_set, &qmi_error))
    {
      snprintf(values[SET_MEDIA_DIR_PAGE_DIR_LIST],
               strlen(media_dir_set )+1, "%s", media_dir_set) ;
      if (qmi_error != QMI_ERR_NO_EFFECT_V01)
      {
        LOG_MSG_ERROR("SetDLNAMediaDir returns Error: 0x%x " ,
                         qmi_error, 0, 0);
      }
      else
      {
        LOG_MSG_ERROR("SetDLNAMediaDir succeeds but restart failed " ,0, 0, 0);
      }
    }
    snprintf(values[SET_MEDIA_DIR_PAGE_RESULT],
    strlen(QMI_Error_String[qmi_error])+1, "%s", QMI_Error_String[qmi_error]) ;
  }

  /* build response msg */
  create_response_buf( values, buf, SET_MEDIA_DIR_PAGE_MAX, set_media_dir_str,
                      SET_MEDIA_DIR_PAGE_PAGE,0);
  #ifdef DEBUG_PRINT
    printf("Response message: %s \n",buf);
  #endif
  /* send response msg to CGI  */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response SetMediaDir result is failure: %d \n",
        result, 0, 0);
  }
}

/*===========================================================================
  FUNCTION GetMediaDir
===========================================================================*/
/*!
@brief
  Retrieves Media Directories list from MobileAP configuration.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int GetMediaDir
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  char media_dir_get[QCMAP_MSGR_MAX_DLNA_DIR_LEN_V01] = "";
  char media_dir_set[QCMAP_MSGR_MAX_DLNA_DIR_LEN_V01] = "";
  char *ptr;
  int result;
  int i=0;
  char tmp[MAX_BUF_LEN] ={0};
  qmi_error=QMI_ERR_NONE_V01;
  /* retrieve Media dir list*/
  memset(media_dir_get,0,QCMAP_MSGR_MAX_DLNA_DIR_LEN_V01);
  memset(media_dir_set,0,QCMAP_MSGR_MAX_DLNA_DIR_LEN_V01);

  if(QcMapClient->GetDLNAMediaDir( media_dir_get, &qmi_error))
  {
    #ifdef DEBUG_PRINT
      printf("Got following dir: %s \n",media_dir_get);
    #endif
    strlcpy(media_dir_set, media_dir_get, sizeof(media_dir_set));
    //replace all newlines with ','
    ptr = strchr(media_dir_set, '\n');
    while (ptr)
    {
      media_dir_set[ptr-media_dir_set] = ',';
      ptr = strchr(ptr+1, '\n');
    }
    snprintf(values[GET_MEDIA_DIR_PAGE_DIR_LIST],strlen(media_dir_set) ,
             "%s", media_dir_set) ;
    #ifdef DEBUG_PRINT
      printf("Got following dir after removing newline: %s \n",
              values[GET_MEDIA_DIR_PAGE_DIR_LIST]);
    #endif
  }
  else
  {
    LOG_MSG_ERROR("Get Media dir list failed!!  error :%d \n", qmi_error, 0,0);
  }
  snprintf(values[GET_MEDIA_DIR_PAGE_RESULT],
    strlen(QMI_Error_String[qmi_error])+1, "%s", QMI_Error_String[qmi_error]) ;
  qmi_error=QMI_ERR_NONE_V01;
  create_response_buf( values, buf, GET_MEDIA_DIR_PAGE_MAX, get_media_dir_str,
                      GET_MEDIA_DIR_PAGE_PAGE,0);
  #ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
  #endif
  /* Send response back to CGI */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response GeMediDir result is failure: %d \n",
        result, 0, 0);
  }
}
/*===========================================================================
  FUNCTION GetSnatEntries
===========================================================================*/
/*!
@brief
  Retrieves Static nat entries from MobileAP configuration.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int GetSnatEntries
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  in_addr tmpIP;
  int i=0, num_entries=0, tmp_size=0;
  int result;
  char tmp[MAX_BUF_LEN] ={0};
  qcmap_msgr_snat_entry_config_v01 snat_config[QCMAP_MSGR_MAX_SNAT_ENTRIES_V01];
  memset(snat_config, 0, QCMAP_MSGR_MAX_SNAT_ENTRIES_V01*sizeof
        (qcmap_msgr_snat_entry_config_v01));
  /* Retrieve SNAT entries */
  if (QcMapClient->GetStaticNatConfig(snat_config, &num_entries, &qmi_error))
  {
    /* build snat entries reply msg to CGI */
    memset(tmp,0,MAX_BUF_LEN);
    strncat(buf,"{",1);
    snprintf(tmp, strlen(GETSNATENTRIES) + strlen(PAGE)+1, PAGE,GETSNATENTRIES);
    strncat(buf,tmp,strlen(tmp));
    memset(tmp,0,MAX_BUF_LEN);
    snprintf(tmp, strlen(SUCCESS) + strlen(SNAT_ERROR)+1 , SNAT_ERROR,SUCCESS);
    strncat(buf,tmp,strlen(tmp));
    memset(tmp,0,MAX_BUF_LEN);
    snprintf(tmp, sizeof(num_entries) + strlen(SNAT_COUNT)+1,
             SNAT_COUNT,num_entries);
    strncat(buf,tmp,strlen(tmp));
    memset(tmp,0,MAX_BUF_LEN);
    snprintf(tmp, strlen(ENTRIES)+1,ENTRIES);
    strncat(buf,tmp,strlen(tmp));
    /* Add snat entries into reply msg */
    for (i=0; i<num_entries; i++)
    {
      #ifdef DEBUG_PRINT
        printf("\n\nEntry %d:",i);
      #endif
      tmpIP.s_addr = ntohl(snat_config[i].private_ip_addr);
      strncat(buf,"{",1);
      memset(tmp,0,MAX_BUF_LEN);
      snprintf(tmp, strlen(inet_ntoa(tmpIP)) + strlen(PRIVATE_IP)+1,PRIVATE_IP,
               inet_ntoa(tmpIP));
      strncat(buf,tmp,strlen(tmp));
      memset(tmp,0,MAX_BUF_LEN);
      tmp_size = log10(snat_config[i].private_port) + PADDING;
      snprintf(tmp,tmp_size +strlen(PRIVATE_PORT)+1,
               PRIVATE_PORT,snat_config[i].private_port);
      strncat(buf,tmp,strlen(tmp));
      memset(tmp,0,MAX_BUF_LEN);
      tmp_size = log10(snat_config[i].global_port) + PADDING;
      snprintf(tmp,tmp_size + strlen(GLOBAL_PORT)+1,
               GLOBAL_PORT,snat_config[i].global_port);
      strncat(buf,tmp,strlen(tmp));
      memset(tmp,0,MAX_BUF_LEN);
      tmp_size = log10(snat_config[i].protocol) + PADDING;
      snprintf(tmp, tmp_size + strlen(PROTO)+1, PROTO,
               snat_config[i].protocol);
      strncat(buf,tmp,strlen(tmp));
      if(i == (num_entries - 1))
      {
        strncat(buf,"}",1);
      }
      else
      {
        strncat(buf,"},",2);
      }
    }
    strncat(buf,"]",1);
    strncat(buf,"}",1);
    #ifdef DEBUG_PRINT
      printf("Response message: %s\n",buf);
    #endif
    /*send response to CGI */
    result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
    if(result == QCMAP_CM_ERROR)
    {
      LOG_MSG_ERROR(" Sent the response  for Get SnatEntries result is failure: %d \n",
          result, 0, 0);
    }
  }
  /* build failure to retreive snat entries msg to CGI */
  else
  {
    LOG_MSG_ERROR("Get Static nat entries failed!!qmi error number : %d ",
                    qmi_error, 0, 0);
    memset(tmp,0,MAX_BUF_LEN);
    strncat(buf,"{",1);
    snprintf(tmp,strlen(GETSNATENTRIES) + strlen(PAGE)+1, PAGE,GETSNATENTRIES);
    strncat(buf,tmp,strlen(tmp));
    memset(tmp,0,MAX_BUF_LEN);
    snprintf(tmp, strlen(QMI_Error_String[qmi_error]) + strlen(SNAT_ERROR),
             SNAT_ERROR,QMI_Error_String[qmi_error]);
    strncat(buf,tmp,strlen(tmp));
    strncat(buf,"}",1);
    #ifdef DEBUG_PRINT
      printf("Response message: %s\n",buf);
    #endif
    /* send response msg to CGI */
    result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
    if(result == QCMAP_CM_ERROR)
    {
      LOG_MSG_ERROR(" Sent the response  for  Get SnatEntries result is failure: %d \n",
          result, 0, 0);
    }
  }
}
/*===========================================================================
  FUNCTION AddSnatEntry
===========================================================================*/
/*!
@brief
  Adds static Nat entry in MobileAP configuration .

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int AddSnatEntry
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  qmi_error=QMI_ERR_NONE_V01;
  int result;
  char tmp[MAX_BUF_LEN] ={0};
  qcmap_msgr_snat_entry_config_v01 snat_entry;
  /* check snat add/delete/modify */
  if(atoi(values[STATIC_NAT_SETTINGS_PAGE_ADD_DELETE_MODIFY_SNAT]) == ADD)
  {
    qcmap_msgr_snat_entry_config_v01 snat_entry;
    snat_entry.private_ip_addr=readable_ip_to_system_ip(values[
                               STATIC_NAT_SETTINGS_PAGE_PRIVATE_IP_ADDR]);
    snat_entry.private_port=atoi(values[STATIC_NAT_SETTINGS_PAGE_PRIVATE_PORT]);
    snat_entry.global_port=atoi(values[STATIC_NAT_SETTINGS_PAGE_GLOBAL_PORT]);
    snat_entry.protocol=atoi(values[STATIC_NAT_SETTINGS_PAGE_PROTOCOL]);
    /* Add static nat entry config into AP */
    if (!QcMapClient->AddStaticNatEntry(&snat_entry,&qmi_error) &&
        (qmi_error != QMI_ERR_INTERFACE_NOT_FOUND_V01))
      LOG_MSG_ERROR("Static nat entry addition failed!!qmi error number : %d ",
                      qmi_error, 0, 0);
    }
  else if(atoi(values[STATIC_NAT_SETTINGS_PAGE_ADD_DELETE_MODIFY_SNAT]) ==
          DELETE)
  {
    qmi_error=QMI_ERR_NONE_V01;
    snat_entry.private_ip_addr=readable_ip_to_system_ip(
                             values[STATIC_NAT_SETTINGS_PAGE_PRIVATE_IP_ADDR]);
    snat_entry.private_port=atoi(values
                                      [STATIC_NAT_SETTINGS_PAGE_PRIVATE_PORT]);
    snat_entry.global_port=atoi(values
                                      [STATIC_NAT_SETTINGS_PAGE_GLOBAL_PORT]);
    snat_entry.protocol=atoi(values[STATIC_NAT_SETTINGS_PAGE_PROTOCOL]);
    /* Delete static nat entry from AP config */
    if (!QcMapClient->DeleteStaticNatEntry(&snat_entry,&qmi_error) &&
        (qmi_error != QMI_ERR_INTERFACE_NOT_FOUND_V01))
      LOG_MSG_ERROR("Static nat entry deletion failed!!qmi error number : \
                     %d ", qmi_error, 0, 0);
  }
  else if(atoi(values[STATIC_NAT_SETTINGS_PAGE_ADD_DELETE_MODIFY_SNAT]) ==
          MODIFY)
  {
    //first delete the snat entry
    qmi_error=QMI_ERR_NONE_V01;
    snat_entry.private_ip_addr=readable_ip_to_system_ip(values
                               [STATIC_NAT_SETTINGS_PAGE_OLD_PRIVATE_IP_ADDR]);
    snat_entry.private_port=atoi(values
                                  [STATIC_NAT_SETTINGS_PAGE_OLD_PRIVATE_PORT]);
    snat_entry.global_port=atoi(values
                                  [STATIC_NAT_SETTINGS_PAGE_OLD_GLOBAL_PORT]);
    snat_entry.protocol=atoi(values[STATIC_NAT_SETTINGS_PAGE_OLD_PROTOCOL]);

    if (QcMapClient->DeleteStaticNatEntry(&snat_entry,&qmi_error) ||
        qmi_error == QMI_ERR_INTERFACE_NOT_FOUND_V01)
    {
      qmi_error=QMI_ERR_NONE_V01;
      //now add the midified snat entry...

      snat_entry.private_ip_addr=readable_ip_to_system_ip(values
                                   [STATIC_NAT_SETTINGS_PAGE_PRIVATE_IP_ADDR]);
      snat_entry.private_port=atoi(values
                                      [STATIC_NAT_SETTINGS_PAGE_PRIVATE_PORT]);
      snat_entry.global_port=atoi(values
                                       [STATIC_NAT_SETTINGS_PAGE_GLOBAL_PORT]);
      snat_entry.protocol=atoi(values
                                          [STATIC_NAT_SETTINGS_PAGE_PROTOCOL]);
      if (!QcMapClient->AddStaticNatEntry(&snat_entry,&qmi_error) &&
          qmi_error != QMI_ERR_INTERFACE_NOT_FOUND_V01)
      {
        //Add failed, so re-add snat entry which was deleted earlier.
        LOG_MSG_ERROR("New entry addition failed: Re-add old entry : %d",
                     qmi_error, 0, 0);
        qmi_error=QMI_ERR_NONE_V01;
        snat_entry.private_ip_addr=readable_ip_to_system_ip(values
                               [STATIC_NAT_SETTINGS_PAGE_OLD_PRIVATE_IP_ADDR]);
        snat_entry.private_port=atoi(values
                                  [STATIC_NAT_SETTINGS_PAGE_OLD_PRIVATE_PORT]);
        snat_entry.global_port=atoi(values
                                   [STATIC_NAT_SETTINGS_PAGE_OLD_GLOBAL_PORT]);
        snat_entry.protocol=atoi(values
                                      [STATIC_NAT_SETTINGS_PAGE_OLD_PROTOCOL]);
        if (!QcMapClient->AddStaticNatEntry(&snat_entry,&qmi_error) &&
            qmi_error != QMI_ERR_INTERFACE_NOT_FOUND_V01)
          LOG_MSG_ERROR("Old Static nat entry re-addition failed: %d",
                        qmi_error, 0, 0);
      }
    }
    else
    {
      LOG_MSG_ERROR("Modify Static nat entry operation deletion failed: %d",
                    qmi_error, 0, 0);
    }
  }
  /* build response msg */
  strncat(buf,"{",1);
  memset(tmp,0,MAX_BUF_LEN);
  snprintf(tmp, strlen(ADDSNATENTRY) + strlen(PAGE)+1, PAGE,ADDSNATENTRY);
  strncat(buf,tmp,strlen(tmp));
  memset(tmp,0,MAX_BUF_LEN);
  if (qmi_error == QMI_ERR_INTERFACE_NOT_FOUND_V01)
  {
    LOG_MSG_INFO1("Backhaul down, SNAT Entry operation %d for  xml file only: %d",
                   atoi(values[STATIC_NAT_SETTINGS_PAGE_ADD_DELETE_MODIFY_SNAT]),
                   qmi_error, 0);
    //now set qmi_error to none to force success return to webserver.
    qmi_error = QMI_ERR_NONE_V01;
  }
  snprintf(tmp, strlen(QMI_Error_String[qmi_error]) + strlen(RESULT)+1, RESULT,
           QMI_Error_String[qmi_error]);
  strncat(buf,tmp,strlen(tmp));
  strncat(buf,"}",1);
  #ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
  #endif
  /* send response msg to CGI  */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response  for AddSnatEntry result is failure: %d \n",
        result, 0, 0);
  }
}
/*===========================================================================
  FUNCTION GetMobileAP_Config
===========================================================================*/
/*!
@brief
  Retrieves MobileAP parameters.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int GetMobileAP_Config
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  boolean flag=0;
  int result;
  int i=0;
  char tmp[MAX_BUF_LEN] ={0};
  qcmap_msgr_wwan_status_enum_v01 v4status;
  qcmap_msgr_wwan_status_enum_v01 v6status;
  qmi_error=QMI_ERR_NONE_V01;
  /* retrieve Auto Connect config*/
  if (QcMapClient->GetAutoconnect( &flag, &qmi_error))
  {
    snprintf(values[MOBILE_AP_PAGE_AUTO_CONNECT],sizeof(int),"%d", (int)flag) ;
  }
  else
  {
    LOG_MSG_ERROR("Get Auto connect failed!!  error :%d \n", qmi_error, 0, 0);
  }
  snprintf(values[MOBILE_AP_PAGE_AUTO_CONNECT_RESULT],
    strlen(QMI_Error_String[qmi_error])+1, "%s", QMI_Error_String[qmi_error]) ;
  qmi_error=QMI_ERR_NONE_V01;
  /* retrieve Roaming config*/
  if (QcMapClient->GetRoaming(&flag, &qmi_error))
  {
    snprintf(values[MOBILE_AP_PAGE_ROAMING],sizeof(int),"%d",(int) flag) ;
  }
  else
  {
    LOG_MSG_ERROR("Get Roaming flag failed!!", 0, 0, 0);
  }
  snprintf(values[MOBILE_AP_PAGE_ROAMING_RESULT],
    strlen(QMI_Error_String[qmi_error])+1, "%s", QMI_Error_String[qmi_error]) ;
  create_response_buf( values, buf, MOBILE_AP_PAGE_MAX, mobileap_str,
                      MOBILE_AP_PAGE_PAGE,0);

  #ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
  #endif
  /* Send response back to CGI */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response  for Get MobileAP_Config result is failure: %d \n",
        result, 0, 0);
  }
}
/*===========================================================================
  FUNCTION SetMobileAP_Config
===========================================================================*/
/*!
@brief
  Applies/Configure MobileAP parameters.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int SetMobileAP_Config
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  uint32_t mask_flag=0;
  uint32_t result;
  char tmp[MAX_BUF_LEN] ={0};
  uint32_t flag;
  qcmap_msgr_wwan_status_enum_v01 v4status;
  qcmap_msgr_wwan_status_enum_v01 v6status;
  //found the MobileAP page!!
  mask_flag=atoi(values[MOBILE_AP_PAGE_MASK]);
  qmi_error=QMI_ERR_NONE_V01;
  flag=atoi(values[MOBILE_AP_PAGE_AUTO_CONNECT]);
  /*check whether we really need to change Auto Connect flag */
  if( ( (mask_flag & FIELD_MASK_2)  == FIELD_MASK_2) )
  {
    if (QcMapClient->SetAutoconnect(flag,&qmi_error))
    {
      snprintf(values[MOBILE_AP_PAGE_AUTO_CONNECT],sizeof(flag),"%d",flag) ;
      snprintf(values[MOBILE_AP_PAGE_AUTO_CONNECT_RESULT],
               strlen(QMI_Error_String[qmi_error])+1,"%s",
               QMI_Error_String[qmi_error]);
    }
    else
    {
      snprintf(values[MOBILE_AP_PAGE_AUTO_CONNECT_RESULT],
      strlen(QMI_Error_String[qmi_error])+1, "%s",QMI_Error_String[qmi_error]);
      LOG_MSG_ERROR("Set Auto connect failed!! qmi error number:%d",
                     qmi_error, 0, 0);
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" Auto Connect:  %d and mask flag : %d \n", flag , mask_flag,
                    0);
    snprintf(values[MOBILE_AP_PAGE_AUTO_CONNECT_RESULT],
             strlen(QMI_Error_String[qmi_error])+1,"%s",
             QMI_Error_String[qmi_error]);
  }

  qmi_error=QMI_ERR_NONE_V01;
  flag=atoi(values[MOBILE_AP_PAGE_ROAMING]);
  /* check we really need to change Roaming flag */
  if( ( (mask_flag & FIELD_MASK_3)  == FIELD_MASK_3) )
  {
    if (QcMapClient->SetRoaming(flag,&qmi_error))
    {
      snprintf(values[MOBILE_AP_PAGE_ROAMING],sizeof(flag),"%d",flag) ;
      snprintf(values[MOBILE_AP_PAGE_ROAMING_RESULT],
      strlen(QMI_Error_String[qmi_error])+1, "%s", QMI_Error_String[qmi_error]);
    }
    else
    {
      LOG_MSG_ERROR("Set Roaming flag failed!!qmi error number:%d ",
                    qmi_error, 0, 0);
      snprintf(values[MOBILE_AP_PAGE_ROAMING_RESULT],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]);
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" Roaming:   %d and mask flag : %d\n", flag , mask_flag , 0);
    snprintf(values[MOBILE_AP_PAGE_ROAMING_RESULT],
             strlen(QMI_Error_String[qmi_error])+1, "%s",
             QMI_Error_String[qmi_error]);
  }
  create_response_buf( values, buf, MOBILE_AP_PAGE_MAX, mobileap_str,
                    MOBILE_AP_PAGE_PAGE,0);
#ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
#endif
  /*send response back to CGI */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response  for SetMobileAP_Config result is failure: %d \n",
        result, 0, 0);
  }
}
/*===========================================================================
  FUNCTION GetUPNP_DLNA_Config
===========================================================================*/
/*!
@brief
  Retrieves UPNP,DLNA,MDNS  parameters.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int GetUPNP_DLNA_Config
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  int flag=0;
  int result;
  int i=0;
  char tmp[MAX_BUF_LEN] ={0};
  qmi_error=QMI_ERR_NONE_V01;
  /* retrieve UPNP config*/
  if (QcMapClient->GetUPNPStatus( (qcmap_msgr_upnp_mode_enum_v01*) &flag,
      &qmi_error))
  {
    snprintf(values[UPNP_DLNA_PAGE_UPNP_ENABLE],sizeof(int),"%d", (int)flag) ;
  }
  else
  {
    LOG_MSG_ERROR("Get UPNP status failed!!  error :%d \n", qmi_error, 0, 0);
  }
  snprintf(values[UPNP_DLNA_PAGE_UPNP_RESULT],
    strlen(QMI_Error_String[qmi_error])+1, "%s", QMI_Error_String[qmi_error]) ;

  qmi_error=QMI_ERR_NONE_V01;
  /* retrieve DLNA config*/
  if (QcMapClient->GetDLNAStatus((qcmap_msgr_dlna_mode_enum_v01*) &flag,
      &qmi_error))
  {
    snprintf(values[UPNP_DLNA_PAGE_DLNA_ENABLE],sizeof(int),"%d",(int) flag) ;
  }
  else
  {
    LOG_MSG_ERROR("Get DLNA status flag failed!!", 0, 0, 0);
  }
  snprintf(values[UPNP_DLNA_PAGE_DLNA_RESULT],
    strlen(QMI_Error_String[qmi_error])+1, "%s", QMI_Error_String[qmi_error]) ;

  qmi_error=QMI_ERR_NONE_V01;
  /* retrieve MDNS config*/
  if (QcMapClient->GetMDNSStatus((qcmap_msgr_mdns_mode_enum_v01*) &flag,
      &qmi_error))
  {
    snprintf(values[UPNP_DLNA_PAGE_MDNS_ENABLE],sizeof(int),"%d",(int) flag) ;
  }
  else
  {
    LOG_MSG_ERROR("Get MDNS status flag failed!!", 0, 0, 0);
  }
  snprintf(values[UPNP_DLNA_PAGE_MDNS_RESULT],
    strlen(QMI_Error_String[qmi_error])+1, "%s", QMI_Error_String[qmi_error]) ;

  create_response_buf( values, buf, UPNP_DLNA_PAGE_MAX, upnp_dlna_str,
                      UPNP_DLNA_PAGE_PAGE,0);

  #ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
  #endif
  /* Send response back to CGI */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response GetUPNP_DLNA result failure: %d \n",
        result, 0, 0);
  }
}

/*===========================================================================
  FUNCTION SetUPNP_DLNA_Config
===========================================================================*/
/*!
@brief
  Applies/Configure UPNP,DLNA,MDNS parameters.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int SetUPNP_DLNA_Config
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  uint32_t mask_flag=0;
  uint32_t result;
  char tmp[MAX_BUF_LEN] ={0};
  uint32_t flag;
  qcmap_msgr_wwan_status_enum_v01 v4status;
  qcmap_msgr_wwan_status_enum_v01 v6status;
  //found the UPNP,DLNA,MDNS page!!
  mask_flag=atoi(values[UPNP_DLNA_PAGE_MASK]);
  qmi_error=QMI_ERR_NONE_V01;
  flag=atoi(values[UPNP_DLNA_PAGE_UPNP_ENABLE]);
  /*check whether we really need to change UPNP enable/disable */
  if( ( (mask_flag & FIELD_MASK_1)  == FIELD_MASK_1) )
  {
    if(flag == ENABLED)
    {
      if(QcMapClient->EnableUPNP(&qmi_error))
      {
        snprintf(values[UPNP_DLNA_PAGE_UPNP_ENABLE],sizeof(flag),"%d",flag) ;
        snprintf(values[UPNP_DLNA_PAGE_UPNP_RESULT],
                 strlen(QMI_Error_String[qmi_error])+1,"%s",
                 QMI_Error_String[qmi_error]);
      }
      else
      {
        snprintf(values[UPNP_DLNA_PAGE_UPNP_RESULT],
        strlen(QMI_Error_String[qmi_error])+1, "%s",QMI_Error_String[qmi_error]);
        LOG_MSG_ERROR("Set UPNP enable failed!! qmi error number:%d",
                       qmi_error, 0, 0);
      }
    }
    else if(flag == DISABLE_PnP)
    {
      if (QcMapClient->DisableUPNP(&qmi_error))
      {
        snprintf(values[UPNP_DLNA_PAGE_UPNP_ENABLE],sizeof(flag),"%d",flag) ;
        snprintf(values[UPNP_DLNA_PAGE_UPNP_RESULT],
                 strlen(QMI_Error_String[qmi_error])+1,"%s",
                 QMI_Error_String[qmi_error]);
      }
      else
      {
        snprintf(values[UPNP_DLNA_PAGE_UPNP_RESULT],
        strlen(QMI_Error_String[qmi_error])+1, "%s",QMI_Error_String[qmi_error]);
        LOG_MSG_ERROR("Set UPNP disable failed!! qmi error number:%d",
                       qmi_error, 0, 0);
      }
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" UPNP enable/disable:  %d and mask flag : %d \n",
                   flag , mask_flag , 0);
    snprintf(values[UPNP_DLNA_PAGE_UPNP_RESULT],
             strlen(QMI_Error_String[qmi_error])+1,"%s",
             QMI_Error_String[qmi_error]);
  }
  qmi_error=QMI_ERR_NONE_V01;
  flag=atoi(values[UPNP_DLNA_PAGE_DLNA_ENABLE]);
  /*check whether we really need to change DLNA enable/disable */
  if( ( (mask_flag & FIELD_MASK_2)  == FIELD_MASK_2) )
  {
    if(flag == ENABLED)
    {
      if (QcMapClient->EnableDLNA(&qmi_error))
      {
        snprintf(values[UPNP_DLNA_PAGE_DLNA_ENABLE],sizeof(flag),"%d",flag) ;
        snprintf(values[UPNP_DLNA_PAGE_DLNA_RESULT],
                 strlen(QMI_Error_String[qmi_error])+1,"%s",
                 QMI_Error_String[qmi_error]);
      }
      else
      {
        snprintf(values[UPNP_DLNA_PAGE_DLNA_RESULT],
        strlen(QMI_Error_String[qmi_error])+1, "%s",QMI_Error_String[qmi_error]);
        LOG_MSG_ERROR("Set DLNA enable failed!! qmi error number:%d",
                       qmi_error, 0, 0);
      }
    }
    else if(flag == DISABLE_PnP)
    {
      if (QcMapClient->DisableDLNA(&qmi_error))
      {
        snprintf(values[UPNP_DLNA_PAGE_DLNA_ENABLE],sizeof(flag),"%d",flag) ;
        snprintf(values[UPNP_DLNA_PAGE_DLNA_RESULT],
                 strlen(QMI_Error_String[qmi_error])+1,"%s",
                 QMI_Error_String[qmi_error]);
      }
      else
      {
        snprintf(values[UPNP_DLNA_PAGE_DLNA_RESULT],
        strlen(QMI_Error_String[qmi_error])+1, "%s",QMI_Error_String[qmi_error]);
        LOG_MSG_ERROR("Set DLNA disable failed!! qmi error number:%d",
                       qmi_error, 0, 0);
      }
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" DLNA enable/disable:%d mask flag : %d \n", flag , mask_flag,
                    0);
    snprintf(values[UPNP_DLNA_PAGE_DLNA_RESULT],
             strlen(QMI_Error_String[qmi_error])+1,"%s",
             QMI_Error_String[qmi_error]);
  }
  qmi_error=QMI_ERR_NONE_V01;
  flag=atoi(values[UPNP_DLNA_PAGE_MDNS_ENABLE]);
  /*check whether we really need to change MDNS enable/disable */
  if( ( (mask_flag & FIELD_MASK_3)  == FIELD_MASK_3) )
  {
    if(flag == ENABLED)
    {
      if (QcMapClient->EnableMDNS(&qmi_error))
      {
        snprintf(values[UPNP_DLNA_PAGE_MDNS_ENABLE],sizeof(flag),"%d",flag) ;
        snprintf(values[UPNP_DLNA_PAGE_MDNS_RESULT],
                 strlen(QMI_Error_String[qmi_error])+1,"%s",
                 QMI_Error_String[qmi_error]);
      }
      else
      {
        snprintf(values[UPNP_DLNA_PAGE_MDNS_RESULT],
        strlen(QMI_Error_String[qmi_error])+1, "%s",QMI_Error_String[qmi_error]);
        LOG_MSG_ERROR("Set MDNS enable failed!! qmi error number:%d",
                       qmi_error, 0, 0);
      }
    }
    else if(flag == DISABLE_PnP)
    {
      if (QcMapClient->DisableMDNS(&qmi_error))
      {
        snprintf(values[UPNP_DLNA_PAGE_MDNS_RESULT],sizeof(flag),"%d",flag) ;
        snprintf(values[UPNP_DLNA_PAGE_MDNS_RESULT],
                 strlen(QMI_Error_String[qmi_error])+1,"%s",
                 QMI_Error_String[qmi_error]);
      }
      else
      {
        snprintf(values[UPNP_DLNA_PAGE_MDNS_RESULT],
        strlen(QMI_Error_String[qmi_error])+1, "%s",QMI_Error_String[qmi_error]);
        LOG_MSG_ERROR("Set MDNS disable failed!! qmi error number:%d",
                       qmi_error, 0, 0);
      }
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" MDNS enable/disable: %d mask flag: %d \n", flag , mask_flag,
                    0);
    snprintf(values[UPNP_DLNA_PAGE_MDNS_RESULT],
             strlen(QMI_Error_String[qmi_error])+1,"%s",
             QMI_Error_String[qmi_error]);
  }

  create_response_buf( values, buf, UPNP_DLNA_PAGE_MAX, upnp_dlna_str,
                    UPNP_DLNA_PAGE_PAGE,0);
#ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
#endif
  /*send response back to CGI */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response SetUPNP_DLNA result is failure: %d \n",
        result, 0, 0);
  }
}

/*===========================================================================
  FUNCTION GetWWANIPV4
===========================================================================*/
/*!
@brief
  Retrieves MobileAP parameters.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int GetWWANIPV4
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  boolean flag=0;
  boolean ipv4_state=0;
  int result;
  int i=0;
  char tmp[MAX_BUF_LEN] ={0};
  qcmap_msgr_wwan_status_enum_v01 v4status;
  qcmap_msgr_wwan_status_enum_v01 v6status;
  qmi_error=QMI_ERR_NONE_V01;
  /* retrieve IPV4 status config*/
  if (QcMapClient->GetIPv4State(&ipv4_state,&qmi_error))
  {
    snprintf(values[WWAN_IPV4_PAGE_IPV4_ENABLE_RESULT],
             strlen(QMI_Error_String[qmi_error])+1, "%s",
             QMI_Error_String[qmi_error]);
    snprintf(values[WWAN_IPV4_PAGE_IPV4_ENABLE],
            sizeof(int),"%d",(int)ipv4_state) ;

  }
  else
  {
    LOG_MSG_ERROR("Get IPV4 status failed: error %d!!\n", qmi_error, 0, 0);
  }
  snprintf(values[WWAN_IPV4_PAGE_IPV4_ENABLE_RESULT],
           strlen(QMI_Error_String[qmi_error])+1, "%s",
           QMI_Error_String[qmi_error]);
  qmi_error=QMI_ERR_NONE_V01;
  /* retrieve WWAN status IPV4 config*/
  if (QcMapClient->GetWWANStatus(&v4status,&v6status,&qmi_error))
  {
    snprintf(values[WWAN_IPV4_PAGE_BACKHAUL],
             strlen(WWAN_Status_String[v4status])+1,"%s",
             WWAN_Status_String[v4status]) ;
  }
  else
  {
    LOG_MSG_ERROR("Get Backhaul status failed: error %d!!\n", qmi_error, 0, 0);
  }
  snprintf(values[WWAN_IPV4_PAGE_BACKHAUL_RESULT],
           strlen(QMI_Error_String[qmi_error])+1, "%s",
           QMI_Error_String[qmi_error]);

  qcmap_msgr_net_policy_info_v01 WWAN_Profile_Data;
  memset(&WWAN_Profile_Data,0,sizeof(qcmap_msgr_net_policy_info_v01));
  memset(&qmi_error,0,sizeof(qmi_error_type_v01));
  int get_wwan_policy=-1;
  qmi_error=QMI_ERR_NONE_V01;
  /* retrieve WWAN policy for IPV4 */
  if (QcMapClient->GetWWANPolicy(&WWAN_Profile_Data, &qmi_error))
  {
    get_wwan_policy=1;
  }
  else
  {
    LOG_MSG_ERROR("Failed to Get WWAN policy .error %d.\n ", qmi_error, 0, 0);
    get_wwan_policy=-1;
  }
  if (WWAN_Profile_Data.ip_family != IPV4V6 )
  {
    qmi_error = QMI_ERR_INVALID_IP_FAMILY_PREF_V01 -1 ;
  }
  if( WWAN_Profile_Data.tech_pref != 0 && WWAN_Profile_Data.tech_pref !=1 &&
      WWAN_Profile_Data.tech_pref != 2 )
  {
    #ifdef DEBUG_PRINT
      printf("Error: invalid tech preference:- %d \n",WWAN_Profile_Data.tech_pref);
    #endif
    qmi_error = QMI_ERR_INVALID_TECH_PREF_V01;
  }
  snprintf(values[WWAN_IPV4_PAGE_TECH_RESULT],
  strlen(QMI_Error_String[qmi_error])+1, "%s", QMI_Error_String[qmi_error]) ;
  /* Build response msg */
  snprintf(values[WWAN_IPV4_PAGE_NETP_TECHNOLOGY],
           sizeof(int),"%d",(int)WWAN_Profile_Data.tech_pref) ;
  snprintf(values[WWAN_IPV4_PAGE_NETP_TECH_IPV4],
           sizeof(int),"%d",(int)WWAN_Profile_Data.v4_profile_id_3gpp) ;
  snprintf(values[WWAN_IPV4_PAGE_NETP_TECH2_IPV4],
           sizeof(int),"%d",(int)WWAN_Profile_Data.v4_profile_id_3gpp2) ;
  create_response_buf( values, buf, WWAN_IPV4_PAGE_MAX, wwan_ipv4_str,
                      WWAN_IPV4_PAGE_PAGE,0);

  #ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
  #endif
  /* Send response back to CGI */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response  for GetWWANIPV4 result is failure: %d \n",
        result, 0, 0);
  }
}
/*===========================================================================
  FUNCTION SetWWANIPV4
===========================================================================*/
/*!
@brief
  Applies/Configure MobileAP parameters.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int SetWWANIPV4
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  uint32_t mask_flag=0;
  uint32_t result;
  char tmp[MAX_BUF_LEN] ={0};
  uint32_t flag, enablev4=1;
  qcmap_msgr_wwan_status_enum_v01 v4status;
  qcmap_msgr_wwan_status_enum_v01 v6status;
  //found the MobileAP page!!
  mask_flag=atoi(values[WWAN_IPV4_PAGE_MASK]);
  qmi_error=QMI_ERR_NONE_V01;
  flag=atoi(values[WWAN_IPV4_PAGE_IPV4_ENABLE]);
  /*check whether we really need to call IPV4 Enable/disable flag */
  if(  ( (mask_flag & FIELD_MASK_1)  == FIELD_MASK_1) )
  {
    if(flag==ENABLED)
    {
      if (QcMapClient->EnableIPV4(&qmi_error))
      {
        snprintf(values[WWAN_IPV4_PAGE_IPV4_ENABLE],sizeof(flag),"%d",flag) ;
        snprintf(values[WWAN_IPV4_PAGE_IPV4_ENABLE_RESULT],
                 strlen(QMI_Error_String[qmi_error])+1,"%s",
                 QMI_Error_String[qmi_error]);
      }
      else
      {
        snprintf(values[WWAN_IPV4_PAGE_IPV4_ENABLE_RESULT],
                 strlen(QMI_Error_String[qmi_error])+1, "%s",
                 QMI_Error_String[qmi_error]);
        enablev4 = 0;
        LOG_MSG_ERROR("Enable IPV4 failed!! qmi error number:%d",
                       qmi_error, 0, 0);
      }
    }
    if(flag==DISABLED)
    {
      if (QcMapClient->DisableIPV4(&qmi_error))
      {
        snprintf(values[WWAN_IPV4_PAGE_IPV4_ENABLE],sizeof(flag),"%d",flag) ;
        snprintf(values[WWAN_IPV4_PAGE_IPV4_ENABLE_RESULT],
         strlen(QMI_Error_String[qmi_error])+1, "%s", QMI_Error_String[qmi_error]);
      }
      else
      {
        snprintf(values[WWAN_IPV4_PAGE_IPV4_ENABLE_RESULT],
                 strlen(QMI_Error_String[qmi_error])+1, "%s",
                 QMI_Error_String[qmi_error]);
        LOG_MSG_ERROR("Enable IPV4 failed!! qmi error number:%d",
                      qmi_error, 0, 0);
      }
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" IPV4 enable/disable:  %d and mask flag : %d \n", flag , mask_flag,
                    0);
    snprintf(values[WWAN_IPV4_PAGE_IPV4_ENABLE_RESULT],
     strlen(QMI_Error_String[qmi_error])+1, "%s", QMI_Error_String[qmi_error]);
  }

  qmi_error=QMI_ERR_NONE_V01;
  flag=atoi(values[WWAN_IPV4_PAGE_BACKHAUL]);
  /* check we really need to Connect backhaul IPV4 */
  if(  (flag == ENABLED ) &&  ( (mask_flag & FIELD_MASK_2)  == FIELD_MASK_2)
                                                  && (enablev4 == ENABLED) )
  {
    if (QcMapClient->ConnectBackHaul(QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01,
                                     &qmi_error))
    {
      snprintf(values[WWAN_IPV4_PAGE_BACKHAUL],strlen(CONNECTED)+1,"%s",
               CONNECTED) ;
      snprintf(values[WWAN_IPV4_PAGE_BACKHAUL_RESULT],
       strlen(QMI_Error_String[qmi_error])+1,"%s",QMI_Error_String[qmi_error]);
    }
    else
    {
      LOG_MSG_ERROR("ConnectBackHaul fails. qmi error number:%d \n",
                    qmi_error, 0, 0);
      snprintf(values[WWAN_IPV4_PAGE_BACKHAUL_RESULT],
               strlen(QMI_Error_String[qmi_error])+1,"%s",
               QMI_Error_String[qmi_error]);
    }
  }
  /* check if we really need to disconnect IPV4 backhaul */
  else if(  (flag == DISABLED ) &&  ( (mask_flag & FIELD_MASK_2)
             == FIELD_MASK_2) )
  {
    if (QcMapClient->DisconnectBackHaul(QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01,
                                        &qmi_error))
    {
      snprintf(values[WWAN_IPV4_PAGE_BACKHAUL],strlen(DISCONNECTED)+1,"%s",
               DISCONNECTED) ;
      snprintf(values[WWAN_IPV4_PAGE_BACKHAUL_RESULT],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]) ;
    }
    else
    {
      LOG_MSG_ERROR("Discon backhaul fails error :%d\n", qmi_error, 0, 0);
      snprintf(values[WWAN_IPV4_PAGE_BACKHAUL_RESULT],
               strlen(QMI_Error_String[qmi_error])+1,"%s",
               QMI_Error_String[qmi_error]);
    }
  }
  else
  {
    //no change required, so just print information
    qmi_error=QMI_ERR_NONE_V01;
    if (QcMapClient->GetWWANStatus(&v4status,&v6status,&qmi_error))
    {
      snprintf(values[WWAN_IPV4_PAGE_BACKHAUL],
      strlen(WWAN_Status_String[v4status])+1,"%s",WWAN_Status_String[v4status]);
    }
    else
    {
      LOG_MSG_ERROR("Get Backhaul status failed: error %d\n", qmi_error, 0, 0);
    }
    snprintf(values[WWAN_IPV4_PAGE_BACKHAUL_RESULT],
    strlen(QMI_Error_String[qmi_error])+1, "%s",QMI_Error_String[qmi_error]);
  }
  int set_wwan_policy=-1;
  qmi_error=QMI_ERR_NONE_V01;
  qcmap_msgr_net_policy_info_v01 WWAN_Profile_Data;
  /* check if we really need to change wwan policy info for IPV4 */
  if(  ( (mask_flag & FIELD_MASK_3)  == FIELD_MASK_3) )
  {
    memset(&WWAN_Profile_Data,0,sizeof(qcmap_msgr_net_policy_info_v01));
    WWAN_Profile_Data.ip_family = QCMAP_MSGR_IP_FAMILY_V4_V01;
    if(atoi(values[WWAN_IPV4_PAGE_NETP_TECHNOLOGY]) == 1)
      WWAN_Profile_Data.tech_pref = QCMAP_MSGR_MASK_TECH_PREF_3GPP_V01;
    else if(atoi(values[WWAN_IPV4_PAGE_NETP_TECHNOLOGY]) == 2)
      WWAN_Profile_Data.tech_pref = QCMAP_MSGR_MASK_TECH_PREF_3GPP2_V01;

    WWAN_Profile_Data.v4_profile_id_3gpp = (uint8_t)atoi(values
                                              [WWAN_IPV4_PAGE_NETP_TECH_IPV4]);
    WWAN_Profile_Data.v4_profile_id_3gpp2 = (uint8_t)atoi(values
                                             [WWAN_IPV4_PAGE_NETP_TECH2_IPV4]);
    if (QcMapClient->SetWWANPolicy(WWAN_Profile_Data, &qmi_error))
    {
      set_wwan_policy = 0;
      snprintf(values[WWAN_IPV4_PAGE_TECH_RESULT],
               strlen(QMI_Error_String[qmi_error])+1, "%s",
               QMI_Error_String[qmi_error]);
    }
    else
    {
      LOG_MSG_ERROR("Failed to Set WWAN policy error %d.\n ", qmi_error, 0, 0);
      set_wwan_policy=-1;
      snprintf(values[WWAN_IPV4_PAGE_TECH_RESULT],
               strlen(QMI_Error_String[qmi_error])+1,"%s",
               QMI_Error_String[qmi_error]);
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_INFO1(" net policy settings:  tech : %d profile 3gpp :%d \
                    profile 3gpp2: %d\n",
                    atoi(values[WWAN_IPV4_PAGE_NETP_TECHNOLOGY]),
                    atoi(values[WWAN_IPV4_PAGE_NETP_TECH_IPV4]),
                    atoi(values[WWAN_IPV4_PAGE_NETP_TECH2_IPV4]) );
    snprintf(values[WWAN_IPV4_PAGE_TECH_RESULT],
             strlen(QMI_Error_String[qmi_error])+1, "%s",
             QMI_Error_String[qmi_error]);
  }
  /* build response msg */
  create_response_buf( values, buf, WWAN_IPV4_PAGE_MAX, wwan_ipv4_str,
                    WWAN_IPV4_PAGE_PAGE,0);
#ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
#endif
  /*send response back to CGI */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response  for SetWWANIPV4 result is failure: %d \n",
        result, 0, 0);
  }
}
/*===========================================================================
 FUNCTION ConfigFirewall()
 ===========================================================================*/
 /*!
 @brief
   Displays Firewall config.

 @input
   values     - cgi form field values.
   buf        - message to be sent to web cgi.
   their_addr  - socket address to which data needs to sent.
   addr_len    - lenght of their_addr.

 @return
   0  - success
   -1 - failure

 @dependencies
   None

 @sideefects
   None
 */
 /*=========================================================================*/
int ConfigFirewall
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  boolean enable_firewall, pkts_allowed;
  char tmp[MAX_BUF_LEN] = {0};
  memset(tmp,0,MAX_BUF_LEN);
  qmi_error = QMI_ERR_NONE_V01;
  int result;
  if(QcMapClient->GetFirewall(&enable_firewall, &pkts_allowed, &qmi_error))
  {

    strncat(buf,"{",1);
    snprintf(tmp, strlen(GETFIREWALLCONFIG) +strlen(PAGE)+1, PAGE,GETFIREWALLCONFIG);
    strncat(buf,tmp,strlen(tmp));
    memset(tmp,0,MAX_BUF_LEN);
    snprintf(tmp, sizeof(enable_firewall) + strlen(FIREWALL_ENABLE)+1,
                                            FIREWALL_ENABLE,enable_firewall);
    strncat(buf,tmp,strlen(tmp));
    memset(tmp,0,MAX_BUF_LEN);
    snprintf(tmp, sizeof(pkts_allowed) + strlen(PKTS_ALLOWED_CONFIG)+1,
                                                  PKTS_ALLOWED_CONFIG,pkts_allowed);
    strncat(buf,tmp,strlen(tmp));
    strncat(buf,"}",1);
  }
  else
  {
   create_error( values,buf, qmi_error);
  }
  #ifdef DEBUG_PRINT
  printf("Response message: %s\n",buf);
  #endif
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  LOG_MSG_ERROR(" Sent the response for Dispaly firewall  result is : %d \n",
        result, 0, 0);
  }
/*===========================================================================
 FUNCTION DisplayFirewall()
 ===========================================================================*/
 /*!
 @brief
   Displays Firewall.

 @input
   values     - cgi form field values.
   buf        - message to be sent to web cgi.
   their_addr  - socket address to which data needs to sent.
   addr_len    - lenght of their_addr.

 @return
   0  - success
   -1 - failure

 @dependencies
   None

 @sideefects
   None
 */
 /*=========================================================================*/
int DisplayFirewall
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  int result,handle_list_len = 0,tmp_size=0;
  int  index = 0, next_hdr_prot = 0 , ret_func = 0 , valid_flag=1;
  char tmp[MAX_BUF_LEN] = {0};
  char str[INET6_ADDRSTRLEN];
  boolean enable_firewall, pkts_allowed;
  qmi_error = QMI_ERR_NONE_V01;

  qcmap_msgr_firewall_conf_t extd_firewall_handle_list;
  qcmap_msgr_firewall_conf_t extd_firewall_get;
  qcmap_msgr_firewall_entry_conf_t *firewall_entry;

  if(QcMapClient->GetFirewall(&enable_firewall, &pkts_allowed, &qmi_error))
  {

    strncat(buf,"{",1);
    snprintf(tmp, strlen(FIREWALLPAGE) +strlen(PAGE)+1, PAGE,FIREWALLPAGE);
    strncat(buf,tmp,strlen(tmp));
    memset(tmp,0,MAX_BUF_LEN);
    snprintf(tmp, sizeof(enable_firewall) + strlen(FIREWALL_ENABLE)+1,
                                            FIREWALL_ENABLE,enable_firewall);
    strncat(buf,tmp,strlen(tmp));
    memset(tmp,0,MAX_BUF_LEN);
    snprintf(tmp, sizeof(pkts_allowed) + strlen(PKTS_ALLOWED)+1,
                                                  PKTS_ALLOWED,pkts_allowed);
    strncat(buf,tmp,strlen(tmp));
    memset(tmp,0,MAX_BUF_LEN);
  }
  else
  {

  goto  error_case;

  }

  memset(&extd_firewall_handle_list, 0, sizeof(qcmap_msgr_firewall_conf_t));
  memset(tmp,0,MAX_BUF_LEN);
  if(strstr(values[FIREWALL_SETTINGS_GET_PAGE_PAGE],GETFIREWALLENTRIES))
  {
    extd_firewall_handle_list.extd_firewall_handle_list.ip_family =
      (ip_version_enum_type)atoi(values[FIREWALL_SETTINGS_GET_PAGE_IP_FAMILY]);
  }
  else
  {
    extd_firewall_handle_list.extd_firewall_handle_list.ip_family =
        (ip_version_enum_type)(atoi(values[FIREWALL_SETTINGS_PAGE_IP_FAMILY]));
  }
  if(QcMapClient->GetFireWallHandlesList(&extd_firewall_handle_list.\
                                        extd_firewall_handle_list, &qmi_error))
  {
    handle_list_len = extd_firewall_handle_list.extd_firewall_handle_list.\
                                                                 num_of_entries;
    snprintf(tmp,sizeof(handle_list_len) +strlen(COUNT)+1 ,COUNT,handle_list_len);
    strncat(buf,tmp,strlen(tmp));
    memset(tmp,0,MAX_BUF_LEN);
    snprintf(tmp, strlen(ENTRIES)+1, ENTRIES);
    strncat(buf,tmp,strlen(tmp));
    memset(tmp,0,MAX_BUF_LEN);
    for (index=0; index<handle_list_len; index++)
    {
      memset(&extd_firewall_get, 0, sizeof(qcmap_msgr_firewall_conf_t));
      memset(values,0,MAX_ELEMENT_COUNT * MAX_ELEMENT_LENGTH );
      extd_firewall_get.extd_firewall_entry.filter_spec.ip_vsn =
      extd_firewall_handle_list.extd_firewall_handle_list.ip_family;
      extd_firewall_get.extd_firewall_entry.firewall_handle =
      extd_firewall_handle_list.extd_firewall_handle_list.handle_list[index];
      if ( QcMapClient->GetFireWallEntry(&extd_firewall_get.\
                                              extd_firewall_entry, &qmi_error))
      {
        firewall_entry = &extd_firewall_get.extd_firewall_entry;
        if (firewall_entry)
        {
          snprintf( values[FIREWALL_SETTINGS_PAGE_FIREWALL_HANDLE],
                                  sizeof(firewall_entry->firewall_handle),"%d",
                                              firewall_entry->firewall_handle);
          snprintf( values[FIREWALL_SETTINGS_PAGE_IP_FAMILY],
                               sizeof(firewall_entry->filter_spec.ip_vsn),"%d",
                                           firewall_entry->filter_spec.ip_vsn);

          if( firewall_entry->filter_spec.ip_vsn == IP_V4 )
          {
            // For IPV4 Firewall entry
            if( firewall_entry->filter_spec.ip_hdr.v4.field_mask &
                                                      IPFLTR_MASK_IP4_SRC_ADDR)
            {

              snprintf( values[FIREWALL_SETTINGS_PAGE_IPV4_SRC_VALID],
                                           sizeof(valid_flag),"%d",valid_flag);

              system_ip_to_readable_ip(AF_INET,&firewall_entry->filter_spec.\
                                    ip_hdr.v4.src.addr.ps_s_addr,(char *)&str);
              snprintf( values[FIREWALL_SETTINGS_PAGE_IPV4_SRC_ADDR],
                                                         sizeof(str),"%s",str);
              system_ip_to_readable_ip(AF_INET,&firewall_entry->filter_spec.\
                             ip_hdr.v4.src.subnet_mask.ps_s_addr,(char *)&str);
              snprintf( values[FIREWALL_SETTINGS_PAGE_IPV4_SRC_SUBNET],
                                                         sizeof(str),"%s",str);
            }
            else
            {
              snprintf( values[FIREWALL_SETTINGS_PAGE_IPV4_SRC_VALID],strlen("0")+1,"0");
            }

            if(  firewall_entry->filter_spec.ip_hdr.v4.field_mask &
                                                           IPFLTR_MASK_IP4_TOS)
            {

              snprintf( values[FIREWALL_SETTINGS_PAGE_IPV4_TOS_VALID],
                                           sizeof(valid_flag),"%d",valid_flag);
              snprintf( values[FIREWALL_SETTINGS_PAGE_IPV4_TOS_VALUE],
                         sizeof(int),
                           "%x",firewall_entry->filter_spec.ip_hdr.v4.tos.val);
              snprintf( values[FIREWALL_SETTINGS_PAGE_IPV4_TOS_MASK],
                         sizeof(int),
                           "%x",firewall_entry->filter_spec.ip_hdr.v4.tos.val);

            }
            else
            {
              snprintf( values[FIREWALL_SETTINGS_PAGE_IPV4_TOS_VALID],
                                                            2,"0");
            }

            if( firewall_entry->filter_spec.ip_hdr.v4.field_mask &
                                                IPFLTR_MASK_IP4_NEXT_HDR_PROT )
            {

              next_hdr_prot = firewall_entry->filter_spec.ip_hdr.v4.\
                                                                 next_hdr_prot;
              snprintf( values[FIREWALL_SETTINGS_PAGE_NEXT_HEADER_PROTO_VALID],
                                           sizeof(valid_flag),"%d",valid_flag);
              snprintf( values[FIREWALL_SETTINGS_PAGE_NEXT_HEADER_PROTO],
                                     sizeof(next_hdr_prot),"%d",next_hdr_prot);

            }
            else
            {
              snprintf( values[FIREWALL_SETTINGS_PAGE_NEXT_HEADER_PROTO_VALID],
                                                            2,"0");
            }
            snprintf( values[FIREWALL_SETTINGS_PAGE_IPV6_SRC_VALID],2,"0");
            snprintf( values[FIREWALL_SETTINGS_PAGE_IPV6_CLASS_VALID],2,"0");


          }
          else
          {
            // Firewall entry IPV6
            if( firewall_entry->filter_spec.ip_hdr.v6.field_mask &
                                                     IPFLTR_MASK_IP6_SRC_ADDR)
            {

              snprintf( values[FIREWALL_SETTINGS_PAGE_IPV6_SRC_VALID],
                                          sizeof(valid_flag),"%d",valid_flag);

              system_ip_to_readable_ip(AF_INET6,(uint32 *)&firewall_entry->\
                 filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr32,(char *)&str);
              snprintf( values[FIREWALL_SETTINGS_PAGE_IPV6_SRC_ADDR],
                                                        sizeof(str),"%s",str);
              snprintf( values[FIREWALL_SETTINGS_PAGE_IPV6_SRC_PREFIX_LENGTH],
                 sizeof(int),
                   "%d",firewall_entry->filter_spec.ip_hdr.v6.src.prefix_len);

            }
            else
            {
              snprintf( values[FIREWALL_SETTINGS_PAGE_IPV6_SRC_VALID],2,"0");
            }

            if( extd_firewall_get.extd_firewall_entry.filter_spec.ip_hdr.v6.\
                                 field_mask & IPFLTR_MASK_IP6_TRAFFIC_CLASS  )
            {

              snprintf( values[FIREWALL_SETTINGS_PAGE_IPV6_CLASS_VALID],
                                          sizeof(valid_flag),"%d",valid_flag);
              snprintf( values[FIREWALL_SETTINGS_PAGE_IPV6_CLASS],
                sizeof(int),"%d",
                            firewall_entry->filter_spec.ip_hdr.v6.trf_cls.val);
              snprintf( values[FIREWALL_SETTINGS_PAGE_IPV6_MASK],
               sizeof(int),"%d",
                           firewall_entry->filter_spec.ip_hdr.v6.trf_cls.mask);

            }
            else
            {
              snprintf( values[FIREWALL_SETTINGS_PAGE_IPV6_CLASS_VALID],2,"0");
            }

            if( (firewall_entry->filter_spec.\
                          ip_hdr.v6.field_mask & IPFLTR_MASK_IP6_NEXT_HDR_PROT))
            {

              snprintf( values[FIREWALL_SETTINGS_PAGE_NEXT_HEADER_PROTO_VALID],
                                           sizeof(valid_flag),"%d",valid_flag);
              next_hdr_prot = firewall_entry->filter_spec.ip_hdr.v6.\
                                                                 next_hdr_prot;
              snprintf( values[FIREWALL_SETTINGS_PAGE_NEXT_HEADER_PROTO],
                                     sizeof(next_hdr_prot),"%d",next_hdr_prot);
            }
            else
            {
              snprintf( values[FIREWALL_SETTINGS_PAGE_NEXT_HEADER_PROTO_VALID],2,"0");
            }
            snprintf( values[FIREWALL_SETTINGS_PAGE_IPV4_SRC_VALID],2,"0");
            snprintf( values[FIREWALL_SETTINGS_PAGE_IPV4_TOS_VALID],2,"0");

          }
          // log10(PORT) +1 gives number of digits in PORT added
          // Further as to take care of terminating NULL character +2 is done
          switch(next_hdr_prot)
          {
            case PS_IPPROTO_TCP:
            if ( firewall_entry->filter_spec.next_prot_hdr.tcp.field_mask &
                                                      IPFLTR_MASK_TCP_SRC_PORT)
            {
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_VALID],
                                            sizeof(valid_flag),"%d",valid_flag);
              tmp_size = log10(firewall_entry->filter_spec.next_prot_hdr.\
                                                       tcp.src.port) + PADDING;
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_PORT],
                              tmp_size,"%d",(int)firewall_entry->filter_spec.\
                                                   next_prot_hdr.tcp.src.port);
              tmp_size = log10(firewall_entry->filter_spec.next_prot_hdr.\
                                                      tcp.src.range) + PADDING;
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_PORT_RANGE],
                              tmp_size,"%d",(int)firewall_entry->filter_spec.\
                                                   next_prot_hdr.tcp.src.range);
            }
            else
            {
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_VALID],2,"0");
            }

            if ( firewall_entry->filter_spec.next_prot_hdr.tcp.field_mask &
                                                       IPFLTR_MASK_TCP_DST_PORT)
            {
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_VALID],
                                           sizeof(valid_flag),"%d",valid_flag);
              tmp_size = log10(firewall_entry->filter_spec.next_prot_hdr.\
                                                                tcp.dst.port) + PADDING;
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_PORT],
                              tmp_size,"%d",(int)firewall_entry->filter_spec.\
                                                    next_prot_hdr.tcp.dst.port);
              tmp_size = log10(firewall_entry->filter_spec.next_prot_hdr.\
                                                                tcp.dst.range) + PADDING;
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_PORT_RANGE],
                              tmp_size,"%d",(int)firewall_entry->filter_spec.\
                                                  next_prot_hdr.tcp.dst.range);
            }
            else
            {
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_VALID],2,"0");
            }

            break;

            case PS_IPPROTO_UDP:
            if (firewall_entry->filter_spec.next_prot_hdr.udp.field_mask &
                                                      IPFLTR_MASK_UDP_SRC_PORT)
            {
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_VALID],
                                           sizeof(valid_flag),"%d",valid_flag);
              tmp_size = log10(firewall_entry->filter_spec.next_prot_hdr.\
                                                                udp.src.port) + PADDING;
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_PORT],
                              tmp_size,"%d",(int)firewall_entry->filter_spec.\
                                                   next_prot_hdr.udp.src.port);
              tmp_size = log10(firewall_entry->filter_spec.next_prot_hdr.\
                                                                udp.src.range) + PADDING;
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_PORT_RANGE],
                              tmp_size,"%d",(int)firewall_entry->filter_spec.\
                                                  next_prot_hdr.udp.src.range);
            }
            else
            {
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_VALID],2,"0");
            }

            if ( firewall_entry->filter_spec.next_prot_hdr.udp.field_mask &
                                                       IPFLTR_MASK_UDP_DST_PORT)
            {
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_VALID],
                                           sizeof(valid_flag),"%d",valid_flag);
              tmp_size = log10(firewall_entry->filter_spec.next_prot_hdr.\
                                                                udp.dst.port) + PADDING;
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_PORT],
                            tmp_size,"%d",(int)firewall_entry->filter_spec.\
                                                   next_prot_hdr.udp.dst.port);
              tmp_size = log10(firewall_entry->filter_spec.next_prot_hdr.\
                                                                udp.dst.range) + PADDING;
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_PORT_RANGE],
                            tmp_size,"%d",(int)firewall_entry->filter_spec.\
                                                  next_prot_hdr.udp.dst.range);
            }
            else
            {
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_VALID],2,"0");
            }


            break;

            case PS_IPPROTO_TCP_UDP:
            if (firewall_entry->filter_spec.next_prot_hdr.udp.field_mask &
                                                 IPFLTR_MASK_TCP_UDP_SRC_PORT)
            {
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_VALID],
                                          sizeof(valid_flag),"%d",valid_flag);
              tmp_size = log10(firewall_entry->filter_spec.next_prot_hdr.\
                                                 tcp_udp_port_range.src.port) + PADDING;
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_PORT],
                             tmp_size,"%d",(int)firewall_entry->filter_spec.\
                                    next_prot_hdr.tcp_udp_port_range.src.port);
              tmp_size = log10(firewall_entry->filter_spec.next_prot_hdr.\
                                                 tcp_udp_port_range.src.range) + PADDING;
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_PORT_RANGE],
                             tmp_size,"%d",(int)firewall_entry->filter_spec.\
                                  next_prot_hdr.tcp_udp_port_range.src.range);

            }
            else
            {
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_VALID],2,"0");
            }

            if ( firewall_entry->filter_spec.next_prot_hdr.udp.field_mask &
                                                  IPFLTR_MASK_TCP_UDP_DST_PORT)
            {
                snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_VALID],
                                          sizeof(valid_flag),"%d",valid_flag);
              tmp_size = log10(firewall_entry->filter_spec.next_prot_hdr.\
                                                  tcp_udp_port_range.dst.port) + PADDING;
                snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_PORT],
                            tmp_size,"%d",(int)firewall_entry->filter_spec.\
                                    next_prot_hdr.tcp_udp_port_range.dst.port);
              tmp_size = log10(firewall_entry->filter_spec.next_prot_hdr.\
                                                  tcp_udp_port_range.dst.range) + PADDING;
                snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_PORT_RANGE],
                              tmp_size,"%d",(int)firewall_entry->filter_spec.\
                                   next_prot_hdr.tcp_udp_port_range.dst.range);
            }
            else
            {
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_VALID],2,"0");
            }


            break;

            default:
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_VALID],
                                                    sizeof(valid_flag),"%d",0);
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_PORT],
                                                        sizeof(int),"%d",0);
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_PORT_RANGE],
                                                        sizeof(int),"%d",0);

              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_VALID],
                                                    sizeof(valid_flag),"%d",0);
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_PORT],
                                                        sizeof(int),"%d",0);
              snprintf( values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_PORT_RANGE],
                                                        sizeof(int),"%d",0);

            break;
          }

          if ( (next_hdr_prot == PS_IPPROTO_ICMP)&& (firewall_entry->filter_spec.\
                        next_prot_hdr.icmp.field_mask & IPFLTR_MASK_ICMP_MSG_TYPE))
          {

            snprintf( values[FIREWALL_SETTINGS_PAGE_ICMP_TYPE_VALID],
                                              sizeof(valid_flag),"%d",valid_flag);
            snprintf( values[FIREWALL_SETTINGS_PAGE_ICMP_TYPE],sizeof(uint32),
                        "%d",firewall_entry->filter_spec.next_prot_hdr.icmp.type);

          }
          else
          {
            snprintf( values[FIREWALL_SETTINGS_PAGE_ICMP_TYPE_VALID],2,"0");
          }
          if ( (next_hdr_prot == PS_IPPROTO_ICMP)&& (firewall_entry->filter_spec.\
                        next_prot_hdr.icmp.field_mask & IPFLTR_MASK_ICMP_MSG_CODE))
          {

            snprintf( values[FIREWALL_SETTINGS_PAGE_ICMP_CODE_VALID],
                                              sizeof(valid_flag),"%d",valid_flag);
            snprintf( values[FIREWALL_SETTINGS_PAGE_ICMP_CODE],sizeof(uint32),
                        "%d",firewall_entry->filter_spec.next_prot_hdr.icmp.code);

          }
          else
          {
            snprintf( values[FIREWALL_SETTINGS_PAGE_ICMP_CODE_VALID],2,"0");
          }

          if ((next_hdr_prot == PS_IPPROTO_ESP)&&firewall_entry->filter_spec.\
                               next_prot_hdr.esp.field_mask & IPFLTR_MASK_ESP_SPI)
          {

            snprintf( values[FIREWALL_SETTINGS_PAGE_ESPSPI_VALID],
                                              sizeof(valid_flag),"%d",valid_flag);
            snprintf( values[FIREWALL_SETTINGS_PAGE_ESPSPI],sizeof(uint32),
                          "%d",firewall_entry->filter_spec.next_prot_hdr.esp.spi);
          }
          else
          {
            snprintf( values[FIREWALL_SETTINGS_PAGE_ESPSPI_VALID],2,"0");
          }

          create_firewall_buf( values, buf);
          if(index == (handle_list_len - 1))
          {
            strncat(buf,"}",1);
          }
          else
          {
            strncat(buf,"},",2);
          }
        }
      }
    }
    strncat(buf,"]",1);
  }
  else
  error_case:
  {
    create_error( values,buf, qmi_error);
    #ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
    #endif
    result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
    LOG_MSG_ERROR(" Sent the response for Dispaly firewall  result is : %d \n",
                                     result, 0, 0);
    return FALSE;
  }
  snprintf(tmp, strlen(RES_SUCCESS), RES_SUCCESS);
  strncat(buf,tmp,strlen(tmp));
  memset(tmp,0,MAX_BUF_LEN);
  strncat(buf,"}",1);
  LOG_MSG_ERROR(" Firewall entries retrived  ", 0,0,0);
  #ifdef DEBUG_PRINT
  printf("Response message: %s\n",buf);
  #endif
  result = send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  LOG_MSG_ERROR(" Sent the response for DisplayFirewall  result is : %d \n",
                                                                  result, 0, 0);
  return TRUE;
}
/*===========================================================================
FUNCTION AddFirewall()
===========================================================================*/
/*!
@brief
  Adds Firewall Entries

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
int AddFirewall
(
char values[][MAX_ELEMENT_LENGTH],
char* buf,
struct sockaddr_un their_addr,
socklen_t addr_len
)
{
  int result, mask_flag, val ,ip;
  qmi_error=QMI_ERR_NONE_V01;
  mask_flag = atoi(values[FIREWALL_SETTINGS_PAGE_MASK]);
  ip = atoi(values[FIREWALL_SETTINGS_PAGE_IP_FAMILY]);
  qcmap_msgr_firewall_conf_t     extd_firewall_add;

  memset(&extd_firewall_add, 0, sizeof(qcmap_msgr_firewall_conf_t));
  extd_firewall_add.extd_firewall_entry.filter_spec.ip_vsn =
                                          ip_version_enum_type (ip);

  if(extd_firewall_add.extd_firewall_entry.filter_spec.ip_vsn == IP_V4)
  {
    // ADD IPV4 Firewall
    if(atoi(values[FIREWALL_SETTINGS_PAGE_IPV4_SRC_VALID]) == 1)
    {
      if(inet_pton (AF_INET, values[FIREWALL_SETTINGS_PAGE_IPV4_SRC_ADDR],
               &extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.\
                                                  v4.src.addr.ps_s_addr) <= 0)
      {
        LOG_MSG_ERROR("Address not in proper format ", 0, 0, 0);
        qmi_error = QMI_ERR_INVALID_DATA_FORMAT_V01;
        goto error_case;
      }
      extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask |=
                                                       IPFLTR_MASK_IP4_SRC_ADDR;
      if(inet_pton (AF_INET, values[FIREWALL_SETTINGS_PAGE_IPV4_SRC_SUBNET],
                    &extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.\
                                             v4.src.subnet_mask.ps_s_addr) <= 0)
      {
        LOG_MSG_ERROR("Address not in proper format ", 0, 0, 0);
        qmi_error = QMI_ERR_INVALID_DATA_FORMAT_V01;
        goto error_case;
      }
    }
    if(atoi(values[FIREWALL_SETTINGS_PAGE_IPV4_TOS_VALID]) == 1)
    {
      extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v4.tos.val =
                  (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_IPV4_TOS_VALUE]) ;
      extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v4.tos.mask =
                   (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_IPV4_TOS_MASK]) ;
    }
  }
  else
  {
    // ADD IPV6 Firewall
    if(atoi(values[FIREWALL_SETTINGS_PAGE_IPV6_SRC_VALID]) == 1)
    {
      if(unencode_string( values[FIREWALL_SETTINGS_PAGE_IPV6_SRC_ADDR],
      strlen(values[FIREWALL_SETTINGS_PAGE_IPV6_SRC_ADDR]) ) == QCMAP_CM_ERROR)
      {
        #ifdef DEBUG_PRINT
          printf("unencoded string is : %s \n",
                                values[FIREWALL_SETTINGS_PAGE_IPV6_SRC_ADDR]);
        #endif
        LOG_MSG_ERROR("unencode of string failed!! ", 0, 0, 0);
      }
      else
      {
        #ifdef DEBUG_PRINT
          printf("unencoded string is : %s \n",
                               values[FIREWALL_SETTINGS_PAGE_IPV6_SRC_ADDR]);
        #endif
      }
      if(inet_pton (AF_INET6,values[FIREWALL_SETTINGS_PAGE_IPV6_SRC_ADDR],
          &extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.src.\
                                                    addr.in6_u.u6_addr8) <= 0)
      LOG_MSG_ERROR("Address not in proper format ", 0, 0, 0);
      extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask |=
                                                       IPFLTR_MASK_IP6_SRC_ADDR;
      extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.src.prefix_len
         = (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_IPV6_SRC_PREFIX_LENGTH]);
    }

	if(atoi(values[FIREWALL_SETTINGS_PAGE_IPV6_CLASS_VALID]) == 1)
    {
      extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.val =
                        (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_IPV6_CLASS]);
      extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask |=
                                                  IPFLTR_MASK_IP6_TRAFFIC_CLASS;
      extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.mask =
                         (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_IPV6_MASK]);
    }
  }
  // Protocol set in Firewall
  if(atoi(values[ FIREWALL_SETTINGS_PAGE_NEXT_HEADER_PROTO_VALID]) == 1)
  {
    val = (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_NEXT_HEADER_PROTO]);
    if( extd_firewall_add.extd_firewall_entry.filter_spec.ip_vsn == IP_V4 )
    {
      extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot
                                                                         = val;
      extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask |=
                                                 IPFLTR_MASK_IP4_NEXT_HDR_PROT;
    }
    else if(extd_firewall_add.extd_firewall_entry.filter_spec.ip_vsn == IP_V6 )
    {
      extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.next_hdr_prot
                                                                          = val;
      extd_firewall_add.extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask |=
                                                  IPFLTR_MASK_IP6_NEXT_HDR_PROT;
    }
    if(val == PS_IPPROTO_TCP)
    {
      if(atoi(values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_VALID]) == 1)
      {
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.\
         src.port =(uint32)atoi(values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_PORT]);
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.\
                                        field_mask |= IPFLTR_MASK_TCP_SRC_PORT;
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.\
        src.range =
           (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_PORT_RANGE]);
      }
      if(atoi(values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_VALID]) == 1)
      {
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.\
        dst.port =
                (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_PORT]);
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.\
                                      field_mask |= IPFLTR_MASK_TCP_DST_PORT;
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.\
        dst.range =
           (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_PORT_RANGE]);
      }
    }
    else if( val == PS_IPPROTO_UDP)
    {
      if(atoi(values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_VALID]) == 1)
      {
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.udp.\
         src.port = (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_PORT]);
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.udp.\
                                           field_mask |= IPFLTR_MASK_UDP_SRC_PORT;
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.udp.\
        src.range =
              (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_PORT_RANGE]);
      }
      if(atoi(values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_VALID]) == 1)
      {
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.udp.\
           dst.port = (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_PORT]);
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.udp.\
                                             field_mask |= IPFLTR_MASK_UDP_DST_PORT;
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.udp.\
        dst.range =
            (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_PORT_RANGE]);
      }
    }
    else if (val == PS_IPPROTO_TCP_UDP)
    {
      if(atoi(values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_VALID]) == 1)
      {
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.\
        tcp_udp_port_range.src.port =
            (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_PORT]);
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.\
           tcp_udp_port_range.field_mask |= IPFLTR_MASK_TCP_UDP_SRC_PORT;
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.\
        tcp_udp_port_range.src.range =
            (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_TCPUDP_SRC_PORT_RANGE]);
      }
      if(atoi(values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_VALID]) == 1)
      {
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.\
        tcp_udp_port_range.dst.port =
            (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_PORT]);
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.\
           tcp_udp_port_range.field_mask |= IPFLTR_MASK_TCP_UDP_DST_PORT;
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.\
        tcp_udp_port_range.dst.range =
            (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_TCPUDP_DST_PORT_RANGE]);
      }
    }
    else if ( val == PS_IPPROTO_ICMP || val == PS_IPPROTO_ICMP6)
    {
      if(atoi(values[FIREWALL_SETTINGS_PAGE_ICMP_TYPE_VALID]) == 1)
      {
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.\
        icmp.type = (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_ICMP_TYPE]);
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.\
                            icmp.field_mask |= IPFLTR_MASK_ICMP_MSG_TYPE;
      }
      if(atoi(values[FIREWALL_SETTINGS_PAGE_ICMP_CODE_VALID]) == 1)
      {
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.\
        icmp.code = (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_ICMP_CODE]);
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.\
                             icmp.field_mask |= IPFLTR_MASK_ICMP_MSG_CODE;
      }
    }
    else if( val == PS_IPPROTO_ESP)
    {
      if(atoi(values[FIREWALL_SETTINGS_PAGE_ESPSPI_VALID]) == 1)
      {
        extd_firewall_add.extd_firewall_entry.filter_spec.next_prot_hdr.\
             esp.spi = (uint32)atoi(values[FIREWALL_SETTINGS_PAGE_ESPSPI]);
        extd_firewall_add.extd_firewall_entry.filter_spec.\
                       next_prot_hdr.esp.field_mask |= IPFLTR_MASK_ESP_SPI;
      }
    }
  }

  if ( QcMapClient->AddFireWallEntry(&extd_firewall_add,&qmi_error ) == true)
  {
    return TRUE;
  }
  if (qmi_error == QMI_ERR_INTERFACE_NOT_FOUND_V01)
  {
    LOG_MSG_ERROR("Backhaul down, Firewall Entry added error number : %d ",
                   qmi_error, 0, 0);
    //now set qmi_error to none to force success return to webserver.
    qmi_error = QMI_ERR_NONE_V01;
    return TRUE;
  }
  LOG_MSG_ERROR("Add Firewall entries failed!!qmi error number : %d ",
                                                            qmi_error, 0, 0);

  error_case:
  create_error( values,buf, qmi_error);
  #ifdef DEBUG_PRINT
  printf("Response message: %s\n",buf);
  #endif
  result = send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  LOG_MSG_ERROR(" Sent the response for Dispaly firewall  result is : %d \n",
                                   result, 0, 0);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response  for AddFirewall result is failure: %d \n",
        result,0,0);
  }
  return FALSE;
}

/*===========================================================================
FUNCTION DeleteFirewall()
===========================================================================*/
/*!
@brief
  Delete Firewall entries.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
int DeleteFirewall(
char values[][MAX_ELEMENT_LENGTH],
char* buf,
struct sockaddr_un their_addr,
socklen_t addr_len)
{

  int result, mask_flag, val;
  qmi_error = QMI_ERR_NONE_V01;
  mask_flag = atoi(values[FIREWALL_SETTINGS_PAGE_MASK]);
  val = atoi(values[FIREWALL_SETTINGS_PAGE_FIREWALL_HANDLE]);
  if ( (QcMapClient->DeleteFireWallEntry(val, &qmi_error)< 0 ) &&
       (qmi_error != QMI_ERR_INTERFACE_NOT_FOUND_V01) )
  {
    LOG_MSG_ERROR("\nDelete firewall Fails, Error: 0x%x", qmi_error, 0, 0);

    create_error( values,buf, qmi_error);
    #ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
    #endif
    result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
    if(result == QCMAP_CM_ERROR)
    {
      LOG_MSG_ERROR(" Sent the response  for  Delete firewall result is failure: %d \n",
          result,0,0);
    }

    return FALSE;
  }
  else
  {
    if (qmi_error == QMI_ERR_INTERFACE_NOT_FOUND_V01)
    {
      LOG_MSG_INFO1("Backhaul down, FIREWALL Entry Deleted error number : %d ",
                     qmi_error, 0, 0);
      //now set qmi_error to none to force success return to webserver.
      qmi_error = QMI_ERR_NONE_V01;
    }
    if ( ( (mask_flag & MODIFY)  == MODIFY) )
    return TRUE;

    create_error( values,buf, qmi_error);

    #ifdef DEBUG_PRINT
      printf("Response message: %s\n",buf);
    #endif
    result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
    if(result == QCMAP_CM_ERROR)
    {
      LOG_MSG_ERROR(" Sent the response  for Delete firewall result is failure: %d \n",
          result,0,0);
    }
    return TRUE;
  }
}
/*===========================================================================
FUNCTION ModifyFirewall()

===========================================================================*/
/*!
@brief
  Modifies firewall.
@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
int ModifyFirewall(
char values[][MAX_ELEMENT_LENGTH],
char* buf,struct sockaddr_un their_addr,
socklen_t addr_len)
{

  int result;
  qmi_error=QMI_ERR_NONE_V01;
  qcmap_msgr_firewall_conf_t extd_firewall_get;
  memset(&extd_firewall_get, 0, sizeof(qcmap_msgr_firewall_conf_t));
  extd_firewall_get.extd_firewall_entry.filter_spec.ip_vsn =
       (ip_version_enum_type)(atoi(values[FIREWALL_SETTINGS_PAGE_IP_FAMILY]));
  extd_firewall_get.extd_firewall_entry.firewall_handle =
                         atoi(values[FIREWALL_SETTINGS_PAGE_FIREWALL_HANDLE]);
  QcMapClient->GetFireWallEntry(&extd_firewall_get.extd_firewall_entry, &qmi_error);

  if (DeleteFirewall( values,buf,their_addr,addr_len ) == TRUE)
  {
    if( AddFirewall( values,buf,their_addr,addr_len ) == TRUE)
    {
      DisplayFirewall( values,buf,their_addr,addr_len);
      #ifdef DEBUG_PRINT
        printf("Response message: %s\n",buf);
      #endif
      return TRUE;
    }
    else
    {
      if (!QcMapClient->AddFireWallEntry(&extd_firewall_get,&qmi_error) &&
          qmi_error != QMI_ERR_INTERFACE_NOT_FOUND_V01 )
      {
        LOG_MSG_ERROR(" Re-Addition of Firewall Entry Fails: %d \n",
                      qmi_error, 0, 0);
      }
      {
        qmi_error=QMI_ERR_NONE_V01;
        return TRUE;
      }
    }
  }
  else
  {
    LOG_MSG_ERROR(" Delete firewall fails: cannot modify  \n", 0,0,0);
  }
  return FALSE;
}
/*===========================================================================
FUNCTION EnableFirewall()
 ===========================================================================*/
 /*!
 @brief
   Enable Firewall.

 @input
   values     - cgi form field values.
   buf        - message to be sent to web cgi.
   their_addr  - socket address to which data needs to sent.
   addr_len    - lenght of their_addr.

 @return
   0  - success
   -1 - failure

 @dependencies
   None

 @sideefects
   None
 */
 /*=========================================================================*/
int EnableFirewall(
char values[][MAX_ELEMENT_LENGTH],
char* buf,struct sockaddr_un their_addr,
socklen_t addr_len)
{
  int result;
  boolean enable_firewall, pkts_allowed;
  qmi_error=QMI_ERR_NONE_V01;

  enable_firewall = atoi(values[FIREWALL_SETTINGS_PAGE_FIREWALL_STATE]);

  pkts_allowed = atoi(values[FIREWALL_SETTINGS_PAGE_PACKETS_ALLOWED]);

  if (QcMapClient->SetFirewall(enable_firewall, pkts_allowed, &qmi_error));
  create_error( values,buf, qmi_error);


  #ifdef DEBUG_PRINT
  printf("Response message: %s\n",buf);
  #endif

  result = send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response  for Enable firewall result is failure: %d \n",
      result,0,0);
  }
  return TRUE;
}
/*===========================================================================
 FUNCTION SetFirewall()
 ===========================================================================*/
 /*!
 @brief
   Sets Firewall based on mask flag.

 @input
   values    - cgi form field values.
   buf       - message to be sent to web cgi.
   their_addr  - socket address to which data needs to sent.
   addr_len    - lenght of their_addr.

 @return
   0  - success
   -1 - failure

 @dependencies
   None

 @sideefects
   None
 */
 /*=========================================================================*/
int SetFirewall
(
char values[][MAX_ELEMENT_LENGTH],
char* buf,
struct sockaddr_un their_addr,
socklen_t addr_len
)
{
  int mask_flag=0;
  int result;
  //found the SetFirewall page!!
  mask_flag=atoi(values[FIREWALL_SETTINGS_PAGE_MASK]);
  LOG_MSG_ERROR(" Mask is :  %d \n",mask_flag,0,0);
  qmi_error=QMI_ERR_NONE_V01;
  /* ADD FIREWALL */
  if (   (mask_flag   == ADD) )
  {
    LOG_MSG_ERROR(" Add Firewall Entry \n", 0, 0,0);
    if( AddFirewall(values,buf,their_addr,addr_len) == TRUE)
    {
      DisplayFirewall( values,buf,their_addr,addr_len);
      printf("Response message: %s\n",buf);
    }
  }
  /* DELETE FIREWALL */
  else if (  mask_flag  == DELETE )
  {
    LOG_MSG_ERROR(" Delete Firewall Entry \n", 0, 0,0);
    DeleteFirewall( values,buf,their_addr,addr_len );
  }
  /* MODIFY FIREWALL */
  else if ( mask_flag  == MODIFY )
  {
    LOG_MSG_ERROR(" Modify Firewall Entry \n", 0, 0,0);
    ModifyFirewall( values,buf,their_addr,addr_len );
  }
  /* ENABLE FIREWALL */
  else if (  mask_flag  == ENABLE )
  {
    LOG_MSG_ERROR(" Enable Firewall Entry \n", 0, 0,0);
    EnableFirewall( values,buf,their_addr,addr_len );
  }
}
/*===========================================================================
 FUNCTION SetIPv6Wwan_State()
 ===========================================================================*/
 /*!
 @brief
   Sets IPV6 wwan configuration.

 @input
   values      - cgi form field values.
   buf         - message to be sent to web cgi.
   their_addr  - socket address to which data needs to sent.
   addr_len    - lenght of their_addr.

 @return
   0  - success
   -1 - failure

 @dependencies
   None

 @sideefects
   None
 */
 /*=========================================================================*/

static int SetIPv6Wwan_State
(
char values[][MAX_ELEMENT_LENGTH],
char* buf,
struct sockaddr_un their_addr,
socklen_t addr_len
)
{
  int mask_flag = 0, result, flag;
  char tmp[MAX_BUF_LEN] ={0};
  //found the IPV6 WWAN page!!
  mask_flag = atoi(values[IPV6_WWAN_SETTINGS_PAGE_MASK]);
  LOG_MSG_ERROR(" Mask is :  %d \n",mask_flag,0,0);
  qmi_error = QMI_ERR_NONE_V01;

  flag = atoi(values[IPV6_WWAN_SETTINGS_PAGE_ENABLE]);
  if(  ( (mask_flag & FIELD_MASK_1)  == FIELD_MASK_1) )
  {
    if (flag)
    {
      QcMapClient->EnableIPV6(&qmi_error);
      snprintf(values[IPV6_WWAN_SETTINGS_PAGE_ENABLE_RESULT],
               strlen(QMI_Error_String[qmi_error])+1,"%s",
               QMI_Error_String[qmi_error]) ;
    }
    else
    {
      QcMapClient->DisableIPV6(&qmi_error);
      snprintf(values[IPV6_WWAN_SETTINGS_PAGE_ENABLE_RESULT],
               strlen(QMI_Error_String[qmi_error])+1,"%s",
               QMI_Error_String[qmi_error]) ;
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_ERROR(" Enable:  %d and mask flag : %d \n", flag , mask_flag , 0);
    snprintf(values[IPV6_WWAN_SETTINGS_PAGE_ENABLE],sizeof(flag),"%d",flag) ;
    snprintf(values[IPV6_WWAN_SETTINGS_PAGE_ENABLE_RESULT],
             strlen(QMI_Error_String[qmi_error])+1,"%s",
             QMI_Error_String[qmi_error]) ;
  }

  qmi_error = QMI_ERR_NONE_V01;
  flag = atoi(values[IPV6_WWAN_SETTINGS_PAGE_BACKHAUL]);
  if(  ( (mask_flag & FIELD_MASK_2)  == FIELD_MASK_2) )
  {
    if (flag)
    {

      if(QcMapClient->ConnectBackHaul(QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01, &qmi_error) )
      {
        snprintf(values[IPV6_WWAN_SETTINGS_PAGE_BACKHAUL],strlen(CONNECTED)+1,"%s",
                CONNECTED) ;
      }
      snprintf(values[IPV6_WWAN_SETTINGS_PAGE_BACKHAUL_RESULT],
               strlen(QMI_Error_String[qmi_error])+1,"%s",
               QMI_Error_String[qmi_error]);

    }
    else
    {
      if(QcMapClient->DisconnectBackHaul(QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01,
                                      &qmi_error) )
      {
        snprintf(values[IPV6_WWAN_SETTINGS_PAGE_BACKHAUL],strlen(DISCONNECTED)+1,"%s",
                DISCONNECTED) ;
      }
      snprintf(values[IPV6_WWAN_SETTINGS_PAGE_BACKHAUL_RESULT],
               strlen(QMI_Error_String[qmi_error])+1,"%s",
               QMI_Error_String[qmi_error]) ;
    }
  }
  else
  {
    //no change required, so just print information
    LOG_MSG_ERROR(" ipv6 backhaul:   %d and mask flag: %d\n", flag , mask_flag , 0);
    snprintf(values[IPV6_WWAN_SETTINGS_PAGE_BACKHAUL],sizeof(flag),"%d",flag) ;
    snprintf(values[IPV6_WWAN_SETTINGS_PAGE_BACKHAUL_RESULT],
             strlen(QMI_Error_String[qmi_error])+1,"%s",
             QMI_Error_String[qmi_error]) ;

  }
  int set_wwan_policy = -1;
  qmi_error = QMI_ERR_NONE_V01;
  qcmap_msgr_net_policy_info_v01 WWAN_Profile_Data;
  if(  ( (mask_flag & FIELD_MASK_3)  == FIELD_MASK_3) )
  {
    memset(&WWAN_Profile_Data,0,sizeof(qcmap_msgr_net_policy_info_v01));
    memset(&qmi_error,0,sizeof(qmi_error_type_v01));
    WWAN_Profile_Data.ip_family = QCMAP_MSGR_IP_FAMILY_V6_V01 ;
    if(atoi(values[IPV6_WWAN_SETTINGS_PAGE_TECH_PREF]) == 1)
    WWAN_Profile_Data.tech_pref = QCMAP_MSGR_MASK_TECH_PREF_3GPP_V01;
    else if(atoi(values[IPV6_WWAN_SETTINGS_PAGE_TECH_PREF]) == 2)
    WWAN_Profile_Data.tech_pref = QCMAP_MSGR_MASK_TECH_PREF_3GPP2_V01;
    WWAN_Profile_Data.v6_profile_id_3gpp =
                 (uint8_t)atoi(values[IPV6_WWAN_SETTINGS_PAGE_PROFILE_ID_GPP]);

    WWAN_Profile_Data.v6_profile_id_3gpp2 =
               (uint8_t)atoi(values[IPV6_WWAN_SETTINGS_PAGE_PROFILE_ID_GPP2]);
    if (QcMapClient->SetWWANPolicy(WWAN_Profile_Data, &qmi_error))
    {
      set_wwan_policy=0;
      snprintf(values[IPV6_WWAN_SETTINGS_PAGE_POLICY_RESULT],
        strlen(QMI_Error_String[qmi_error])+1,"%s",QMI_Error_String[qmi_error]) ;
    }
    else
    {
      LOG_MSG_ERROR("Failed to Set WWAN policy .error %d.\n ", qmi_error, 0, 0);
      set_wwan_policy = -1;
      snprintf(values[IPV6_WWAN_SETTINGS_PAGE_POLICY_RESULT],
               strlen(QMI_Error_String[qmi_error])+1,"%s",
               QMI_Error_String[qmi_error]) ;

    }
  }
  else

  {
    snprintf(values[IPV6_WWAN_SETTINGS_PAGE_POLICY_RESULT],
             strlen(QMI_Error_String[qmi_error])+1,"%s",
             QMI_Error_String[qmi_error]);
  }

  create_response_buf(values,  buf, IPV6_WWAN_SETTINGS_PAGE_TECH_PREF,
                      WWAN_v6_string,0,0);
  #ifdef DEBUG_PRINT
  printf("Response message: %s\n",buf);
  #endif
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  LOG_MSG_ERROR(" Sent the response for SetWWan IPV6  result is : %d \n",
                result,0,0);

}
/*===========================================================================
FUNCTION GetIPv6Wwan_State()
===========================================================================*/
/*!
@brief
  Retrieves IPV6 wwan configuration.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return
  0  - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/

static int GetIPv6Wwan_State
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{

  boolean flag=0;
  int result;
  char tmp[MAX_BUF_LEN] ={0};
  qmi_error=QMI_ERR_NONE_V01;

  qcmap_msgr_net_policy_info_v01 wwan_policy;


  memset(&wwan_policy,0,sizeof(qcmap_msgr_net_policy_info_v01));

  qcmap_msgr_wwan_status_enum_v01 v4status;
  qcmap_msgr_wwan_status_enum_v01 v6status;

  if(!QcMapClient->GetIPv6State( &flag, &qmi_error))
    LOG_MSG_ERROR(" get IPV6 state Fails:  \n", 0, 0, 0);
  snprintf(values[IPV6_GET_STATE_RESULT],
  strlen(QMI_Error_String[qmi_error])+1,"%s",QMI_Error_String[qmi_error]) ;
  snprintf(values[IPV6_GET_STATE],sizeof(int),"%d",(int)flag) ;

  qmi_error = QMI_ERR_NONE_V01;
  if(!QcMapClient->GetWWANStatus(&v4status,&v6status,&qmi_error))
    LOG_MSG_ERROR(" get WWAN status Fail:  \n", 0, 0, 0);

  snprintf(values[IPV6_GET_WWAN_STATUS_ERROR],
           strlen(QMI_Error_String[qmi_error])+1,"%s",
           QMI_Error_String[qmi_error]) ;
  snprintf(values[IPV6_GET_WWAN_STATUS],
           strlen(WWAN_Status_String[v6status])+1,"%s",
           WWAN_Status_String[v6status]) ;

  qmi_error=QMI_ERR_NONE_V01;
  if(!QcMapClient->GetWWANPolicy(&wwan_policy, &qmi_error))
    LOG_MSG_ERROR(" get WWAN Policy Fails:  \n", 0, 0, 0);
  if (wwan_policy.ip_family != IPV6 && wwan_policy.ip_family != IPV4V6)
  {
   // calling in IPV6 page, but wwan is not IPV6 family so return error
    qmi_error = QMI_ERR_INVALID_IP_FAMILY_PREF_V01 -1;
  }
  snprintf(values[IPV6_GET_WWAN_POLICY_ERROR],
  strlen(QMI_Error_String[qmi_error])+1,"%s",QMI_Error_String[qmi_error]);

  switch (wwan_policy.tech_pref)
    {
      case 0:
        snprintf(values[IPV6_GET_WWAN_POLICY_TECH],
          2,"0") ;
        break;

      case 1:
        snprintf(values[IPV6_GET_WWAN_POLICY_TECH],
          2,"1") ;
        break;

      case 2:
        snprintf(values[IPV6_GET_WWAN_POLICY_TECH],
          2,"2") ;
        break;

      default:
        printf("Error: invalid tech preference:- %d \n",wwan_policy.tech_pref);
        snprintf(values[IPV6_GET_WWAN_POLICY_TECH],
          2,"3") ;
        break;
    }

  snprintf(values[IPV6_GET_WWAN_POLICY_3GPP],
  sizeof(int),"%d",(int)wwan_policy.v6_profile_id_3gpp) ;
  snprintf(values[IPV6_GET_WWAN_POLICY_3GPP2],
  sizeof(int),"%d",(int)wwan_policy.v6_profile_id_3gpp2);

  create_response_buf(values,  buf, IPV6_GET_WWAN_POLICY_MAX,
                                                        ipv6_state_string,0,0);
  #ifdef DEBUG_PRINT
  printf("Response message: %s\n",buf);
  #endif

  result = send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  LOG_MSG_ERROR(" Sent the response for GetIpv6 wwan state result is : %d \n",
                                                                  result,0,0);
}
/*===========================================================================
  FUNCTION SetNotify_Interval
===========================================================================*/
/*!
@brief
  Configure UPNP, DLNA notify interval.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int SetNotify_Interval
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  uint32_t mask_flag=0;
  uint32_t result;
  int upnp_notify_int;
  qcmap_msgr_upnp_mode_enum_v01 upnp_state;
  int dlna_notify_int;
  qcmap_msgr_dlna_mode_enum_v01 dlna_state;
  int temp_size=0;
  mask_flag=atoi(values[UPNP_DLNA_NOTIFY_PAGE_MASK]);
  qmi_error=QMI_ERR_NONE_V01;
  upnp_notify_int=atoi(values[UPNP_DLNA_NOTIFY_PAGE_UPNP]);
  dlna_notify_int=atoi(values[UPNP_DLNA_NOTIFY_PAGE_DLNA]);
  /*check whether we really need to change UPNP Notify interval */
  if( ( (mask_flag & FIELD_MASK_1)  == FIELD_MASK_1) )
  {
    if(QcMapClient->SetUPNPNotifyInterval(upnp_notify_int, &qmi_error))
    {
      temp_size = log10(upnp_notify_int)+ PADDING;
      if(QcMapClient->GetUPNPStatus( &upnp_state, &qmi_error))
      {
        if (upnp_state == QCMAP_MSGR_UPNP_MODE_UP_V01)
        {
          if (QcMapClient->DisableUPNP(&qmi_error))
          {
            if (QcMapClient->EnableUPNP(&qmi_error))
            {
              snprintf(values[UPNP_DLNA_NOTIFY_PAGE_UPNP],temp_size,"%d",
                       upnp_notify_int) ;
            }
            else
            {
              LOG_MSG_ERROR("Set UPNP notify interval failed restart!error:%d",
                             qmi_error, 0, 0);
            }
          }
          else
          {
            LOG_MSG_ERROR("Set UPNP notify interval failed disable!error:%d",
                           qmi_error, 0, 0);
          }
        }
        else
        {
          snprintf(values[UPNP_DLNA_NOTIFY_PAGE_UPNP],temp_size,"%d",
                   upnp_notify_int) ;
        }
      }
      else
      {
        LOG_MSG_ERROR("get UPNP status failed:%d",
                       qmi_error, 0, 0);
      }
      snprintf(values[UPNP_DLNA_NOTIFY_PAGE_UPNP_RESULT],
               strlen(QMI_Error_String[qmi_error])+1,"%s",
               QMI_Error_String[qmi_error]);
    }
    else
    {
      snprintf(values[UPNP_DLNA_NOTIFY_PAGE_UPNP_RESULT],
      strlen(QMI_Error_String[qmi_error])+1, "%s",QMI_Error_String[qmi_error]);
      LOG_MSG_ERROR("Set UPNP notify interval failed!! qmi error number:%d",
                     qmi_error, 0, 0);
    }
  }

  else
  {

    snprintf(values[UPNP_DLNA_NOTIFY_PAGE_UPNP_RESULT],
             strlen(QMI_Error_String[qmi_error])+1,"%s",
             QMI_Error_String[qmi_error]);
  }
  qmi_error=QMI_ERR_NONE_V01;
  /*check whether we really need to change DLNA notify interval */
  if( ( (mask_flag & FIELD_MASK_2)  == FIELD_MASK_2) )
  {
    if(QcMapClient->SetDLNANotifyInterval(dlna_notify_int, &qmi_error))
    {
      temp_size = log10(dlna_notify_int)+ PADDING;
      if(QcMapClient->GetDLNAStatus( &dlna_state, &qmi_error))
      {
        if (dlna_state == QCMAP_MSGR_DLNA_MODE_UP_V01)
        {
          if (QcMapClient->DisableDLNA(&qmi_error))
          {
            if (QcMapClient->EnableDLNA(&qmi_error))
            {
              snprintf(values[UPNP_DLNA_NOTIFY_PAGE_DLNA],temp_size,"%d",
                       dlna_notify_int) ;
            }
            else
            {
              LOG_MSG_ERROR("Set DLNA notify interval failed restart!error:%d",
                             qmi_error, 0, 0);
            }
          }
          else
          {
            LOG_MSG_ERROR("Set DLNA notify interval failed disable!error:%d",
                           qmi_error, 0, 0);
          }
        }
        else
        {
          snprintf(values[UPNP_DLNA_NOTIFY_PAGE_DLNA],temp_size,"%d",
                   dlna_notify_int) ;
        }
      }
      else
      {
        LOG_MSG_ERROR("get DLNA status failed:%d",
                       qmi_error, 0, 0);
      }
      snprintf(values[UPNP_DLNA_NOTIFY_PAGE_DLNA_RESULT],
               strlen(QMI_Error_String[qmi_error])+1,"%s",
               QMI_Error_String[qmi_error]);
    }
    else
    {
      snprintf(values[UPNP_DLNA_NOTIFY_PAGE_DLNA_RESULT],
               strlen(QMI_Error_String[qmi_error])+1,
               "%s",QMI_Error_String[qmi_error]);
      LOG_MSG_ERROR("Set DLNA notify interval failed!! qmi error number:%d",
                     qmi_error, 0, 0);
    }
  }

  else
  {

    snprintf(values[UPNP_DLNA_NOTIFY_PAGE_DLNA_RESULT],
             strlen(QMI_Error_String[qmi_error])+1,"%s",
             QMI_Error_String[qmi_error]);
  }
  create_response_buf( values, buf, UPNP_DLNA_NOTIFY_PAGE_MAX,
                       upnp_dlna_notify_str, UPNP_DLNA_NOTIFY_PAGE_PAGE,0);
#ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
#endif
  /*send response back to CGI */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response SetUPNP_DLNA notify interval result is \
                    failure: %d \n",
        result, 0, 0);
  }
}
/*===========================================================================
  FUNCTION GetNotify_Interval
===========================================================================*/
/*!
@brief
  UPNP, DLNA notify interval are retrieved.

@input
  values      - cgi form field values.
  buf         - message to be sent to web cgi.
  their_addr  - socket address to which data needs to sent.
  addr_len    - lenght of their_addr.

@return

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int GetNotify_Interval
(
  char values[][MAX_ELEMENT_LENGTH],
  char* buf,
  struct sockaddr_un their_addr,
  socklen_t addr_len
)
{
  uint32_t result;
  int upnp_notify_int =0;
  int dlna_notify_int =0;
  int temp_size =0;
  qmi_error=QMI_ERR_NONE_V01;

  if(QcMapClient->GetUPNPNotifyInterval(&upnp_notify_int, &qmi_error))
  {
    temp_size = log10(upnp_notify_int)+ PADDING;
    snprintf(values[UPNP_DLNA_NOTIFY_PAGE_UPNP],temp_size,"%d",
             upnp_notify_int) ;
    snprintf(values[UPNP_DLNA_NOTIFY_PAGE_UPNP_RESULT],
             strlen(QMI_Error_String[qmi_error])+1,"%s",
             QMI_Error_String[qmi_error]);
  }
  else
  {
    snprintf(values[UPNP_DLNA_NOTIFY_PAGE_UPNP_RESULT],
    strlen(QMI_Error_String[qmi_error])+1, "%s",QMI_Error_String[qmi_error]);
    LOG_MSG_ERROR("Get UPNP notify interval failed!! qmi error number:%d",
                   qmi_error, 0, 0);
  }
  if(QcMapClient->GetDLNANotifyInterval(&dlna_notify_int, &qmi_error))
  {
    temp_size = log10(dlna_notify_int)+ PADDING;
    snprintf(values[UPNP_DLNA_NOTIFY_PAGE_DLNA],temp_size,"%d",
             dlna_notify_int) ;
    snprintf(values[UPNP_DLNA_NOTIFY_PAGE_DLNA_RESULT],
             strlen(QMI_Error_String[qmi_error])+1,"%s",
             QMI_Error_String[qmi_error]);
  }
  else
  {
    snprintf(values[UPNP_DLNA_NOTIFY_PAGE_DLNA_RESULT],
    strlen(QMI_Error_String[qmi_error])+1, "%s",QMI_Error_String[qmi_error]);
    LOG_MSG_ERROR("Get DLNA notify interval failed!! qmi error number:%d",
                   qmi_error, 0, 0);
  }
  create_response_buf( values, buf, UPNP_DLNA_NOTIFY_PAGE_MAX,
                       upnp_dlna_notify_str, UPNP_DLNA_NOTIFY_PAGE_PAGE,0);
#ifdef DEBUG_PRINT
    printf("Response message: %s\n",buf);
#endif
  /*send response back to CGI */
  result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
  if(result == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Sent the response GetUPNP_DLNA notify interval\
                   result is failure: %d \n",
        result, 0, 0);
  }
}

/*===========================================================================
  FUNCTION sigHandler
===========================================================================*/
/*!
@brief
  signal handler.

@input
  signal - signal to be handled.

@return
  None

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static void sigHandler(int signal)
{
  qmi_error_type_v01 err_num=0;
  switch (signal)
  {
    case SIGTERM:
      if (QcMapClient)
      {
        if (!QcMapClient->DisableMobileAP(&qmi_error))
          LOG_MSG_ERROR("MobileAP Disable failed qmi err:%d", qmi_error, 0, 0);
        /* Server socket close */
        close(qcmap_webclient_sockfd);
        exit(0);
      }
      break;
    default:
      LOG_MSG_ERROR("Received unexpected signal %d\n", signal, 0, 0);
      break;
  }
}
/*===========================================================================
  FUNCTION create_socket_web
===========================================================================*/
/*!
@brief
  Create unix socket.

@input
  sockfd - sockfd file descriptor. this will be value returned from
           socket() call.

@return
   0 - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int create_socket_web(unsigned int *sockfd)
{
  if ((*sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR("Error creating socket, errno: %d", errno, 0, 0);
    return QCMAP_CM_ERROR;
  }
  if(fcntl(*sockfd, F_SETFD, FD_CLOEXEC) < 0)
  {
    LOG_MSG_ERROR("Couldn't set Close on Exec, errno: %d", errno, 0, 0);
    close(*sockfd);
    return QCMAP_CM_ERROR;
  }
  return QCMAP_CM_SUCCESS;
}
/*===========================================================================
  FUNCTION create_qcmap_webclient_socket
===========================================================================*/
/*!
@brief
  Create web client server socket and set fcntl and socket options.

@input
  None

@return
   0 - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int create_qcmap_webclient_socket()
{
  int val, rval;
  struct sockaddr_un qcmap_webclient_addr;
  int len;
  struct timeval rcv_timeo;
  rval = create_socket_web(&qcmap_webclient_sockfd);
  if(rval == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR("Error creating the socket!!:", 0, 0, 0);
    close(qcmap_webclient_sockfd);
    return QCMAP_CM_ERROR;
  }
  rcv_timeo.tv_sec = 0;
  rcv_timeo.tv_usec = 100000;
  if(setsockopt(qcmap_webclient_sockfd, SOL_SOCKET, SO_RCVTIMEO,
              (void *)&rcv_timeo, sizeof(rcv_timeo)) !=QCMAP_CM_SUCCESS)
  {
    LOG_MSG_ERROR("socket options set failed!! errno %d:", errno, 0, 0);
    close(qcmap_webclient_sockfd);
    return QCMAP_CM_ERROR;
  }
  val = fcntl(qcmap_webclient_sockfd, F_GETFL, 0);
  if(val == QCMAP_CM_ERROR)
  {
    close(qcmap_webclient_sockfd);
    LOG_MSG_ERROR("fcntl of  the socket failed!!%d:", errno, 0, 0);
    return QCMAP_CM_ERROR;
  }
  val = fcntl(qcmap_webclient_sockfd, F_SETFL, val | O_NONBLOCK);
  if(val == QCMAP_CM_ERROR)
  {
    close(qcmap_webclient_sockfd);
    LOG_MSG_ERROR("fcntl of  the socket failed!! %d:", errno, 0, 0);
    return QCMAP_CM_ERROR;
  }
  /* fd_set doesn't return anything!!*/
  FD_ZERO(&initial_sockfd);
  FD_SET(qcmap_webclient_sockfd, &initial_sockfd);
  memset(&qcmap_webclient_addr,0,sizeof(sockaddr_un));

  qcmap_webclient_addr.sun_family = AF_UNIX;
  #ifdef DEBUG_PRINT
  printf("file name: %s",QCMAP_WEBCLIENT_CGI_FILE );
  #endif
  strncpy(qcmap_webclient_addr.sun_path, QCMAP_WEBCLIENT_CGI_FILE,
          strlen(QCMAP_WEBCLIENT_CGI_FILE));
  #ifdef DEBUG_PRINT
  printf("file name: %s",qcmap_webclient_addr.sun_path );
  #endif
  if( (unlink(qcmap_webclient_addr.sun_path) == QCMAP_CM_ERROR )
      && (errno !=ENOENT) )
  {
    LOG_MSG_ERROR("unlink of socket file fail, errno: %d", errno, 0, 0);
    if(errno == ENOENT)
    {
      #ifdef DEBUG_PRINT
      printf("no such file or directory : %d",ENOENT );
      #endif
    }
    close(qcmap_webclient_sockfd);
    return QCMAP_CM_ERROR;
  }
  len = strlen(qcmap_webclient_addr.sun_path) + sizeof(
                                              qcmap_webclient_addr.sun_family);
  if (bind(qcmap_webclient_sockfd, (struct sockaddr *)&qcmap_webclient_addr,
            len) == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR("Error binding the socket, errno: %d", errno, 0, 0);
    close(qcmap_webclient_sockfd);
    return QCMAP_CM_ERROR;
  }
  if ( (system("chmod 777 /www/qcmap_webclient_cgi_file"))  !=QCMAP_CM_SUCCESS)
  {
    LOG_MSG_ERROR("chmod of socket file failed, errno: %d", errno, 0, 0);
    close(qcmap_webclient_sockfd);
    return QCMAP_CM_ERROR;
  }
  return QCMAP_CM_SUCCESS;
}

/*===========================================================================
  FUNCTION qcmap_webclient_msgr_init
===========================================================================*/
/*!
@brief
  Create necessary sockets

@input
  None

@return
   0 - success
  -1 - failure

@dependencies
  None

@sideefects
  None
*/
/*=========================================================================*/
static int qcmap_webclient_msgr_init ( void )
{
  /* Create qcmap -> weclient server socket */
  if ( (create_qcmap_webclient_socket() ) == QCMAP_CM_ERROR)
  {
    return QCMAP_CM_ERROR;
  }
  return QCMAP_CM_SUCCESS;
}
int main(int argc, char **argv)
{
  int test_mode = 0, nbytes=0;
  char buf[MAX_BUF_LEN];
  uint32_t *buf_ptr;
  // To store individual Element Names
  char fields[MAX_ELEMENT_COUNT][MAX_ELEMENT_LENGTH];
  // To Store individual Element values.
  char values[MAX_ELEMENT_COUNT][MAX_ELEMENT_LENGTH];
  //sleep some time for QCMAP Mgr to comeup.
  sleep(5);
  struct sockaddr_un their_addr;
  socklen_t addr_len;
  fd_set master_fd_set;
  size_t len = 0;
  char tmp[MAX_BUF_LEN] ={0};
  int result;
  int ap_status=0;
  int token_pos = 0;
  char token_recv[MAX_ELEMENT_LENGTH] ={0};
  FILE *fp = NULL;  //File pointer token to File
  char token_sesn[MAX_ELEMENT_LENGTH] ={0};
  /* Register the sighandlers, so the app may be shutdown with kill command.*/
  if(signal(SIGTERM, sigHandler) == SIG_ERR)
  {
    LOG_MSG_ERROR("unable to register sig handler errno %d!!", errno, 0, 0);
    exit(0);
  }
  QcMapClient = new QCMAP_Client( NULL );
  if (QcMapClient==NULL)
  {
    LOG_MSG_ERROR("QcMapClient not created exiting!!", 0, 0, 0);
    exit(0);
  }
  if (QcMapClient->qmi_qcmap_msgr_handle == 0)
  {
    LOG_MSG_ERROR("\nCouldn't setup QcMap WEB Client..exiting", 0, 0, 0);
    exit(0);
  }
  /* Initialize qcmap_webcli module */
  if( (qcmap_webclient_msgr_init()) == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR(" Unable to create server socket!!: ", 0, 0, 0);
    /* MobileAP is not enabled, no point in continuing further */
    close(qcmap_webclient_sockfd);
    exit(0);
  }
  /* infinite loop  */
  while(1)
  {
    if (select(qcmap_webclient_sockfd+1, &initial_sockfd, NULL, NULL, NULL) == -1)
    {
      LOG_MSG_ERROR("Error in select, errno:%d", errno, 0, 0);
      /*try next select call might pass */
      continue;
    }
    /* check FD ready for with data */
    if (FD_ISSET(qcmap_webclient_sockfd, &initial_sockfd))
    {
      addr_len = sizeof(struct sockaddr_storage);
      memset(buf,0,MAX_BUF_LEN);
      memset(fields,0,MAX_ELEMENT_COUNT * MAX_ELEMENT_LENGTH );
      memset(values,0,MAX_ELEMENT_COUNT * MAX_ELEMENT_LENGTH );
      /* receive data from unix socket i.e CGI */
      if ( (nbytes = recvfrom(qcmap_webclient_sockfd, buf, (MAX_BUF_LEN-1) ,
           0 , (struct sockaddr *)&their_addr, &addr_len)) <= 0 )
      {
        LOG_MSG_ERROR("recvfrom returned error,errno:%d", errno, 0, 0);
      }
      else
      {
        //less no.of bytes read, not even type,length available
        //  ,so let's ignore the packet!!.
        if(nbytes < len)
        {
          LOG_MSG_ERROR("received less no.of bytes nbytes %d len %d %d \n ",
                            nbytes, (int)len, 0);
        }
        buf[nbytes+1]='\0';
        #ifdef DEBUG_PRINT
        printf("String equivalent are \n ");
        printf(" String is: %s \n",buf);
        #endif
        /* Check and Enable MobileAP so that we are ready to service CGI requests */
        if(!mobile_ap_enable_flag)
        {
          if (!QcMapClient->EnableMobileAP(&qmi_error))
          {
            LOG_MSG_ERROR("MobileAP Enable failed!!qmi error : %d ", qmi_error, 0, 0);
            /* MobileAP is not enabled, no point in continuing further */
            values[0][0] = MOBILEAP_BRINGUP_FAIL;
            create_msg_to_send( values,buf,MOBILEAP_BRINGUP_FAIL);
            result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
            LOG_MSG_ERROR(" Sent the response for page mobileap failed to enable : %d \n",
                            result, 0, 0);
          }
          else
          {
            LOG_MSG_INFO1("Enabled mobile ap and set flag", 0, 0, 0);
            mobile_ap_enable_flag = 1;
          }
        }
        //parse the buffer and store values also get token position.
        Tokenizer(buf, fields, values, &token_pos);
        memset(buf,0,MAX_BUF_LEN);
        //copy token recieved from HTML page and read from file
        memset(token_recv, 0, Token_size);
        strncpy(token_recv, values[token_pos],Token_size);
        memset(token_sesn,0,Token_size);
        fp = fopen(SESSION_TOKEN, "r");
        if (fp)
        {
          //Read session token info
          fread(&token_sesn, 1, Token_size, fp);
          //Close file after reading
          fclose(fp);
          if(strncmp(token_recv,token_sesn,strlen(token_sesn)))
          {
            printf("token mismatch");
            //The current token value should be set to a new random value
            // when a mismatch is detected.
            //Then a new token issued after the re-login,
            //this will restrict hackers to restrict attempts.
            fp = fopen(SESSION_TOKEN, "w+");
            if(fp)
            {
              //file access restrict to only owner, and group,
              //as per security recommendations.
              system(CHG_SESSION_TOKEN_FILE);
              //change owner,group of file to www-data,
              //as per security recommendations.
              system(CHG_SESSION_TOKEN_FILE_OWNR);
              //generate a random number and store in session token file.
              system(TOKEN_GENERATE);
              fclose(fp);
            }
            create_msg_to_send( values,buf,TOKEN_MISMACTH);
            result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
            LOG_MSG_ERROR(" Sent the response for page,Token mismatch  result : %d \n",
                            result, 0, 0);
          }
          else
          {
            /* Check Page request is for MobileAP Settings */
            if(strstr(fields[MOBILE_AP_PAGE_PAGE],SETMOBILEAP))
             SetMobileAP_Config(values,buf,their_addr,addr_len);
            else if(strstr(fields[MOBILE_AP_PAGE_PAGE],GETMOBILEAP))
              GetMobileAP_Config(values,buf,their_addr,addr_len);
            else if(strstr(fields[STATIC_NAT_SETTINGS_PAGE_PAGE],ADDSNATENTRY))
              AddSnatEntry(values,buf,their_addr,addr_len);
            else if(strstr(fields[STATIC_NAT_SETTINGS_PAGE_PAGE],GETSNATENTRIES))
              GetSnatEntries(values,buf,their_addr,addr_len);
            else if(strstr(fields[NAT_SETTINGS_PAGE_PAGE],SETNATSETTINGS))
              SetNatSettings(values,buf,their_addr,addr_len);
            else if(strstr(fields[NAT_SETTINGS_PAGE_PAGE],GETNATSETTINGS))
              GetNatSettings(values,buf,their_addr,addr_len);
            else if(strstr(fields[LAN_PAGE_PAGE],SETLAN_CONFIG))
              SetLanConfig(values,buf,their_addr,addr_len);
            else if(strstr(fields[LAN_PAGE_PAGE],GETLAN_CONFIG))
              GetLanConfig(values,buf,their_addr,addr_len);
            else if(strstr(fields[DHCP_RESERVATION_SETTINGS_PAGE_PAGE],
                    ADDDHCPRESERVATIONRECORD))
              AddDHCPReservationRecord(values,buf,their_addr,addr_len);
            else if(strstr(fields[DHCP_RESERVATION_SETTINGS_PAGE_PAGE],
                    GETDHCPRESERVATIONRECORDS))
              GetDHCPReservationRecords(values,buf,their_addr,addr_len);
            else if(strstr(fields[WWAN_IPV4_PAGE_PAGE],GETWWANIPV4))
              GetWWANIPV4(values,buf,their_addr,addr_len);
            else if(strstr(fields[WWAN_IPV4_PAGE_PAGE],SETWWANIPV4))
              SetWWANIPV4(values,buf,their_addr,addr_len);
            else if(strstr(fields[UPNP_DLNA_PAGE_PAGE],GETUPNPDLNA))
              GetUPNP_DLNA_Config(values,buf,their_addr,addr_len);
            else if(strstr(fields[WWAN_IPV4_PAGE_PAGE],SETUPNPDLNA))
              SetUPNP_DLNA_Config(values,buf,their_addr,addr_len);
            else if(strstr(fields[DATA_RATE_PAGE_PAGE],GETDATARATE))
              GetDatarate(values,buf,their_addr,addr_len);
            else if(strstr(fields[GET_MEDIA_DIR_PAGE_PAGE],GETMEDIA_DIR_LIST))
              GetMediaDir(values,buf,their_addr,addr_len);
            else if(strstr(fields[SET_MEDIA_DIR_PAGE_PAGE],SETMEDIA_DIR_LIST))
              SetMediaDir(values,buf,their_addr,addr_len);
            else if(strstr(fields[WWAN_STATS_PAGE_PAGE],WWAN_STATS))
            {
              if(atoi(values[WWAN_STATS_PAGE_GET_RESET_MODE]) == 0)
                GetWWANStats(values,buf,their_addr,addr_len);
              else if(atoi(values[WWAN_STATS_PAGE_GET_RESET_MODE]) == 1)
              ResetWWANStats(values,buf,their_addr,addr_len);
            }
            else if(strstr(fields[FIREWALL_SETTINGS_PAGE_PAGE],GETFIREWALLCONFIG))
              ConfigFirewall(values,buf,their_addr,addr_len);
            else if(strstr(fields[FIREWALL_SETTINGS_PAGE_PAGE],SETFIREWALLENTRIES))
              SetFirewall(values,buf,their_addr,addr_len);
            else if(strstr(fields[FIREWALL_SETTINGS_PAGE_PAGE],GETFIREWALLENTRIES))
              DisplayFirewall(values,buf,their_addr,addr_len);
            else if(strstr(fields[IPV6_WWAN_SETTINGS_PAGE_PAGE],IPV6SETWWAN))
              SetIPv6Wwan_State(values,buf,their_addr,addr_len);
            else if(strstr(fields[IPV6_WWAN_SETTINGS_PAGE_PAGE],IPV6GETWWAN))
              GetIPv6Wwan_State(values,buf,their_addr,addr_len);
            else if(strstr(fields[UPNP_DLNA_NOTIFY_PAGE_PAGE],SETNOTIFYINTERVAL))
              SetNotify_Interval(values,buf,their_addr,addr_len);
            else if(strstr(fields[UPNP_DLNA_NOTIFY_PAGE_PAGE],GETNOTIFYINTERVAL))
              GetNotify_Interval(values,buf,their_addr,addr_len);
            else
            {
              #ifdef DEBUG_PRINT
              printf("Default : Page string %s \n ",fields[0]);
              LOG_MSG_ERROR("\n\n Page string wrong!! please check \n", 0, 0, 0);
              #endif
            }
          }
        }
        else
        {
          LOG_MSG_ERROR("\n\n session token missing,relogin \n", 0, 0, 0);
          create_msg_to_send( values,buf,TOKEN_MISMACTH);
          result=send_response_webclient_socket(buf,strlen(buf),their_addr,addr_len);
          LOG_MSG_ERROR(" Sent the response for page,Token mismatch  result : %d \n",
                          result, 0, 0);
        }
      }
    }
    else /* Someother message from the client */
    {
      LOG_MSG_ERROR("\n\n wrong msg received or not for us \n", 0, 0, 0);
    }
  }
  if (!QcMapClient->DisableMobileAP(&qmi_error))
  LOG_MSG_ERROR("MobileAP Disable failed!! qmi error : %d", qmi_error, 0, 0);
  //don't expect it to return...
  close(qcmap_webclient_sockfd);
  return QCMAP_CM_ERROR;
}

#ifdef __cplusplus
}
#endif
