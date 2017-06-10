/*!
  @file
  qcmap_cm_xml.c

  @brief
  This file implements the XML specific parsing functionality.

*/

/*===========================================================================

  Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
07/11/12   gk      9x25
10/26/12   cp      Added support for Dual AP and different types of NAT.
12/19/12   sb      Added support for RNDIS/ECM USB tethering.
06/12/13   sg      Added DHCP Reservation Feature
===========================================================================*/

/*===========================================================================

                      INCLUDE FILES FOR MODULE

===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <arpa/inet.h>
#include "xmllib_common.h"
#include "xmllib_parser.h"
#include "ds_util.h"
#include "qcmap_cm_api.h"

/* Private Type definitions */

/* Max allowed size of the XML file (2 MB) */
#define QCMAP_CM_XML_MAX_FILESIZE       (2 << 20)

/* Defines for clipping space or space & quotes (single, double) */
#define QCMAP_CM_XML_CLIP_SPACE         " "
#define QCMAP_CM_XML_CLIP_SPACE_QUOTES  " '\""

/* All the XML TAG */
#define system_TAG                      "system"
#define Debug_TAG                       "MobileAPDbg"
#define MobileAPCfg_TAG                 "MobileAPCfg"
#define Cradle_TAG                      "Cradle"
/* Debug Tags */
#define SUPPLICANT_DEBUG_TAG            "SupplicantCmdAppend"
#define HOSTAPD_DEBUG_TAG               "HostapdCmdAppend"
#define HOSTAPD_GUEST_DEBUG_TAG         "GuestHostapdCmdAppend"
/* IPA ODU Tags */
#define odu_cfg_TAG                     "ODUCFG"
#define mode_TAG                        "Mode"
/* IPA Cradle Tags */
#define IPACM_TAG                       "IPACM"
#define IPACMIface_TAG                  "IPACMIface"
#define Iface_TAG                       "Iface"
#define Name_TAG                        "Name"
#define Category_TAG                    "Category"
/* Prefix Delegation TAG */
#define PrefixDelegation_TAG            "PrefixDelegation"
/* NAT Config Entries */
#define MobileAPNatCfg_TAG              "MobileAPNatCfg"
#define NATType_TAG                     "NATType"
#define SYMMETRIC_TAG                   "SYM"
#define PRC_TAG                         "PRC"
#define FULLCONE_TAG                    "FC"
#define ARC_TAG                         "ARC"
#define FirewallEnabled_TAG             "FirewallEnabled"
#define FirewallPktsAllowed_TAG         "FirewallPktsAllowed"
#define Firewall_TAG                    "Firewall"
#define FirewallStartPort_TAG           "FirewallStartPort"
#define FirewallEndPort_TAG             "FirewallEndPort"
#define FirewallProtocol_TAG            "FirewallProtocol"
#define PortFwding_TAG                  "PortFwding"
#define PortFwdingPrivateIP_TAG         "PortFwdingPrivateIP"
#define PortFwdingPrivatePort_TAG       "PortFwdingPrivatePort"
#define PortFwdingGlobalPort_TAG        "PortFwdingGlobalPort"
#define PortFwdingProtocol_TAG          "PortFwdingProtocol"
#define NatEntryGenericTimeout_TAG         "NatEntryGenericTimeout"
#define NatEntryICMPTimeout_TAG            "NatEntryICMPTimeout"
#define NatEntryTCPEstablishedTimeout_TAG  "NatEntryTCPEstablishedTimeout"
#define NatEntryUDPTimeout_TAG             "NatEntryUDPTimeout"
#define DmzIP_TAG                       "DmzIP"
#define EnableIPSECVpnPassthrough_TAG   "EnableIPSECVpnPassthrough"
#define EnablePPTPVpnPassthrough_TAG    "EnablePPTPVpnPassthrough"
#define EnableL2TPVpnPassthrough_TAG    "EnableL2TPVpnPassthrough"
#define EnableWebserverWWANAccess_TAG   "EnableWebserverWWANAccess"
#define ALGCfg_TAG                      "ALGCfg"
#define EnableRTSPAlg_TAG               "EnableRTSPAlg"
#define EnableSIPAlg_TAG                "EnableSIPAlg"

#define MobileAPFirewallCfg_TAG "MobileAPFirewallCfg"
#define IPFamily_TAG "IPFamily"
#define IPV4SourceAddress_TAG "IPV4SourceAddress"
#define IPV4SourceIPAddress_TAG "IPV4SourceIPAddress"
#define IPV4SourceSubnetMask_TAG "IPV4SourceSubnetMask"

#define IPV4DestinationAddress_TAG "IPV4DestinationAddress"
#define IPV4DestinationIPAddress_TAG "IPV4DestinationIPAddress"
#define IPV4DestinationSubnetMask_TAG "IPV4DestinationSubnetMask"

#define IPV4TypeOfService_TAG "IPV4TypeOfService"
#define TOSValue_TAG "TOSValue"
#define TOSMask_TAG "TOSMask"

#define IPV4NextHeaderProtocol_TAG "IPV4NextHeaderProtocol"

#define IPV6SourceAddress_TAG "IPV6SourceAddress"
#define IPV6SourceIPAddress_TAG "IPV6SourceIPAddress"
#define IPV6SourcePrefix_TAG "IPV6SourcePrefix"

#define IPV6DestinationAddress_TAG "IPV6DestinationAddress"
#define IPV6DestinationIPAddress_TAG "IPV6DestinationIPAddress"
#define IPV6DestinationPrefix_TAG "IPV6DestinationPrefix"

#define IPV6TrafficClass_TAG "IPV6TrafficClass"
#define TrfClsValue_TAG "TrfClsValue"
#define TrfClsMask_TAG "TrfClsMask"

#define IPV6NextHeaderProtocol_TAG "IPV6NextHeaderProtocol"

#define TCPSource_TAG "TCPSource"
#define TCPSourcePort_TAG "TCPSourcePort"
#define TCPSourceRange_TAG "TCPSourceRange"

#define TCPDestination_TAG "TCPDestination"
#define TCPDestinationPort_TAG "TCPDestinationPort"
#define TCPDestinationRange_TAG "TCPDestinationRange"

#define UDPSource_TAG "UDPSource"
#define UDPSourcePort_TAG "UDPSourcePort"
#define UDPSourceRange_TAG "UDPSourceRange"

#define UDPDestination_TAG "UDPDestination"
#define UDPDestinationPort_TAG "UDPDestinationPort"
#define UDPDestinationRange_TAG "UDPDestinationRange"

#define ICMPType_TAG "ICMPType"
#define ICMPCode_TAG "ICMPCode"

#define ESP_TAG "ESP"
#define ESPSPI_TAG "ESPSPI"

#define TCP_UDPSource_TAG "TCP_UDPSource"
#define TCP_UDPSourcePort_TAG "TCP_UDPSourcePort"
#define TCP_UDPSourceRange_TAG "TCP_UDPSourceRange"

#define TCP_UDPDestination_TAG "TCP_UDPDestination"
#define TCP_UDPDestinationPort_TAG "TCP_UDPDestinationPort"
#define TCP_UDPDestinationRange_TAG "TCP_UDPDestinationRange"

#define DHCPReservationRecords_TAG "DHCPReservationRecord"
#define ClientMACAddr_TAG "ClientMACAddr"
#define ClientReservedIP_TAG "ClientReservedIP"
#define ClientName_TAG "ClientName"
#define Reservation_TAG "Reservation"

/* LAN Config Entries */
#define MobileAPLanCfg_TAG              "MobileAPLanCfg"
#define Module_TAG                      "Module"
#define WlanMode_TAG                    "WlanMode"
#define HostAPDCfg_TAG                  "HostAPDCfg"
#define HostAPDEntropy_TAG              "HostAPDEntropy"
#define STAModeHostAPDCfg_TAG           "STAModeHostAPDCfg"
#define APIPAddr_TAG                    "APIPAddr"
#define SubNetMask_TAG                  "SubNetMask"
#define AccessProfile_TAG               "AccessProfile"
#define EnableDHCPServer_TAG            "EnableDHCPServer"
#define DHCPCfg_TAG                     "DHCPCfg"
#define DHCPStartIP_TAG                 "StartIP"
#define DHCPEndIP_TAG                   "EndIP"
#define DHCPLeaseTime_TAG               "LeaseTime"
#define SupplicantCfg_TAG               "SupplicantCfg"
#define STAModeConnType_TAG             "STAModeConnType"
#define STAModeStaticIPAddr_TAG         "StaticIPAddr"
#define STAModeStaticConfigDNSAddr_TAG  "StaticConfigDNSAddr"
#define STAModeStaticConfigGWAddr_TAG   "StaticConfigGWAddr"
#define STAModeStaticConfigNetMask_TAG  "StaticConfigNetMask"
#define GuestAPCfg_TAG                  "GuestAPCfg"
#define StationModeCfg_TAG              "StationModeCfg"
#define EnableIPV6_TAG                  "EnableIPV6"
#define EnableIPV4_TAG                  "EnableIPV4"
/* WAN Config Entries */
#define MobileAPWanCfg_TAG              "MobileAPWanCfg"
#define AutoConnect_TAG                 "AutoConnect"
#define Roaming_TAG                     "Roaming"
#define EriConfig_TAG                   "EriConfig"
#define TECH_TAG                        "TECH"
#define V4_UMTS_PROFILE_INDEX_TAG       "V4_UMTS_PROFILE_INDEX"
#define V4_CDMA_PROFILE_INDEX_TAG       "V4_CDMA_PROFILE_INDEX"
#define V6_UMTS_PROFILE_INDEX_TAG       "V6_UMTS_PROFILE_INDEX"
#define V6_CDMA_PROFILE_INDEX_TAG       "V6_CDMA_PROFILE_INDEX"
#define IPFamily_TAG                    "IPFamily"
#define TECH_ANY_TAG                    "ANY"
#define TECH_3GPP_TAG                   "3GPP"
#define TECH_3GPP2_TAG                  "3GPP2"
#define IPV4_TAG                        "IPv4"
#define IPV6_TAG                        "IPv6"
#define IPV4V6_TAG                      "IPv4v6"
#define AP_TAG                          "AP"
#define AP_AP_TAG                       "AP-AP"
#define AP_STA_TAG                      "AP-STA"
#define FULL_TAG                        "FULL"
#define INTERNET_TAG                    "INTERNETONLY"

/* Daemon Services Congig Entries*/
#define UPNP_TAG                        "UPnP"
#define DLNA_TAG                        "DLNA"
#define MDNS_TAG                        "MDNS"

/* QCMAP bootup flags*/
#define MobileAPBootUp_TAG             "MobileAPBootUpCfg"
#define MobileAPEnable_TAG              "MobileAPEnableAtBootup"
#define WLANEnable_TAG                  "WLANEnableAtBootup"

/*SIP server information flags*/
#define DefaultSIPServerConfigType_TAG  "DefaultSIPServerConfigType"
#define DefaultSIPServerConfig_TAG      "DefaultSIPServerConfig"
#define FQDN_TAG                        "FQDN"
#define IP_TAG                          "IP"

/* Router/Bridge Config flags */
#define EnableBridgeMode_TAG            "MobileAPSTABridgeEnable"

/* Cradle flags */
#define CradleMode_TAG                  "CradleMode"

/* Structure for storing the xml data from file, current read position and
   size information */
struct xml_data
{
   uint32 size;  /* Actual size of the file data */
   uint32 read;  /* Current read position in buff */
   char *buff;   /* Buffer containing the xml file contents */
};


/* Private function declarations */

static int32 qcmap_cm_xml_peekbytes_cb(
   struct xmllib_metainfo_s_type *metainfo,
   int32                          offset,
   int32                          bytecount,
   uint8                         *buffer
);

static int32 qcmap_cm_xml_getbytes_cb(
   struct xmllib_metainfo_s_type *metainfo,
   int32                          bytecount,
   char                          *buffer
);

static int qcmap_cm_xml_read_xml_file(
   const char *xm_file,
   struct xml_data *xml
);

static int qcmap_cm_xml_parse_tree(
   xmllib_parsetree_node_s_type *xml_root,
   qcmap_cm_conf_t *config
);

static int qcmap_cm_odu_xml_parse_tree(
   xmllib_parsetree_node_s_type *xml_node,
   int *mode
);

static int qcmap_cm_ipa_xml_parse_tree_ecm_cat
(
   xmllib_parsetree_node_s_type *xml_node,
   int *mode
);

static int qcmap_cm_firewall_xml_parse_tree(
   xmllib_parsetree_node_s_type *xml_root,
   qcmap_cm_conf_t *config
);

/*===========================================================================
  FUNCTION:  qcmap_cm_util_icmp_string
  ===========================================================================*/
  /*!
      @brief
      This function returns the result of case insensitive comparison of a
      xmllib's string (xml_str) and a regular string (str)

      @params
      xml_str [in] - xmllib string
      str     [in] - regular string

      @return
      0 - if both strings are equal
      1 - First string has greater value
     -1 - Second string has greater value
  */
/*=========================================================================*/

static int32 qcmap_cm_util_icmp_string
(
   const xmllib_string_s_type *xml_str,
   const char *str
)
{
   int32 ret = -1;

   if (NULL != xml_str && NULL != str)
   {
      uint32 len = strlen(str);

      /* If the lengths match, do the string comparison */
      if (xml_str->len == len)
      {
         ret = strncasecmp(xml_str->string, str, len);
      }
   }

   return ret;
}

/*===========================================================================
  FUNCTION:  qcmap_cm_write_xml
  ===========================================================================*/
  /*!
      @brief
      This function write QCMAP CM XML based on QCMAP CM Cfg

      @params

      @return
      QCMAP_CM_ERROR
      QCMAP_CM_SUCCESS
  */
/*=========================================================================*/
int qcmap_cm_write_xml(char *xml_file, qcmap_cm_conf_t *config)
{
   FILE *fp = NULL;
   int i;
   struct in_addr addr;
   char mac_addr_str[QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01];
   uint8 *mac = NULL;

   fp = fopen(xml_file, "wb");

   /* File not found at the given path */
   if (NULL == fp)
   {
      return QCMAP_CM_ERROR;
   }

   fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
   fprintf(fp, "<system xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"qcmap_cfg.xsd\">\n");

   //Debug Config
   if (config->debug_config.hostapd_debug || config->debug_config.supplicant_debug || config->debug_config.hostapd_guest_debug)
   {
       fprintf(fp, "<MobileAPDbg>\n");

       if (config->debug_config.supplicant_debug)
           fprintf(fp, "\t<SupplicantCmdAppend>%s</SupplicantCmdAppend>\n",
                   config->debug_config.supplicant_dbg_cmd);
       if (config->debug_config.hostapd_debug)
           fprintf(fp, "\t<HostapdCmdAppend>%s</HostapdCmdAppend>\n",
                    config->debug_config.hostapd_dbg_cmd);
       if (config->debug_config.hostapd_guest_debug)
           fprintf(fp, "\t<HostapdGuestCmdAppend>%s</HostapdGuestCmdAppend>\n",
                    config->debug_config.hostapd_guest_dbg_cmd);

       fprintf(fp, "</MobileAPDbg>\n");
   }

   fprintf(fp, "<MobileAPCfg>\n");
   // NAT Config
   fprintf(fp, "\t<MobileAPNatCfg>\n");
   if( config->nat_config.nat_type == QCMAP_MSGR_NAT_SYMMETRIC_NAT_V01)
      fprintf(fp, "\t<NATType>SYM</NATType>\n");
   if( config->nat_config.nat_type == QCMAP_MSGR_NAT_PORT_RESTRICTED_CONE_NAT_V01)
      fprintf(fp, "\t<NATType>PRC</NATType>\n");
   if( config->nat_config.nat_type == QCMAP_MSGR_NAT_FULL_CONE_NAT_V01)
      fprintf(fp, "\t<NATType>FC</NATType>\n");
   if( config->nat_config.nat_type == QCMAP_MSGR_NAT_ADDRESS_RESTRICTED_NAT_V01)
      fprintf(fp, "\t<NATType>ARC</NATType>\n");
   fprintf(fp, "\t<Firewall>%s</Firewall>\n", config->nat_config.firewall_config_file);
   if (config->nat_config.num_port_fwding_entries > 0)
   {
     for (i = 0; i < config->nat_config.num_port_fwding_entries; i++)
     {
       fprintf(fp, "\t<PortFwding>\n");
       addr.s_addr = htonl(config->nat_config.port_fwding_entries[i].port_fwding_private_ip);
       fprintf(fp, "\t\t<PortFwdingPrivateIP>%s</PortFwdingPrivateIP>\n", inet_ntoa(addr));
       fprintf(fp, "\t\t<PortFwdingPrivatePort>%d</PortFwdingPrivatePort>\n", config->nat_config.port_fwding_entries[i].port_fwding_private_port);
       fprintf(fp, "\t\t<PortFwdingGlobalPort>%d</PortFwdingGlobalPort>\n", config->nat_config.port_fwding_entries[i].port_fwding_global_port);
       fprintf(fp, "\t\t<PortFwdingProtocol>%d</PortFwdingProtocol>\n", config->nat_config.port_fwding_entries[i].port_fwding_protocol);
       fprintf(fp, "\t</PortFwding>\n");
     }
   }
   if (config->nat_config.nat_entry_generic_timeout > 0)
   {
     fprintf(fp, "\t<NatEntryGenericTimeout>%d</NatEntryGenericTimeout>\n", config->nat_config.nat_entry_generic_timeout);
   }
   if (config->nat_config.nat_entry_icmp_timeout > 0)
   {
     fprintf(fp, "\t<NatEntryICMPTimeout>%d</NatEntryICMPTimeout>\n", config->nat_config.nat_entry_icmp_timeout);
   }
   if (config->nat_config.nat_entry_tcp_established_timeout > 0)
   {
     fprintf(fp, "\t<NatEntryTCPEstablishedTimeout>%d</NatEntryTCPEstablishedTimeout>\n",
             config->nat_config.nat_entry_tcp_established_timeout);
   }
   if (config->nat_config.nat_entry_udp_timeout > 0)
   {
     fprintf(fp, "\t<NatEntryUDPTimeout>%d</NatEntryUDPTimeout>\n", config->nat_config.nat_entry_udp_timeout);
   }
   addr.s_addr = htonl(config->nat_config.dmz_ip);
   fprintf(fp, "\t<DmzIP>%s</DmzIP>\n", inet_ntoa(addr));
   fprintf(fp, "\t<EnableIPSECVpnPassthrough>%d</EnableIPSECVpnPassthrough>\n", config->nat_config.enable_ipsec_vpn_pass_through);
   fprintf(fp, "\t<EnablePPTPVpnPassthrough>%d</EnablePPTPVpnPassthrough>\n", config->nat_config.enable_pptp_vpn_pass_through);
   fprintf(fp, "\t<EnableL2TPVpnPassthrough>%d</EnableL2TPVpnPassthrough>\n", config->nat_config.enable_l2tp_vpn_pass_through);
   fprintf(fp, "\t<EnableWebserverWWANAccess>%d</EnableWebserverWWANAccess>\n", config->nat_config.enable_webserver_wwan_access);
   fprintf(fp, "\t<ALGCfg>\n");
   fprintf(fp, "\t\t<EnableRTSPAlg>%d</EnableRTSPAlg>\n", ((config->nat_config.enable_alg_mask & QCMAP_MSGR_MASK_RTSP_ALG_V01)?1:0));
   fprintf(fp, "\t\t<EnableSIPAlg>%d</EnableSIPAlg>\n", ((config->nat_config.enable_alg_mask & QCMAP_MSGR_MASK_SIP_ALG_V01)?1:0));
   fprintf(fp, "\t</ALGCfg>\n");
   fprintf(fp, "</MobileAPNatCfg>\n");
   // LAN Config
   fprintf(fp, "<MobileAPLanCfg>\n");
   fprintf(fp, "\t<Module>%s</Module>\n", config->lan_config.module);
   if ( config->lan_config.wlan_mode == QCMAP_MSGR_WLAN_MODE_AP_V01)
     fprintf(fp,"\t<WlanMode>AP</WlanMode>\n");
   if ( config->lan_config.wlan_mode == QCMAP_MSGR_WLAN_MODE_AP_AP_V01)
     fprintf(fp,"\t<WlanMode>AP-AP</WlanMode>\n");
   if ( config->lan_config.wlan_mode == QCMAP_MSGR_WLAN_MODE_AP_STA_V01)
     fprintf(fp,"\t<WlanMode>AP-STA</WlanMode>\n");
   // Flag to set Bridge Mode for AP-STA configuration
   fprintf(fp, "\t<MobileAPSTABridgeEnable>%d</MobileAPSTABridgeEnable>\n", config->lan_config.ap_sta_bridge_mode);
   fprintf(fp, "\t<EnableIPV4>%d</EnableIPV4>\n", config->lan_config.enable_ipv4);
   fprintf(fp, "\t<EnableIPV6>%d</EnableIPV6>\n", config->lan_config.enable_ipv6);
   fprintf(fp, "\t<HostAPDCfg>%s</HostAPDCfg>\n", config->lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].path_to_hostapd_conf);
   fprintf(fp, "\t<HostAPDEntropy>%s</HostAPDEntropy>\n", config->lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].path_to_hostapd_entropy);
   fprintf(fp, "\t<STAModeHostAPDCfg>%s</STAModeHostAPDCfg>\n", config->lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].path_to_sta_mode_hostapd_conf);
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].a5_ip_addr);
   fprintf(fp, "\t<APIPAddr>%s</APIPAddr>\n", inet_ntoa(addr));
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].sub_net_mask);
   fprintf(fp, "\t<SubNetMask>%s</SubNetMask>\n", inet_ntoa(addr));
   fprintf(fp, "\t<EnableDHCPServer>%d</EnableDHCPServer>\n", config->lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].enable_dhcpd);
   fprintf(fp, "\t<DHCPCfg>\n");
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_start_address);
   fprintf(fp, "\t\t<StartIP>%s</StartIP>\n", inet_ntoa(addr));
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_end_address);
   fprintf(fp, "\t\t<EndIP>%s</EndIP>\n", inet_ntoa(addr));
   fprintf(fp, "\t\t<LeaseTime>%s</LeaseTime>\n", config->lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_lease_time);
   fprintf(fp, "\t</DHCPCfg>\n");

   for (i = 0; i < config->lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].num_dhcp_reservation_records; i++)
   {
       fprintf(fp, "\t<DHCPReservationRecord>\n");
       mac = config->lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
             dhcp_reservation_records[i].client_mac_addr;
       snprintf(mac_addr_str,QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01,"%02x:%02x:%02x:%02x:%02x:%02x",\
                mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
       if ( strncmp(mac_addr_str,MAC_NULL_STR,QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01)!=0 )
       {
         fprintf(fp, "\t\t<ClientMACAddr>%s</ClientMACAddr>\n",mac_addr_str );
       }
       if ( config->lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                           dhcp_reservation_records[i].client_reserved_ip != 0 )
       {
           addr.s_addr = htonl(config->lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                           dhcp_reservation_records[i].client_reserved_ip);
           fprintf(fp, "\t\t<ClientReservedIP>%s</ClientReservedIP>\n", inet_ntoa(addr));
       }
       if ( config->lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_reservation_records[i].\
            client_device_name[0] != '\0')
       {
        fprintf(fp, "\t\t<ClientName>%s</ClientName>\n", config->lan_config.\
                interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_reservation_records[i].client_device_name);
       }
       fprintf(fp, "\t\t<Reservation>%d</Reservation>\n", config->lan_config.\
               interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_reservation_records[i].enable_reservation);
       fprintf(fp, "\t</DHCPReservationRecord>\n");
   }
   /* <GuestAPCfg> */
   fprintf(fp, "\t<GuestAPCfg>\n");
   fprintf(fp, "\t\t<HostAPDCfg>%s</HostAPDCfg>\n", config->lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].path_to_hostapd_conf);
   fprintf(fp, "\t\t<HostAPDEntropy>%s</HostAPDEntropy>\n", config->lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].path_to_hostapd_entropy);
   fprintf(fp, "\t\t<AccessProfile>%s</AccessProfile>\n", (config->lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].access_profile  == QCMAP_MSGR_PROFILE_FULL_ACCESS_V01) ? FULL_TAG : INTERNET_TAG);
   fprintf(fp, "\t</GuestAPCfg>\n");

   /* <StationModeCfg> */
   fprintf(fp, "\t<StationModeCfg>\n");
   fprintf(fp, "\t\t<SupplicantCfg>%s</SupplicantCfg>\n", config->lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].path_to_supplicant_conf);
   fprintf(fp, "\t\t<STAModeConnType>%d</STAModeConnType>\n", config->lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].conn_type);
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].static_ip_config.ip_addr);
   fprintf(fp, "\t\t<StaticIPAddr>%s</StaticIPAddr>\n", inet_ntoa(addr));
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].static_ip_config.dns_addr);
   fprintf(fp, "\t\t<StaticConfigDNSAddr>%s</StaticConfigDNSAddr>\n", inet_ntoa(addr));
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].static_ip_config.gw_ip);
   fprintf(fp, "\t\t<StaticConfigGWAddr>%s</StaticConfigGWAddr>\n", inet_ntoa(addr));
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].static_ip_config.netmask);
   fprintf(fp, "\t\t<StaticConfigNetMask>%s</StaticConfigNetMask>\n", inet_ntoa(addr));
   fprintf(fp, "\t</StationModeCfg>\n");

   /* End MobileAPLanCfg tag */
   fprintf(fp, "</MobileAPLanCfg>\n");

   // WAN Config
   fprintf(fp, "<MobileAPWanCfg>\n");
   fprintf(fp, "\t<AutoConnect>%d</AutoConnect>\n", config->wan_config.auto_connect);
   fprintf(fp, "\t<Roaming>%d</Roaming>\n", config->wan_config.roaming);
   fprintf(fp, "\t<EriConfig>%s</EriConfig>\n", config->wan_config.eri_config_file);
   if (config->wan_config.tech == QCMAP_WAN_TECH_ANY)
     fprintf(fp, "\t<TECH>%s</TECH>\n", TECH_ANY_TAG);
   if (config->wan_config.tech == QCMAP_WAN_TECH_3GPP)
     fprintf(fp, "\t<TECH>%s</TECH>\n", TECH_3GPP_TAG);
   if (config->wan_config.tech == QCMAP_WAN_TECH_3GPP2)
     fprintf(fp, "<TECH>%s</TECH>\n", TECH_3GPP2_TAG);
   fprintf(fp, "\t<V4_UMTS_PROFILE_INDEX>%d</V4_UMTS_PROFILE_INDEX>\n", config->wan_config.profile_id.v4.umts_profile_index);
   fprintf(fp, "\t<V4_CDMA_PROFILE_INDEX>%d</V4_CDMA_PROFILE_INDEX>\n", config->wan_config.profile_id.v4.cdma_profile_index);
   fprintf(fp, "\t<V6_UMTS_PROFILE_INDEX>%d</V6_UMTS_PROFILE_INDEX>\n", config->wan_config.profile_id.v6.umts_profile_index);
   fprintf(fp, "\t<V6_CDMA_PROFILE_INDEX>%d</V6_CDMA_PROFILE_INDEX>\n", config->wan_config.profile_id.v6.cdma_profile_index);
   if (config->wan_config.default_sip_server_info.sip_serv_type == QCMAP_CM_SIP_SERVER_ADDR)
   {
     addr.s_addr = config->wan_config.default_sip_server_info.sip_serv_info.sip_serv_ip;
     fprintf(fp, "\t<DefaultSIPServerConfigType>IP</DefaultSIPServerConfigType>\n");
     fprintf(fp, "\t<DefaultSIPServerConfig>%s</DefaultSIPServerConfig>\n",inet_ntoa(addr));
   }
   else if (config->wan_config.default_sip_server_info.sip_serv_type == QCMAP_CM_SIP_SERVER_FQDN)
   {
     fprintf(fp, "\t<DefaultSIPServerConfigType>FQDN</DefaultSIPServerConfigType>\n");
     fprintf(fp, "\t<DefaultSIPServerConfig>%s</DefaultSIPServerConfig>\n",
             config->wan_config.default_sip_server_info.sip_serv_info.sip_fqdn);
   }
   fprintf(fp, "\t<PrefixDelegation>%d</PrefixDelegation>\n", config->wan_config.prefix_delegation);
   fprintf(fp, "</MobileAPWanCfg>\n");

   //Daemon Services Config
   fprintf(fp, "\t<UPnP>%d</UPnP>\n", config->srvc_config.upnp_config);
   fprintf(fp, "\t<DLNA>%d</DLNA>\n", config->srvc_config.dlna_config);
   fprintf(fp, "\t<MDNS>%d</MDNS>\n", config->srvc_config.mdns_config);

   /* Bootup Config Flags*/
   fprintf(fp, "<MobileAPBootUpCfg>\n");
   fprintf(fp, "\t<MobileAPEnableAtBootup>%d</MobileAPEnableAtBootup>\n", config->bootup_config.enable_mobileap_at_bootup);
   fprintf(fp, "\t<WLANEnableAtBootup>%d</WLANEnableAtBootup>\n", config->bootup_config.enable_wlan_at_bootup);
   fprintf(fp, "</MobileAPBootUpCfg>\n");

   /* Cradle Flags */
   fprintf(fp, "<Cradle>\n");
   fprintf(fp, "\t<CradleMode>%d</CradleMode>\n", config->cradle_config.cradle_mode);
   fprintf(fp, "</Cradle>\n");

   fprintf(fp, "</MobileAPCfg>\n");
   fprintf(fp, "</system>\n");

   fclose(fp);

   return (QCMAP_CM_SUCCESS);
}

/*===========================================================================
  FUNCTION:  qcmap_cm_write_firewall_xml
  ===========================================================================*/
  /*!
      @brief
      This function write QCMAP CM firewall XML based on QCMAP CM Cfg

      @params

      @return
      QCMAP_CM_ERROR
      QCMAP_CM_SUCCESS
  */
/*=========================================================================*/
int qcmap_cm_write_firewall_xml(char *xml_file, qcmap_cm_conf_t *config)
{
   FILE *fp = NULL;
   int i;
   struct in_addr addr;
   struct in6_addr ip6_addr;
   char ip6_addr_input[INET6_ADDRSTRLEN];
   int next_hdr_prot;
   ds_assert(xml_file != NULL);
   ds_assert(config != NULL);

   fp = fopen(xml_file, "wb");


   /* File not found at the given path */
   if (NULL == fp)
   {
      return QCMAP_CM_ERROR;
   }

   fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
   fprintf(fp,
           "<system xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"mobileap_firewall_cfg.xsd\">\n");
   fprintf(fp, "\t<MobileAPFirewallCfg>\n");
   fprintf(fp, "\t<FirewallEnabled>%d</FirewallEnabled>\n", config->nat_config.firewall_enabled);
   fprintf(fp, "\t<FirewallPktsAllowed>%d</FirewallPktsAllowed>\n", config->nat_config.firewall_pkts_allowed);
   if (config->nat_config.num_firewall_entries > 0)
   {
     for (i = 0; i < config->nat_config.num_firewall_entries; i++)
     {

       next_hdr_prot = PS_NO_NEXT_HDR;
       fprintf(fp, "\t\t<Firewall>\n");
       fprintf(fp,
               "\t\t\t<IPFamily>%d</IPFamily>\n",config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_vsn);

      if(config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_vsn == IP_V4)
       {
           if(config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask &
              IPFLTR_MASK_IP4_SRC_ADDR)
           {
               fprintf(fp, "\t\t\t<IPV4SourceAddress>\n");
               addr.s_addr =
                   (config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v4.src.addr.ps_s_addr);
               fprintf(fp, "\t\t\t\t<IPV4SourceIPAddress>%s</IPV4SourceIPAddress>\n",inet_ntoa(addr));
               addr.s_addr =
                   (config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v4.src.subnet_mask.ps_s_addr);
               fprintf(fp, "\t\t\t\t<IPV4SourceSubnetMask>%s</IPV4SourceSubnetMask>\n",inet_ntoa(addr));
               fprintf(fp, "\t\t\t</IPV4SourceAddress>\n");
           }

           if(config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask &
              IPFLTR_MASK_IP4_DST_ADDR)
           {
               fprintf(fp, "\t\t\t<IPV4DestinationAddress>\n");
               addr.s_addr =
                   (config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v4.dst.addr.ps_s_addr);
               fprintf(fp, "\t\t\t\t<IPV4DestinationIPAddress>%s</IPV4DestinationIPAddress>\n",inet_ntoa(addr));
               addr.s_addr =
                   (config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v4.dst.subnet_mask.ps_s_addr);
               fprintf(fp, "\t\t\t\t<IPV4DestinationSubnetMask>%s</IPV4DestinationSubnetMask>\n",inet_ntoa(addr));
               fprintf(fp, "\t\t\t</IPV4DestinationAddress>\n");
           }

           if(config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask &
              IPFLTR_MASK_IP4_TOS)
           {
               fprintf(fp, "\t\t\t<IPV4TypeOfService>\n");
               fprintf(fp, "\t\t\t\t<TOSValue>%d</TOSValue>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v4.tos.val);
               fprintf(fp, "\t\t\t\t<TOSMask>%d</TOSMask>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v4.tos.mask);
               fprintf(fp, "\t\t\t</IPV4TypeOfService>\n");
           }

           if( config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask &
               IPFLTR_MASK_IP4_NEXT_HDR_PROT )
           {
               fprintf(fp, "\t\t\t<IPV4NextHeaderProtocol>%d</IPV4NextHeaderProtocol>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot);
               next_hdr_prot = config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot;
           }
       }

       if(config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_vsn == IP_V6)
       {
           if(config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask &
              IPFLTR_MASK_IP6_SRC_ADDR)
           {
               fprintf(fp, "\t\t\t<IPV6SourceAddress>\n");
               memcpy(ip6_addr.s6_addr,
                      config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr8,
                      QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8));
               bzero(&ip6_addr_input,INET6_ADDRSTRLEN);
               if(inet_ntop(AF_INET6,&ip6_addr,ip6_addr_input,INET6_ADDRSTRLEN) != NULL)
               {
                 fprintf(fp, "\t\t\t\t<IPV6SourceIPAddress>%s</IPV6SourceIPAddress>\n",ip6_addr_input);
               }
               fprintf(fp, "\t\t\t\t<IPV6SourcePrefix>%d</IPV6SourcePrefix>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v6.src.prefix_len);
               fprintf(fp, "\t\t\t</IPV6SourceAddress>\n");
           }

           if(config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask &
              IPFLTR_MASK_IP6_DST_ADDR)
           {
               fprintf(fp, "\t\t\t<IPV6DestinationAddress>\n");
               memcpy(ip6_addr.s6_addr,
                      config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v6.dst.addr.in6_u.u6_addr8,
                      QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8));
               bzero(&ip6_addr_input,INET6_ADDRSTRLEN);
               if(inet_ntop(AF_INET6,&ip6_addr,ip6_addr_input,INET6_ADDRSTRLEN) != NULL)
               {
                 fprintf(fp, "\t\t\t\t<IPV6DestinationIPAddress>%s</IPV6DestinationIPAddress>\n",ip6_addr_input);
               }
               fprintf(fp, "\t\t\t\t<IPV6DestinationPrefix>%d</IPV6DestinationPrefix>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v6.dst.prefix_len);
               fprintf(fp, "\t\t\t</IPV6DestinationAddress>\n");
           }

           if(config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask &
              IPFLTR_MASK_IP6_TRAFFIC_CLASS)
           {
               fprintf(fp, "\t\t\t<IPV6TrafficClass>\n");
               fprintf(fp, "\t\t\t\t<TrfClsValue>%d</TrfClsValue>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.val);
               fprintf(fp, "\t\t\t\t<TrfClsMask>%d</TrfClsMask>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.mask);
               fprintf(fp, "\t\t\t</IPV6TrafficClass>\n");
           }

           if( config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask &
               IPFLTR_MASK_IP6_NEXT_HDR_PROT )
           {
               fprintf(fp, "\t\t\t<IPV6NextHeaderProtocol>%d</IPV6NextHeaderProtocol>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v6.next_hdr_prot);
               next_hdr_prot = config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_hdr.v6.next_hdr_prot;
           }
       }

       switch(next_hdr_prot)
       {
           case PS_IPPROTO_TCP:
             if(config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp.field_mask &
                IPFLTR_MASK_TCP_SRC_PORT )
             {
               fprintf(fp, "\t\t\t<TCPSource>\n");
               fprintf(fp, "\t\t\t\t<TCPSourcePort>%d</TCPSourcePort>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp.src.port);
               fprintf(fp, "\t\t\t\t<TCPSourceRange>%d</TCPSourceRange>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp.src.range);
               fprintf(fp, "\t\t\t</TCPSource>\n");
             }

             if(config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp.field_mask &
                IPFLTR_MASK_TCP_DST_PORT)
             {
               fprintf(fp, "\t\t\t<TCPDestination>\n");
               fprintf(fp, "\t\t\t\t<TCPDestinationPort>%d</TCPDestinationPort>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.port);
               fprintf(fp, "\t\t\t\t<TCPDestinationRange>%d</TCPDestinationRange>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.range);
               fprintf(fp, "\t\t\t</TCPDestination>\n");
             }
             break;

           case PS_IPPROTO_UDP:
             if(config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.udp.field_mask &
                IPFLTR_MASK_UDP_SRC_PORT)
             {
               fprintf(fp, "\t\t\t<UDPSource>\n");
               fprintf(fp, "\t\t\t\t<UDPSourcePort>%d</UDPSourcePort>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.udp.src.port);
               fprintf(fp, "\t\t\t\t<UDPSourceRange>%d</UDPSourceRange>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.udp.src.range);
               fprintf(fp, "\t\t\t</UDPSource>\n");
             }

             if(config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.udp.field_mask &
                IPFLTR_MASK_UDP_DST_PORT)
             {
               fprintf(fp, "\t\t\t<UDPDestination>\n");
               fprintf(fp, "\t\t\t\t<UDPDestinationPort>%d</UDPDestinationPort>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.port);
               fprintf(fp, "\t\t\t\t<UDPDestinationRange>%d</UDPDestinationRange>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.range);
               fprintf(fp, "\t\t\t</UDPDestination>\n");
             }
             break;

           case PS_IPPROTO_ICMP:
           case PS_IPPROTO_ICMP6:
             if(config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.icmp.field_mask &
                IPFLTR_MASK_ICMP_MSG_TYPE)
             {
               fprintf(fp, "\t\t\t<ICMPType>%d</ICMPType>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.icmp.type);
             }

             if(config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.icmp.field_mask &
                IPFLTR_MASK_ICMP_MSG_CODE)
             {
               fprintf(fp, "\t\t\t<ICMPCode>%d</ICMPCode>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.icmp.code);
             }
             break;

           case PS_IPPROTO_ESP:
             if(config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.esp.field_mask &
                IPFLTR_MASK_ESP_SPI)
             {
               fprintf(fp, "\t\t\t<ESPSPI>%d</ESPSPI>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.esp.spi);
             }
             break;

           case PS_IPPROTO_TCP_UDP:
             if(config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask &
              IPFLTR_MASK_TCP_UDP_SRC_PORT)
             {
               fprintf(fp, "\t\t\t<TCP_UDPSource>\n");
               fprintf(fp, "\t\t\t\t<TCP_UDPSourcePort>%d</TCP_UDPSourcePort>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.src.port);
               fprintf(fp, "\t\t\t\t<TCP_UDPSourceRange>%d</TCP_UDPSourceRange>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.src.range);
               fprintf(fp, "\t\t\t</TCP_UDPSource>\n");
             }

             if(config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask &
              IPFLTR_MASK_TCP_UDP_DST_PORT)
             {
               fprintf(fp, "\t\t\t<TCP_UDPDestination>\n");
               fprintf(fp, "\t\t\t\t<TCP_UDPDestinationPort>%d</TCP_UDPDestinationPort>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.port);
               fprintf(fp, "\t\t\t\t<TCP_UDPDestinationRange>%d</TCP_UDPDestinationRange>\n",
                       config->nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.range);
               fprintf(fp, "\t\t\t</TCP_UDPDestination>\n");
             }
             break;

           default:
               break;
       }
       fprintf(fp, "\t\t</Firewall>\n");
     }
   }
   fprintf(fp, "\t</MobileAPFirewallCfg>\n");
   fprintf(fp, "</system>\n");
   fclose(fp);

   return (QCMAP_CM_SUCCESS);
}


/*===========================================================================
  FUNCTION:  qcmap_cm_read_xml
  ===========================================================================*/
  /*!
      @brief
      This function read QCMAP CM XML and populate the QCMAP CM Cfg

      @params

      @return
      QCMAP_CM_ERROR
      QCMAP_CM_SUCCESS
  */
/*=========================================================================*/
int qcmap_cm_read_xml(char *xml_file, qcmap_cm_conf_t *config)
{
   xmllib_parsetree_node_s_type *xml_root = NULL;
   struct xml_data xml_data = {0, 0, NULL};

   /* Update the xml_data structure with data from the xml file */
   int32 ret_val = qcmap_cm_xml_read_xml_file(xml_file, &xml_data);

   if (QCMAP_CM_SUCCESS == ret_val)
   {
      xmllib_error_e_type error = 0;
      int32 xmllib_ret;
      struct xmllib_metainfo_s_type metainfo = {
                                                  &xml_data,
                                                  (xmllib_memalloc_fptr_type)malloc,
                                                  free,
                                                  qcmap_cm_xml_peekbytes_cb,
                                                  qcmap_cm_xml_getbytes_cb
                                               };

      /* Invoke the XML parser and obtain the parse tree */
      xmllib_ret = xmllib_parser_parse(XMLLIB_ENCODING_UTF8,
                                       &metainfo,
                                       &xml_root,
                                       &error);

      if (XMLLIB_SUCCESS == xmllib_ret)
      {
         ret_val = QCMAP_CM_SUCCESS;
      }
      else
      {
         printf("qcmap_cm_xml_parse: xmllib returned parse error\n");
         ret_val = QCMAP_CM_ERROR;
      }
   }

   if (ret_val != QCMAP_CM_SUCCESS)
   {
     if (xml_data.buff)
       free(xml_data.buff);

     return ret_val;
   }

   if (QCMAP_CM_SUCCESS == ret_val)
   {

      /* parse the xml tree returned by the xmllib */
      ret_val = qcmap_cm_xml_parse_tree(xml_root, config);

      if (ret_val != QCMAP_CM_SUCCESS)
         printf("qcmap_cm_xml_parse: qcmap_cm_xml_parse_tree returned parse error\n");

      /* Free up the xmllib's parse tree */
      xmllib_parser_free(free, xml_root);
   }

   /* Free the buffer allocated by the xml file reading utility function */
   if (xml_data.buff)
     free(xml_data.buff);

   return ret_val;

}

/*===========================================================================
  FUNCTION:  qcmap_cm_read_odu_xml
  ===========================================================================*/
  /*!
      @brief
      This function read QCMAP CM XML and populate the ODU mode

      @params

      @return
      QCMAP_CM_ERROR
      QCMAP_CM_SUCCESS
  */
/*=========================================================================*/
int qcmap_cm_read_odu_xml(char *xml_file, int *mode)
{
   xmllib_parsetree_node_s_type *xml_root = NULL;
   struct xml_data xml_data = {0, 0, NULL};

   /* Update the xml_data structure with data from the xml file */
   int32 ret_val = qcmap_cm_xml_read_xml_file(xml_file, &xml_data);

   if (QCMAP_CM_SUCCESS == ret_val)
   {
      xmllib_error_e_type error = 0;
      int32 xmllib_ret;
      struct xmllib_metainfo_s_type metainfo = {
                                                  &xml_data,
                                                  (xmllib_memalloc_fptr_type)malloc,
                                                  free,
                                                  qcmap_cm_xml_peekbytes_cb,
                                                  qcmap_cm_xml_getbytes_cb
                                               };

      /* Invoke the XML parser and obtain the parse tree */
      xmllib_ret = xmllib_parser_parse(XMLLIB_ENCODING_UTF8,
                                       &metainfo,
                                       &xml_root,
                                       &error);

      if (XMLLIB_SUCCESS == xmllib_ret)
      {
         ret_val = QCMAP_CM_SUCCESS;
      }
      else
      {
         LOG_MSG_ERROR("qcmap_cm_xml_parse: xmllib returned parse error",0,0,0);
         ret_val = QCMAP_CM_ERROR;
      }
   }

   if (ret_val != QCMAP_CM_SUCCESS)
   {
     if (xml_data.buff)
       free(xml_data.buff);

     return ret_val;
   }

   if (QCMAP_CM_SUCCESS == ret_val)
   {

      /* parse the xml tree returned by the xmllib */
      ret_val = qcmap_cm_odu_xml_parse_tree(xml_root, mode);

      /* Free up the xmllib's parse tree */
      xmllib_parser_free(free, xml_root);
   }

   /* Free the buffer allocated by the xml file reading utility function */
   if (xml_data.buff)
     free(xml_data.buff);

   return ret_val;
}

/*===========================================================================
  FUNCTION:  qcmap_cm_get_ecm_cat_ipa_xml
  ===========================================================================*/
  /*!
      @brief
      This function read IPACM XML and populates the ECM category

      @params

      @return
      QCMAP_CM_ERROR
      QCMAP_CM_SUCCESS
  */
/*=========================================================================*/
int qcmap_cm_get_ecm_cat_ipa_xml(char *xml_file, int *category)
{
   xmllib_parsetree_node_s_type *xml_root = NULL;
   struct xml_data xml_data = {0, 0, NULL};

   /* Update the xml_data structure with data from the xml file */
   int32 ret_val = qcmap_cm_xml_read_xml_file(xml_file, &xml_data);

   if (QCMAP_CM_SUCCESS == ret_val)
   {
      xmllib_error_e_type error = 0;
      int32 xmllib_ret;
      struct xmllib_metainfo_s_type metainfo = {
                                                  &xml_data,
                                                  (xmllib_memalloc_fptr_type)malloc,
                                                  free,
                                                  qcmap_cm_xml_peekbytes_cb,
                                                  qcmap_cm_xml_getbytes_cb
                                               };

      /* Invoke the XML parser and obtain the parse tree */
      xmllib_ret = xmllib_parser_parse(XMLLIB_ENCODING_UTF8,
                                       &metainfo,
                                       &xml_root,
                                       &error);

      if (XMLLIB_SUCCESS == xmllib_ret)
      {
         ret_val = QCMAP_CM_SUCCESS;
      }
      else
      {
         LOG_MSG_ERROR("qcmap_cm_xml_parse: xmllib returned parse error",0,0,0);
         ret_val = QCMAP_CM_ERROR;
      }
   }

   if (ret_val != QCMAP_CM_SUCCESS)
   {
     if (xml_data.buff)
       free(xml_data.buff);

     return ret_val;
   }

   if (QCMAP_CM_SUCCESS == ret_val)
   {
      /* parse the xml tree returned by the xmllib */
      ret_val = qcmap_cm_ipa_xml_parse_tree_ecm_cat(xml_root, category);

      /* Free up the xmllib's parse tree */
      xmllib_parser_free(free, xml_root);
   }

   /* Free the buffer allocated by the xml file reading utility function */
   if (xml_data.buff)
     free(xml_data.buff);

   return ret_val;
}

/*===========================================================================
  FUNCTION:  qcmap_cm_read_firewall_xml
  ===========================================================================*/
  /*!
      @brief
      This function read QCMAP CM Firewall XML and populate the QCMAP CM Cfg

      @params

      @return
      QCMAP_CM_ERROR
      QCMAP_CM_SUCCESS
  */
/*=========================================================================*/
int qcmap_cm_read_firewall_xml(char *xml_file, qcmap_cm_conf_t *config)
{
   xmllib_parsetree_node_s_type *xml_root = NULL;
   struct xml_data xml_data = {0, 0, NULL};

   ds_assert(xml_file != NULL);
   ds_assert(config != NULL);

   /* Update the xml_data structure with data from the xml file */
   int32 ret_val = qcmap_cm_xml_read_xml_file(xml_file, &xml_data);

   if (QCMAP_CM_SUCCESS == ret_val)
   {
      xmllib_error_e_type error = 0;
      int32 xmllib_ret;
      struct xmllib_metainfo_s_type metainfo = {
                                                  &xml_data,
                                                  (xmllib_memalloc_fptr_type)malloc,
                                                  free,
                                                  qcmap_cm_xml_peekbytes_cb,
                                                  qcmap_cm_xml_getbytes_cb
                                               };

      /* Invoke the XML parser and obtain the parse tree */
      xmllib_ret = xmllib_parser_parse(XMLLIB_ENCODING_UTF8,
                                       &metainfo,
                                       &xml_root,
                                       &error);

      if (XMLLIB_SUCCESS == xmllib_ret)
      {
         ret_val = QCMAP_CM_SUCCESS;
      }
      else
      {
         printf("qcmap_cm_xml_parse: xmllib returned parse error\n");
         ret_val = QCMAP_CM_ERROR;
      }
   }

   if (ret_val != QCMAP_CM_SUCCESS)
   {
     if (xml_data.buff)
       free(xml_data.buff);

     return ret_val;
   }

   if (QCMAP_CM_SUCCESS == ret_val)
   {
      /* parse the xml tree returned by the xmllib */
      ret_val = qcmap_cm_firewall_xml_parse_tree(xml_root, config);

      if (ret_val != QCMAP_CM_SUCCESS)
         printf("qcmap_cm_xml_parse: qcmap_cm_xml_parse_tree returned parse error\n");

      /* Free up the xmllib's parse tree */
      xmllib_parser_free(free, xml_root);
   }

   /* Free the buffer allocated by the xml file reading utility function */
   if (xml_data.buff)
     free(xml_data.buff);

   return ret_val;

}



/*===========================================================================
  FUNCTION:  qcmap_cm_xml_read_xml_file
  ===========================================================================*/
  /*!
      @brief
      This function is reads the given XML file and stores the contents in
      the given xml_data structure

      @params
      file_name [in]  -  XML file name
      xml_data  [out] -  Structure to read data into

      @return
      QCMAP_CM_EFAIL
      QCMAP_CM_SUCCESS
  */
/*=========================================================================*/

static int qcmap_cm_xml_read_xml_file
(
   const char *file_name,
   struct xml_data *xml_data
)
{
   FILE *fp = NULL;
   int32 ret_val = QCMAP_CM_ERROR;
   int32 cur_pos;

   fp = fopen(file_name, "rb");

   /* File not found at the given path */
   if (NULL == fp)
   {
      printf("qcmap_cm_xml_read_xml_file: unable to open file %s\n",
                    file_name);
      ret_val = QCMAP_CM_ERROR;
   }
   /* If seek to the end failed or file size is greater than what we support */
   else if (fseek(fp, 0, SEEK_END) ||
            ((cur_pos = ftell(fp)) <= 0 || cur_pos > QCMAP_CM_XML_MAX_FILESIZE))
   {
      fclose(fp);
   }
   else
   {
      xml_data->size = cur_pos;

      /* Allocate storage for reading the xml file into memory */
      if (NULL == (xml_data->buff = malloc(xml_data->size)))
      {
         printf("qcmap_cm_xml_read_xml_file: failed to allocate "
                       "memory for read buffer\n");
         ret_val = QCMAP_CM_ERROR;
      }
      /* Reset to the beginning of the file */
      else if (!fseek(fp, 0, SEEK_SET))
      {
         size_t read_size;

         /* Read the data from file to buffer */
         read_size = fread(xml_data->buff, 1, xml_data->size, fp);

         if (!ferror(fp) && (read_size == xml_data->size))
         {
            xml_data->read = 0;
            ret_val = QCMAP_CM_SUCCESS;
         }
      }

      fclose(fp);
   }

   return ret_val;
}

static int32 qcmap_cm_xml_peekbytes_cb
(
  struct xmllib_metainfo_s_type *metainfo,
  int32                          offset,
  int32                          bytecount,
  uint8                         *buffer
)
{
   struct xml_data *xml_data = NULL;

   /* Validate arguments */
   if ((NULL == metainfo)                       ||
       (NULL == (xml_data = metainfo->xmltext)) ||
       (offset < 0)                             ||
       (bytecount < 0)                          ||
       (NULL == buffer)                         ||
       (xml_data->read+offset+bytecount > xml_data->size))
   {
      return XMLLIB_ERROR;
   }

   memcpy(buffer, xml_data->buff+xml_data->read+offset, bytecount);
   return XMLLIB_SUCCESS;
}


static int32 qcmap_cm_xml_getbytes_cb
(
  struct xmllib_metainfo_s_type *metainfo,
  int32                          bytecount,
  char                          *buffer
)
{
   struct xml_data *xml_data = NULL;

   /* If requesting to read more than what we have return error */
   if ((NULL == metainfo)                       ||
       (NULL == (xml_data = metainfo->xmltext)) ||
       (bytecount < 0)                          ||
       (xml_data->read+bytecount > xml_data->size))
   {
      return XMLLIB_ERROR;
   }

   /* If a valid buffer is given, copy the data */
   if (NULL != buffer)
   {
      memcpy(buffer, xml_data->buff+xml_data->read, bytecount);
   }

   /* Increment to the next unread data block */
   xml_data->read += bytecount;

   return XMLLIB_SUCCESS;
}

/*----------------------------------------------------------------------------
 * FUNCTION      qcmap_cm_read_content_element

 * DESCRIPTION   Reads content (stored as child) of the element

 * DEPENDENCIES

 * RETURN VALUE  xmllib_string_s_type*

 * SIDE EFFECTS
 *--------------------------------------------------------------------------*/
static xmllib_string_s_type* qcmap_cm_read_content_element
(
 xmllib_parsetree_node_s_type* element
)
{
  xmllib_parsetree_node_s_type* child_ptr;

  for ( child_ptr  = element->payload.element.child;
        child_ptr != 0;
        child_ptr  = child_ptr->sibling )
  {
    if ( child_ptr->nodetype == XMLLIB_PARSETREE_NODE_CONTENT )
    {
      return &(child_ptr->payload.content);
    }
  }

  return ((xmllib_string_s_type*)NULL);
}



/*===========================================================================
  FUNCTION:  qcmap_cm_xml_parse_tree
  ===========================================================================*/
  /*!
      @brief
      This function traverses the xml tree

      @params
      xml_node [in] - xmllib's parse tree node

      @return
      QCMAP_CM_ERROR
      QCMAP_CM_SUCCESS

  */
/*=========================================================================*/

static int qcmap_cm_xml_parse_tree
(
   xmllib_parsetree_node_s_type *xml_node,
   qcmap_cm_conf_t *config
)
{
   static int ssid = 0, dhcp = 0;
   int32 ret_val = QCMAP_CM_SUCCESS;
   xmllib_string_s_type* content;
   char content_buf[QCMAP_CM_MAX_FILE_LEN];
   int i;
   uint8 mac[QCMAP_MSGR_MAC_ADDR_LEN_V01];
   uint32 num_records =0;

   if (NULL == xml_node)
      return ret_val;

   num_records = config->lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].num_dhcp_reservation_records;

   while ( (xml_node != (xmllib_parsetree_node_s_type *)NULL) &&
          (ret_val == QCMAP_CM_SUCCESS) )
   {
     switch (xml_node->nodetype)
     {
       case XMLLIB_PARSETREE_NODE_ELEMENT:
       {
         if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         system_TAG) ||
             0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         Debug_TAG) ||
             0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         MobileAPCfg_TAG) ||
             0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         PortFwding_TAG) ||
             0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         MobileAPNatCfg_TAG) ||
             0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         MobileAPLanCfg_TAG) ||
             0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         DHCPCfg_TAG) ||
             0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         GuestAPCfg_TAG) ||
             0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         StationModeCfg_TAG) ||
             0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         MobileAPWanCfg_TAG) ||
             0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         MobileAPBootUp_TAG)||
             0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         DHCPReservationRecords_TAG) ||
             0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         ALGCfg_TAG) ||
             0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         Cradle_TAG)
             )
         {
           if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         PortFwding_TAG))
           {
             // increase port forwarding entry num
             config->nat_config.num_port_fwding_entries++;
           }
           if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         GuestAPCfg_TAG))
           {
             // Put the network config in the SSID2 bin
             ssid = QCMAP_MSGR_INTF_GUEST_AP_INDEX;
           }
           if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         StationModeCfg_TAG))
           {
             // Put the network config in the SSID3 bin
             ssid = QCMAP_MSGR_INTF_STATION_INDEX;
           }
           if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         DHCPCfg_TAG))
           {
             // Updating DHCP config, don't change SSID number on exit
             dhcp = 1;
           }
           if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         DHCPReservationRecords_TAG))
           {
             // increase DHCP reservation records num
             config->lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].num_dhcp_reservation_records++;
           }
           // go to child
           ret_val = qcmap_cm_xml_parse_tree(xml_node->payload.element.child,
                                                   config);
           // Put any remaining network config into the main SSID bin
           if (dhcp) dhcp = 0;
           else if (ssid) ssid = 0;
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         Module_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             strlcpy(config->lan_config.module, content_buf, sizeof(config->lan_config.module));
             printf("Module %s\n", config->lan_config.module);
           }
	 }
	 else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                  WlanMode_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             if (strncasecmp(AP_STA_TAG, content_buf, strlen(AP_STA_TAG)) == 0)
             {
               config->lan_config.wlan_mode = QCMAP_MSGR_WLAN_MODE_AP_STA_V01;
             }
             else if (strncasecmp(AP_AP_TAG, content_buf, strlen(AP_AP_TAG)) == 0)
             {
               config->lan_config.wlan_mode = QCMAP_MSGR_WLAN_MODE_AP_AP_V01;
             }
             else
             {
               /* Default Value. */
               config->lan_config.wlan_mode = QCMAP_MSGR_WLAN_MODE_AP_V01;
             }
             printf("WlanMode %d.\n", config->lan_config.wlan_mode);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                  EnableBridgeMode_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->lan_config.ap_sta_bridge_mode = (boolean)atoi(content_buf);
             printf("AP-STA Bridge Mode is %s.\n", (config->lan_config.ap_sta_bridge_mode ? "Enabled":"Disabled"));
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                          EnableIPV6_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->lan_config.enable_ipv6 = atoi(content_buf);
             printf("Enable IPv6 %d.\n", config->lan_config.enable_ipv6);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                          EnableIPV4_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->lan_config.enable_ipv4 = atoi(content_buf);
             printf("Enable IPv4 %d.\n", config->lan_config.enable_ipv4);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         AccessProfile_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             if (strncasecmp(FULL_TAG, content_buf, MIN(sizeof(FULL_TAG) - 1, content->len)) == 0)
             {
               config->lan_config.interface[ssid].access_profile = QCMAP_MSGR_PROFILE_FULL_ACCESS_V01;
             }
             else if (strncasecmp(INTERNET_TAG, content_buf, MIN(sizeof(INTERNET_TAG) - 1, content->len)) == 0)
             {
               config->lan_config.interface[ssid].access_profile = QCMAP_MSGR_PROFILE_INTERNET_ONLY_V01;
             }
             printf("AccessProfile %d ssid %d\n", config->lan_config.interface[ssid].access_profile, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         HostAPDCfg_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             strlcpy(config->lan_config.interface[ssid].path_to_hostapd_conf, content_buf,
                     sizeof(config->lan_config.interface[ssid].path_to_hostapd_conf));
             printf("HostAPD config %s ssid %d", config->lan_config.interface[ssid].path_to_hostapd_conf, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         HostAPDEntropy_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             strlcpy(config->lan_config.interface[ssid].path_to_hostapd_entropy, content_buf,
                     sizeof(config->lan_config.interface[ssid].path_to_hostapd_entropy));
             printf("HostAPD Entropy file %s ssid %d", config->lan_config.interface[ssid].path_to_hostapd_entropy, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         STAModeHostAPDCfg_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           /* Applicable only for Primary AP. */
           if ( content && ssid == QCMAP_MSGR_INTF_AP_INDEX )
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             strlcpy(config->lan_config.interface[ssid].path_to_sta_mode_hostapd_conf, content_buf,
                     sizeof(config->lan_config.interface[ssid].path_to_sta_mode_hostapd_conf));
             printf("STA Mode HostAPD config %s ssid %d",
                    config->lan_config.interface[ssid].path_to_sta_mode_hostapd_conf, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         APIPAddr_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->lan_config.interface[ssid].a5_ip_addr = ntohl(inet_addr(content_buf));
             printf("A5IPAddr %x ssid %d\n", config->lan_config.interface[ssid].a5_ip_addr, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         SubNetMask_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->lan_config.interface[ssid].sub_net_mask = ntohl(inet_addr(content_buf));
             printf("SubnetMask %x ssid %d\n", config->lan_config.interface[ssid].sub_net_mask, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         EnableDHCPServer_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->lan_config.interface[ssid].enable_dhcpd = atoi(content_buf);
             printf("EnableDHCPD %d ssid %d\n", config->lan_config.interface[ssid].enable_dhcpd, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         DHCPStartIP_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             struct in_addr addr;
             memset(content_buf, 0, sizeof(content_buf));
             memset(&addr, 0, sizeof(addr));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             if (inet_aton(content_buf, &addr))
               config->lan_config.interface[ssid].dhcp_start_address = ntohl(addr.s_addr);
             printf("DHCPStartIP %x ssid %d\n", config->lan_config.interface[ssid].dhcp_start_address, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         DHCPEndIP_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             struct in_addr addr;
             memset(content_buf, 0, sizeof(content_buf));
             memset(&addr, 0, sizeof(addr));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             if (inet_aton(content_buf, &addr))
                 config->lan_config.interface[ssid].dhcp_end_address = ntohl(addr.s_addr);
              printf("DHCPEndIP %x ssid %d\n", config->lan_config.interface[ssid].dhcp_end_address, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         DHCPLeaseTime_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             strlcpy(config->lan_config.interface[ssid].dhcp_lease_time, content_buf,
                     sizeof(config->lan_config.interface[ssid].dhcp_lease_time));
           }
           printf("LeaseTime %s ssid %d\n", config->lan_config.interface[ssid].dhcp_lease_time, ssid);
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         ClientMACAddr_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : sizeof(content_buf) - 1));
             for (i = 0; i < QCMAP_MSGR_MAC_ADDR_LEN_V01; i++)
             {
                 mac[i] = (ds_hex_to_dec(content_buf[i * 3]) << 4) |
                           ds_hex_to_dec(content_buf[i * 3 + 1]);
             }
             memcpy(config->lan_config.interface[ssid].dhcp_reservation_records[num_records -1].\
                    client_mac_addr,mac,QCMAP_MSGR_MAC_ADDR_LEN_V01);
           }
           printf("Client MAC address is: %s\n",content_buf );
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         ClientReservedIP_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : sizeof(content_buf) - 1));
             config->lan_config.interface[ssid].dhcp_reservation_records[num_records -1].\
                 client_reserved_ip = ntohl(inet_addr(content_buf));
           }
           printf("Client IP address is: %s\n",content_buf );
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         ClientName_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : sizeof(content_buf) - 1));
             strlcpy(config->lan_config.interface[ssid].dhcp_reservation_records[num_records -1].\
                     client_device_name,content_buf, sizeof(config->lan_config.interface[ssid].dhcp_reservation_records[num_records -1].\
                     client_device_name));
           }
           printf("Client Name is: %s\n",content_buf );
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         Reservation_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : sizeof(content_buf) - 1));
             config->lan_config.interface[ssid].dhcp_reservation_records[num_records -1].\
                 enable_reservation=atoi(content_buf);
           }
           printf("Reservation Enabled: %d\n",atoi(content_buf));
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         SupplicantCfg_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             strlcpy(config->lan_config.interface[ssid].path_to_supplicant_conf, content_buf,
                     sizeof(config->lan_config.interface[ssid].path_to_supplicant_conf));
             printf("SupplicantCfg %s ssid %d\n", config->lan_config.interface[ssid].path_to_supplicant_conf, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         STAModeConnType_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             if ( atoi(content_buf) == 2 )
               config->lan_config.interface[ssid].conn_type = QCMAP_MSGR_STA_CONNECTION_STATIC_V01;
             else
               config->lan_config.interface[ssid].conn_type = QCMAP_MSGR_STA_CONNECTION_DYNAMIC_V01;
             printf("STAMode Connection Type %d ssid %d\n", config->lan_config.interface[ssid].conn_type, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         STAModeStaticIPAddr_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             struct in_addr addr;
             memset(content_buf, 0, sizeof(content_buf));
             memset(&addr, 0, sizeof(addr));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             if (inet_aton(content_buf, &addr))
               config->lan_config.interface[ssid].static_ip_config.ip_addr = ntohl(addr.s_addr);
             printf("STA Mode Static IP %x ssid %d\n", config->lan_config.interface[ssid].static_ip_config.ip_addr, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         STAModeStaticConfigDNSAddr_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             struct in_addr addr;
             memset(content_buf, 0, sizeof(content_buf));
             memset(&addr, 0, sizeof(addr));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             if (inet_aton(content_buf, &addr))
               config->lan_config.interface[ssid].static_ip_config.dns_addr = ntohl(addr.s_addr);
             printf("STA Mode Static Config DNS IP %x ssid %d\n", config->lan_config.interface[ssid].static_ip_config.dns_addr, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         STAModeStaticConfigGWAddr_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             struct in_addr addr;
             memset(content_buf, 0, sizeof(content_buf));
             memset(&addr, 0, sizeof(addr));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             if (inet_aton(content_buf, &addr))
               config->lan_config.interface[ssid].static_ip_config.gw_ip = ntohl(addr.s_addr);
             printf("STA Mode Static Config GW IP %x ssid %d\n", config->lan_config.interface[ssid].static_ip_config.gw_ip, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         STAModeStaticConfigNetMask_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             struct in_addr addr;
             memset(content_buf, 0, sizeof(content_buf));
             memset(&addr, 0, sizeof(addr));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             if (inet_aton(content_buf, &addr))
               config->lan_config.interface[ssid].static_ip_config.netmask = ntohl(addr.s_addr);
             printf("STA Mode Static Config netmask %x ssid %d\n", config->lan_config.interface[ssid].static_ip_config.netmask, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         AutoConnect_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->wan_config.auto_connect = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         Roaming_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->wan_config.roaming = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         EriConfig_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             strlcpy(config->wan_config.eri_config_file, content_buf, sizeof(config->wan_config.eri_config_file));
             printf("EriConfig %s\n", config->wan_config.eri_config_file);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         V4_UMTS_PROFILE_INDEX_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->wan_config.profile_id.v4.umts_profile_index = atoi(content_buf);
             printf("v4 umts profile id %d\n", config->wan_config.profile_id.v4.umts_profile_index);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         V4_CDMA_PROFILE_INDEX_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->wan_config.profile_id.v4.cdma_profile_index = atoi(content_buf);
             printf("v4 cdma profile id %d\n", config->wan_config.profile_id.v4.cdma_profile_index);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         V6_UMTS_PROFILE_INDEX_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->wan_config.profile_id.v6.umts_profile_index = atoi(content_buf);
             printf("v6 umts profile id %d\n", config->wan_config.profile_id.v6.umts_profile_index);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         V6_CDMA_PROFILE_INDEX_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->wan_config.profile_id.v6.cdma_profile_index = atoi(content_buf);
             printf("v6 cdma profile id %d\n", config->wan_config.profile_id.v6.cdma_profile_index);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TECH_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             if (0 == strcasecmp(content_buf, TECH_ANY_TAG))
             {
               config->wan_config.tech = QCMAP_WAN_TECH_ANY;
             }
             if (0 == strcasecmp(content_buf, TECH_3GPP_TAG))
             {
               config->wan_config.tech = QCMAP_WAN_TECH_3GPP;
             }
             if (0 == strcasecmp(content_buf, TECH_3GPP2_TAG))
             {
               config->wan_config.tech = QCMAP_WAN_TECH_3GPP2;
             }
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         DefaultSIPServerConfigType_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             if (strncasecmp(IP_TAG, content_buf, strlen(IP_TAG)) == 0)
             {
               config->wan_config.default_sip_server_info.sip_serv_type =
                   QCMAP_CM_SIP_SERVER_ADDR;
             }
             else if (strncasecmp(FQDN_TAG, content_buf, strlen(FQDN_TAG)) == 0)
             {
               config->wan_config.default_sip_server_info.sip_serv_type =
                   QCMAP_CM_SIP_SERVER_FQDN;
             }
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         DefaultSIPServerConfig_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             struct in_addr addr;
             memset(content_buf, 0, sizeof(content_buf));
             memset(&addr, 0, sizeof(addr));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));

             if (config->wan_config.default_sip_server_info.sip_serv_type ==
                 QCMAP_CM_SIP_SERVER_ADDR)
             {
               if (inet_aton(content_buf, &addr))
                 config->wan_config.default_sip_server_info.sip_serv_info.sip_serv_ip = addr.s_addr;
             }
             else if (config->wan_config.default_sip_server_info.sip_serv_type ==
                      QCMAP_CM_SIP_SERVER_FQDN)
             {
               strlcpy(config->wan_config.default_sip_server_info.sip_serv_info.sip_fqdn,
                       content_buf,
                       sizeof(config->wan_config.default_sip_server_info.sip_serv_info.sip_fqdn));
             }
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         Roaming_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->wan_config.roaming = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         PortFwdingPrivateIP_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.port_fwding_entries[config->nat_config.num_port_fwding_entries - 1].port_fwding_private_ip
                = ntohl(inet_addr(content_buf));
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         PortFwdingPrivatePort_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.port_fwding_entries[config->nat_config.num_port_fwding_entries - 1].port_fwding_private_port
                = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         PortFwdingGlobalPort_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.port_fwding_entries[config->nat_config.num_port_fwding_entries - 1].port_fwding_global_port
                = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         PortFwdingProtocol_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.port_fwding_entries[config->nat_config.num_port_fwding_entries - 1].port_fwding_protocol
                = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         NATType_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             if (strncasecmp(PRC_TAG, content_buf, strlen(PRC_TAG)) == 0)
             {
               config->nat_config.nat_type = QCMAP_MSGR_NAT_PORT_RESTRICTED_CONE_NAT_V01;
             }
             else if (strncasecmp(FULLCONE_TAG, content_buf, strlen(FULLCONE_TAG)) == 0)
             {
               config->nat_config.nat_type = QCMAP_MSGR_NAT_FULL_CONE_NAT_V01;
             }
             else if (strncasecmp(ARC_TAG, content_buf, strlen(ARC_TAG)) == 0)
             {
               config->nat_config.nat_type = QCMAP_MSGR_NAT_ADDRESS_RESTRICTED_NAT_V01;
             }
             else
             {
               /* Default SYMMETRIC NAT. */
               config->nat_config.nat_type = QCMAP_MSGR_NAT_SYMMETRIC_NAT_V01;
             }
             printf("NAT Type %d \n", config->nat_config.nat_type);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         Firewall_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             strlcpy(config->nat_config.firewall_config_file, content_buf, sizeof(config->nat_config.firewall_config_file));
             printf("Firewall config xml %s\n", config->nat_config.firewall_config_file);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         NatEntryGenericTimeout_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             if ( atoi(content_buf) >= QCMAP_NAT_ENTRY_MIN_TIMEOUT )
               config->nat_config.nat_entry_generic_timeout = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         NatEntryICMPTimeout_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             if ( atoi(content_buf) >= QCMAP_NAT_ENTRY_MIN_TIMEOUT )
               config->nat_config.nat_entry_icmp_timeout = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         NatEntryTCPEstablishedTimeout_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             if ( atoi(content_buf) >= QCMAP_NAT_ENTRY_MIN_TIMEOUT )
               config->nat_config.nat_entry_tcp_established_timeout = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         NatEntryUDPTimeout_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             if ( atoi(content_buf) >= QCMAP_NAT_ENTRY_MIN_TIMEOUT )
               config->nat_config.nat_entry_udp_timeout = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         DmzIP_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.dmz_ip = ntohl(inet_addr(content_buf));
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         EnableIPSECVpnPassthrough_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.enable_ipsec_vpn_pass_through = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         EnablePPTPVpnPassthrough_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.enable_pptp_vpn_pass_through = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         EnableL2TPVpnPassthrough_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.enable_l2tp_vpn_pass_through = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                                 EnableWebserverWWANAccess_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.enable_webserver_wwan_access = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                                 EnableRTSPAlg_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             if (atoi(content_buf) == 1)
               config->nat_config.enable_alg_mask |= QCMAP_MSGR_MASK_RTSP_ALG_V01;
             else
               config->nat_config.enable_alg_mask &= ~QCMAP_MSGR_MASK_RTSP_ALG_V01;
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                                 EnableSIPAlg_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             if (atoi(content_buf) == 1)
               config->nat_config.enable_alg_mask |= QCMAP_MSGR_MASK_SIP_ALG_V01;
             else
               config->nat_config.enable_alg_mask &= ~QCMAP_MSGR_MASK_SIP_ALG_V01;
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                                 UPNP_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->srvc_config.upnp_config  = atoi(content_buf);
			 printf("UPNP CONFIG FLAG = %x \n ",config->srvc_config.upnp_config);
           }
         }
		 else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         DLNA_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->srvc_config.dlna_config  = atoi(content_buf);
             printf("DLNA CONFIG FLAG = %x \n ",config->srvc_config.dlna_config);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         MDNS_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->srvc_config.mdns_config  = atoi(content_buf);
             printf("MDNS CONFIG FLAG = %x \n",config->srvc_config.mdns_config);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         MobileAPEnable_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->bootup_config.enable_mobileap_at_bootup  = atoi(content_buf);
             printf("MobileAP Enable At Bootup CONFIG FLAG = %x \n",config->bootup_config.enable_mobileap_at_bootup);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         WLANEnable_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->bootup_config.enable_wlan_at_bootup  = atoi(content_buf);
             printf("WLAN Enable At Bootup CONFIG FLAG = %x \n",config->bootup_config.enable_wlan_at_bootup);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name, SUPPLICANT_DEBUG_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->debug_config.supplicant_debug  = 1;
             strlcpy(config->debug_config.supplicant_dbg_cmd, content_buf, sizeof(config->debug_config.supplicant_dbg_cmd));
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name, HOSTAPD_DEBUG_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->debug_config.hostapd_debug = 1;
             strlcpy(config->debug_config.hostapd_dbg_cmd, content_buf, sizeof(config->debug_config.hostapd_dbg_cmd));
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name, HOSTAPD_GUEST_DEBUG_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->debug_config.hostapd_guest_debug = 1;
             strlcpy(config->debug_config.hostapd_guest_dbg_cmd, content_buf, sizeof(config->debug_config.hostapd_guest_dbg_cmd));
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name, PrefixDelegation_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->wan_config.prefix_delegation = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name, CradleMode_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->cradle_config.cradle_mode = atoi(content_buf);
           }
         }
      }
      break;
      default:
      break;
    }
    // go to sibling
    xml_node = xml_node->sibling;
  } // while

  return ret_val;
}


/*===========================================================================
  FUNCTION:  qcmap_cm_odu_xml_parse_tree
  ===========================================================================*/
  /*!
      @brief
      This function traverses the xml tree

      @params
      xml_node [in] - xmllib's parse tree node

      @return
      QCMAP_CM_ERROR
      QCMAP_CM_SUCCESS

  */
/*=========================================================================*/

static int qcmap_cm_odu_xml_parse_tree
(
   xmllib_parsetree_node_s_type *xml_node,
   int *mode
)
{
   static int ssid = 0, dhcp = 0;
   int32 ret_val = QCMAP_CM_SUCCESS;
   xmllib_string_s_type* content;
   char content_buf[QCMAP_CM_MAX_FILE_LEN];

   if (NULL == xml_node)
      return QCMAP_CM_ERROR;

   while ( (xml_node != (xmllib_parsetree_node_s_type *)NULL) &&
          (ret_val == QCMAP_CM_SUCCESS) )
   {
      switch (xml_node->nodetype)
      {
         case XMLLIB_PARSETREE_NODE_ELEMENT:
            if (qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                        system_TAG) == 0 ||
                  qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                          odu_cfg_TAG) == 0)
            {
               /* go to child */
               ret_val = qcmap_cm_odu_xml_parse_tree(xml_node->payload.element.child,
                                                  mode);
            }
            else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                                  mode_TAG))
            {
               content = qcmap_cm_read_content_element(xml_node);
               if (content)
               {
                  memset(content_buf, 0, sizeof(content_buf));
                  memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));

                  if (strncasecmp(content_buf, "bridge", content->len) == 0)
                  {
                     *mode = 0;
                  }
                  else
                  {
                      //default mode is router
                      *mode = 1;
                  }
               }
            }
         break;

         default:
            break;
      }
    // go to sibling
    xml_node = xml_node->sibling;
  } // while

  return ret_val;
}


/*===========================================================================
  FUNCTION:  qcmap_cm_ipa_xml_parse_tree_ecm_cat
  ===========================================================================*/
  /*!
      @brief
      This function traverses the xml tree and retrieves the ECM category

      @params
      xml_node [in] - xmllib's parse tree node

      @return
      QCMAP_CM_ERROR
      QCMAP_CM_SUCCESS

  */
/*=========================================================================*/

static int qcmap_cm_ipa_xml_parse_tree_ecm_cat
(
   xmllib_parsetree_node_s_type *xml_node,
   int *mode
)
{
   static int ssid = 0, dhcp = 0;
   int32 ret_val = QCMAP_CM_SUCCESS;
   xmllib_string_s_type* content;
   char content_buf[QCMAP_CM_MAX_FILE_LEN];

   if (NULL == xml_node)
      return QCMAP_CM_ERROR;

   while ( (xml_node != (xmllib_parsetree_node_s_type *)NULL) &&
          (ret_val == QCMAP_CM_SUCCESS) )
   {
      switch (xml_node->nodetype)
      {
         case XMLLIB_PARSETREE_NODE_ELEMENT:
            if (qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                          system_TAG) == 0 ||
                qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                          IPACM_TAG) == 0 ||
                qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                          IPACMIface_TAG) == 0 ||
                qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                          Iface_TAG) == 0)
            {
               /* go to child */
               ret_val = qcmap_cm_ipa_xml_parse_tree_ecm_cat(xml_node->payload.element.child,
                                                  mode);
            }
            else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                                  Name_TAG))
            {
               content = qcmap_cm_read_content_element(xml_node);
               if (content)
               {
                  memset(content_buf, 0, sizeof(content_buf));
                  memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));

                  if (strncasecmp(content_buf, "ecm0", content->len) == 0)
                  {
                      xml_node = xml_node->sibling;
                      content = qcmap_cm_read_content_element(xml_node);
                      if (content)
                      {
                          memset(content_buf, 0, sizeof(content_buf));
                          memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
                          if (strncasecmp(content_buf, "WAN", content->len) == 0)
                          {
                              *mode = 1;
                          }
                          else
                          {
                              //default is LAN
                              *mode = 0;
                          }
                      }
                  }

                  return ret_val;
               }
            }
         break;

         default:
            break;
      }
      // go to sibling
      xml_node = xml_node->sibling;
  } // while

  return ret_val;
}


/*===========================================================================
  FUNCTION:  qcmap_cm_firewall_xml_parse_tree
  ===========================================================================*/
  /*!
      @brief
      This function traverses the firewall xml tree

      @params
      xml_node [in] - xmllib's parse tree node

      @return
      QCMAP_CM_ERROR
      QCMAP_CM_SUCCESS

  */
/*=========================================================================*/

static int qcmap_cm_firewall_xml_parse_tree
(
   xmllib_parsetree_node_s_type *xml_node,
   qcmap_cm_conf_t *config
)
{
   int32 ret_val = QCMAP_CM_SUCCESS;
   xmllib_string_s_type* content;
   char content_buf[QCMAP_CM_MAX_FILE_LEN];
   struct in6_addr ip6_addr;

   ds_assert(config != NULL);

   if (NULL == xml_node)
      return ret_val;

   while ( (xml_node != (xmllib_parsetree_node_s_type *)NULL) &&
          (ret_val == QCMAP_CM_SUCCESS) )
   {
     switch (xml_node->nodetype)
     {
       case XMLLIB_PARSETREE_NODE_ELEMENT:
       {
         if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         system_TAG) ||
             0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         MobileAPFirewallCfg_TAG) ||
             0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         Firewall_TAG)
             )
         {
           if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                 Firewall_TAG))
           {
             // increase firewall entry num
             config->nat_config.num_firewall_entries++;
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]=
               (qcmap_msgr_firewall_conf_t *)calloc(1,sizeof(qcmap_msgr_firewall_conf_t));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries-1]->extd_firewall_entry.firewall_handle
               =config->nat_config.num_firewall_entries-1;
           }

            // go to child
           ret_val = qcmap_cm_firewall_xml_parse_tree(xml_node->payload.element.child,
                                                   config);
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         FirewallEnabled_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
              config->nat_config.firewall_enabled = atoi(content_buf);
           }
         }
          else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         FirewallPktsAllowed_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.firewall_pkts_allowed = atoi(content_buf);
           }
         }
         else if ( config->nat_config.num_firewall_entries > 0 )
         {
            if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                 IPFamily_TAG))
           {
             content = qcmap_cm_read_content_element(xml_node);
             if (content)
             {
               memset(content_buf, 0, sizeof(content_buf));
               memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
               config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_vsn
                 = atoi(content_buf);

             }
           }
           else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                 IPV4SourceAddress_TAG))
           {
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask |=
               IPFLTR_MASK_IP4_SRC_ADDR;

            // go to child
           ret_val = qcmap_cm_firewall_xml_parse_tree(xml_node->payload.element.child,
                                                   config);
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV4SourceIPAddress_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < (sizeof(content_buf)))? content->len : (sizeof(content_buf)-1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v4.src.addr.ps_s_addr
                = (inet_addr(content_buf));

           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV4SourceSubnetMask_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < (sizeof(content_buf)))? content->len : (sizeof(content_buf)-1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v4.src.subnet_mask.ps_s_addr
                = (inet_addr(content_buf));

           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV4DestinationAddress_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask |=
               IPFLTR_MASK_IP4_DST_ADDR;

           // go to child
           ret_val = qcmap_cm_firewall_xml_parse_tree(xml_node->payload.element.child,
                                                   config);
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV4DestinationIPAddress_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < (sizeof(content_buf)))? content->len : (sizeof(content_buf)-1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v4.dst.addr.ps_s_addr
                = (inet_addr(content_buf));
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV4DestinationSubnetMask_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < (sizeof(content_buf)))? content->len : (sizeof(content_buf)-1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v4.dst.subnet_mask.ps_s_addr
                = (inet_addr(content_buf));
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV4TypeOfService_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask |=
               IPFLTR_MASK_IP4_TOS;

           // go to child
           ret_val = qcmap_cm_firewall_xml_parse_tree(xml_node->payload.element.child,
                                                   config);
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TOSValue_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v4.tos.val
                = atoi(content_buf);

           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TOSMask_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v4.tos.mask
                = atoi(content_buf);

           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV4NextHeaderProtocol_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot
                = atoi(content_buf);
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask |= IPFLTR_MASK_IP4_NEXT_HDR_PROT;

           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV6SourceAddress_TAG))
         {
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask |=
               IPFLTR_MASK_IP6_SRC_ADDR;
             // go to child
             ret_val = qcmap_cm_firewall_xml_parse_tree(xml_node->payload.element.child,
                                                   config);
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV6SourceIPAddress_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             inet_pton(AF_INET6,content_buf, &ip6_addr);
             memcpy(config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr8,
                    ip6_addr.s6_addr, QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8));
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV6SourcePrefix_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v6.src.prefix_len
                = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV6DestinationAddress_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask |=
               IPFLTR_MASK_IP6_DST_ADDR;

           // go to child
           ret_val = qcmap_cm_firewall_xml_parse_tree(xml_node->payload.element.child,
                                                   config);
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV6DestinationIPAddress_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             inet_pton(AF_INET6,content_buf, &ip6_addr);
             memcpy(config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v6.dst.addr.in6_u.u6_addr8,
                    ip6_addr.s6_addr, QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8));
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV6DestinationPrefix_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v6.dst.prefix_len
                = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV6TrafficClass_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask |=
               IPFLTR_MASK_IP6_TRAFFIC_CLASS;
           // go to child
           ret_val = qcmap_cm_firewall_xml_parse_tree(xml_node->payload.element.child,
                                                   config);
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TrfClsValue_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.val
                = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TrfClsMask_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.mask
                = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV6NextHeaderProtocol_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v6.next_hdr_prot
                = atoi(content_buf);

           config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask |=IPFLTR_MASK_IP6_NEXT_HDR_PROT;
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCPSource_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp.field_mask |=
               IPFLTR_MASK_TCP_SRC_PORT;
           // go to child
           ret_val = qcmap_cm_firewall_xml_parse_tree(xml_node->payload.element.child,
                                                   config);
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCPSourcePort_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp.src.port
                = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCPSourceRange_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp.src.range
                = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCPDestination_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp.field_mask |=
               IPFLTR_MASK_TCP_DST_PORT;
           // go to child
           ret_val = qcmap_cm_firewall_xml_parse_tree(xml_node->payload.element.child,
                                                   config);
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCPDestinationPort_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.port
                = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCPDestinationRange_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.range
                = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         UDPSource_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.udp.field_mask |=
               IPFLTR_MASK_UDP_SRC_PORT;
           // go to child
           ret_val = qcmap_cm_firewall_xml_parse_tree(xml_node->payload.element.child,
                                                   config);
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         UDPSourcePort_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.udp.src.port
                = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         UDPSourceRange_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.udp.src.range
                = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         UDPDestination_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.udp.field_mask |=
               IPFLTR_MASK_UDP_DST_PORT;
           // go to child
           ret_val = qcmap_cm_firewall_xml_parse_tree(xml_node->payload.element.child,
                                                   config);
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         UDPDestinationPort_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.port
                = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         UDPDestinationRange_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.range
                = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         ICMPType_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.icmp.type
                = atoi(content_buf);
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.icmp.field_mask
                 |= IPFLTR_MASK_ICMP_MSG_TYPE;
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         ICMPCode_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.icmp.code
                = atoi(content_buf);
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.icmp.field_mask
                 |= IPFLTR_MASK_ICMP_MSG_CODE;
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         ESPSPI_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.esp.spi
                = atoi(content_buf);
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.esp.field_mask
                 |= IPFLTR_MASK_ESP_SPI;
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCP_UDPSource_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask |=
               IPFLTR_MASK_TCP_UDP_SRC_PORT;
           // go to child
           ret_val = qcmap_cm_firewall_xml_parse_tree(xml_node->payload.element.child,
                                                   config);
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCP_UDPSourcePort_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.src.port
                = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCP_UDPSourceRange_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
             config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.src.range
                = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCP_UDPDestination_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask |=
               IPFLTR_MASK_UDP_DST_PORT;
           }
           else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                 TCP_UDPDestinationPort_TAG))
           {
             content = qcmap_cm_read_content_element(xml_node);
             if (content)
             {
               memset(content_buf, 0, sizeof(content_buf));
               memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
               config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.port
                 = atoi(content_buf);
             }
           }
           else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                 TCP_UDPDestinationRange_TAG))
           {
             content = qcmap_cm_read_content_element(xml_node);
             if (content)
             {
               memset(content_buf, 0, sizeof(content_buf));
               memcpy(content_buf, (void *)content->string, ((content->len < sizeof(content_buf))? content->len : (sizeof(content_buf) - 1)));
               config->nat_config.extd_firewall_entries[config->nat_config.num_firewall_entries - 1]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.range  = atoi(content_buf);

             }
           }
         }
       }
       break;
         default:
       break;
    }
    // go to sibling
    xml_node = xml_node->sibling;
  } // while
  return ret_val;
}
