/*!
  @file
  qcmap_cm_xml.c

  @brief
  This file implements the XML specific parsing functionality.

*/

/*===========================================================================

  Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
01/28/14   rk      KW fixes.
09/11/12   mp      Adding support for DualAPN in MobileAP.
08/03/12   mp      Adding support for Port Restricted Cone NAT.
07/07/12   sc      Adding support for Tethering mode.
05/04/12   cp      Added static IP configuration for STA mode.
03/02/12   ss      Adding support for Concurrent STA+AP mode.
03/01/12   SDT     Adding support for Dual AP mode.
02/20/12   SDT     Adding support for IPv6.

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
#include "ds_Utils_DebugMsg.h"
#include "qcmap_cm_api.h"
#ifdef USE_GLIB
#include <glib.h>
#define strlcpy g_strlcpy
#endif
/* Private Type definitions */

/* Max allowed size of the XML file (2 MB) */
#define QCMAP_CM_XML_MAX_FILESIZE       (2 << 20)

/* Defines for clipping space or space & quotes (single, double) */
#define QCMAP_CM_XML_CLIP_SPACE         " "
#define QCMAP_CM_XML_CLIP_SPACE_QUOTES  " '\""

#define MAX_XML_STR_LEN                 120

/* All the XML TAG */
#define system_TAG                      "system"
#define MobileAPCfg_TAG                 "MobileAPCfg"
/* NAT Config Entries */
#define IoEMode_TAG                     "IoEMode"
#define MobileAPNatCfg_TAG              "MobileAPNatCfg"
#define TetheringOnly_TAG               "TetheringOnly"
#define NATType_TAG                     "NATType"
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
#define NatEntryTimeout_TAG             "NatEntryTimeout"
#define DmzIP_TAG                       "DmzIP"
#define EnableIPSECVpnPassthrough_TAG   "EnableIPSECVpnPassthrough"
#define EnablePPTPVpnPassthrough_TAG    "EnablePPTPVpnPassthrough"
#define EnableL2TPVpnPassthrough_TAG    "EnableL2TPVpnPassthrough"

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

/* LAN Config Entries */
#define MobileAPLanCfg_TAG              "MobileAPLanCfg"
#define Enable_TAG                      "Enable"
#define Module_TAG                      "Module"
#define Wlan_Gpio_TAG                   "WlanGpioNum"
#define DevMode_TAG                     "DevMode"
#define HostAPDCfg_TAG                  "HostAPDCfg"
#define APIPAddr_TAG                    "APIPAddr"
#define SubNetMask_TAG                  "SubNetMask"
#define AccessProfile_TAG               "AccessProfile"
#define EnableDHCPServer_TAG            "EnableDHCPServer"
#define DHCPCfg_TAG                     "DHCPCfg"
#define DHCPStartIP_TAG                 "StartIP"
#define DHCPEndIP_TAG                   "EndIP"
#define DHCPLeaseTime_TAG               "LeaseTime"
#define EnableSupplicant_TAG            "EnableSupplicant"
#define SupplicantCfg_TAG               "SupplicantCfg"
#define ExternalAPSSID_TAG              "ExternalAPSSID"
#define STAModeConnType_TAG             "STAModeConnType"
#define STAModeStaticIPAddr_TAG         "StaticIPAddr"
#define STAModeStaticConfigDNSAddr_TAG  "StaticConfigDNSAddr"
#define STAModeStaticConfigGWAddr_TAG   "StaticConfigGWAddr"
#define STAModeStaticConfigNetMask_TAG  "StaticConfigNetMask"
#define SSID2Cfg_TAG                    "SSID2Cfg"
#define SSID3Cfg_TAG                    "SSID3Cfg"
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
#define STA_TAG                         "STA"
#define FULL_TAG                        "FULL"
#define INTERNET_TAG                    "INTERNETONLY"

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

   fp = fopen(xml_file, "wb");

   /* File not found at the given path */
   if (NULL == fp)
   {
      printf("qcmap_cm_xml_write_xml_file: unable to open file %s\n",
                    xml_file);
      return QCMAP_CM_ERROR;
   }

   fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
   fprintf(fp, "<system xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"qcmap_cfg.xsd\">\n");
   fprintf(fp, "<MobileAPCfg>\n");
   // NAT Config
   fprintf(fp, "\t<MobileAPNatCfg>\n");
   fprintf(fp, "\t<IoEMode>%d</IoEMode>\n", config->nat_config.ioe_mode);
   fprintf(fp, "\t<TetheringOnly>%d</TetheringOnly>\n", config->nat_config.tethering_only);
   fprintf(fp, "\t<NATType>%s</NATType>\n", ((int)config->nat_config.nat_type ? "PRC":"SYM"));
   fprintf(fp, "\t<FirewallEnabled>%d</FirewallEnabled>\n", config->nat_config.firewall_enabled);
   fprintf(fp, "\t<FirewallPktsAllowed>%d</FirewallPktsAllowed>\n", config->nat_config.firewall_pkts_allowed);
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
   if (config->nat_config.nat_entry_timeout > 0)
   {
     fprintf(fp, "\t<NatEntryTimeout>%d</NatEntryTimeout>\n", config->nat_config.nat_entry_timeout);
   }
   addr.s_addr = htonl(config->nat_config.dmz_ip);
   fprintf(fp, "\t<DmzIP>%s</DmzIP>\n", inet_ntoa(addr));
   fprintf(fp, "\t<EnableIPSECVpnPassthrough>%d</EnableIPSECVpnPassthrough>\n", config->nat_config.enable_ipsec_vpn_pass_through);
   fprintf(fp, "\t<EnablePPTPVpnPassthrough>%d</EnablePPTPVpnPassthrough>\n", config->nat_config.enable_pptp_vpn_pass_through);
   fprintf(fp, "\t<EnableL2TPVpnPassthrough>%d</EnableL2TPVpnPassthrough>\n", config->nat_config.enable_l2tp_vpn_pass_through);
   fprintf(fp, "</MobileAPNatCfg>\n");
   // LAN Config
   fprintf(fp, "<MobileAPLanCfg>\n");
   fprintf(fp, "\t<Module>%s</Module>\n", config->lan_config.module);
   if(config->lan_config.wlan_gpio_num != 0)
   {
     fprintf(fp, "\t<WlanGpioNum>%d</WlanGpioNum>\n", config->lan_config.wlan_gpio_num);
   }
   fprintf(fp, "\t<DevMode>%s</DevMode>\n", (config->lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].devmode == QCMAP_CM_DEVMODE_AP) ? AP_TAG : STA_TAG);
   fprintf(fp, "\t<HostAPDCfg>%s</HostAPDCfg>\n", config->lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].path_to_hostapd_conf);
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].a5_ip_addr);
   fprintf(fp, "\t<APIPAddr>%s</APIPAddr>\n", inet_ntoa(addr));
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].sub_net_mask);
   fprintf(fp, "\t<SubNetMask>%s</SubNetMask>\n", inet_ntoa(addr));
   fprintf(fp, "\t<AccessProfile>%s</AccessProfile>\n", (config->lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].access_profile == QCMAP_CM_PROFILE_FULL_ACCESS) ? FULL_TAG : INTERNET_TAG);
   fprintf(fp, "\t<EnableDHCPServer>%d</EnableDHCPServer>\n", config->lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].enable_dhcpd);
   fprintf(fp, "\t<DHCPCfg>\n");
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].dhcp_start_address);
   fprintf(fp, "\t\t<StartIP>%s</StartIP>\n", inet_ntoa(addr));
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].dhcp_end_address);
   fprintf(fp, "\t\t<EndIP>%s</EndIP>\n", inet_ntoa(addr));
   fprintf(fp, "\t\t<LeaseTime>%s</LeaseTime>\n", config->lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].dhcp_lease_time);
   fprintf(fp, "\t</DHCPCfg>\n");

   /* <SSID2Cfg> */
   fprintf(fp, "\t<SSID2Cfg>\n");
   fprintf(fp, "\t\t<Enable>%d</Enable>\n", config->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].enable);
   fprintf(fp, "\t\t<DevMode>%s</DevMode>\n", (config->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].devmode == QCMAP_CM_DEVMODE_AP) ? AP_TAG : STA_TAG);
   fprintf(fp, "\t\t<HostAPDCfg>%s</HostAPDCfg>\n", config->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].path_to_hostapd_conf);
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].a5_ip_addr);
   fprintf(fp, "\t\t<APIPAddr>%s</APIPAddr>\n", inet_ntoa(addr));
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].sub_net_mask);
   fprintf(fp, "\t\t<SubNetMask>%s</SubNetMask>\n", inet_ntoa(addr));
   fprintf(fp, "\t\t<AccessProfile>%s</AccessProfile>\n", (config->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].access_profile  == QCMAP_CM_PROFILE_FULL_ACCESS) ? FULL_TAG : INTERNET_TAG);
   fprintf(fp, "\t\t<EnableDHCPServer>%d</EnableDHCPServer>\n", config->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].enable_dhcpd);
   fprintf(fp, "\t\t<DHCPCfg>\n");
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].dhcp_start_address);
   fprintf(fp, "\t\t\t<StartIP>%s</StartIP>\n", inet_ntoa(addr));
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].dhcp_end_address);
   fprintf(fp, "\t\t\t<EndIP>%s</EndIP>\n", inet_ntoa(addr));
   fprintf(fp, "\t\t\t<LeaseTime>%s</LeaseTime>\n", config->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].dhcp_lease_time);
   fprintf(fp, "\t\t</DHCPCfg>\n");
   fprintf(fp, "\t\t<EnableSupplicant>%d</EnableSupplicant>\n", config->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].enable_supplicant);
   fprintf(fp, "\t\t<SupplicantCfg>%s</SupplicantCfg>\n", config->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].path_to_supplicant_conf);
   fprintf(fp, "\t\t<ExternalAPSSID>%s</ExternalAPSSID>\n", config->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].external_ap_ssid);
   fprintf(fp, "\t\t<STAModeConnType>%d</STAModeConnType>\n", config->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].conn_type);
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].static_ip_config.ip_addr);
   fprintf(fp, "\t\t<StaticIPAddr>%s</StaticIPAddr>\n", inet_ntoa(addr));
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].static_ip_config.dns_addr);
   fprintf(fp, "\t\t<StaticConfigDNSAddr>%s</StaticConfigDNSAddr>\n", inet_ntoa(addr));
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].static_ip_config.gw_ip);
   fprintf(fp, "\t\t<StaticConfigGWAddr>%s</StaticConfigGWAddr>\n", inet_ntoa(addr));
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].static_ip_config.netmask);
   fprintf(fp, "\t\t<StaticConfigNetMask>%s</StaticConfigNetMask>\n", inet_ntoa(addr));
   fprintf(fp, "\t</SSID2Cfg>\n");

   /* <SSID3Cfg> */
   fprintf(fp, "\t<SSID3Cfg>\n");
   fprintf(fp, "\t\t<Enable>%d</Enable>\n", config->lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].enable);
   fprintf(fp, "\t\t<DevMode>%s</DevMode>\n", (config->lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].devmode == QCMAP_CM_DEVMODE_AP) ? AP_TAG : STA_TAG);
   fprintf(fp, "\t\t<HostAPDCfg>%s</HostAPDCfg>\n", config->lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].path_to_hostapd_conf);
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].a5_ip_addr);
   fprintf(fp, "\t\t<APIPAddr>%s</APIPAddr>\n", inet_ntoa(addr));
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].sub_net_mask);
   fprintf(fp, "\t\t<SubNetMask>%s</SubNetMask>\n", inet_ntoa(addr));
   fprintf(fp, "\t\t<AccessProfile>%s</AccessProfile>\n", (config->lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].access_profile  == QCMAP_CM_PROFILE_FULL_ACCESS) ? FULL_TAG : INTERNET_TAG);
   fprintf(fp, "\t\t<EnableDHCPServer>%d</EnableDHCPServer>\n", config->lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].enable_dhcpd);
   fprintf(fp, "\t\t<DHCPCfg>\n");
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].dhcp_start_address);
   fprintf(fp, "\t\t\t<StartIP>%s</StartIP>\n", inet_ntoa(addr));
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].dhcp_end_address);
   fprintf(fp, "\t\t\t<EndIP>%s</EndIP>\n", inet_ntoa(addr));
   fprintf(fp, "\t\t\t<LeaseTime>%s</LeaseTime>\n", config->lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].dhcp_lease_time);
   fprintf(fp, "\t\t</DHCPCfg>\n");
   fprintf(fp, "\t\t<EnableSupplicant>%d</EnableSupplicant>\n", config->lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].enable_supplicant);
   fprintf(fp, "\t\t<SupplicantCfg>%s</SupplicantCfg>\n", config->lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].path_to_supplicant_conf);
   fprintf(fp, "\t\t<ExternalAPSSID>%s</ExternalAPSSID>\n", config->lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].external_ap_ssid);
   fprintf(fp, "\t\t<STAModeConnType>%d</STAModeConnType>\n", config->lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].conn_type);
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].static_ip_config.ip_addr);
   fprintf(fp, "\t\t<StaticIPAddr>%s</StaticIPAddr>\n", inet_ntoa(addr));
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].static_ip_config.dns_addr);
   fprintf(fp, "\t\t<StaticConfigDNSAddr>%s</StaticConfigDNSAddr>\n", inet_ntoa(addr));
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].static_ip_config.gw_ip);
   fprintf(fp, "\t\t<StaticConfigGWAddr>%s</StaticConfigGWAddr>\n", inet_ntoa(addr));
   addr.s_addr = htonl(config->lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].static_ip_config.netmask);
   fprintf(fp, "\t\t<StaticConfigNetMask>%s</StaticConfigNetMask>\n", inet_ntoa(addr));
   fprintf(fp, "\t</SSID3Cfg>\n");

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
   if (config->wan_config.ip_family == QCMAP_IP_V4_V01)
     fprintf(fp, "\t<IPFamily>%s</IPFamily>\n", IPV4_TAG);
   if (config->wan_config.ip_family == QCMAP_IP_V6_V01)
     fprintf(fp, "\t<IPFamily>%s</IPFamily>\n", IPV6_TAG);
   if (config->wan_config.ip_family == QCMAP_IP_V4V6_V01)
     fprintf(fp, "\t<IPFamily>%s</IPFamily>\n", IPV4V6_TAG);
   fprintf(fp, "</MobileAPWanCfg>\n");
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
   char ip6_addr_input[48];
   int next_hdr_prot;

   ds_assert(xml_file != NULL);
   ds_assert(config != NULL);

   fp = fopen(xml_file, "wb");

   /* File not found at the given path */
   if (NULL == fp)
   {
      printf("qcmap_cm_xml_write_xml_file: unable to open file %s\n",
                    xml_file);
      return QCMAP_CM_ERROR;
   }

   fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
   fprintf(fp, 
           "<system xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"mobileap_firewall_cfg.xsd\">\n");
   fprintf(fp, "\t<MobileAPFirewallCfg>\n");
   if (config->nat_config.num_extd_firewall_entries > 0)
   {
     for (i = 0; i < config->nat_config.num_extd_firewall_entries; i++)
     {
       next_hdr_prot = PS_NO_NEXT_HDR;
       fprintf(fp, "\t\t<Firewall>\n");
       fprintf(fp, 
               "\t\t\t<IPFamily>%d</IPFamily>\n",config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_vsn);

       if(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_vsn == IP_V4)
       {
           if(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask & 
              IPFLTR_MASK_IP4_SRC_ADDR)
           {
               fprintf(fp, "\t\t\t<IPV4SourceAddress>\n");
               addr.s_addr = 
                   htonl(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v4.src.addr.ps_s_addr);
               fprintf(fp, "\t\t\t\t<IPV4SourceIPAddress>%s</IPV4SourceIPAddress>\n",inet_ntoa(addr));
               addr.s_addr = 
                   htonl(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v4.src.subnet_mask.ps_s_addr);
               fprintf(fp, "\t\t\t\t<IPV4SourceSubnetMask>%s</IPV4SourceSubnetMask>\n",inet_ntoa(addr));
               fprintf(fp, "\t\t\t</IPV4SourceAddress>\n");
           }

           if(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask & 
              IPFLTR_MASK_IP4_DST_ADDR)
           {
               fprintf(fp, "\t\t\t<IPV4DestinationAddress>\n");
               addr.s_addr = 
                   htonl(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v4.dst.addr.ps_s_addr);
               fprintf(fp, "\t\t\t\t<IPV4DestinationIPAddress>%s</IPV4DestinationIPAddress>\n",inet_ntoa(addr));
               addr.s_addr = 
                   htonl(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v4.dst.subnet_mask.ps_s_addr);
               fprintf(fp, "\t\t\t\t<IPV4DestinationSubnetMask>%s</IPV4DestinationSubnetMask>\n",inet_ntoa(addr));
               fprintf(fp, "\t\t\t</IPV4DestinationAddress>\n");
           }

           if(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask & 
              IPFLTR_MASK_IP4_TOS)
           {
               fprintf(fp, "\t\t\t<IPV4TypeOfService>\n");
               fprintf(fp, "\t\t\t\t<TOSValue>%d</TOSValue>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v4.tos.val);
               fprintf(fp, "\t\t\t\t<TOSMask>%d</TOSMask>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v4.tos.mask);
               fprintf(fp, "\t\t\t</IPV4TypeOfService>\n");
           }

           fprintf(fp, "\t\t\t<IPV4NextHeaderProtocol>%d</IPV4NextHeaderProtocol>\n",
                   config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot);

           next_hdr_prot = 
               config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot;
       }

       if(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_vsn == IP_V6)
       {
           if(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask & 
              IPFLTR_MASK_IP6_SRC_ADDR)
           {
               fprintf(fp, "\t\t\t<IPV6SourceAddress>\n");
               memcpy(ip6_addr.s6_addr,
                      config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr8,
                      QCMAP_IPV6_ADDR_LEN_V01*sizeof(uint8));
               fprintf(fp, "\t\t\t\t<IPV6SourceIPAddress>%s</IPV6SourceIPAddress>\n",
                       inet_ntop(AF_INET6,&ip6_addr,ip6_addr_input,sizeof(ip6_addr)));
               fprintf(fp, "\t\t\t\t<IPV6SourcePrefix>%d</IPV6SourcePrefix>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v6.src.prefix_len);
               fprintf(fp, "\t\t\t</IPV6SourceAddress>\n");
           }

           if(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask & 
              IPFLTR_MASK_IP6_DST_ADDR)
           {
               fprintf(fp, "\t\t\t<IPV6DestinationAddress>\n");
               memcpy(ip6_addr.s6_addr,
                      config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v6.dst.addr.in6_u.u6_addr8,
                      QCMAP_IPV6_ADDR_LEN_V01*sizeof(uint8));
               fprintf(fp, "\t\t\t\t<IPV6DestinationIPAddress>%s</IPV6DestinationIPAddress>\n",
                       inet_ntop(AF_INET6,&ip6_addr,ip6_addr_input,sizeof(ip6_addr)));
               fprintf(fp, "\t\t\t\t<IPV6DestinationPrefix>%d</IPV6SetinationPrefix>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v6.dst.prefix_len);
               fprintf(fp, "\t\t\t</IPV6DestinationAddress>\n");
           }

           if(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask & 
              IPFLTR_MASK_IP6_TRAFFIC_CLASS)
           {
               fprintf(fp, "\t\t\t<IPV6TrafficClass>\n");
               fprintf(fp, "\t\t\t\t<TrfClsValue>%d</TrfClsValue>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.val);
               fprintf(fp, "\t\t\t\t<TrfClsMask>%d</TrfClsMask>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.mask);
               fprintf(fp, "\t\t\t</IPV6TrafficClass>\n");
           }

           fprintf(fp, "\t\t\t<IPV6NextHeaderProtocol>%d</IPV6NextHeaderProtocol>\n",
                   config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v6.next_hdr_prot);
           next_hdr_prot = 
               config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v6.next_hdr_prot;
       }

       switch(next_hdr_prot)
       {
           case PS_IPPROTO_TCP:
             if(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.field_mask & 
                IPFLTR_MASK_TCP_SRC_PORT )
             {
               fprintf(fp, "\t\t\t<TCPSource>\n");
               fprintf(fp, "\t\t\t\t<TCPSourcePort>%d</TCPSourcePort>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.src.port);
               fprintf(fp, "\t\t\t\t<TCPSourceRange>%d</TCPSourceRange>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.src.range);
               fprintf(fp, "\t\t\t</TCPSource>\n");
             }

             if(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.field_mask & 
                IPFLTR_MASK_TCP_DST_PORT)
             {
               fprintf(fp, "\t\t\t<TCPDestination>\n");
               fprintf(fp, "\t\t\t\t<TCPDestinationPort>%d</TCPDestinationPort>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.port);
               fprintf(fp, "\t\t\t\t<TCPDestinationRange>%d</TCPDestinationRange>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.range);
               fprintf(fp, "\t\t\t</TCPDestination>\n");
             }
             break;

           case PS_IPPROTO_UDP:
             if(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.udp.field_mask & 
                IPFLTR_MASK_UDP_SRC_PORT)
             {
               fprintf(fp, "\t\t\t<UDPSource>\n");
               fprintf(fp, "\t\t\t\t<UDPSourcePort>%d</UDPSourcePort>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.udp.src.port);
               fprintf(fp, "\t\t\t\t<UDPSourceRange>%d</UDPSourceRange>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.udp.src.range);
               fprintf(fp, "\t\t\t</UDPSource>\n");
             }

             if(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.udp.field_mask & 
                IPFLTR_MASK_UDP_DST_PORT)
             {
               fprintf(fp, "\t\t\t<UDPDestination>\n");
               fprintf(fp, "\t\t\t\t<UDPDestinationPort>%d</UDPDestinationPort>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.port);
               fprintf(fp, "\t\t\t\t<UDPDestinationRange>%d</UDPDestinationRange>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.range);
               fprintf(fp, "\t\t\t</UDPDestination>\n");
             }
             break;

           case PS_IPPROTO_ICMP:
           case PS_IPPROTO_ICMP6:
             if(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.icmp.field_mask & 
                IPFLTR_MASK_ICMP_MSG_TYPE)
             {
               fprintf(fp, "\t\t\t<ICMPType>%d</ICMPType>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.icmp.type);
             }

             if(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.icmp.field_mask & 
                IPFLTR_MASK_ICMP_MSG_CODE)
             {
               fprintf(fp, "\t\t\t<ICMPCode>%d</ICMPCode>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.icmp.code);
             }
             break;

           case PS_IPPROTO_ESP:
             if(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.esp.field_mask & 
                IPFLTR_MASK_ESP_SPI)
             {
               fprintf(fp, "\t\t\t<ESPSPI>%d</ESPSPI>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.esp.spi);
             }
             break;

           case PS_IPPROTO_TCP_UDP:
             if(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask & 
              IPFLTR_MASK_TCP_UDP_SRC_PORT)
             {
               fprintf(fp, "\t\t\t<TCP_UDPSource>\n");
               fprintf(fp, "\t\t\t\t<TCP_UDPSourcePort>%d</TCP_UDPSourcePort>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.src.port);
               fprintf(fp, "\t\t\t\t<TCP_UDPSourceRange>%d</TCP_UDPSourceRange>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.src.range);
               fprintf(fp, "\t\t\t</TCP_UDPSource>\n");
             }

             if(config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask & 
              IPFLTR_MASK_TCP_UDP_DST_PORT)
             {
               fprintf(fp, "\t\t\t<TCP_UDPDestination>\n");
               fprintf(fp, "\t\t\t\t<TCP_UDPDestinationPort>%d</TCP_UDPDestinationPort>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.port);
               fprintf(fp, "\t\t\t\t<TCP_UDPDestinationRange>%d</TCP_UDPDestinationRange>\n",
                       config->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.range);
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
      memset(config, 0, sizeof(qcmap_cm_conf_t));

      /* Force the first interface to be enabled. */
      config->lan_config.interface[0].enable = TRUE;

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
            ((cur_pos = ftell(fp)) < 0 || cur_pos > QCMAP_CM_XML_MAX_FILESIZE) )
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
   char content_buf[MAX_XML_STR_LEN];

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
                                         SSID2Cfg_TAG) ||
             0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         SSID3Cfg_TAG) ||
             0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         MobileAPWanCfg_TAG)
             )
         {
           if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         PortFwding_TAG))
           {
             // increase port forwarding entry num
             config->nat_config.num_port_fwding_entries++;
           }
           if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         SSID2Cfg_TAG))
           {
             // Put the network config in the SSID2 bin
             ssid = 1;
           }
           if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         SSID3Cfg_TAG))
           {
             // Put the network config in the SSID3 bin
             ssid = 2;
           }
           if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         DHCPCfg_TAG))
           {
             // Updating DHCP config, don't change SSID number on exit
             dhcp = 1;
           }
           // go to child
           ret_val = qcmap_cm_xml_parse_tree(xml_node->payload.element.child,
                                                   config);
           // Put any remaining network config into the main SSID bin
           if (dhcp) dhcp = 0;
           else if (ssid) ssid = 0;
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         Enable_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->lan_config.interface[ssid].enable = atoi(content_buf);
             printf("Enable %d ssid %d\n", config->lan_config.interface[ssid].enable, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         Module_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             strlcpy(config->lan_config.module, content_buf, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             printf("Module %s\n", config->lan_config.module);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         Wlan_Gpio_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->lan_config.wlan_gpio_num = atoi(content_buf);
             printf("WlanGpio %d\n", config->lan_config.wlan_gpio_num);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         DevMode_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             if (strncasecmp(AP_TAG, content_buf, MIN(sizeof(AP_TAG) - 1, content->len)) == 0)
             {
               config->lan_config.interface[ssid].devmode = QCMAP_CM_DEVMODE_AP;
             }
             else if (strncasecmp("STA", content_buf, MIN(sizeof(STA_TAG) - 1, content->len)) == 0)
             {
               config->lan_config.interface[ssid].devmode = QCMAP_CM_DEVMODE_STA;
             }
             printf("DevMode %d ssid %d\n", config->lan_config.interface[ssid].devmode, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         AccessProfile_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             if (strncasecmp(FULL_TAG, content_buf, MIN(sizeof(FULL_TAG) - 1, content->len)) == 0)
             {
               config->lan_config.interface[ssid].access_profile = QCMAP_CM_PROFILE_FULL_ACCESS;
             }
             else if (strncasecmp(INTERNET_TAG, content_buf, MIN(sizeof(INTERNET_TAG) - 1, content->len)) == 0)
             {
               config->lan_config.interface[ssid].access_profile = QCMAP_CM_PROFILE_INTERNET_ONLY;
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             strlcpy(config->lan_config.interface[ssid].path_to_hostapd_conf,
                     content_buf, ((content->len < QCMAP_CM_MAX_FILE_LEN)?
                     ((content->len)+1) : (QCMAP_CM_MAX_FILE_LEN)));
             printf("HostAPD config %s ssid %d", config->lan_config.interface[ssid].path_to_hostapd_conf, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         APIPAddr_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             if (inet_aton(content_buf, &addr))
               config->lan_config.interface[ssid].dhcp_end_address = ntohl(addr.s_addr);
             printf("DHCPStartIP %x ssid %d\n", config->lan_config.interface[ssid].dhcp_end_address, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         DHCPLeaseTime_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             strlcpy(config->lan_config.interface[ssid].dhcp_lease_time,
                     content_buf, ((content->len < QCMAP_CM_MAX_FILE_LEN)?
                     ((content->len)+1) : (QCMAP_CM_MAX_FILE_LEN)));
             printf("LeaseTime %s ssid %d\n", config->lan_config.interface[ssid].dhcp_lease_time, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         EnableSupplicant_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->lan_config.interface[ssid].enable_supplicant= atoi(content_buf);
             printf("EnableSupplicant %d ssid %d\n", config->lan_config.interface[ssid].enable_supplicant, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         SupplicantCfg_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             strlcpy(config->lan_config.interface[ssid].path_to_supplicant_conf,
                     content_buf, ((content->len < QCMAP_CM_MAX_FILE_LEN)?
                     ((content->len)+1) : (QCMAP_CM_MAX_FILE_LEN)));
             printf("SupplicantCfg %s ssid %d\n", config->lan_config.interface[ssid].path_to_supplicant_conf, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         ExternalAPSSID_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             strlcpy(config->lan_config.interface[ssid].external_ap_ssid,
                     content_buf, ((content->len < QCMAP_CM_MAX_FILE_LEN)?
                     ((content->len)+1) : (QCMAP_CM_MAX_FILE_LEN)));
             printf("ExternalAPSSID %s ssid %d\n", config->lan_config.interface[ssid].external_ap_ssid, ssid);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         STAModeConnType_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->lan_config.interface[ssid].conn_type = atoi(content_buf);
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             strlcpy(config->wan_config.eri_config_file, content_buf,
                     ((content->len < QCMAP_CM_MAX_FILE_LEN)? ((content->len)+1)
                     : (QCMAP_CM_MAX_FILE_LEN)));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
                                         IPFamily_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             if (0 == strcasecmp(content_buf, IPV4_TAG))
             {
               config->wan_config.ip_family = 4;
             }
             if (0 == strcasecmp(content_buf, IPV6_TAG))
             {
               LOG_MSG_INFO1("Qualcomm Mobile AP does not support IPv6 only. Setting IPv4v6", content_buf, 0, 0);
               config->wan_config.ip_family = 6;
             }
             if (0 == strcasecmp(content_buf, IPV4V6_TAG))
             {
               config->wan_config.ip_family = 10;
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.port_fwding_entries[config->nat_config.num_port_fwding_entries - 1].port_fwding_protocol
                = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IoEMode_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             memcpy(content_buf, (void *)content->string, content->len);
             config->nat_config.ioe_mode = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TetheringOnly_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.tethering_only = atoi(content_buf);
           }
         } 
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         NATType_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             if (strncasecmp("PRC", content_buf, MIN(sizeof("PRC") - 1, content->len)) == 0)
             {
               config->nat_config.nat_type = QCMAP_CM_PORT_RESTRICTED_CONE_NAT;
             }
             else
             {
               config->nat_config.nat_type = QCMAP_CM_SYMMETRIC_NAT;
             }
             printf("NAT Type %s \n", (config->nat_config.nat_type ? "PRC":"SYM"));
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         FirewallEnabled_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.firewall_pkts_allowed = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         Firewall_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             strlcpy(config->nat_config.firewall_config_file, content_buf,
                     ((content->len < QCMAP_CM_MAX_FILE_LEN)? ((content->len)+1)
                     : (QCMAP_CM_MAX_FILE_LEN)));
             printf("Firewall config xml %s\n", config->nat_config.firewall_config_file);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         NatEntryTimeout_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.nat_entry_timeout = atoi(content_buf);
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         DmzIP_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.enable_l2tp_vpn_pass_through = atoi(content_buf);
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
   char content_buf[MAX_XML_STR_LEN];
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
             config->nat_config.num_extd_firewall_entries++;
           }

            // go to child
           ret_val = qcmap_cm_firewall_xml_parse_tree(xml_node->payload.element.child,
                                                   config);
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPFamily_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_vsn
                = atoi(content_buf); 
             printf("\n IP family type is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_vsn);

           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV4SourceAddress_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask |=
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v4.src.addr.ps_s_addr 
                = ntohl(inet_addr(content_buf)); 

             printf("\n IPv4 source address is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v4.src.addr.ps_s_addr);
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV4SourceSubnetMask_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v4.src.subnet_mask.ps_s_addr 
                = ntohl(inet_addr(content_buf));

             printf("\n IPv4 source subnet mask is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v4.src.subnet_mask.ps_s_addr);
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV4DestinationAddress_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask |=
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v4.dst.addr.ps_s_addr 
                = ntohl(inet_addr(content_buf)); 
             printf("\n IPv4 destination address is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v4.dst.addr.ps_s_addr );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV4DestinationSubnetMask_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v4.dst.subnet_mask.ps_s_addr 
                = ntohl(inet_addr(content_buf)); 
             printf("\n IPV4 destination subnet mask is %d \n",config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v4.dst.subnet_mask.ps_s_addr);
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV4TypeOfService_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask |=
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v4.tos.val 
                = atoi(content_buf); 

             printf("\n IPV4 TOS val is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v4.tos.val );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TOSMask_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v4.tos.mask 
                = atoi(content_buf); 

             printf("\n IPv4 TOS mask is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v4.tos.mask );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV4NextHeaderProtocol_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot 
                = atoi(content_buf); 
             printf("\n IPv4 next header prot is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot);
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV6SourceAddress_TAG))
         {
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask |=
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             inet_pton(AF_INET6,content_buf, &ip6_addr);
             memcpy(config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr8,
                    ip6_addr.s6_addr, QCMAP_IPV6_ADDR_LEN_V01*sizeof(uint8)); 

             printf("\n ipv6 source addr is %d \n ",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr8[0] );
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV6SourcePrefix_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v6.src.prefix_len 
                = atoi(content_buf); 
             printf("\n ipv6 source prefix is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v6.src.prefix_len  );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV6DestinationAddress_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask |=
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             inet_pton(AF_INET6,content_buf, &ip6_addr);
             memcpy(config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v6.dst.addr.in6_u.u6_addr8,
                    ip6_addr.s6_addr, QCMAP_IPV6_ADDR_LEN_V01*sizeof(uint8)); 
             printf("\n ipv6 dest addr is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v6.dst.addr.in6_u.u6_addr8[0]);
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV6DestinationPrefix_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v6.dst.prefix_len 
                = atoi(content_buf); 
             printf("\n ipv6 dest prefix is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v6.dst.prefix_len );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV6TrafficClass_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask |=
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.val 
                = atoi(content_buf); 
             printf("\n ipv6 trf class val is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.val );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TrfClsMask_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.mask 
                = atoi(content_buf); 
             printf("\n ipv6 trf class mask is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.mask );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         IPV6NextHeaderProtocol_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v6.next_hdr_prot 
                = atoi(content_buf); 
             printf("\n ipv6 next header protocol is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.ip_hdr.v6.next_hdr_prot );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCPSource_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.field_mask |=
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.src.port 
                = atoi(content_buf); 
             printf("\n tcp src port is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.src.port );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCPSourceRange_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.src.range 
                = atoi(content_buf); 
             printf("\n tcp src range is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.src.range );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCPDestination_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.field_mask |=
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.port 
                = atoi(content_buf); 
             printf("\n tcp dest port is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.port );
           }
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCPDestinationRange_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.range 
                = atoi(content_buf); 
             printf("\n tcp dest range is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.range );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         UDPSource_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.udp.field_mask |=
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.udp.src.port 
                = atoi(content_buf); 
             printf("\n udp src port is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.udp.src.port );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         UDPSourceRange_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.udp.src.range 
                = atoi(content_buf); 
             printf("\n udp src range is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.udp.src.range );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         UDPDestination_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.udp.field_mask |=
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.port 
                = atoi(content_buf); 
             printf("\n udp dest port is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.port );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         UDPDestinationRange_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.range 
                = atoi(content_buf); 
             printf("\n udp dest range is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.range );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         ICMPType_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.icmp.type 
                = atoi(content_buf); 
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.icmp.field_mask
                 |= IPFLTR_MASK_ICMP_MSG_TYPE;
             printf("\n icmp type is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.icmp.type );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         ICMPCode_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.icmp.code 
                = atoi(content_buf); 
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.icmp.field_mask
                 |= IPFLTR_MASK_ICMP_MSG_CODE;
             printf("\n icmp code is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.icmp.code ); 
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         ESPSPI_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.esp.spi 
                = atoi(content_buf); 
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.esp.field_mask
                 |= IPFLTR_MASK_ESP_SPI; 
             printf("\n esp spi is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.esp.spi );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCP_UDPSource_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask |=
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
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.src.port 
                = atoi(content_buf); 
             printf("\n tcp_udp src port is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.src.port );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCP_UDPSourceRange_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.src.range 
                = atoi(content_buf); 
             printf("\n tcp_udp src range is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.src.range );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCP_UDPDestination_TAG))
         {
           config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask |=
               IPFLTR_MASK_UDP_DST_PORT; 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCP_UDPDestinationPort_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.port 
                = atoi(content_buf); 
             printf("\n tcp_udp dest port is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.port );
           } 
         }
         else if (0 == qcmap_cm_util_icmp_string(&xml_node->payload.element.name,
                                         TCP_UDPDestinationRange_TAG))
         {
           content = qcmap_cm_read_content_element(xml_node);
           if (content)
           {
             memset(content_buf, 0, sizeof(content_buf));
             strlcpy(content_buf, content->string, ((content->len <
                     sizeof(content_buf))? ((content->len)+1)
                     : (sizeof(content_buf))));
             config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.range 
                = atoi(content_buf); 
             printf(" \n tcp_udp dest range is %d \n",
                    config->nat_config.extd_firewall_entries[config->nat_config.num_extd_firewall_entries - 1].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.range);
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
