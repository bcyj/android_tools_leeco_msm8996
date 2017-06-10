#ifndef _QCMAP_CONNECTION_MANAGER_H_
#define _QCMAP_CONNECTION_MANAGER_H_

/*====================================================

FILE:  QCMAP_ConnectionManager.h

SERVICES:
   QCMAP Connection Manager Class

=====================================================

Copyright (c) 2011-2012 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary

=====================================================*/
/*===========================================================================
  EDIT HISTORY FOR MODULE

  Please notice that the changes are listed in reverse chronological order.

  when       who        what, where, why
  --------   ---        -------------------------------------------------------
  02/20/14   at         Fix for IPv6 address accumulation on rmnet interface.
  10/03/12   mp         Fix to prevent QCMAP starting call before QTI Link UP.
  10/03/12   mp         Fix to make QTI use QCMAP WWAN config.
  08/31/12   mp         Added support for Extended Firewalls in AP+STA.
  08/03/12   mp         Added support for Port Restricted Cone NAT.
  07/19/12   cp         Added support for mcast routing.
  07/07/12   SC         Added support for tethering call bringup.  
  06/27/12   cp         Enabled VPN passthrough on A5 side.
  06/25/12   mp         Added support for Firewall functionalities in AP+STA.
  06/14/12   rk         Fixed compilation warnings and Embedded call issues.
  05/18/12   vb         Added support for embedded call bringup.
  05/04/12   cp         Added static IP configuration for STA mode.
  04/16/12   sb         IPV6 state machine fix
  03/09/12   sb         Adding extended firewall support.
  03/02/12   ss         Adding support for Concurrent STA+AP mode.
  03/01/12   SDT        Adding support for Dual AP mode.
  02/20/12   SDT        Adding support for IPv6.

===========================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/select.h>
#include <pthread.h>
#include "stringl.h"
#include "ds_util.h"
#include "comdef.h"
#include "amssassert.h"
#include "dssocket.h"
#include "ps_iface_defs.h"
#include "ds_Utils_DebugMsg.h"
#include "dsnet_lib.h"
#include "dssock_lib.h"
#include "dss_init.h"
#include "qcmap_cm_api.h"

//===================================================================
//              Class Definitions
//===================================================================

#define QCMAP_LAN_CONNECTED_TIMEOUT 60 // 60 seconds for LAN connected
#define QCMAP_WAN_CONNECTED_TIMEOUT 60 // 60 seconds for WAN connected

class QCMAP_ConnectionManager
{
public:

   QCMAP_ConnectionManager(char *xml_path = NULL);
   virtual ~QCMAP_ConnectionManager(void);
   /* ---------------------------MobileAP Execution---------------------------*/

   /* Enable MobileAP */
   boolean Enable(void);

   /* Disable MobileAP */
   boolean Disable(void);

   /* Enable STA mode (Concurrent AP+STA) */
   boolean EnableStaMode(qcmap_sta_connection_config *cfg);

   /* Disable STA mode */
   boolean DisableStaMode(void);

   /* ConnectBackHaul */
   boolean ConnectBackHaul(void);

  /* Enable DNS */
   boolean EnableDNS(void);

   /* DisconnectBackHual */
   boolean DisconnectBackHaul(void);

   /* IPv4v6 family Only */
   boolean IPv4v6Only(void);

   /*IoEMode*/
   boolean IoEMode(void);

   /* TetheringOnly */
   boolean TetheringOnly(void);

   /* Send MobileAP WWAN config to QTI */
   boolean SendWanConfig(void);

   /* -----------------------------Modem Config-------------------------------*/

   /* Display the current modem configuration. */
   boolean DisplayModemConfig(void);

   /* Add a Static Nat Entry to the configuration and update XML file. */
   boolean AddStaticNatEntry(qcmap_cm_port_fwding_entry_conf_t* nat_entry);

   /* Delete a Static Nat Entry from the configuration and update XML file. */
   boolean DeleteStaticNatEntry(qcmap_cm_port_fwding_entry_conf_t* nat_entry);

   /* Get a Static Nat Entry from the configuration. */
   int GetStaticNatEntry(qcmap_cm_port_fwding_entry_conf_t* nat_entry,
                         int max_entries);

   /* Add a FireWall Entry rule to the configuration and update XML file.*/
   boolean AddFireWallEntry(qcmap_cm_firewall_entry_conf_t* firewall_entry);

   /* Delete a FireWall Entry rule from the configuration and update XML file.*/
   boolean DeleteFireWallEntry(qcmap_cm_firewall_entry_conf_t* firewall_entry);

   /* Get a FireWall Entry rule from the configuration.*/
   int GetFireWallEntry(qcmap_cm_firewall_entry_conf_t* firewall_entry,
                        int max_entries);

   /* Add a DMZ IP address to the configuration and update XML file. */
   boolean AddDMZ(uint32 dmz_ip);

   /* Get the DMZ IP address from the configuration . */
   uint32 GetDMZ(void);

   /* Delete a DMZ IP address from the configuration and update XML file. */
   boolean DeleteDMZ(uint32 dmz_ip);

   /* Set NAT Timeout in the configuration and update XML file. */
   boolean SetNATEntryTimeout(uint16 timeout);

   /* Get NAT Timeout from the configuration and update XML file. */
   uint16 GetNATEntryTimeout(void);

   /* Set VPN Pass Through modes in the configuration and update XML file. */
   boolean SetIPSECVpnPassThrough(boolean enable);
   boolean SetL2TPVpnPassThrough(boolean enable);
   boolean SetPPTPVpnPassThrough(boolean enable);

   /* Set firewall state in the configuration and update XML file. */
   boolean SetFirewall(boolean enable, boolean pkts_allowed);
   int SetFirewallOnA5(boolean enable, boolean pkts_allowed);
   boolean ModifyExtdFirewallEntryOnA5(qcmap_cm_extd_firewall_entry_conf_t
                                                           *firewall_entry,
                                       boolean enable,
                                       boolean pkts_allowed);

   boolean SetAutoconnect(boolean enable);
   boolean GetAutoconnect(void);
   void    SetQtiState(boolean enable);
   boolean GetQtiState(void);
   boolean SetRoaming(boolean enable);

   /* Get WWAN statistics */
   boolean GetWWANStatistics(ip_version_enum_type ip_family, 
                             qcmap_cm_statistics_t *wwan_stats, 
                             int *p_error);

   /* Reset WWAN statistics */
   boolean ResetWWANStatistics(ip_version_enum_type ip_family, 
                               int *p_error);

   /*Add an Extended Firewall rule to the configuration*/
   boolean AddExtdFireWallEntry(qcmap_cm_extd_firewall_conf_t* firewall_entry);

   /*Get an Extended Firewall rule from the configuration*/
   boolean GetExtdFireWallEntry(qcmap_cm_extd_firewall_conf_t* firewall_entry);

   /*Delete extended firewall rule from the configuration*/
   boolean DeleteExtdFireWallEntry(qcmap_cm_extd_firewall_conf_t* firewall_entry);

   /*Get Firewall rule handles from the configuration*/
   boolean GetFireWallHandleList(qcmap_cm_extd_firewall_conf_t* firewall_entry);

   boolean GetIPv4WWANNetworkConfiguration(uint32 *public_ip, uint32 *primary_dns, uint32 *secondary_dns);

   /*Change NAT Type based on the option given */
   boolean ChangeNatType(qcmap_cm_nat_type nat_type);
   boolean GetNatType(qcmap_cm_nat_type * cur_nat_type);
   boolean SetNatType(void);

   /* -----------------------------Linux Config-------------------------------*/

   /* Display the current Linux configuration. */
   boolean DisplayLinuxConfig(void);

   /* Bring up Linux Tethering DNS Routing. */
   boolean EnableTethering(void);

   /* Bring up Linux LAN. */
   boolean EnableWLAN(void);

   /* Enable IPV6 Forwarding */
   boolean EnableIPV6Forwarding(void);

   /* Bring down Linux LAN. */
   boolean DisableWLAN(void);

   /* Bring up/down and configure DualAP mode. */
   boolean SetDualAPConfig(boolean enable, uint32 a5_ip_addr, uint32 sub_net_mask);
   boolean EnableDualAP(void);
   boolean DisableDualAP(void);

   /* Configure, start and stop the Linux HostAPD server. */
   boolean SetHostAPDConfig(int intf, char * cfg_path);
   boolean StopHostAPD(int intf);
   boolean StartHostAPD(int intf);

   /* Configure, start and stop the Linux DHCPD server. */
   boolean SetDHCPDConfig(int intf, uint32 start, uint32 end, char * leasetime);
   boolean StartDHCPD(void);
   boolean StopDHCPD(void);

   /* Start, stop and restart the linux MCAST routing daemon. */
   void StartMcastDaemon(void);
   void StopMcastDaemon(void);
   void ReStartMcastDaemon(void);

   /* Used in case of Concurrent STA+AP mode */
   boolean AddSNATEntryOnA5(qcmap_cm_port_fwding_entry_conf_t* nat_entry);
   boolean DeleteSNATEntryOnA5(qcmap_cm_port_fwding_entry_conf_t* nat_entry);

   boolean AddFirewallEntryOnA5(qcmap_cm_firewall_entry_conf_t* firewall_entry);
   boolean DeleteFirewallEntryOnA5(qcmap_cm_firewall_entry_conf_t* firewall_entry);

   boolean AddDMZOnA5(uint32 dmz_ip);
   boolean DeleteDMZOnA5(uint32 dmz_ip);

   boolean SetIPSECVpnPassThroughOnA5(boolean enable);
   boolean SetL2TPVpnPassThroughOnA5(boolean enable);
   boolean SetPPTPVpnPassThroughOnA5(boolean enable);

   /* Use for Signal the LAN/WAN connected */
   pthread_mutex_t             cm_mutex;
   pthread_cond_t              cm_cond;
   boolean                     lan_connecting_in_process;
   boolean                     wan_connecting_in_process;
   boolean                     enable_dns;
   /* Which mode is wifi brought up in */
   qcmap_cm_wifi_mode_type     wifi_mode;
   /* Is STA connected */
   boolean                     sta_connected;
   /* Autoconnect state prior to STA mode.*/
   boolean                     auto_connect;
   /* Index of STA iface in interfaces array in lan_config */
   int                         sta_iface_index;
   /*STA IP address*/
   char                        staIpAddr[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
   /* NAT Type change in progress */
   boolean                     qcmap_nat_type_change_in_progress;
   /* eth device number for first AP iface */
   int                         ap_dev_num;
   /*To track qcmap tear down*/
   boolean                     qcmap_tear_down_in_progress;

  /* use for storing IPV6 prefix info */
   qcmap_cm_nl_prefix_info_t  ipv6_prefix_info;

   /* Get IPV6 prefix information from wwan */
   boolean GetIPV6PrefixInfo(void );

   /* remove Ipv6 prefix info */
   void RemoveIPV6Address(void);

private:

   /* Read QCMAP config from XML file */
   boolean ReadConfigFromXML(void);

   /* Write QCMAP config to XML file */
   boolean WriteConfigToXML(void);

   /*Read QCMAP extended firewall config from XML */
   boolean ReadConfigFromFirewallXML(void);

   /*Write QCMAP extended firewall config to XML */
   boolean WriteConfigToFirewallXML(void);

   /* Connect/Disconnect the Station interface to external hotspot */
   boolean ConnectSTA(qcmap_sta_connection_e conn_type,
                      qcmap_sta_static_ip_config * static_ip_config);
   boolean DisconnectSTA(void);

   /* Find IP address assigned to the STA interface and its netmask */
   boolean GetStaIP(char *staIP, int staIPBufLen, char *netMask, int netMaskBufLen);

   /* Enable/Disable NAT on A5 (In Station mode) */
   boolean EnableNATonA5(void);
   boolean DisableNATonA5(void);

   /* XML file name */
   char                        xml_path[QCMAP_CM_MAX_FILE_LEN];

   /*Firewall XML file name*/
   char                        firewall_xml_path[QCMAP_CM_MAX_FILE_LEN];

   /* QCMAP CM Config */
   boolean                     qcmap_enable;
   qcmap_cm_conf_t             cfg;

   int                         qcmap_cm_handle;
};

#endif
