
/*====================================================

FILE:  QCMAP_ConnectionManager.cpp

SERVICES:
   QCMAP Connection Manager Implementation

=====================================================

Copyright (c) 2011-2014 Qualcomm Technologies, Inc.
All Rights Reserved.
Qualcomm Technologies Confidential and Proprietary

=====================================================*/
/*===========================================================================
  EDIT HISTORY FOR MODULE

  Please notice that the changes are listed in reverse chronological order.

  when       who        what, where, why
  --------   ---        -------------------------------------------------------
  02/20/14   at         Fix for IPv6 address accumulation on rmnet interface.
  01/28/14   rk         KW fixes.
  08/29/13   rk         call pimd -l to effectively apply pimd.conf values.
  04/11/13   mp         Added support for DUN call over MobileAP.
  03/30/13   mp         Corrections for A5_RMNET_SUB_NET_MASK changes and
                        changes related to managing routes in AP_STA mode.
  03/29/13   sb         QCMAP boot up optimizations for RNDIS.
  12/06/12   at         Fix to disable WLAN in case of ds_system_call failure
  11/09/12   rk         Fix to STATIC CONFIG Enable in STA.  
  11/09/12   at         Changing A5_RMNET_SUB_NET_MASK to ease restriction on
                        A5_IP_ADDR specified.
  10/03/12   mp         Fix to prevent QCMAP starting call before QTI Link UP.
                        Added support for QCMAP to scan HS2.0 AP.
  10/03/12   mp         Fix to make QTI use QCMAP WWAN config. 
  08/31/12   mp         Added support for dualAPN in MobileAP and
                        Extended Firewalls in AP+STA.
  08/03/12   mp         Added support for Port Restricted Cone NAT.
  07/25/12   mp         Added AP+STA modifications and checks DHCP config.
  07/19/12   cp         Added support for mcast routing.
  07/07/12   SC         Added support for tethering bringup.
  07/02/12   vb         Added MobileAP events for embedded call.
  06/27/12   cp         Enabled VPN passthrough on A5 side. Also added support
                        to display Linux side Config in STA mode.
  06/25/12   mp         Changes in AP+STA mode-Modified firewall functionality,
                        delete dmz IP. Added rule to block traffic to SSID1.
                        Fixed SDIO polling issue.
  06/14/12   rk         Fixed compilation warnings.
  05/18/12   vb         Added support for embedded call bringup.
  05/17/12   cp         AP-STA IPv6 support.
  05/14/12   cp         Dual SSID IPv6 Support.
  05/07/12   sb         Merge firewall XML and make CLI enable/teardown robust
  05/04/12   cp         Added static IP configuration for STA mode.
  05/03/12   sb         Coex service changes
  04/24/12   cp         AP_STA mode fixes.
  04/17/12   sb         IPV6 state machine fix
  04/17/12   cp         DUAL SSID and AP-STA mode fixes.
  03/29/12   cp         Adding wpa_supplicant support for STA mode.
  03/09/12   sb         Adding extended firewall support.
  03/02/12   ss         Adding support for Concurrent STA+AP mode.
  03/01/12   SDT        Adding support for Dual AP mode.
  02/20/12   SDT        Adding support for IPv6.

===========================================================================*/
#include <fstream>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "ds_util.h"
#include "ds_string.h"
#include "qcmap_cm_api.h"
#include "QCMAP_ConnectionManager.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

/* Default network config */
#define MODULE_NAME "ar6000"
#define HOSTAPD_CFG_PATH "/etc/hostapd.conf"
#define A5_IP_ADDR "192.168.225.1"
#define A5_SUB_NET_MASK "255.255.255.0"

#define A5_RMNET_SUB_NET_MASK "255.255.255.128"

#define HOSTAPD_CFG_PATH_SSID2 "/etc/hostapd-eth1.conf"
#define A5_IP_ADDR_SSID2 "192.168.2.1"
#define A5_SUB_NET_MASK_SSID2 "255.255.255.0"

#define A5_RMNET_IP_ADDR "192.168.225.2"
#define Q6_RMNET_FACING_A5_IP_ADDR "192.168.225.3"
#define USB_RMNET_IP_ADDR "192.168.225.4"
#define Q6_RMNET_FACTING_USB_RMNET_IP_ADDR "192.168.225.5"
#define NAT_IP_ADDR "192.168.225.6"
#define DUN_CLIENT_IP_ADDR "192.168.225.7"

#define MAX_COMMAND_STR_LEN 200
#define MAX_WIFI_CLIENTS 20
#define MAX_IPV6_PREFIX 40
#define SASTORAGE_DATA(addr)    (addr).__ss_padding



#define QCMAP_ENABLE_POLLING "echo 1 > /sys/devices/platform/msm_sdcc.2/polling"
#define QCMAP_DISABLE_POLLING "echo 0 > /sys/devices/platform/msm_sdcc.2/polling"

#define SUPPLICANT_CFG_PATH "/etc/wpa_supplicant.conf"
/* ip checks definations for A5 ip address */
#define RESTRICTED_A5_IP_1 "0.0.0.124"
#define RESTRICTED_A5_IP_2 "0.0.0.125"
#define RESTRICTED_A5_IP_3 "0.0.0.126"
#define RESTRICTED_A5_IP_4 "0.0.0.127"
#define READJUSTED_BASE_A5_RMNET_NETMASK "255.255.255.0"
#define RESERVED_IP_CALCULATION_FACTOR 127

#define QCMAP_QTI_MTYPE_ID 1
#define QCMAP_QTI_KEY_ID 100
#define QCMAP_QTI_MSG_SIZE (sizeof(int)*QCMAP_QTI_WWAN_PARAMS)

/* WIFI iDriver/Firmware Init Delay seconds */
#define WIFI_DEV_INIT_DELAY 4
#define WIFI_DEV_INIT_DELAYS_MAX 15

#define QCMAP_CM_LOG(...)                         \
  fprintf( stderr, "%s %d:", __FILE__, __LINE__); \
  fprintf( stderr, __VA_ARGS__ )

#define QCMAP_CM_LOG_FUNC_ENTRY()  \
    QCMAP_CM_LOG                   \
    (                              \
        "Entering function %s\n",  \
        __FUNCTION__               \
    )

#define QCMAP_CM_LOG_FUNC_EXIT()   \
    QCMAP_CM_LOG                   \
    (                              \
        "Exiting function %s\n",   \
        __FUNCTION__

static void QCMAP_ConnectionManager_callback
(
  int               handle,               /* MobileAP Application id */
  qcmap_cm_event_e  event,                /* Type of MobileAP Event  */
  void             *qcmap_cm_cb_user_data /* Call back user data     */
)
{
  QCMAP_ConnectionManager *manager = (QCMAP_ConnectionManager *) qcmap_cm_cb_user_data;

  if (event == QCMAP_CM_EVENT_LAN_CONNECTED ||
      event == QCMAP_CM_EVENT_LAN_CONNECTING_FAIL ||
      event == QCMAP_CM_EVENT_LAN_DISCONNECTED)
  {
    pthread_mutex_lock(&manager->cm_mutex);
    manager->lan_connecting_in_process = false;
    pthread_cond_signal(&manager->cm_cond);
    pthread_mutex_unlock(&manager->cm_mutex);
  }
  if (event == QCMAP_CM_EVENT_WAN_CONNECTED ||
      event == QCMAP_CM_EVENT_WAN_CONNECTING_FAIL ||
      event == QCMAP_CM_EVENT_WAN_DISCONNECTED)
  {
    pthread_mutex_lock(&manager->cm_mutex);
    manager->wan_connecting_in_process = false;
    pthread_cond_signal(&manager->cm_cond);
    pthread_mutex_unlock(&manager->cm_mutex);
  }

  QCMAP_CM_LOG("QCMAP AP Handle %04X Event %04X\n", handle, event);

  switch (event)
  {
  case QCMAP_CM_EVENT_ENABLED:
    QCMAP_CM_LOG("QCMAP_CM_EVENT_ENABLED\n");
    break;
  case QCMAP_CM_EVENT_LAN_CONNECTING:
    QCMAP_CM_LOG("QCMAP_CM_EVENT_LAN_CONNECTING\n");
    break;
  case QCMAP_CM_EVENT_LAN_CONNECTING_FAIL:
    QCMAP_CM_LOG("QCMAP_CM_EVENT_LAN_CONNECTING_FAIL\n");
    break;
  case QCMAP_CM_EVENT_LAN_IPv6_CONNECTING_FAIL:
    QCMAP_CM_LOG("QCMAP_CM_EVENT_LAN_IPv6_CONNECTING_FAIL\n");
    break;
  case QCMAP_CM_EVENT_LAN_CONNECTED:
    QCMAP_CM_LOG("QCMAP_CM_EVENT_LAN_CONNECTED\n");

    /*-----------------------------------------------------------------------
      Posting the signal for QTI to make it aware that LAN
      has come up.
    -----------------------------------------------------------------------*/

    /* Queue QCMAP WAN params in a message queue to inform QTI */
    if(!manager->SendWanConfig())
    {
      QCMAP_CM_LOG("Error sending Wan config message to QTI\n");
    }
    else
    {
      printf("Posting signal USR1 to QTI for IPv4 \n");
      ds_system_call("pkill -USR1 qti", strlen("pkill -USR1 qti"));
    }

    /*-----------------------------------------------------------------------
     If we are in tethering only mode perform actions specific only to 
     tethering only mode. 
    -----------------------------------------------------------------------*/
    if (manager->TetheringOnly())
    {
      QCMAP_CM_LOG("QCMAP_ConnectionManager in Tethering mode\n");
      manager->EnableTethering();
    }
    /*--------------------------------------------------------------------- 
     Else enable WLAN and connect backhaul for IPv4 if autoconnect is
     enabled.
    ----------------------------------------------------------------------*/
    else
    {
      manager->EnableWLAN();
      if(manager->GetAutoconnect())
      {
        manager->ConnectBackHaul();
      }
    }
    break;
  case QCMAP_CM_EVENT_LAN_IPv6_CONNECTED:
    QCMAP_CM_LOG("QCMAP_CM_EVENT_LAN_IPv6_CONNECTED\n");

    /*-------------------------------------------------------------------- 
    . In tethering only mode.
    ---------------------------------------------------------------------*/
    if (manager->TetheringOnly())
    {
      QCMAP_CM_LOG("QCMAP_ConnectionManager in Tethering mode\n");
      /*-----------------------------------------------------------------------
        Bring up IPv6 backhaul if QTI has signaled to bring up backhaul and
        autoconnect is enabled.
      -----------------------------------------------------------------------*/
      if(manager->GetQtiState() && manager->GetAutoconnect())
      {
        manager->ConnectBackHaul();
      }
    }

    /*---------------------------------------------------------------------
     Else enable IPv6 forwarding and bring IPv6 backhaul if autoconnect
     is enabled
    -------------------------------------------------------------------- */
    else
    {
      manager->EnableIPV6Forwarding();
      if(manager->GetAutoconnect())
      {
        manager->ConnectBackHaul();
      }
    }
    break;
  case QCMAP_CM_EVENT_WAN_CONNECTING:
    QCMAP_CM_LOG("QCMAP_CM_EVENT_WAN_CONNECTING\n");
    break;
  case QCMAP_CM_EVENT_WAN_CONNECTING_FAIL:
    QCMAP_CM_LOG("QCMAP_CM_EVENT_WAN_CONNECTING_FAIL\n");
    break;
  case QCMAP_CM_EVENT_WAN_IPv6_CONNECTING_FAIL:
    QCMAP_CM_LOG("QCMAP_CM_EVENT_WAN_IPv6_CONNECTING_FAIL\n");
    break;
  case QCMAP_CM_EVENT_WAN_CONNECTED:
    QCMAP_CM_LOG("QCMAP_CM_EVENT_WAN_CONNECTED\n");
    manager->EnableDNS();
    break;
  case QCMAP_CM_EVENT_WAN_IPv6_CONNECTED:
    QCMAP_CM_LOG("QCMAP_CM_EVENT_WAN_IPv6_CONNECTED\n");
    manager->GetIPV6PrefixInfo();
    break;
  case QCMAP_CM_EVENT_WAN_DISCONNECTED:
    QCMAP_CM_LOG("QCMAP_CM_EVENT_WAN_DISCONNECTED\n");
    if(manager->qcmap_nat_type_change_in_progress == TRUE)
    {
      manager->SetNatType();
    }
    break;
  case QCMAP_CM_EVENT_WAN_IPv6_DISCONNECTED:
    QCMAP_CM_LOG("QCMAP_CM_EVENT_WAN_IPv6_DISCONNECTED\n");
    manager->RemoveIPV6Address();
    break;
  case QCMAP_CM_EVENT_LAN_DISCONNECTED:
    QCMAP_CM_LOG("QCMAP_CM_EVENT_LAN_DISCONNECTED\n");
    break;
  case QCMAP_CM_EVENT_LAN_IPv6_DISCONNECTED:
    QCMAP_CM_LOG("QCMAP_CM_EVENT_LAN_IPv6_DISCONNECTED\n");
    break;
  case QCMAP_CM_EVENT_DISABLED:
    QCMAP_CM_LOG("QCMAP_CM_EVENT_DISABLED\n");
    manager->DisableWLAN();
    break;
  default:
    QCMAP_CM_LOG("QCMAP_CM_EVENT_UNKNOWN %08X\n", event);
    break;
  }
  return;
}

/*===================================================================
                             Class Definitions
  ===================================================================*/

QCMAP_ConnectionManager::QCMAP_ConnectionManager
(
  char     *xml_path       /* Path and file name for XML config file */
)
{
  QCMAP_CM_LOG_FUNC_ENTRY();

  this->qcmap_enable = true;
  /* Populate default Config */
  memset(&this->cfg, 0, sizeof(this->cfg));
  this->qcmap_cm_handle = 0;
  this->lan_connecting_in_process = false;
  this->wan_connecting_in_process = false;
  this->enable_dns = true;
  this->qcmap_tear_down_in_progress = false;
  this->wifi_mode = QCMAP_CM_WIFI_MODE_NOT_SET;
  this->sta_connected = false;
  this->sta_iface_index = QCMAP_LAN_INVALID_IFACE_INDEX;
  this->ap_dev_num = QCMAP_LAN_INVALID_IFACE_INDEX;
  this->cfg.lan_config.wlan_gpio_num=0;
  pthread_mutex_init(&this->cm_mutex, NULL);
  pthread_cond_init(&this->cm_cond, NULL);
  struct in_addr addr;

  /* Read the configuration. */
  if (xml_path)
  {
    strlcpy(this->xml_path, xml_path, QCMAP_CM_MAX_FILE_LEN);
    /* Read configuration from XML file. */
    if (ReadConfigFromXML())
    {
      /* reassign A5 i.p. address if it is .124, .125, .126  and  .127 */
      if (((this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                              a5_ip_addr & ntohl(inet_addr(RESTRICTED_A5_IP_1))) 
                                      == ntohl(inet_addr(RESTRICTED_A5_IP_1)))||
          ((this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                              a5_ip_addr & ntohl(inet_addr(RESTRICTED_A5_IP_2)))
                                      == ntohl(inet_addr(RESTRICTED_A5_IP_2)))||
           ((this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                              a5_ip_addr & ntohl(inet_addr(RESTRICTED_A5_IP_3)))
                                      == ntohl(inet_addr(RESTRICTED_A5_IP_3)))||
           ((this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                               a5_ip_addr & ntohl(inet_addr(RESTRICTED_A5_IP_4)))
                                      == ntohl(inet_addr(RESTRICTED_A5_IP_4)))
         )
      {
       this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].a5_ip_addr =
                 this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                a5_ip_addr & ntohl(inet_addr(READJUSTED_BASE_A5_RMNET_NETMASK))+ 1;
       addr.s_addr = htonl(this->cfg.lan_config.\
                            interface[QCMAP_CM_INTF_PRIMARY_INDEX].a5_ip_addr);
       QCMAP_CM_LOG("modified a5_ip_addr = %s \n",inet_ntoa(addr));
      }

      /* Program the rest of the IP */
      if(this->cfg.nat_config.tethering_only)
      {
        this->cfg.lan_config.a5_rmnet_ip_addr = this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].a5_ip_addr;
      }
      else
      {
        this->cfg.lan_config.a5_rmnet_ip_addr = this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].a5_ip_addr + 1;
      }

      this->cfg.lan_config.q6_ip_addr_facing_a5 = this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].a5_ip_addr + 2;
      this->cfg.lan_config.usb_rmnet_ip_addr = this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].a5_ip_addr + 3;
      this->cfg.lan_config.q6_ip_addr_facing_usb_rmnet = this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].a5_ip_addr + 4;
      this->cfg.lan_config.nat_ip_addr = this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].a5_ip_addr + 5;
      this->cfg.lan_config.dun_client_ip_addr = this->cfg.lan_config.\
                   interface[QCMAP_CM_INTF_PRIMARY_INDEX].a5_ip_addr + 6;

      SetDualAPConfig(
       this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].enable,
       this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].a5_ip_addr,
       this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].sub_net_mask);

      /* DHCP config checks. */
      if(
        ((this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                                                             a5_ip_addr &
          this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                                                         sub_net_mask) !=
         (this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                                                      dhcp_start_address &
          this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                                                          sub_net_mask))||
         (this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                                                     dhcp_start_address <=
                                      this->cfg.lan_config.dun_client_ip_addr))
      {
        this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
              dhcp_start_address =
        this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].a5_ip_addr+
                                                                          20;
        addr.s_addr =
        htonl(this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                                                      dhcp_start_address );
        QCMAP_CM_LOG("SSID1 DHCP Start addr in XML is invalid."
                     "Setting it to default value %s\n",inet_ntoa(addr));
      }

      if(
        ((this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                                                             a5_ip_addr &
          this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                                                          sub_net_mask) !=
         (this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                                                       dhcp_end_address &
          this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                                                          sub_net_mask))||
         (this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                                                       dhcp_end_address <
          this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                                                      dhcp_start_address))
      {
        this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                                                        dhcp_end_address =
        this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                                     dhcp_start_address + MAX_WIFI_CLIENTS;
        addr.s_addr =
        htonl(this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                                                         dhcp_end_address );
        QCMAP_CM_LOG("SSID1 DHCP End addr in XML is invalid."
                     "Setting it to default value %s\n",inet_ntoa(addr));
      }

      /* DHCP config checks for SSID2*/
      if(this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].devmode !=
                                                         QCMAP_CM_DEVMODE_STA )
      {
        if(
          ((this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].\
                                                              a5_ip_addr &
            this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].\
                                                          sub_net_mask) !=
           (this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].\
                                                       dhcp_start_address &
            this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].\
                                                           sub_net_mask))||
           (this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].\
                                                      dhcp_start_address <=
            this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].\
                                                              a5_ip_addr))
        {
          this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].\
                                                       dhcp_start_address =
          this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].\
                                                            a5_ip_addr + 20;
          addr.s_addr =
          htonl(this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].\
                                                          dhcp_start_address );
          QCMAP_CM_LOG("SSID2 DHCP Start addr in XML is invalid."
                       "Setting it to default value %s\n",inet_ntoa(addr));
        }

        if(
          ((this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].\
                                                               a5_ip_addr &
            this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].\
                                                           sub_net_mask) !=
           (this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].\
                                                          dhcp_end_address &
            this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].\
                                                            sub_net_mask))||
           (this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].\
                                                         dhcp_end_address <
            this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].\
                                                         dhcp_start_address))
        {
          this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].\
                                                      dhcp_end_address =
          this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].\
                                   dhcp_start_address + MAX_WIFI_CLIENTS;
          addr.s_addr =
          htonl(this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].\
                                                          dhcp_end_address );
          QCMAP_CM_LOG("SSID2 DHCP End addr in XML is invalid."
                       "Setting it to default value %s\n",inet_ntoa(addr));
        }
      }

      QCMAP_CM_LOG("QCMAP Read XML OK\n");

      if(this->cfg.nat_config.firewall_config_file)
      {
        QCMAP_CM_LOG("\n Firewall XML file is %s \n",this->cfg.nat_config.firewall_config_file);
        if(ReadConfigFromFirewallXML())
        {
          QCMAP_CM_LOG("\n QCMAP Firewall XML read OK \n");
          return;
        }
        QCMAP_CM_LOG("\n QCMAP Firewall XML read failed \n");
      }
      else
      {
        QCMAP_CM_LOG("\n No firewall xml mentioned \n");
        this->cfg.nat_config.firewall_config_file[0] = '\0';
      }
      this->WriteConfigToXML();
      return;
    }
    /* Read Config from XML failed. Use defaults. */
    QCMAP_CM_LOG("QCMAP Read XML failed.\n");
  }
  else
  {
    this->xml_path[0] = '\0';
  }

  /* Fill in the default config */
  /* Default mode - AP */
  /* Interface 1 - AP - enabled */
  this->cfg.nat_config.nat_type = QCMAP_CM_SYMMETRIC_NAT; /*Symmetric NAT as default */

  strlcpy(this->cfg.lan_config.module, MODULE_NAME, QCMAP_CM_MAX_FILE_LEN);
  this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].enable = true;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].devmode = QCMAP_CM_DEVMODE_AP;
  strlcpy(this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].path_to_hostapd_conf, HOSTAPD_CFG_PATH, QCMAP_CM_MAX_FILE_LEN);
  this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].access_profile = QCMAP_CM_PROFILE_FULL_ACCESS;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].a5_ip_addr = ntohl(inet_addr(A5_IP_ADDR));
  this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].sub_net_mask = ntohl(inet_addr(A5_SUB_NET_MASK));
  this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].enable_dhcpd = true;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].dhcp_start_address = ntohl(inet_addr(A5_IP_ADDR)) + 20;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].dhcp_end_address = ntohl(inet_addr(A5_SUB_NET_MASK)) + 20 + MAX_WIFI_CLIENTS;
  strlcpy(this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].dhcp_lease_time, "12h", QCMAP_CM_MAX_FILE_LEN);
  this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].enable_supplicant = false;
  strlcpy(this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].path_to_supplicant_conf, "NA", QCMAP_CM_MAX_FILE_LEN);
  strlcpy(this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].external_ap_ssid, "NA", QCMAP_CM_MAX_FILE_LEN);

  this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].conn_type =(qcmap_sta_connection_e) 0;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].static_ip_config.dns_addr = 0;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].static_ip_config.gw_ip = 0;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].static_ip_config.netmask = 0;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].static_ip_config.ip_addr = 0;

  /* Interface 2 - AP - disabled */
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].enable = false;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].devmode = QCMAP_CM_DEVMODE_AP;
  strlcpy(this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].path_to_hostapd_conf, HOSTAPD_CFG_PATH_SSID2, QCMAP_CM_MAX_FILE_LEN);
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].access_profile = QCMAP_CM_PROFILE_INTERNET_ONLY;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].a5_ip_addr = ntohl(inet_addr(A5_IP_ADDR_SSID2));
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].sub_net_mask = ntohl(inet_addr(A5_SUB_NET_MASK_SSID2));
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].enable_dhcpd = true;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].dhcp_start_address = ntohl(inet_addr(A5_IP_ADDR_SSID2)) + 20;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].dhcp_end_address = ntohl(inet_addr(A5_SUB_NET_MASK_SSID2)) + 20 + MAX_WIFI_CLIENTS;
  strlcpy(this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].dhcp_lease_time, "12h", QCMAP_CM_MAX_FILE_LEN);
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].enable_supplicant = false;
  strlcpy(this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].path_to_supplicant_conf, "NA", QCMAP_CM_MAX_FILE_LEN);
  strlcpy(this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].external_ap_ssid, "NA", QCMAP_CM_MAX_FILE_LEN);

  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].conn_type =(qcmap_sta_connection_e) 0;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].static_ip_config.dns_addr = 0;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].static_ip_config.gw_ip = 0;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].static_ip_config.netmask = 0;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].static_ip_config.ip_addr = 0;


  /* Interface 3 - STA - disabled */
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].enable = false;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].devmode = QCMAP_CM_DEVMODE_STA;
  strlcpy(this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].path_to_hostapd_conf, "NA", QCMAP_CM_MAX_FILE_LEN);
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].access_profile =(qcmap_cm_access_profile_type) 0;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].a5_ip_addr = 0;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].sub_net_mask = 0;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].enable_dhcpd = false;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].dhcp_start_address = 0;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].dhcp_end_address = 0;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].dhcp_lease_time[0] = '\0';
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].enable_supplicant = false;
  strlcpy(this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].path_to_supplicant_conf, SUPPLICANT_CFG_PATH, QCMAP_CM_MAX_FILE_LEN);
  strlcpy(this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].external_ap_ssid, "NA", QCMAP_CM_MAX_FILE_LEN);
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].conn_type = QCMAP_STA_CONNECTION_DYNAMIC;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].static_ip_config.dns_addr = 0;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].static_ip_config.gw_ip = 0;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].static_ip_config.netmask = 0;
  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID3_INDEX].static_ip_config.ip_addr = 0;


  this->cfg.lan_config.a5_rmnet_ip_addr = ntohl(inet_addr(A5_RMNET_IP_ADDR));
  this->cfg.lan_config.q6_ip_addr_facing_a5 = ntohl(inet_addr(Q6_RMNET_FACING_A5_IP_ADDR));
  this->cfg.lan_config.usb_rmnet_ip_addr = ntohl(inet_addr(USB_RMNET_IP_ADDR));
  this->cfg.lan_config.q6_ip_addr_facing_usb_rmnet = ntohl(inet_addr(Q6_RMNET_FACTING_USB_RMNET_IP_ADDR));
  this->cfg.lan_config.nat_ip_addr = ntohl(inet_addr(NAT_IP_ADDR));
  this->cfg.lan_config.dun_client_ip_addr =
                          ntohl(inet_addr(DUN_CLIENT_IP_ADDR));

  this->cfg.wan_config.tech = QCMAP_WAN_TECH_3GPP;
  this->cfg.wan_config.profile_id.v4.umts_profile_index =
               this->cfg.wan_config.profile_id.v6.umts_profile_index = 0;
  this->cfg.wan_config.profile_id.v4.cdma_profile_index =
               this->cfg.wan_config.profile_id.v6.cdma_profile_index = 0;
  this->cfg.wan_config.ip_family = QCMAP_IP_V4_V01;

  return;
}

QCMAP_ConnectionManager::~QCMAP_ConnectionManager()
{
  QCMAP_CM_LOG_FUNC_ENTRY();

  pthread_mutex_destroy(&this->cm_mutex);
  pthread_cond_destroy(&this->cm_cond);
}

boolean QCMAP_ConnectionManager::IPv4v6Only()
{
  if (this->cfg.wan_config.ip_family == QCMAP_IP_V4V6_V01)
  {
    /* IPv4v6 is enabled */
    return true;
  }
  else
  {
    return false;
  }
}

boolean QCMAP_ConnectionManager::IoEMode()
{
  if (this->cfg.nat_config.ioe_mode)
  {
    /* IoE is enabled */
    return true;
  }
  else
  {
    return false;
  }
}

boolean QCMAP_ConnectionManager::TetheringOnly()
{
  if (this->cfg.nat_config.tethering_only)
  {
    /* Tethering is enabled */
    return true;
  }
  else
  {
    return false;
  }
}

boolean QCMAP_ConnectionManager::Enable()
{
  int qcmap_cm_errno;
  int rc = 0;
  struct timespec ts;

  QCMAP_CM_LOG_FUNC_ENTRY();

  pthread_mutex_lock(&this->cm_mutex);
  this->lan_connecting_in_process = true;
  pthread_mutex_unlock(&this->cm_mutex);
  this->qcmap_cm_handle = qcmap_cm_enable(&this->cfg, QCMAP_ConnectionManager_callback, this, &qcmap_cm_errno);
  if (this->qcmap_cm_handle <= 0)
  {
    pthread_mutex_lock(&this->cm_mutex);
    this->lan_connecting_in_process = false;
    pthread_mutex_unlock(&this->cm_mutex);
    QCMAP_CM_LOG("QCMAP LAN Connection failed \n");
    return false;
  }

  QCMAP_CM_LOG("QCMAP Enabled\n");

  if((qcmap_cm_errno == QCMAP_CM_ENOERROR) && (qcmap_cm_get_state() >= QCMAP_CM_LAN_CONNECTED))
  {
    pthread_mutex_lock(&this->cm_mutex);
    this->lan_connecting_in_process = false;
    pthread_mutex_unlock(&this->cm_mutex);
    QCMAP_CM_LOG("QCMAP LAN Connected \n");
    return true;
  }

  QCMAP_CM_LOG("QCMAP LAN Connection in progress.\n");
  return true;
}

boolean QCMAP_ConnectionManager::Disable()
{
  int ret = 0, qcmap_cm_errno;

  QCMAP_CM_LOG_FUNC_ENTRY();

  if (!this->qcmap_enable)
  {
    /* QCMAP is not enabled */
    QCMAP_CM_LOG("QCMAP not enabled\n");
    return false;
  }

  pthread_mutex_lock(&this->cm_mutex);
  this->qcmap_tear_down_in_progress = true;
  pthread_cond_signal(&this->cm_cond);
  pthread_mutex_unlock(&this->cm_mutex);  

  ret = qcmap_cm_disable(this->qcmap_cm_handle, &qcmap_cm_errno);

  if (ret < 0)
  {
    QCMAP_CM_LOG("QCMAP LAN Disable fails: %d\n", qcmap_cm_errno);
    return false;
  }
  else
  {
    QCMAP_CM_LOG("QCMAP LAN Disable in progress\n");
    return true;
  }
}

boolean QCMAP_ConnectionManager::EnableTethering()
{
  char command[MAX_COMMAND_STR_LEN];
  struct in_addr addr;
  char gw_ip[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char devname[16];
  int qcmap_cm_error;

 /* Setup Routes */
  if (qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_IP_V4_V01, devname, &qcmap_cm_error) != QCMAP_CM_SUCCESS)
  {
    QCMAP_CM_LOG("Couldn't get rmnet name. error %d\n", qcmap_cm_error);
    strlcpy(devname, QCMAP_V4_DEFAULT_DEVICE_NAME, 16);
  }

  #if 0
  /* Delete default route to q6_ip_addr_facing_usb_rmnet */ 
  addr.s_addr = htonl(this->cfg.lan_config.q6_ip_addr_facing_usb_rmnet);
  snprintf(command, MAX_COMMAND_STR_LEN, "route del default gw %s", inet_ntoa(addr));
  ds_system_call(command, strlen(command));
  #endif

  /* Route any traffic destined to RMNET USB TE towards A5 Rmnet interface */

  addr.s_addr = htonl(this->cfg.lan_config.q6_ip_addr_facing_a5);
  strlcpy(gw_ip, inet_ntoa(addr), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
  addr.s_addr = htonl(this->cfg.lan_config.usb_rmnet_ip_addr);
  snprintf(command, MAX_COMMAND_STR_LEN, "route add -host %s gw %s dev %s", inet_ntoa(addr), gw_ip, devname);
  ds_system_call(command, strlen(command));

  addr.s_addr = htonl(this->cfg.lan_config.q6_ip_addr_facing_a5);
  snprintf(command, MAX_COMMAND_STR_LEN, "route add default gw %s dev %s", inet_ntoa(addr), devname);
  ds_system_call(command, strlen(command));


  /* call dnsmasq without bringup the WLAN*/
  /* Restart DHCPD */
  StopDHCPD();
  snprintf(command, MAX_COMMAND_STR_LEN, "dnsmasq -2 --no-dhcp-interface=%s", devname);
  ds_system_call(command, strlen(command));

 return true;
}


boolean QCMAP_ConnectionManager::EnableWLAN()
{
  char command[MAX_COMMAND_STR_LEN];
  struct in_addr addr;
  char a5_ip[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char gw_ip[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char netmask[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char devmode[16] = { '\0' };
  char devname[16];
  int s, i, qcmap_cm_error, dev;
  struct ifreq buffer;
  int lte_frequency = -1;
  boolean ret_val;
  char scratch_buf[16];

  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Enable IP forwarding */
  ds_system_call("echo 1 > /proc/sys/net/ipv4/ip_forward",
                 strlen("echo 1 > /proc/sys/net/ipv4/ip_forward"));

  /* Enable polling */
    ds_system_call("echo 1 > /sys/devices/platform/msm_sdcc.2/polling",
                   strlen("echo 1 > /sys/devices/platform/msm_sdcc.2/polling"));

  /* Determine the devmode parameter for the module. */
  for (i = 0; i < QCMAP_MAX_NUM_INTF; i++)
  {
    /* The module does not allow partial config. If one is not enabled,
       no further interfaces may be enabled. */
    if (!this->cfg.lan_config.interface[i].enable)
      break;
    /* If not the first interface, place a comma before the devmode. */
    if (i)
    {
      strlcat(devmode, ",", 16);
    }
    /* */
    switch (this->cfg.lan_config.interface[i].devmode)
    {
    case QCMAP_CM_DEVMODE_STA:
      strlcat(devmode, "sta", 16);
      break;
    case QCMAP_CM_DEVMODE_AP:
    default:
      strlcat(devmode, "ap", 16);
      break;
    }
  }

  /* Verify that the devmode is supported. Allowed ones are:
     AP, AP+AP, AP+STA, AP+AP+STA.
     Order of checks below is important, since "ap,ap,sta" will also match
     for "ap,ap" and so on. */
  if( strncmp(devmode, "ap,ap,sta", (strlen(devmode) > 9)?strlen(devmode):9)
        == 0 )
  {
    /* Set STA iface index in interface array in lan_cfg */
    this->sta_iface_index = 2;

    /* Change the devmode parameter to "sta,ap,ap" - sta should be the first
       parameter as specified by AR Card documentation */
    strlcpy(devmode, "sta,ap,ap", sizeof(devmode));
    this->ap_dev_num = 1;
    this->wifi_mode = QCMAP_CM_WIFI_AP_AP_STA_MODE;
  }
  else if( strncmp(devmode, "ap,sta", (strlen(devmode) > 6)?strlen(devmode):6)
             == 0 )
  {
    /* Set STA iface index in interface array in lan_cfg */
    this->sta_iface_index = 1;

    /* Change the devmode parameter to "sta,ap" - sta should be the first
       parameter as specified by AR Card documentation */
    strlcpy(devmode, "sta,ap", sizeof(devmode));
    this->ap_dev_num = 1;
    this->wifi_mode = QCMAP_CM_WIFI_AP_STA_MODE;
  }
  else if( strncmp(devmode, "ap,ap", (strlen(devmode) > 5)?strlen(devmode):5)
             == 0)
  {
    this->ap_dev_num = 0;
    this->sta_iface_index = 2;
    this->wifi_mode = QCMAP_CM_WIFI_AP_AP_MODE;
  }
  else if( strncmp(devmode, "ap", (strlen(devmode) > 2)?strlen(devmode):2)
             == 0 )
  {
    this->ap_dev_num = 0;
    this->sta_iface_index = 1;
    this->wifi_mode = QCMAP_CM_WIFI_AP_MODE;
  }
  else
  {
    QCMAP_CM_LOG("Error: Invalid devmode configuration tried: %s.\n", devmode);
    return false;
  }
  QCMAP_CM_LOG( "sta_iface_index: %d, ap_dev_num: %d\n",
                this->sta_iface_index, this->ap_dev_num);

    /* Load the module for the WiFi adapter. */
  if(this->cfg.lan_config.wlan_gpio_num!=0)
  {
    snprintf(  \
              command, \
              MAX_COMMAND_STR_LEN, \
              "modprobe  %s targetconf=\"router\" devmode=%s wlan_reset_gpio=%d ", \
              this->cfg.lan_config.module, \
              devmode, \
              this->cfg.lan_config.wlan_gpio_num \
            );
  }
  else
  {
    snprintf(command, MAX_COMMAND_STR_LEN, "modprobe %s targetconf=\"router\" devmode=%s", this->cfg.lan_config.module, devmode);
  }

  ds_system_call(command,strlen(command));

  printf("MODPROBE: %s\n", command);
  /* delay until wifi device is ready */
  for (i = 0; i < QCMAP_MAX_NUM_INTF; i++)
  {
    int j = 0;
    /* The module does not allow partial config. If one is not enabled,
       no further interfaces may be enabled. */
    if (!this->cfg.lan_config.interface[i].enable)
      break;
    snprintf(command, MAX_COMMAND_STR_LEN, "/proc/sys/net/ipv4/conf/eth%d", i);
    while (j++ < WIFI_DEV_INIT_DELAYS_MAX)
    {
      std::ifstream ifile(command);
      std::cout << command << std::endl;
      if (ifile) break;
      sleep(WIFI_DEV_INIT_DELAY);
    }
  }

  ds_system_call("killall -9 cxmapp", strlen("killall -9 cxmapp"));

  LOG_MSG_INFO1("Enable WLAN: get lte freq",0,0,0);
  
  ret_val = coex_get_wwan_status(&lte_frequency);
  
  LOG_MSG_INFO1("Enable WLAN:get_coex_wwan_status %d freq %d", ret_val, lte_frequency,0);
  if(ret_val && lte_frequency!= -1)
  {
    LOG_MSG_INFO1("lte_frequency: wmiconfig -i eth0 --lte_freq %d", lte_frequency, 0, 0);
    snprintf(command, MAX_COMMAND_STR_LEN, "wmiconfig -i eth0 --lte_freq %d", lte_frequency);
    ds_system_call(command, strlen(command));
  }
  
  ds_system_call("killall -9 cxmapp", strlen("killall -9 cxmapp"));

  LOG_MSG_INFO1("Start cxmapp daemon",0,0,0);

  ds_system_call("/usr/bin/cxmapp &", strlen("/usr/bin/cxmapp &"));

  /* This function stops all currently running HostAPD daemons. */
  StopHostAPD(0);

  QCMAP_CM_LOG("Wlan card brought up in mode %d.\n", this->wifi_mode);

  /* Bring up interfaces.
     This part needs to be done only for AP interfaces.
     Proceed only upto STA iface index since STA is always last in the
     list (ap; ap,ap; ap,sta; ap,ap,sta supported). */
  for (i=0, dev=this->ap_dev_num; i < this->sta_iface_index; i++, dev++)
  {
    /* The module does not allow partial config. If one is not enabled,
       no further interfaces may be enabled. */
    if (!this->cfg.lan_config.interface[i].enable)
      break;
    /* Start the HostAP daemon. */
    StartHostAPD(i);
    /* Config WLAN interface IP */
    addr.s_addr = htonl(this->cfg.lan_config.interface[i].a5_ip_addr);
    strlcpy(a5_ip, inet_ntoa(addr), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
    addr.s_addr = htonl(this->cfg.lan_config.interface[i].sub_net_mask);
    strlcpy(netmask, inet_ntoa(addr), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
    // snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig %s %s netmask %s", this->cfg.lan_config.wifi_ap_intf_name, a5_ip, netmask);
    snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig eth%d %s netmask %s up", dev, a5_ip, netmask);
    ds_system_call(command, strlen(command));
  }

  /* Restart DHCPD */
  StopDHCPD();
  StartDHCPD();

  /* Setup Routes */
  if (qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_IP_V4_V01, devname, &qcmap_cm_error) != QCMAP_CM_SUCCESS)
  {
    QCMAP_CM_LOG("Couldn't get rmnet name. error %d\n", qcmap_cm_error);
    strlcpy(devname, QCMAP_V4_DEFAULT_DEVICE_NAME, 16);
  }

#if 0
  /* Delete default route to q6_ip_addr_facing_usb_rmnet */ 
  addr.s_addr = htonl(this->cfg.lan_config.q6_ip_addr_facing_usb_rmnet);
  snprintf(command, MAX_COMMAND_STR_LEN, "route del default gw %s", inet_ntoa(addr));
  ds_system_call(command, strlen(command));
#endif

  addr.s_addr = htonl(this->cfg.lan_config.a5_rmnet_ip_addr);
  strlcpy(gw_ip,inet_ntoa(addr), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
  addr.s_addr = inet_addr(A5_RMNET_SUB_NET_MASK); /* TODO make it dynamic. */
  snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig %s %s netmask %s allmulti multicast", devname, gw_ip, inet_ntoa(addr));
  ds_system_call(command, strlen(command));

  /* Route any traffic destined to RMNET USB TE or DUN client
     towards A5 Rmnet interface */
  addr.s_addr = htonl(this->cfg.lan_config.q6_ip_addr_facing_a5);
  strlcpy(gw_ip, inet_ntoa(addr), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
  addr.s_addr = htonl(this->cfg.lan_config.usb_rmnet_ip_addr);
  snprintf(command, MAX_COMMAND_STR_LEN, "route add -host %s gw %s dev %s", inet_ntoa(addr), gw_ip, devname);
  ds_system_call(command, strlen(command));

  addr.s_addr = htonl(this->cfg.lan_config.dun_client_ip_addr);
  snprintf(command, MAX_COMMAND_STR_LEN, "route add -host %s gw %s dev %s",
                                            inet_ntoa(addr), gw_ip, devname);
  ds_system_call(command, strlen(command));

  addr.s_addr = htonl(this->cfg.lan_config.q6_ip_addr_facing_a5);
  snprintf(command, MAX_COMMAND_STR_LEN, "route add default gw %s dev %s", inet_ntoa(addr), devname);
  ds_system_call(command, strlen(command));

  /* Delete the subnet route to RMNET. This will make the WLAN subnet
     route assume more priority (which gets added auto by kernel)
   */
  addr.s_addr = htonl(this->cfg.lan_config.a5_rmnet_ip_addr &
                      ntohl(inet_addr(A5_RMNET_SUB_NET_MASK)));
  strlcpy(gw_ip, inet_ntoa(addr), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
  addr.s_addr = inet_addr(A5_RMNET_SUB_NET_MASK); /* TODO make it dynamic. */
  snprintf(command, MAX_COMMAND_STR_LEN, "route del -net %s netmask %s dev %s",
           gw_ip, inet_ntoa(addr), devname);
  ds_system_call(command, strlen(command));

  s = socket(PF_INET, SOCK_DGRAM, 0);
  memset(&buffer, 0x00, sizeof(buffer));
  memset(&scratch_buf, 0x00, sizeof(scratch_buf));
  snprintf(scratch_buf, 16, "eth%d,", this->ap_dev_num);
  strlcpy(buffer.ifr_name, scratch_buf, IFNAMSIZ);
  ioctl(s, SIOCGIFHWADDR, &buffer);
  close(s);

  /* For first AP iface */
  snprintf(command, MAX_COMMAND_STR_LEN, "arp -s   %s %02x:%02x:%02x:%02x:%02x:%02x -i eth%d",
           inet_ntoa(addr),
           buffer.ifr_hwaddr.sa_data[0],
           buffer.ifr_hwaddr.sa_data[1],
           buffer.ifr_hwaddr.sa_data[2],
           buffer.ifr_hwaddr.sa_data[3],
           buffer.ifr_hwaddr.sa_data[4],
           buffer.ifr_hwaddr.sa_data[5],
           this->ap_dev_num );
  ds_system_call(command, strlen(command));

  snprintf( command, MAX_COMMAND_STR_LEN,
            "echo 1 > /proc/sys/net/ipv4/conf/eth%d/proxy_arp", this->ap_dev_num );
  ds_system_call( command, strlen(command));

  /* Install access restrictions for the Second SSID if the profile is
     INTERNETONLY. */
  if ( ( this->wifi_mode == QCMAP_CM_WIFI_AP_AP_STA_MODE ||
         this->wifi_mode == QCMAP_CM_WIFI_AP_AP_MODE ) &&
       ( this->cfg.lan_config.interface[this->ap_dev_num+1].access_profile ==
                                               QCMAP_CM_PROFILE_INTERNET_ONLY ))
  {
    /* Install Firewall Rules to block traffic to clients connected
       to First SSID */
    snprintf( command, MAX_COMMAND_STR_LEN,
            "iptables -A FORWARD -s 0/0 -i eth%d -d 0/0 -o eth%d -p all -j DROP",
            this->ap_dev_num+1, this->ap_dev_num);
    ds_system_call( command, strlen(command));
    /* Install Firewall Rules to block traffic to First SSID */
    addr.s_addr = htonl(this->cfg.lan_config.interface[ap_dev_num].a5_ip_addr);
    strlcpy(a5_ip, inet_ntoa(addr), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
    addr.s_addr = htonl(
    this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].sub_net_mask);
    strlcpy(netmask, inet_ntoa(addr), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
    snprintf( command, MAX_COMMAND_STR_LEN,
            "iptables -A INPUT -i eth%d -d %s/%s -j DROP",
            this->ap_dev_num+1,a5_ip,netmask);
    ds_system_call( command, strlen(command));
    /* Install Firewall Rules to block traffic to USB IP. */
    addr.s_addr = htonl(this->cfg.lan_config.usb_rmnet_ip_addr);
    snprintf( command, MAX_COMMAND_STR_LEN,
            "iptables -A FORWARD -i eth%d -d %s -j DROP",
            this->ap_dev_num+1, inet_ntoa(addr));
    ds_system_call( command, strlen(command));

    addr.s_addr = htonl(this->cfg.lan_config.dun_client_ip_addr);
    snprintf( command, MAX_COMMAND_STR_LEN,
            "iptables -A FORWARD -i eth%d -d %s -j DROP",
            this->ap_dev_num+1, inet_ntoa(addr));
    ds_system_call( command, strlen(command));

    if ( this->cfg.wan_config.ip_family == QCMAP_IP_V4V6_V01 )
    {
      /* Install IPv6 Firewall Rules to block traffic to clients connected
         to First SSID */
      snprintf( command, MAX_COMMAND_STR_LEN,
               "ip6tables -A FORWARD -s 0/0 -i eth%d -d 0/0 -o eth%d -p all -j DROP",
               this->ap_dev_num+1, this->ap_dev_num);
      ds_system_call( command, strlen(command));
    }
  }
  QCMAP_CM_LOG("Linux LAN Connected\n");

  QCMAP_CM_LOG(" Start PIMD to enable MCAST forwarding.\n");
  StartMcastDaemon();

  QCMAP_CM_LOG("Disabling SDIO polling\n");
  ds_system_call("echo 0 > /sys/devices/platform/msm_sdcc.2/polling",
                 strlen("echo 0 > /sys/devices/platform/msm_sdcc.2/polling"));
  /*SDIO polling is disabled to save power*/
  return true;
}

boolean QCMAP_ConnectionManager::EnableIPV6Forwarding()
{
  char command[MAX_COMMAND_STR_LEN];
  char devname[16];
  int qcmap_cm_error;

  QCMAP_CM_LOG("Entering IPV6 Forwarding");

  if (this->cfg.wan_config.ip_family == QCMAP_IP_V4V6_V01)
  {
    if (qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_IP_V6_V01, devname, &qcmap_cm_error) != 
        QCMAP_CM_SUCCESS)
    {
      QCMAP_CM_LOG("Couldn't get ipv6 rmnet name. error %d\n", qcmap_cm_error);
      strlcpy(devname, QCMAP_V6_DEFAULT_DEVICE_NAME, 16);
    }

    QCMAP_CM_LOG("\nSetting forwarding for ipv6");
    snprintf(command, MAX_COMMAND_STR_LEN,
             "echo 2 > /proc/sys/net/ipv6/conf/%s/accept_ra", devname);
    ds_system_call(command, strlen(command));
    ds_system_call("echo 2 > /proc/sys/net/ipv6/conf/all/forwarding",
                    strlen("echo 2 > /proc/sys/net/ipv6/conf/all/forwarding"));
    ds_system_call("echo 2 > /proc/sys/net/ipv6/conf/all/proxy_ndp", 
                   strlen("echo 2 > /proc/sys/net/ipv6/conf/all/proxy_ndp"));

    QCMAP_CM_LOG("\n Adding route for inet6 ::/0\n");
    snprintf(command, MAX_COMMAND_STR_LEN,
             "ip -6 route add default dev %s", devname);
    ds_system_call(command, strlen(command));

    ds_system_call("killall -9 radish", strlen("killall -9 radish"));

    QCMAP_CM_LOG("\n Starting radish \n");
    /* Starting radish in AP mode*/
    if(this->ap_dev_num == 0)
    {
      if ( this->wifi_mode == QCMAP_CM_WIFI_AP_AP_MODE )
      {
        snprintf(command, MAX_COMMAND_STR_LEN,
                 "radish -i %s -x -i eth%d -x -i eth%d -x > /dev/null 2>&1 &",
                 devname, this->ap_dev_num, this->ap_dev_num+1);
      }
      else if ( this->wifi_mode == QCMAP_CM_WIFI_AP_MODE )
      {
        snprintf(command, MAX_COMMAND_STR_LEN,
                 "radish -i eth%d -x -i %s -x > /dev/null 2>&1 &", 
                 this->ap_dev_num, devname);
    }
    }
    else if (this->ap_dev_num == 1)
    {
      if ( this->wifi_mode == QCMAP_CM_WIFI_AP_STA_MODE )
      {
        snprintf(command, MAX_COMMAND_STR_LEN,
                 "radish -i %s -x -i eth0 -x -i eth%d -x > /dev/null 2>&1 &",
                 devname, this->ap_dev_num);
      }
      else if ( this->wifi_mode == QCMAP_CM_WIFI_AP_STA_MODE )
      {
        snprintf(command, MAX_COMMAND_STR_LEN,
                 "radish -i %s -x -i eth0 -x -i eth%d -x -i eth%d -x > /dev/null 2>&1 &",
                 devname, this->ap_dev_num, this->ap_dev_num+1);
      }
    }
    ds_system_call(command, strlen(command));
  }

  return true;
}

boolean QCMAP_ConnectionManager::DisableWLAN()
{
  char command[MAX_COMMAND_STR_LEN];
  struct in_addr addr;
  char devname[16];
  int qcmap_cm_error;

  /* Shut down interfaces.
     Proceed only upto STA iface index since STA is always last in the
     list (ap; ap,ap; ap,sta; ap,ap,sta supported).*/
  for (int i=0; i < this->sta_iface_index; i++)
  {
    if (this->cfg.lan_config.interface[i].enable)
    {
      StopHostAPD(i);
    }
  }

  LOG_MSG_INFO1("Kill cxmapp",0,0,0);
  ds_system_call("pkill cxmapp", strlen("pkill cxmapp"));

  StopDHCPD();

  /* Disconnect station mode iface from the external hotspot */
  DisconnectSTA();

  /* Unload the module for the WiFi adapter. */
  snprintf(command, MAX_COMMAND_STR_LEN, "rmmod %s", this->cfg.lan_config.module);
  ds_system_call(command,strlen(command));

  ds_system_call("echo 0 > /sys/devices/platform/msm_sdcc.2/polling",
                 strlen("echo 0 > /sys/devices/platform/msm_sdcc.2/polling"));

  ds_system_call("echo 0 > /proc/sys/net/ipv4/ip_forward",
                 strlen("echo 0 > /proc/sys/net/ipv4/ip_forward"));

  QCMAP_CM_LOG(" Stop PIMD to disable MCAST forwarding.\n");
  StopMcastDaemon();

  if(this->cfg.wan_config.ip_family == QCMAP_IP_V4V6_V01)
  {
    if (qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_IP_V6_V01, devname, &qcmap_cm_error) != 
        QCMAP_CM_SUCCESS)
    {
      QCMAP_CM_LOG("Couldn't get ipv6 rmnet name. error %d\n", qcmap_cm_error);
      strlcpy(devname, QCMAP_V6_DEFAULT_DEVICE_NAME, 16);
    }

    QCMAP_CM_LOG("\nResetting forwarding for ipv6");
    snprintf(command, MAX_COMMAND_STR_LEN,
             "echo 1 > /proc/sys/net/ipv6/conf/%s/accept_ra", devname);
    ds_system_call(command, strlen(command));
    ds_system_call("echo 0 > /proc/sys/net/ipv6/conf/all/forwarding", 
                   strlen("echo 2 > /proc/sys/net/ipv6/conf/all/forwarding"));
    ds_system_call("echo 0 > /proc/sys/net/ipv6/conf/all/proxy_ndp", 
                   strlen("echo 2 > /proc/sys/net/ipv6/conf/all/proxy_ndp"));

    QCMAP_CM_LOG("\n Deleting route for inet6 ::/0\n");
    snprintf(command, MAX_COMMAND_STR_LEN,
             "ip -6 route delete default dev %s", devname);
    ds_system_call(command, strlen(command));

    QCMAP_CM_LOG("\n Stopping radish \n");
    /* Stopping radish in AP mode*/
    ds_system_call("killall -9 radish", strlen("killall -9 radish"));

    /*-----------------------------------------------------------------------
      Delete ip6tables entries and flush tables.
    -----------------------------------------------------------------------*/
    snprintf( command, MAX_COMMAND_STR_LEN, "ip6tables --flush");
    ds_system_call(command, strlen(command));
  }

  /*-----------------------------------------------------------------------
    Delete iptables entries and flush tables.
  -----------------------------------------------------------------------*/
  snprintf( command, MAX_COMMAND_STR_LEN, "iptables --flush");
  ds_system_call(command, strlen(command));

  snprintf( command, MAX_COMMAND_STR_LEN, "iptables --table nat --flush");
  ds_system_call(command, strlen(command));

  /* Reset wifi mode */
  this->wifi_mode = QCMAP_CM_WIFI_MODE_NOT_SET;
  /* Reset STA mode specific parameters */
  this->sta_connected = false;
  this->sta_iface_index = QCMAP_LAN_INVALID_IFACE_INDEX;

  pthread_mutex_lock(&this->cm_mutex);
  this->qcmap_tear_down_in_progress = false;
  pthread_cond_signal(&this->cm_cond);
  pthread_mutex_unlock(&this->cm_mutex); 

  return true;
}

boolean QCMAP_ConnectionManager::SetDualAPConfig(boolean enable, uint32 a5_ip_addr, uint32 sub_net_mask)
{
  QCMAP_CM_LOG_FUNC_ENTRY();

  if(this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].devmode ==
                                                         QCMAP_CM_DEVMODE_STA )
  {
    QCMAP_CM_LOG("SSID2 is in STA mode. Cannot set DualAP config\n");
    return false;
  }

  /* SSID2 a5_ip_addr provided should not be in Primary SSID subnet*/
  if((this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                                                          a5_ip_addr &
      this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                                                        sub_net_mask) ==
     ( a5_ip_addr &
          this->cfg.lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].\
                                                          sub_net_mask))
  {
    QCMAP_CM_LOG("A5 IP address provided is in Primary interface subnet\n");
    this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].a5_ip_addr =
                                           ntohl(inet_addr(A5_IP_ADDR_SSID2));
    this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].sub_net_mask =
                                       ntohl(inet_addr(A5_SUB_NET_MASK_SSID2));
    QCMAP_CM_LOG("Setting SSID2 A5 IP address as %s\n",A5_IP_ADDR_SSID2);
    QCMAP_CM_LOG("Setting SSID2 subnet mask as %s\n",A5_SUB_NET_MASK_SSID2);
  }
  else
  {
    if((a5_ip_addr != 0) && (a5_ip_addr != 0xFFFFFFFFU))
    {
      this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].a5_ip_addr =
                                                                  a5_ip_addr;
    }
    else
    {
      this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].a5_ip_addr =
                                          ntohl(inet_addr(A5_IP_ADDR_SSID2));
      QCMAP_CM_LOG("SSID2 A5 IP address invalid."
                   "Changing to default value %s\n",A5_IP_ADDR_SSID2);
    }
    if((sub_net_mask != 0)&&(sub_net_mask != 0xFFFFFFFFU))
    {
      this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].sub_net_mask=
                                                                sub_net_mask;
    }
    else
    {
      this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].sub_net_mask=
                                     ntohl(inet_addr(A5_SUB_NET_MASK_SSID2));
      QCMAP_CM_LOG("SSID2 Subnet mask address invalid."
                   "Changing to default value %s\n",A5_SUB_NET_MASK_SSID2);
    }
  }

  this->cfg.lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].enable = enable;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

boolean QCMAP_ConnectionManager::EnableDualAP(void)
{
  /* Not suppported yet. Need more information about ioctl calls to.*/
  /* ifconfig eth1 up */
  /* Add routes for */
  return false;
}

boolean QCMAP_ConnectionManager::DisableDualAP(void)
{
  /* ifconfig eth1 down */
  return false;
}


/* Argument specified interface which is running in station mode. */
boolean QCMAP_ConnectionManager::EnableStaMode
(
  qcmap_sta_connection_config *cfg
)
{
  int qcmap_cm_errno;
  int ret_val;
  struct in_addr addr;
  char command[MAX_COMMAND_STR_LEN];
  char devname[16];
  boolean bring_up_wwan = FALSE;
  qcmap_sta_connection_e conn_type;
  qcmap_sta_static_ip_config static_ip_config;
/*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Can't continue if MobileAP is not enabled */
  if (!this->qcmap_enable)
  {
    QCMAP_CM_LOG("Error: QCMAP not enabled.\n");
    return FALSE;
  }

  /* Local LAN must be up by this time */
  if ( qcmap_cm_get_state() < QCMAP_CM_LAN_CONNECTED )
  {
    QCMAP_CM_LOG( "Error: QCMAP not in proper state: %d.\n",
                  qcmap_cm_get_state() );
    return FALSE;
  }

  /* Check if already in STA mode */
  if(this->sta_connected)
  {
    QCMAP_CM_LOG("Error: Already in STA connected mode.\n");
    return true;
  }

  /* Check for proper wlan card mode.
     Should be set if EnableWlan has been called before. */
  if( this->wifi_mode != QCMAP_CM_WIFI_AP_STA_MODE &&
      this->wifi_mode != QCMAP_CM_WIFI_AP_AP_STA_MODE )
  {
    QCMAP_CM_LOG("Error: Invalid wifi mode: %d.\n", this->wifi_mode);
    return false;
  }

  /* Verify valid STA iface index */
  if( this->sta_iface_index < 0 )
  {
    QCMAP_CM_LOG("Error: Invalid STA interface index.\n");
    return false;
  }

  if ( cfg == NULL )
  {
    /* Use the existing configuration from the XML. */
    conn_type = this->cfg.lan_config.interface[this->sta_iface_index].conn_type;
    static_ip_config.ip_addr = this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.ip_addr;
    static_ip_config.gw_ip = this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.gw_ip;
    static_ip_config.netmask = this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.netmask;
    static_ip_config.dns_addr = this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.dns_addr;
  }
  else
  {
    conn_type = cfg->conn_type;
    static_ip_config.ip_addr = cfg->static_ip_config.ip_addr;
    static_ip_config.gw_ip = cfg->static_ip_config.gw_ip;
    static_ip_config.netmask = cfg->static_ip_config.netmask;
    static_ip_config.dns_addr = cfg->static_ip_config.dns_addr;
  }
  /* Connect the STA to required hotspot */
  if( !ConnectSTA(conn_type,&static_ip_config) )
  {
    QCMAP_CM_LOG("Error: Connecting to hotspot failed.\n");
    return false;
  }

  /* Store the previous backhaul state. */
  if ( qcmap_cm_get_state() == QCMAP_CM_WAN_CONNECTED ||
       qcmap_cm_get_state() == QCMAP_CM_WAN_CONNECTING )
  {
    bring_up_wwan = true;
  }

  /* Disable Autoconnect. Remember the previous state. */
  this->auto_connect = this->cfg.wan_config.auto_connect;
  ret_val = qcmap_cm_set_auto_connect(this->qcmap_cm_handle, &qcmap_cm_errno, false);
  if (ret_val != QCMAP_CM_SUCCESS)
  {
    QCMAP_CM_LOG("Disable Autoconnect failed. ret_val: %d, Err: %d\n", ret_val, qcmap_cm_errno);
    QCMAP_CM_LOG("Disconnect STA mode.\n");
    DisconnectSTA();
    this->sta_connected = false;
    return false;
  }
  /* Disconnect backhaul. */
  QCMAP_CM_LOG("Disconnect Backhaul.\n");
  if ( false == DisconnectBackHaul() )
  {
    QCMAP_CM_LOG("Disconnect backhaul failed.\n");
    QCMAP_CM_LOG("Disconnect STA mode.\n");
    DisconnectSTA();
    this->sta_connected = false;
    /* Restore Autoconnect value. */
    ret_val = qcmap_cm_set_auto_connect(this->qcmap_cm_handle, &qcmap_cm_errno,
                                        this->auto_connect);
    if (ret_val != QCMAP_CM_SUCCESS)
  {
      QCMAP_CM_LOG("Restore Autoconnect failed!ret_val: %d, Error: %s\n",
                   ret_val, qcmap_cm_errno);
    }
    return false;
  }

  QCMAP_CM_LOG("Disable Nat on Q6.\n");
  /* Call qcmap CM API to send notification to modem side */
  pthread_mutex_lock(&this->cm_mutex);
  ret_val = qcmap_cm_enable_sta_mode(this->qcmap_cm_handle, &qcmap_cm_errno);
  if (ret_val != QCMAP_CM_SUCCESS)
  {
    QCMAP_CM_LOG("qcmap_cm_enable_sta_mode failed. ret_val: %d, Err: %d\n", ret_val, qcmap_cm_errno);
    pthread_mutex_unlock(&this->cm_mutex);
    DisconnectSTA();
    this->sta_connected = false;
    /* Bring up the backhaul based on the previous state. */
    if ( bring_up_wwan == true )
    {
      ConnectBackHaul();
    }
    /* Restore Autoconnect value. */
    ret_val = qcmap_cm_set_auto_connect(this->qcmap_cm_handle, &qcmap_cm_errno,
                                        this->auto_connect);
    if (ret_val != QCMAP_CM_SUCCESS)
    {
      QCMAP_CM_LOG("Restore Autoconnect failed!ret_val: %d, Error: %s\n",
                   ret_val, qcmap_cm_errno);
    }
    return false;
  }
  pthread_mutex_unlock(&this->cm_mutex);

  /* Success - Set STA mode enabled */
  this->sta_connected = true;
  QCMAP_CM_LOG("QCMAP STA MODE Enabled\n");

  /* Restart PIMD to enable MCAST forwarding on STA interface. */
  QCMAP_CM_LOG(" Restart PIMD to enable MCAST forwarding on STA interface.\n");
  ReStartMcastDaemon();

  /* Now time to enable NAT, ALGs etc., on A5*/
  this->EnableNATonA5();

  /* Store the values onto EFS */
  this->cfg.lan_config.interface[this->sta_iface_index].conn_type = conn_type;
  this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.ip_addr = static_ip_config.ip_addr;
  this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.gw_ip = static_ip_config.gw_ip;
  this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.netmask = static_ip_config.netmask;
  this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.dns_addr = static_ip_config.dns_addr;

    /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

boolean QCMAP_ConnectionManager::DisableStaMode()
{
  int qcmap_cm_errno;
  int rc = 0;
  struct timespec ts;
  char command[MAX_COMMAND_STR_LEN];
  struct in_addr addr;
  char a5_ip[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char gw_ip[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char dhcp_start_address[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char dhcp_end_address[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char netmask[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  int ret_val;
  char devname[16];
/*-------------------------------------------------------------------------*/

  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Cant continue if MobileAP is not enabled! */
  if (!this->qcmap_enable)
  {
    return FALSE;
  }

  /* Local LAN must be up by this time */
  if ( qcmap_cm_get_state() < QCMAP_CM_LAN_CONNECTED )
  {
    QCMAP_CM_LOG( "Error: QCMAP not in proper state: %d.\n",
                  qcmap_cm_get_state() );
    return FALSE;
  }

  /* Verify STA mode */
  if(!this->sta_connected)
  {
    return FALSE;
  }

  /* Send notification to modem side via QCMAP CM API */
  QCMAP_CM_LOG("Sending msg to modem with NAT type %d\n",
                                  (int)this->cfg.nat_config.nat_type);
  pthread_mutex_lock(&this->cm_mutex);
  ret_val = qcmap_cm_disable_sta_mode(this->qcmap_cm_handle,
                                      &qcmap_cm_errno);

  if (ret_val != QCMAP_CM_SUCCESS)
  {
    QCMAP_CM_LOG("Error: Disable STA Mode Failed with error code: %d.\n",
                 qcmap_cm_errno );
    pthread_mutex_unlock(&this->cm_mutex);
    return FALSE;
  }
  pthread_mutex_unlock(&this->cm_mutex);

  QCMAP_CM_LOG("QCMAP STA MODE Disabled\n");

  /* Disconnect STA mode. */
  this->DisconnectSTA();

  /* Disable NAT on A5 */
  this->DisableNATonA5();

  /* Reset STA mode specific parameters */
  this->sta_connected = false;

  /* Restart PIMD to disable MCAST forwarding on STA interface. */
  QCMAP_CM_LOG(" Restart PIMD to disable MCAST forwarding on STA interface.\n");
  ReStartMcastDaemon();

  /* Connect to Backhaul based on the previous state. */
  if ( this->auto_connect == true )
  {
    if ( false == ConnectBackHaul() )
    {
      QCMAP_CM_LOG("Connecting to backhaul fails. Try connecting later.\n");
    }
    else
    {
      QCMAP_CM_LOG("Connected to backhaul.\n");
    }
    /* Restore Autoconnect value. */
    ret_val = qcmap_cm_set_auto_connect(this->qcmap_cm_handle, &qcmap_cm_errno,
                                        this->auto_connect);
    if (ret_val != QCMAP_CM_SUCCESS)
    {
      QCMAP_CM_LOG("Restore Autoconnect failed!ret_val: %d, Error: %s\n",
                   ret_val, qcmap_cm_errno);
    }
    else
    {
      this->WriteConfigToXML();
    }
  }
  return true;
}

boolean QCMAP_ConnectionManager::EnableNATonA5()
{
  char command[MAX_COMMAND_STR_LEN];
  struct in_addr addr;
  int i = 0;
  char devname[16];
  qcmap_cm_firewall_entry_conf_t firewall_entry;
  int qcmap_cm_error;
  char netMask[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
/*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Cant continue if MobileAP is not enabled! */
  if (!this->qcmap_enable)
  {
    QCMAP_CM_LOG("Error: QCMAP not enabled.\n");
    return false;
  }

  /* Must be in STA mode before we start setting up NAT on A5 */
  if(!this->sta_connected)
  {
    QCMAP_CM_LOG("Error: QCMAP not in STA mode.\n");
    return false;
  }

  /*-----------------------------------------------------------------------
    Delete existing rules and flush tables
  -----------------------------------------------------------------------*/
  snprintf( command, MAX_COMMAND_STR_LEN, "iptables --flush");
  ds_system_call(command, strlen(command));
  QCMAP_CM_LOG("%s\n", command);

  snprintf( command, MAX_COMMAND_STR_LEN, "iptables --table nat --flush");
  ds_system_call(command, strlen(command));
  QCMAP_CM_LOG("%s\n", command);

  snprintf( command, MAX_COMMAND_STR_LEN, "iptables --delete-chain");
  ds_system_call(command, strlen(command));
  QCMAP_CM_LOG("%s\n", command);

  snprintf( command, MAX_COMMAND_STR_LEN, "iptables --table nat --delete-chain");
  ds_system_call(command, strlen(command));
  QCMAP_CM_LOG("%s\n", command);

  if(this->cfg.nat_config.nat_type == QCMAP_CM_PORT_RESTRICTED_CONE_NAT)
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -t nat -A POSTROUTING -o eth0 -j SNAT --to-source %s",
              this->staIpAddr);
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);
  }
  else //Symmetric NAT
  {
    /*-----------------------------------------------------------------------
      Set up forwarding and masquerading.
    -----------------------------------------------------------------------*/
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE" );
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);
  }

  /* Both AP dev are in interfaces */
  snprintf( command, MAX_COMMAND_STR_LEN,
            "iptables --append FORWARD --in-interface eth%d -j ACCEPT",
            this->ap_dev_num );
  ds_system_call(command, strlen(command));
  QCMAP_CM_LOG("%s\n", command);
  if(this->wifi_mode == QCMAP_CM_WIFI_AP_AP_STA_MODE)
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables --append FORWARD --in-interface eth%d -j ACCEPT",
              this->ap_dev_num+1 );
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);
  }

  /* Set the rmnet ifaces as in interfaces */
  strlcpy(devname, QCMAP_V4_DEFAULT_DEVICE_NAME, 16);
  snprintf( command, MAX_COMMAND_STR_LEN,
            "iptables --append FORWARD --in-interface %s -j ACCEPT", devname);
  ds_system_call(command, strlen(command));
  QCMAP_CM_LOG("%s\n", command);

  /*-----------------------------------------------------------------------
    Enable packet forwarding
  -----------------------------------------------------------------------*/
  snprintf( command, MAX_COMMAND_STR_LEN,
            "echo 1 > /proc/sys/net/ipv4/ip_forward");
  ds_system_call(command, strlen(command));
  QCMAP_CM_LOG("%s\n", command);

  /*-----------------------------------------------------------------------
    Enable ARP Proxy on STA interface
  -----------------------------------------------------------------------*/
  snprintf( command, MAX_COMMAND_STR_LEN,
            "echo 1 > /proc/sys/net/ipv4/conf/eth0/proxy_arp" );
  ds_system_call(command, strlen(command));
  QCMAP_CM_LOG("%s\n", command);

  QCMAP_CM_LOG(" NAT Enabled.\n");

  /*-----------------------------------------------------------------------
    Delete default routes via rmnet to force packets through eth0
  -----------------------------------------------------------------------*/
  memset(devname,0,16);
  if (qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_IP_V4_V01, devname,
                                       &qcmap_cm_error) != QCMAP_CM_SUCCESS)
  {
    QCMAP_CM_LOG("Couldn't get ipv4 rmnet name. error %d\n", qcmap_cm_error);
    strlcpy(devname, QCMAP_V4_DEFAULT_DEVICE_NAME, 16);
  }

  snprintf(command, MAX_COMMAND_STR_LEN,
           "route del default dev %s", devname);
  ds_system_call(command, strlen(command));

  /* Enable IPv6 Forwarding in STA mode. */
  if ( this->cfg.wan_config.ip_family == QCMAP_IP_V4V6_V01 )
  {
    /* Delete the existing default route. */
    memset(devname,0,16);
    if (qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_IP_V6_V01, devname,
                              &qcmap_cm_error) != QCMAP_CM_SUCCESS)
    {
      QCMAP_CM_LOG("Couldn't get ipv6 rmnet name. error %d\n", qcmap_cm_error);
      strlcpy(devname, QCMAP_V6_DEFAULT_DEVICE_NAME, 16);
    }
    snprintf(command, MAX_COMMAND_STR_LEN,
             "ip -6 route del default dev %s metric 1024", devname);
    ds_system_call(command, strlen(command));
    /* Enable accept_ra for STA interface. */
    snprintf(command, MAX_COMMAND_STR_LEN,
             "echo 2 > /proc/sys/net/ipv6/conf/eth0/accept_ra");
    ds_system_call(command, strlen(command));
    /* Add the default route. */
    snprintf(command, MAX_COMMAND_STR_LEN,
             "ip -6 route add default dev eth0 metric 0");
    ds_system_call(command, strlen(command));
  }


  /* Propagating existing rules. */

  /* Config DMZ IP.*/
  if ( this->cfg.nat_config.dmz_ip != 0 )
  {
    AddDMZOnA5(this->cfg.nat_config.dmz_ip);
  }

  /* Config SNAT */
  for (i = 0; i < this->cfg.nat_config.num_port_fwding_entries; i++)
  {
      AddSNATEntryOnA5(&this->cfg.nat_config.port_fwding_entries[i]);
  }

  /* Config Firewall Entries */
  /*Propagate existing v4 and v6 firewall entries to A5 only if firewall is enabled*/
  if (this->cfg.nat_config.firewall_enabled)
  {
   for (i = 0; i < this->cfg.nat_config.num_extd_firewall_entries; i++)
   {
      /*In STA mode, not using the handle previously assigned by the modem.
      These handles are updated with the handles assigned by modem while
      disabling STA mode */
      ModifyExtdFirewallEntryOnA5(
            &this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry, 
            TRUE,
            this->cfg.nat_config.firewall_pkts_allowed);
  }
 }

  /* Configure VPN PassThrough. */
  SetIPSECVpnPassThroughOnA5(this->cfg.nat_config.enable_ipsec_vpn_pass_through);
  SetL2TPVpnPassThroughOnA5(this->cfg.nat_config.enable_l2tp_vpn_pass_through);
  SetPPTPVpnPassThroughOnA5(this->cfg.nat_config.enable_pptp_vpn_pass_through);

  return true;
}

boolean QCMAP_ConnectionManager::DisableNATonA5()
{
  char command[MAX_COMMAND_STR_LEN];
  char devname[16];
  int qcmap_cm_error;
/*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Cant continue if SoftAP is not enabled! */
  if (!this->qcmap_enable)
  {
    QCMAP_CM_LOG("Error: QCMAP not enabled.\n");
    return false;
  }

  if(!this->sta_connected)
  {
    QCMAP_CM_LOG("Error: QCMAP not in STA mode.\n");
    return false;
  }

  /*-----------------------------------------------------------------------
    Delete iptables entries and flush tables.
  -----------------------------------------------------------------------*/
  snprintf( command, MAX_COMMAND_STR_LEN, "iptables --flush");
  ds_system_call(command, strlen(command));

  snprintf( command, MAX_COMMAND_STR_LEN, "iptables --table nat --flush");
  ds_system_call(command, strlen(command));

  /* Add the default v4 route via rmnet. */
  memset(devname,0,16);
  if (qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_IP_V4_V01, devname,
                                       &qcmap_cm_error) != QCMAP_CM_SUCCESS)
  {
    QCMAP_CM_LOG("Couldn't get ipv4 rmnet name. error %d\n", qcmap_cm_error);
    strlcpy(devname, QCMAP_V4_DEFAULT_DEVICE_NAME, 16);
  }

  snprintf(command, MAX_COMMAND_STR_LEN,
           "route add default dev %s", devname);
  ds_system_call(command, strlen(command));

  /* Disable IPv6 Forwarding in STA mode. */
  if ( this->cfg.wan_config.ip_family == QCMAP_IP_V4V6_V01 )
  {
    /* Delete the existing default route. */
    snprintf(command, MAX_COMMAND_STR_LEN,
             "ip -6 route del default dev eth0 metric 0");
    ds_system_call(command, strlen(command));
    /* Disable accept_ra for STA interface. */
    snprintf(command, MAX_COMMAND_STR_LEN,
             "echo 0 > /proc/sys/net/ipv6/conf/eth0/accept_ra");
    ds_system_call(command, strlen(command));
    /* Add the default route. */
    memset(devname,0,16);
    if (qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_IP_V6_V01, devname,
                              &qcmap_cm_error) != QCMAP_CM_SUCCESS)
    {
      QCMAP_CM_LOG("Couldn't get ipv6 rmnet name. error %d\n", qcmap_cm_error);
      strlcpy(devname, QCMAP_V6_DEFAULT_DEVICE_NAME, 16);
    }
    snprintf(command, MAX_COMMAND_STR_LEN,
             "ip -6 route add default dev %s metric 0", devname);
    ds_system_call(command, strlen(command));
  }

  return true;
}


boolean QCMAP_ConnectionManager::ReadConfigFromXML()
{
  if ((this->xml_path) && (this->xml_path[0] != '\0') &&
      (qcmap_cm_read_xml(this->xml_path, &this->cfg) == QCMAP_CM_SUCCESS))
  {
    return true;
  }
  else
    return false;
}

boolean QCMAP_ConnectionManager::ReadConfigFromFirewallXML()
{

  if ((this->cfg.nat_config.firewall_config_file) && (this->cfg.nat_config.firewall_config_file[0] != '\0') &&
      (qcmap_cm_read_firewall_xml(this->cfg.nat_config.firewall_config_file, &this->cfg) == QCMAP_CM_SUCCESS))
  {
    return true;
  }
  else
    return false;
}

boolean QCMAP_ConnectionManager::WriteConfigToXML()
{
  if ((this->xml_path) && (this->xml_path[0] != '\0') &&
      (qcmap_cm_write_xml(this->xml_path, &this->cfg) == QCMAP_CM_SUCCESS))
  {
    return true;
  }
  else
    return false;
}

boolean QCMAP_ConnectionManager::WriteConfigToFirewallXML()
{
  if ((this->cfg.nat_config.firewall_config_file) && (this->cfg.nat_config.firewall_config_file[0] != '\0') &&
      (qcmap_cm_write_firewall_xml(this->cfg.nat_config.firewall_config_file, &this->cfg) == QCMAP_CM_SUCCESS))
  {
    return true;
  }
  else
    return false;
}

boolean QCMAP_ConnectionManager::ConnectBackHaul()
{
  int qcmap_cm_errno;
  int ret_val;
  int rc = 0;
  struct timespec ts;
  uint32 public_ip = 0, pri_dns_addr = 0, sec_dns_addr = 0;
  struct in_addr addr;
  char command[MAX_COMMAND_STR_LEN];

  QCMAP_CM_LOG_FUNC_ENTRY();

  if (this->sta_connected)
  {
    QCMAP_CM_LOG("Currently in STA mode - ConnectBackHaul not valid\n");
    return false;
  }

  pthread_mutex_lock(&this->cm_mutex);
  this->wan_connecting_in_process = true;
  pthread_mutex_unlock(&this->cm_mutex);
  ret_val = qcmap_cm_connect_backhaul(this->qcmap_cm_handle, &qcmap_cm_errno);

  if (ret_val == QCMAP_CM_SUCCESS)
  {
    pthread_mutex_lock(&this->cm_mutex);
    this->wan_connecting_in_process = false;
    pthread_mutex_unlock(&this->cm_mutex);
    return true;
  }

  if (ret_val == QCMAP_CM_ERROR && qcmap_cm_errno == QCMAP_CM_EWOULDBLOCK)
  {
    QCMAP_CM_LOG("QCMAP WAN Connecting Inprogress \n");
    return true;
  }
  else
  {
    pthread_mutex_lock(&this->cm_mutex);
    this->wan_connecting_in_process = false;
    pthread_mutex_unlock(&this->cm_mutex);
    QCMAP_CM_LOG( "QCMAP WAN Connecting Fail, ret_val %d qcmap_cm_errno %d\n",
                  ret_val, qcmap_cm_errno );
    return false;
  }
}

boolean QCMAP_ConnectionManager::EnableDNS()
{
  int qcmap_cm_errno;
  int ret_val;
  int rc = 0;
  struct timespec ts;
  uint32 public_ip = 0, pri_dns_addr = 0, sec_dns_addr = 0;
  struct in_addr addr;
  char command[MAX_COMMAND_STR_LEN];

  if (qcmap_cm_get_ipv4_net_conf(this->qcmap_cm_handle, &public_ip, &pri_dns_addr, &sec_dns_addr, &qcmap_cm_errno) == QCMAP_CM_SUCCESS)
  {
    if (pri_dns_addr)
    {
      addr.s_addr = htonl(pri_dns_addr);
      snprintf(command, MAX_COMMAND_STR_LEN, "echo 'nameserver %s' > /etc/resolv.conf", inet_ntoa(addr));
      QCMAP_CM_LOG("QCMAP PRI DNS %s\n", inet_ntoa(addr));
      ds_system_call(command, strlen(command));
    }

    if (sec_dns_addr)
    {
      addr.s_addr = htonl(sec_dns_addr);
      snprintf(command, MAX_COMMAND_STR_LEN, "echo 'nameserver %s' >> /etc/resolv.conf", inet_ntoa(addr));
      QCMAP_CM_LOG("QCMAP SEC DNS %s\n", inet_ntoa(addr));
      ds_system_call(command, strlen(command));
    }
          
    QCMAP_CM_LOG("QCMAP DNS Enabled \n");
    return true;
  }	
  QCMAP_CM_LOG("QCMAP DNS not enabled \n");

  return false;
}


boolean QCMAP_ConnectionManager::DisconnectBackHaul()
{
  int qcmap_cm_errno;
  int ret_val;

  QCMAP_CM_LOG_FUNC_ENTRY();
  ret_val = qcmap_cm_disconnect_backhaul(this->qcmap_cm_handle, &qcmap_cm_errno);

  if (ret_val == QCMAP_CM_SUCCESS)
  {
    this->enable_dns = true;

    /* In order to proceed changing NAT Type if WAN is already disconnected*/
    if(this->qcmap_nat_type_change_in_progress)
    {
      this->SetNatType();
    }

    return true;
  }
  if (ret_val == QCMAP_CM_ERROR && qcmap_cm_errno == QCMAP_CM_EWOULDBLOCK)
  {
    /* Backhaul disconnecting in progress */
    this->enable_dns = true;
    return true;
  }
  else
  {
    return false;
  }
}

boolean QCMAP_ConnectionManager::DisplayModemConfig(void)
{
  int i, err = 0;
  int *qcmap_cm_errno = &err;
  in_addr addr;
  qcmap_cm_port_fwding_entry_conf_t port_fwding_entries[QCMAP_MAX_SNAT_ENTRIES_V01];
  qcmap_cm_firewall_entry_conf_t    firewall_entries[QCMAP_MAX_FIREWALL_ENTRIES_V01];
  in_addr dmz_ip;
  uint16_t nat_timeout = 0;
  uint8_t firewall_enabled = 0, pkts_allowed = 0, ipsec = 0, pptp = 0, l2tp = 0;

  if ( this->sta_connected )
  {
    /* STA mode */
    printf(" In STA mode, no need to print Modem Side Config!!\n\n");
    return true;
  }

  /* Display SNAT Config */
  i = qcmap_cm_static_nat_op(this->qcmap_cm_handle, QCMAP_CM_GET_SNAT,
                           port_fwding_entries, qcmap_cm_errno);

  if (i > 0)
  {
    while (i--)
    {
      printf("Index %d : \n", i);
      addr.s_addr = htonl(port_fwding_entries[i].port_fwding_private_ip);
      printf("  port_fwding_private_ip   : %s\n", inet_ntoa(addr));
      printf("  port_fwding_private_port : %d\n", port_fwding_entries[i].port_fwding_private_port);
      printf("  port_fwding_global_port  : %d\n", port_fwding_entries[i].port_fwding_global_port);
      printf("  port_fwding_protocol     : %d\n\n", port_fwding_entries[i].port_fwding_protocol);
    }
  }
  else if (i == 0)
  {
    printf("No Static NAT Entries configured.\n");
  }
  else
  {
    printf("Get Static Nat returned %d sam_cm_errno %d.\n", i, *qcmap_cm_errno);
  }

  /* Display Firewall Config */
  i = qcmap_cm_firewall_op(this->qcmap_cm_handle, QCMAP_CM_GET_FIREWALL, &pkts_allowed, qcmap_cm_errno);
  if (i < 0)
  {
    printf("Get firewall state returned %d qcmap_cm_errno %d.\n", i, *qcmap_cm_errno);
  }
  else if (i > 0)
  {
    printf("Firewall is enabled. Packets allowed? %d.\n", pkts_allowed);
  }
  else
  {
    printf("Firewall is disabled.\n");
  }

  /* Display Firewall Entries Config */
  i = qcmap_cm_firewall_entry_op(this->qcmap_cm_handle, QCMAP_CM_GET_FIREWALL_ENTRY_RULE,
                         firewall_entries, qcmap_cm_errno);
  if (i > 0)
  {
    while (i--)
    {
      printf("Index %d : \n", i);
      printf("  firewall_start_dest_port : %d\n", firewall_entries[i].firewall_start_dest_port);
      printf("  firewall_end_dest_port   : %d\n", firewall_entries[i].firewall_end_dest_port);
      printf("  firewall_protocol        : %d\n", firewall_entries[i].firewall_protocol);
      printf("  firewall_handle          : %X\n", firewall_entries[i].firewall_handle);
    }
  }
  else if (i == 0)
  {
    printf("No Firewall Rules configured.\n");
  }
  else
  {
    printf("Get Firewall Rule returned %d qcmap_cm_errno.%d\n", i, *qcmap_cm_errno);
  }

  dmz_ip.s_addr = 0;

  /* Config DMZ */
  i = qcmap_cm_dmz_op(this->qcmap_cm_handle, QCMAP_CM_GET_DMZ, (uint32*) &(dmz_ip.s_addr), qcmap_cm_errno);

  if (i < 0)
  {
    printf("Get DMZ returned %d qcmap_cm_errno %d.\n", i, *qcmap_cm_errno);
  }
  else
  {
    dmz_ip.s_addr = htonl(dmz_ip.s_addr);
    printf("DMZ IP = %s\n", inet_ntoa(dmz_ip));
  }
      /* Config NAT timeout */
  i = qcmap_cm_nat_timeout_op(this->qcmap_cm_handle, QCMAP_CM_GET_NAT_ENTRY_TIMEOUT,
                            &nat_timeout, qcmap_cm_errno);

  if (i < 0)
  {
    printf("Get NAT Timeout returned %d qcmap_cm_errno %d.\n", i, *qcmap_cm_errno);
  }
  else
  {
    printf("NAT Entry Timeout = %d\n", nat_timeout);
  }

  i = qcmap_cm_vpn_op(this->qcmap_cm_handle, QCMAP_CM_GET_IPSEC, &ipsec, qcmap_cm_errno);

  if (i < 0)
  {
    printf("Get IPSEC VPN Pass Through returned %d qcmap_cm_errno.\n", i, *qcmap_cm_errno);
  }
  else
  {
    printf("IPSEC VPN Pass Through = %d\n", ipsec);
  }

  i = qcmap_cm_vpn_op(this->qcmap_cm_handle, QCMAP_CM_GET_PPTP, &pptp, qcmap_cm_errno);

  if (i < 0)
  {
    printf("Get PPTP VPN Pass Through returned %d qcmap_cm_errno.\n", i, *qcmap_cm_errno);
  }
  else
  {
    printf("PPTP VPN Pass Through = %d\n", pptp);
  }

  i = qcmap_cm_vpn_op(this->qcmap_cm_handle, QCMAP_CM_GET_L2TP, &l2tp, qcmap_cm_errno);

  if (i < 0)
  {
    printf("Get L2TP VPN Pass Through returned %d qcmap_cm_errno.\n", i, *qcmap_cm_errno);
  }
  else
  {
    printf("L2TP VPN Pass Through = %d\n", l2tp);
  }

  return true;
}

boolean QCMAP_ConnectionManager::AddStaticNatEntry(qcmap_cm_port_fwding_entry_conf_t* nat_entry)
{
  int qcmap_cm_errno;
  int ret_val, i;

  QCMAP_CM_LOG_FUNC_ENTRY();
  for (i = 0; i < this->cfg.nat_config.num_port_fwding_entries; i++)
  {
    if (memcmp(nat_entry, &this->cfg.nat_config.port_fwding_entries[i], sizeof(qcmap_cm_port_fwding_entry_conf_t)) == 0)
    {
      /* we already added this one */
      return false;
    }
  }

  if (this->cfg.nat_config.num_port_fwding_entries == QCMAP_MAX_SNAT_ENTRIES_V01)
  {
    /* we reach MAX entries */
    return false;
  }

  if (!this->sta_connected)
  {
    /* call CM lib to add NAT entry */
    ret_val = qcmap_cm_static_nat_op(this->qcmap_cm_handle, QCMAP_CM_ADD_SNAT, nat_entry, &qcmap_cm_errno);

    if (ret_val != QCMAP_CM_SUCCESS)
      return false;
  }
  else
  {
    if(!AddSNATEntryOnA5(nat_entry))
      return false;
  }

  /* save into the config */
  memcpy(&this->cfg.nat_config.port_fwding_entries[this->cfg.nat_config.num_port_fwding_entries],
         nat_entry, sizeof(qcmap_cm_port_fwding_entry_conf_t));
  this->cfg.nat_config.num_port_fwding_entries++;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

boolean QCMAP_ConnectionManager::DeleteStaticNatEntry(qcmap_cm_port_fwding_entry_conf_t* nat_entry)
{
  int qcmap_cm_errno;
  int ret_val, i, j;

  for (i = 0; i < this->cfg.nat_config.num_port_fwding_entries; i++)
  {
    if ((nat_entry->port_fwding_protocol == this->cfg.nat_config.port_fwding_entries[i].port_fwding_protocol) &&
        (nat_entry->port_fwding_private_ip == this->cfg.nat_config.port_fwding_entries[i].port_fwding_private_ip) &&
        (nat_entry->port_fwding_private_port == this->cfg.nat_config.port_fwding_entries[i].port_fwding_private_port) &&
        (nat_entry->port_fwding_global_port == this->cfg.nat_config.port_fwding_entries[i].port_fwding_global_port))
    {
      break;
    }
  }

  if (i == this->cfg.nat_config.num_port_fwding_entries)
  {
      /* we do not have this one  */
      return false;
  }

  if(!this->sta_connected)
  {
    /* call CM lib to add NAT entry */
    ret_val = qcmap_cm_static_nat_op(this->qcmap_cm_handle, QCMAP_CM_DELETE_SNAT, nat_entry, &qcmap_cm_errno);

    if (ret_val != QCMAP_CM_SUCCESS)
      return false;
  }
  else
  {
    if(!DeleteSNATEntryOnA5(nat_entry))
      return false;
  }

  /* save into the config */
  for (j = i; j < (this->cfg.nat_config.num_port_fwding_entries - 1); j++)
  {
    memcpy(&this->cfg.nat_config.port_fwding_entries[j], &this->cfg.nat_config.port_fwding_entries[j+1],
           sizeof(qcmap_cm_port_fwding_entry_conf_t));
  }
  this->cfg.nat_config.num_port_fwding_entries--;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

int QCMAP_ConnectionManager::GetStaticNatEntry(qcmap_cm_port_fwding_entry_conf_t* nat_entry, int max_entries)
{
  int i;
  int ret_entry;

  if (this->cfg.nat_config.num_port_fwding_entries > max_entries)
    ret_entry = max_entries;
  else
    ret_entry = this->cfg.nat_config.num_port_fwding_entries;

  for (i = 0; i < ret_entry; i++)
  {
    memcpy(&nat_entry[i], &this->cfg.nat_config.port_fwding_entries[i], sizeof(qcmap_cm_port_fwding_entry_conf_t));
  }

  return ret_entry;
}

boolean QCMAP_ConnectionManager::AddFireWallEntry(qcmap_cm_firewall_entry_conf_t* firewall_entry)
{
  int qcmap_cm_errno;
  int ret_val, i;
  qcmap_cm_extd_firewall_entry_conf_t extd_firewall_entry;

  LOG_MSG_INFO1("enter firewall add",0,0,0);

  memset(&extd_firewall_entry, 0, sizeof(qcmap_cm_extd_firewall_entry_conf_t));

  extd_firewall_entry.filter_spec.ip_vsn = IP_V4;

  extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot = firewall_entry->firewall_protocol;

  switch(firewall_entry->firewall_protocol)
  {
  case PS_IPPROTO_ICMP:
    extd_firewall_entry.filter_spec.next_prot_hdr.icmp.type = firewall_entry->firewall_start_dest_port;
    extd_firewall_entry.filter_spec.next_prot_hdr.icmp.code = firewall_entry->firewall_end_dest_port;
    extd_firewall_entry.filter_spec.next_prot_hdr.icmp.field_mask|= IPFLTR_MASK_ICMP_MSG_TYPE;
    extd_firewall_entry.filter_spec.next_prot_hdr.icmp.field_mask|= IPFLTR_MASK_ICMP_MSG_CODE;
    LOG_MSG_INFO1("firewall protocol add icmp type %d code %d", 
                  extd_firewall_entry.filter_spec.next_prot_hdr.icmp.type,
                  extd_firewall_entry.filter_spec.next_prot_hdr.icmp.code,0);
    break;

  case PS_IPPROTO_TCP:
    extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.port = firewall_entry->firewall_start_dest_port;
    extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.range = 
      firewall_entry->firewall_end_dest_port - firewall_entry->firewall_start_dest_port;
    extd_firewall_entry.filter_spec.next_prot_hdr.tcp.field_mask|= IPFLTR_MASK_TCP_DST_PORT;
    LOG_MSG_INFO1("firewall protocol add tcp port %d range %d", 
                  extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.port,
                  extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.range,0);
    break;

  case PS_IPPROTO_UDP:
    extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.port = firewall_entry->firewall_start_dest_port;
    extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.range = 
      firewall_entry->firewall_end_dest_port - firewall_entry->firewall_start_dest_port;
    extd_firewall_entry.filter_spec.next_prot_hdr.udp.field_mask|= IPFLTR_MASK_UDP_DST_PORT;
    LOG_MSG_INFO1("firewall protocol add udp port %d range %d", 
                  extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.port,
                  extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.range,0);
    break;

  case PS_IPPROTO_TCP_UDP:
    extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.port = firewall_entry->firewall_start_dest_port;
    extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.range = 
      firewall_entry->firewall_end_dest_port - firewall_entry->firewall_start_dest_port;
    extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask|= IPFLTR_MASK_TCP_UDP_DST_PORT;
    LOG_MSG_INFO1("firewall protocol add tcp_udp port %d range %d", 
                  extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.port,
                  extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.range,0);
    break;

  default:
    LOG_MSG_ERROR("Unsupported protocol %d", firewall_entry->firewall_protocol,0,0);
    }
  
  for (i = 0; i < this->cfg.nat_config.num_extd_firewall_entries; i++)
  {
    if (memcmp(&(extd_firewall_entry.filter_spec), 
               &(this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec), 
               sizeof(ip_filter_type)) == 0)
    {
      /* we already added this one */
      LOG_MSG_ERROR("Firewall entry already present",0,0,0);
      return false;
    }
  }

  if (this->cfg.nat_config.num_extd_firewall_entries == QCMAP_MAX_FIREWALL_ENTRIES_V01)
  {
    /* we reach MAX entries */
    LOG_MSG_ERROR("Exceeded maximum number of firewall entries",0,0,0);
    return false;
  }

  LOG_MSG_INFO1("add firewall protocol %d port %d range %d",
                firewall_entry->firewall_protocol,
                firewall_entry->firewall_start_dest_port,
                firewall_entry->firewall_end_dest_port);

  if(!this->sta_connected)
  {
    /* call CM lib to add Firewall entry */
    ret_val = qcmap_cm_firewall_entry_op(this->qcmap_cm_handle, QCMAP_CM_ADD_FIREWALL_ENTRY_RULE, firewall_entry, &qcmap_cm_errno);

    if (ret_val != QCMAP_CM_SUCCESS)
      return false;
  }
  else
  {  /*Add Firewall entry on A5 only when firewall enabled*/
    if(this->cfg.nat_config.firewall_enabled)
    {
      if(!this->ModifyExtdFirewallEntryOnA5(
                            &extd_firewall_entry,
                            TRUE,
                            this->cfg.nat_config.firewall_pkts_allowed))
      return false;
    }
  }

  memset(&(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry),
         0, sizeof(qcmap_cm_extd_firewall_entry_conf_t));

  /* When not in AP+STA mode, the handle got from the modem side is saved. */
  if(!this->sta_connected)
    this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.firewall_handle =
      firewall_entry->firewall_handle;

  this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.ip_vsn = 
    IP_V4;
  this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot=
    firewall_entry->firewall_protocol;

  switch(firewall_entry->firewall_protocol)
  {
  case PS_IPPROTO_TCP:
    memcpy(&(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.next_prot_hdr.tcp),
           &(extd_firewall_entry.filter_spec.next_prot_hdr.tcp),
           sizeof(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.next_prot_hdr.tcp));
    break;

  case PS_IPPROTO_UDP:
    memcpy(&(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.next_prot_hdr.udp),
           &(extd_firewall_entry.filter_spec.next_prot_hdr.udp),
           sizeof(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.next_prot_hdr.udp));
    break;

  case PS_IPPROTO_ICMP:
    memcpy(&(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.next_prot_hdr.icmp),
           &(extd_firewall_entry.filter_spec.next_prot_hdr.icmp),
           sizeof(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.next_prot_hdr.icmp));
    break;

  case PS_IPPROTO_TCP_UDP:
    memcpy(&(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range),
           &(extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range),
           sizeof(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range));
    break;

  default:
    break;
  }
  this->cfg.nat_config.num_extd_firewall_entries++;

  /* if it is OK, call write to XML */
  this->WriteConfigToFirewallXML();

  return true;
}

boolean QCMAP_ConnectionManager::DeleteFireWallEntry(qcmap_cm_firewall_entry_conf_t* firewall_entry)
{
  int qcmap_cm_errno;
  int ret_val, i, j;
  qcmap_cm_extd_firewall_entry_conf_t extd_firewall_entry;

  memset(&extd_firewall_entry, 0, sizeof(qcmap_cm_extd_firewall_entry_conf_t));

  extd_firewall_entry.filter_spec.ip_vsn = IP_V4;
  extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot = firewall_entry->firewall_protocol;

  LOG_MSG_INFO1("delete firewall protocol %d, port %d, range %d",
                firewall_entry->firewall_protocol,
                firewall_entry->firewall_start_dest_port,
                firewall_entry->firewall_start_dest_port);

  switch(firewall_entry->firewall_protocol)
  {
  case PS_IPPROTO_ICMP:
    extd_firewall_entry.filter_spec.next_prot_hdr.icmp.type = firewall_entry->firewall_start_dest_port;
    extd_firewall_entry.filter_spec.next_prot_hdr.icmp.code = firewall_entry->firewall_end_dest_port;
    extd_firewall_entry.filter_spec.next_prot_hdr.icmp.field_mask|= IPFLTR_MASK_ICMP_MSG_TYPE;
    extd_firewall_entry.filter_spec.next_prot_hdr.icmp.field_mask|= IPFLTR_MASK_ICMP_MSG_CODE;
    break;

  case PS_IPPROTO_TCP:
    extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.port = firewall_entry->firewall_start_dest_port;
    extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.range = 
      firewall_entry->firewall_end_dest_port - firewall_entry->firewall_start_dest_port;
    extd_firewall_entry.filter_spec.next_prot_hdr.tcp.field_mask|= IPFLTR_MASK_TCP_DST_PORT;
    break;

  case PS_IPPROTO_UDP:
    extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.port = firewall_entry->firewall_start_dest_port;
    extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.range = 
      firewall_entry->firewall_end_dest_port - firewall_entry->firewall_start_dest_port;
    extd_firewall_entry.filter_spec.next_prot_hdr.udp.field_mask|= IPFLTR_MASK_UDP_DST_PORT;
    break;

  case PS_IPPROTO_TCP_UDP:
    extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.port = firewall_entry->firewall_start_dest_port;
    extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.range = 
      firewall_entry->firewall_end_dest_port - firewall_entry->firewall_start_dest_port;
    extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask|= IPFLTR_MASK_TCP_UDP_DST_PORT;
    break;

  default:
    LOG_MSG_ERROR("Unsupported protocol %d", firewall_entry->firewall_protocol,0,0);
  }

  for (i = 0; i < this->cfg.nat_config.num_extd_firewall_entries; i++)
  {
    /* Do not compare the handle value, the last uint32 bytes of the struct. */
    if (memcmp(&(extd_firewall_entry.filter_spec), &(this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec), 
               sizeof(extd_firewall_entry.filter_spec)) == 0)
    {
      QCMAP_CM_LOG("Found matching handle %d and index is %d",
                   this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.firewall_handle, i,0 );
      firewall_entry->firewall_handle = this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.firewall_handle;
      break;
    }
  }

  if (i == this->cfg.nat_config.num_extd_firewall_entries)
  {
    /* we do not have this one  */
    QCMAP_CM_LOG("Did not find matching handle");
    return false;
  }

  
  if(!this->sta_connected)
  {
  /* call CM lib to Delete FireWall entry */
    ret_val = qcmap_cm_firewall_entry_op( this->qcmap_cm_handle,
                                          QCMAP_CM_DELETE_FIREWALL_ENTRY_RULE,
                                          firewall_entry, &qcmap_cm_errno);
    if (ret_val != QCMAP_CM_SUCCESS)
      return false;
  }
  else
  {
    if(!this->ModifyExtdFirewallEntryOnA5(
                       &extd_firewall_entry,
                       FALSE,
                       this->cfg.nat_config.firewall_pkts_allowed))
      return false;
  }


  /* save into the config */
  for (j = i; j < (this->cfg.nat_config.num_extd_firewall_entries - 1); j++)
  {
    memcpy(&(this->cfg.nat_config.extd_firewall_entries[j]), &(this->cfg.nat_config.extd_firewall_entries[j+1]), 
           sizeof(qcmap_cm_extd_firewall_conf_t));
  }
  this->cfg.nat_config.num_extd_firewall_entries--;

  /* if it is OK, call write to XML */
  this->WriteConfigToFirewallXML();

  return true;
}

int QCMAP_ConnectionManager::GetFireWallEntry(qcmap_cm_firewall_entry_conf_t* firewall_entry, int max_entries)
{
  int num_firewalls = 0, i = 0;
  int ret_entry = 0;
  int qcmap_cm_errno;

  num_firewalls = qcmap_cm_firewall_entry_op(this->qcmap_cm_handle, QCMAP_CM_GET_FIREWALL_ENTRY_RULE,
                         firewall_entry, &qcmap_cm_errno);
  if (num_firewalls > max_entries)
    ret_entry = max_entries;
  else
    ret_entry = num_firewalls;

  i = ret_entry;

  if (i > 0)
  {
    while (i--)
    {
      LOG_MSG_INFO1("Index %d ", i,0,0);
      LOG_MSG_INFO1("firewall_start_dest_port: %d", firewall_entry[i].firewall_start_dest_port,0,0);
      LOG_MSG_INFO1("firewall_end_dest_port: %d", firewall_entry[i].firewall_end_dest_port,0,0);
      LOG_MSG_INFO1("firewall_protocol: %d", firewall_entry[i].firewall_protocol,0,0);
    }
  }
  else if (ret_entry == 0)
  {
    LOG_MSG_INFO1("No Firewall Rules configured",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Get Firewall Rule returned %d qcmap_cm_errno.%d\n", num_firewalls, qcmap_cm_errno,0);
  }
  return ret_entry;
}

boolean QCMAP_ConnectionManager::AddDMZ(uint32 dmz_ip)
{
  int qcmap_cm_errno;
  int ret_val;

  if(!this->sta_connected)
  {
    ret_val = qcmap_cm_dmz_op(this->qcmap_cm_handle, QCMAP_CM_ADD_DMZ, &dmz_ip, &qcmap_cm_errno);
    if (ret_val != QCMAP_CM_SUCCESS)
      return false;
  }
  else
  {
    if(!AddDMZOnA5(dmz_ip))
      return false;
  }

  this->cfg.nat_config.dmz_ip = dmz_ip;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

boolean QCMAP_ConnectionManager::DeleteDMZ(uint32 dmz_ip)
{
  int qcmap_cm_errno;
  int ret_val;

  if(!this->sta_connected)
  {
    ret_val = qcmap_cm_dmz_op(this->qcmap_cm_handle, QCMAP_CM_DELETE_DMZ, &dmz_ip, &qcmap_cm_errno);
    if (ret_val != QCMAP_CM_SUCCESS)
      return false;
  }
  else
  {
    if(!DeleteDMZOnA5(this->cfg.nat_config.dmz_ip))
      return false;
  }

  this->cfg.nat_config.dmz_ip = 0;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

uint32 QCMAP_ConnectionManager::GetDMZ(void)
{
  uint32 dmz_ip = 0;
  int qcmap_cm_errno;
  int ret_val;

  if(!this->sta_connected)
  {
    ret_val = qcmap_cm_dmz_op(this->qcmap_cm_handle, QCMAP_CM_GET_DMZ, &dmz_ip, &qcmap_cm_errno);
    if (ret_val != QCMAP_CM_SUCCESS)
      return 0;
  }
  else
  {
    dmz_ip = this->cfg.nat_config.dmz_ip;
  }


  return dmz_ip;
}

/* Set NAT Time out */
boolean QCMAP_ConnectionManager::SetNATEntryTimeout(uint16 timeout)
{
  int qcmap_cm_errno;
  int ret_val;

  /* Config NAT timeout */
  ret_val = qcmap_cm_nat_timeout_op(this->qcmap_cm_handle, QCMAP_CM_SET_NAT_ENTRY_TIMEOUT,
                                  &timeout, &qcmap_cm_errno);
  if (ret_val != QCMAP_CM_SUCCESS)
    return false;

  this->cfg.nat_config.nat_entry_timeout = timeout;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

uint16 QCMAP_ConnectionManager::GetNATEntryTimeout(void)
{
  uint16 timeout = 0;
  int qcmap_cm_errno;
  int ret_val;

  ret_val = qcmap_cm_nat_timeout_op(this->qcmap_cm_handle, QCMAP_CM_GET_NAT_ENTRY_TIMEOUT,
                                  &timeout, &qcmap_cm_errno);

  return timeout;
}

boolean QCMAP_ConnectionManager::SetIPSECVpnPassThrough(boolean enable)
{
  int qcmap_cm_errno;
  int ret_val;
  uint8 vpn_value = (enable) ? 1 : 0;

  if (!this->sta_connected)
  {
    ret_val = qcmap_cm_vpn_op(this->qcmap_cm_handle, QCMAP_CM_SET_IPSEC,
                              &vpn_value, &qcmap_cm_errno);
    if (ret_val != QCMAP_CM_SUCCESS)
      return false;
  }
  else
  {
    /* STA mode. */
    if (!SetIPSECVpnPassThroughOnA5(enable))
      return false;
  }

  this->cfg.nat_config.enable_ipsec_vpn_pass_through = enable;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

boolean QCMAP_ConnectionManager::SetL2TPVpnPassThrough(boolean enable)
{
  int qcmap_cm_errno;
  int ret_val;
  uint8 vpn_value = (enable) ? 1 : 0;

  if (!this->sta_connected)
  {
    ret_val = qcmap_cm_vpn_op(this->qcmap_cm_handle, QCMAP_CM_SET_L2TP,
                              &vpn_value, &qcmap_cm_errno);
    if (ret_val != QCMAP_CM_SUCCESS)
      return false;
  }
  else
  {
    /* STA mode. */
    if (!SetL2TPVpnPassThroughOnA5(enable))
      return false;
  }

  this->cfg.nat_config.enable_l2tp_vpn_pass_through = enable;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

boolean QCMAP_ConnectionManager::SetPPTPVpnPassThrough(boolean enable)
{
  int qcmap_cm_errno;
  int ret_val;
  uint8 vpn_value = (enable) ? 1 : 0;

  if (!this->sta_connected)
  {
    ret_val = qcmap_cm_vpn_op(this->qcmap_cm_handle, QCMAP_CM_SET_PPTP,
                              &vpn_value, &qcmap_cm_errno);
    if (ret_val != QCMAP_CM_SUCCESS)
      return false;
  }
  else
  {
    /* STA mode. */
    if (!SetPPTPVpnPassThroughOnA5(enable))
      return false;
  }

  this->cfg.nat_config.enable_pptp_vpn_pass_through = enable;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

boolean QCMAP_ConnectionManager::SetFirewall(boolean enable, boolean pkts_allowed)
{
  int qcmap_cm_errno;
  int ret_val;
  char command[MAX_COMMAND_STR_LEN];
  uint8 pkts_allowed_value = pkts_allowed;

  if (enable)
  {
    if (this->cfg.nat_config.firewall_enabled &&
        (this->cfg.nat_config.firewall_pkts_allowed == pkts_allowed))
    {
      QCMAP_CM_LOG("Firewall already Enabled with same pkts_allowed value");
      return true;
    }
    if (!this->sta_connected)
    {
      ret_val = qcmap_cm_firewall_op(this->qcmap_cm_handle, 
                                 QCMAP_CM_ENABLE_FIREWALL,
                                 &pkts_allowed_value, &qcmap_cm_errno);
    }
    else
    {
      if (this->cfg.nat_config.firewall_enabled &&
          (this->cfg.nat_config.firewall_pkts_allowed != pkts_allowed))
      {
        /* Changing pkts_allowed value -delete previously added rules*/
        QCMAP_CM_LOG("Firewall enabled with different pkts_allowed value");
        ret_val = this->SetFirewallOnA5(FALSE,
                                  this->cfg.nat_config.firewall_pkts_allowed);
        if(ret_val != QCMAP_CM_SUCCESS)
        {
          QCMAP_CM_LOG("Failed to delete previously added rules\n");
          return FALSE;
        }
      }

      ret_val = this->SetFirewallOnA5(enable, pkts_allowed);
    }
  }
  else
  {
    if (!this->cfg.nat_config.firewall_enabled)
    {
      QCMAP_CM_LOG("Firewall already Disabled.");
      return true;
    }

    if (!this->sta_connected)
    {
      ret_val = qcmap_cm_firewall_op(this->qcmap_cm_handle, QCMAP_CM_DISABLE_FIREWALL,
                                 NULL, &qcmap_cm_errno);
    }
    else
    {
      ret_val = this->SetFirewallOnA5(enable,
                                this->cfg.nat_config.firewall_pkts_allowed);
    }
  }

  if (ret_val != QCMAP_CM_SUCCESS)
    return false;

  this->cfg.nat_config.firewall_enabled = enable;
  this->cfg.nat_config.firewall_pkts_allowed = pkts_allowed;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

int QCMAP_ConnectionManager::SetFirewallOnA5(boolean enable, boolean pkts_allowed)
{
  int i=0,ret_val= -1;
  qcmap_cm_firewall_entry_conf_t firewall_entry;
  QCMAP_CM_LOG("SetFirewallOnA5(): enable = %d, pkts_allowed=%d\n",
                enable, pkts_allowed);

  /* Cant continue if SoftAP is not enabled! */
  if (!this->qcmap_enable)
  {
    QCMAP_CM_LOG("Error: QCMAP not enabled.\n");
    return QCMAP_CM_ERROR;
  }

  QCMAP_CM_LOG("Modifying the existing Firewall entries on A5\n");
  for (i = 0; i < this->cfg.nat_config.num_extd_firewall_entries; i++)
  {
    this->ModifyExtdFirewallEntryOnA5(
           &this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry,
           enable,
           pkts_allowed);
  }

  this->WriteConfigToFirewallXML();
  return QCMAP_CM_SUCCESS;
}

boolean QCMAP_ConnectionManager::SetAutoconnect(boolean enable)
{
  int qcmap_cm_errno;
  int ret_val;

  if (this->sta_connected)
  {
    QCMAP_CM_LOG("Currently in STA mode - SetAutoconnect not valid\n");
    return false;
  }

  /* Config Auto Connect */
  ret_val = qcmap_cm_set_auto_connect(this->qcmap_cm_handle, &qcmap_cm_errno, enable);
  if (ret_val != QCMAP_CM_SUCCESS)
    return false;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

boolean QCMAP_ConnectionManager::GetAutoconnect()
{
  return this->cfg.wan_config.auto_connect;
}

boolean QCMAP_ConnectionManager::SetRoaming(boolean enable)
{
  int qcmap_cm_errno;
  int ret_val;

  if (this->sta_connected)
  {
    QCMAP_CM_LOG("Currently in STA mode - SetRoaming not valid\n");
    return false;
  }

  /* Config Auto Connect */
  ret_val = qcmap_cm_set_roaming(this->qcmap_cm_handle, &qcmap_cm_errno, enable);
  if (ret_val != QCMAP_CM_SUCCESS)
    return false;

  this->cfg.wan_config.roaming = enable;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

boolean QCMAP_ConnectionManager::GetWWANStatistics(
                                   ip_version_enum_type ip_family, 
                                   qcmap_cm_statistics_t *wwan_stats, 
                                   int *p_error)
{
  int ret_val;

  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Get WWAN Statistics */
  ret_val = qcmap_cm_get_wwan_statistics(this->qcmap_cm_handle,
                                         ip_family, 
                                         wwan_stats, 
                                         p_error); 
  if (ret_val != QCMAP_CM_SUCCESS)
    return false;

  return true;
}

boolean QCMAP_ConnectionManager::ResetWWANStatistics(
                                   ip_version_enum_type ip_family, 
                                   int *p_error)
{
  int ret_val;

  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Reset WWAN Statistics */
  ret_val = qcmap_cm_reset_wwan_statistics(this->qcmap_cm_handle,
                                           ip_family, 
                                           p_error); 
  if (ret_val != QCMAP_CM_SUCCESS)
    return false;

  return true;
}

boolean QCMAP_ConnectionManager::DisplayLinuxConfig(void)
{
  int i = 0;
  in_addr addr;
  in_addr dmz_ip;
  qcmap_cm_firewall_entry_conf_t    firewall_entries[QCMAP_MAX_FIREWALL_ENTRIES_V01];

  if ( !this->sta_connected )
  {
    /* Not STA mode */
    printf(" Not in STA mode, no need to print Linux Side Config!!\n\n");
    return true;
  }

  /* Display SNAT Entries. */
  printf("Number of SNAT entries configured:- %d\n\n",this->cfg.nat_config.num_port_fwding_entries);
  for ( i=0; i < this->cfg.nat_config.num_port_fwding_entries &&
        i < QCMAP_MAX_SNAT_ENTRIES_V01; i++)
  {
    printf("  Index %d : \n", i);
    addr.s_addr = htonl(this->cfg.nat_config.port_fwding_entries[i].port_fwding_private_ip);
    printf("    port_fwding_private_ip   : %s\n", inet_ntoa(addr));
    printf("    port_fwding_private_port : %d\n", this->cfg.nat_config.port_fwding_entries[i].port_fwding_private_port);
    printf("    port_fwding_global_port  : %d\n", this->cfg.nat_config.port_fwding_entries[i].port_fwding_global_port);
    printf("    port_fwding_protocol     : %d\n\n", this->cfg.nat_config.port_fwding_entries[i].port_fwding_protocol);
  }

  /* Display Firewall Config */
  if ( this->cfg.nat_config.firewall_enabled )
  {
    printf("Firewall is enabled. pkts allowed:- %d\n\n",
           this->cfg.nat_config.firewall_pkts_allowed);
  }
  else
  {
    printf("Firewall is Disabled.\n\n");
  }

  /* Display Firewall Entries. */
  printf("Number of Firewall entries configured:- %d\n\n",
         this->cfg.nat_config.num_extd_firewall_entries);
  for ( i=0; i < this->cfg.nat_config.num_extd_firewall_entries &&
        i < QCMAP_MAX_FIREWALL_ENTRIES_V01 ; i++)
  {
      firewall_entries[i].firewall_handle = this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.firewall_handle;
      firewall_entries[i].firewall_protocol = this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot;
      switch(this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot)
      {
        case PS_IPPROTO_TCP:
       {
          firewall_entries[i].firewall_start_dest_port = this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.port;
          firewall_entries[i].firewall_end_dest_port = this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.port +
            this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.range;
        }
        break;

        case PS_IPPROTO_UDP:
        {
          firewall_entries[i].firewall_start_dest_port = this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.port;
          firewall_entries[i].firewall_end_dest_port = this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.port +
            this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.range;
        }
        break;

        case PS_IPPROTO_TCP_UDP:
        {
          firewall_entries[i].firewall_start_dest_port = this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.port;
          firewall_entries[i].firewall_end_dest_port = this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.port +
            this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.range;
        }
        break;

        case PS_IPPROTO_ICMP:
        {
          firewall_entries[i].firewall_start_dest_port = this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.icmp.type;
          firewall_entries[i].firewall_end_dest_port = this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.icmp.code;
        }
        break;

        default:
          printf("  Protocol for Firewall Entry index: %d is not supported\n\n", i);
          continue;
      }

    printf("  Index %d : \n", i);
    printf("    firewall_start_dest_port : %d\n", firewall_entries[i].firewall_start_dest_port);
    printf("    firewall_end_dest_port   : %d\n", firewall_entries[i].firewall_end_dest_port);
    printf("    firewall_protocol        : %d\n\n", firewall_entries[i].firewall_protocol);
  }

  /* Config DMZ */
  dmz_ip.s_addr = htonl(this->cfg.nat_config.dmz_ip);
  printf("DMZ IP = %s\n\n", inet_ntoa(dmz_ip));

  /* VPN Passthrough. */
  printf("IPSEC VPN Pass Through = %d\n", this->cfg.nat_config.enable_ipsec_vpn_pass_through);
  printf("PPTP VPN Pass Through = %d\n", this->cfg.nat_config.enable_pptp_vpn_pass_through);
  printf("L2TP VPN Pass Through = %d\n\n", this->cfg.nat_config.enable_l2tp_vpn_pass_through);

  return true;
}

boolean QCMAP_ConnectionManager::SetDHCPDConfig(int intf, uint32 start, uint32 end, char * leasetime)
{
  struct in_addr addr;
  char tempIP[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];

  if(this->cfg.lan_config.interface[intf].devmode == QCMAP_CM_DEVMODE_STA)
  {
    QCMAP_CM_LOG("Cannot set DHCPD config for interface in STA mode\n");
    return false;
  }

  if (intf < 0 || intf > 2 )
  {
    QCMAP_CM_LOG("Invalid interface number provided\n");
    return false;
  }

  /* check to make sure the start address is on the same subnet as the IP router. */
  if ((this->cfg.lan_config.interface[intf].a5_ip_addr & this->cfg.lan_config.interface[intf].sub_net_mask) !=
      (start & this->cfg.lan_config.interface[intf].sub_net_mask))
  {
    QCMAP_CM_LOG("a5_ip_addr %X start %X\n", this->cfg.lan_config.interface[intf].a5_ip_addr, start);
    return false;
  }
  /* check to make sure the end address is on the same subnet as the IP router. */
  if ((this->cfg.lan_config.interface[intf].a5_ip_addr & this->cfg.lan_config.interface[intf].sub_net_mask) !=
      (end & this->cfg.lan_config.interface[intf].sub_net_mask))
  {
    QCMAP_CM_LOG("a5_ip_addr %X end %X\n", this->cfg.lan_config.interface[intf].a5_ip_addr, start);
    return false;
  }

  /*check to make sure the start or end IP is not within the reserved range */
  if (intf == QCMAP_CM_INTF_PRIMARY_INDEX)
  {
    if ( start <= this->cfg.lan_config.dun_client_ip_addr )
    {
      start = this->cfg.lan_config.interface[intf].a5_ip_addr + 20;
    }
  }
  else if ( start <= this->cfg.lan_config.interface[intf].a5_ip_addr )
  {
    start = this->cfg.lan_config.interface[intf].a5_ip_addr + 20;
  }
  if ( end < start )
  {
    end = start + MAX_WIFI_CLIENTS;
  }

  addr.s_addr = htonl(start);
  strlcpy(tempIP, inet_ntoa(addr), sizeof(tempIP));
  QCMAP_CM_LOG("SetDHCPDConfig: Setting DHCP Start addr as %s\n",tempIP);
  addr.s_addr = htonl(end);
  strlcpy(tempIP, inet_ntoa(addr), sizeof(tempIP));
  QCMAP_CM_LOG("SetDHCPDConfig: Setting DHCP End addr as %s\n",tempIP);

  this->cfg.lan_config.interface[intf].dhcp_start_address = start;
  this->cfg.lan_config.interface[intf].dhcp_end_address = end;
  strlcpy(this->cfg.lan_config.interface[intf].dhcp_lease_time, leasetime, QCMAP_CM_MAX_FILE_LEN);

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

boolean QCMAP_ConnectionManager::StartDHCPD()
{
  char command[MAX_COMMAND_STR_LEN];
  char scratch_buf[16];
  in_addr start, end;
  int s, ret;
  in_addr reserved_addr;
  struct ifreq buffer;
  char eth0_mac_addr[18]="00:00:00:00:00:00";
  char RESERVED_IP[16];
  /* Open a socket */
  s = socket(PF_INET, SOCK_DGRAM, 0);
  if (s < 0)
  {
    QCMAP_CM_LOG("Unable to open socket to get eth0 H/W address.\n");
  }
  else
  {
    /* Set up the interface request buffer for eth0. */
      memset(&buffer, 0x00, sizeof(buffer));
      memset(&scratch_buf, 0x00, sizeof(scratch_buf));
      snprintf(scratch_buf, 16, "eth%d,", this->ap_dev_num);
      strlcpy(buffer.ifr_name, scratch_buf, IFNAMSIZ);

    /* Call the ioctl to get the address. */
    ret = ioctl(s, SIOCGIFHWADDR, &buffer);
    if (ret <0)
    {
      close(s);
      QCMAP_CM_LOG("Unable to call ioctl to get eth0 H/W address\n");
      QCMAP_CM_LOG("using default address %s to reserve ip.\n",eth0_mac_addr);
 
    }
     
    close(s);
  }

  reserved_addr.s_addr = (htonl((inet_addr(A5_RMNET_SUB_NET_MASK))) &
                                 ((this->cfg.lan_config.a5_rmnet_ip_addr)))
                                          + RESERVED_IP_CALCULATION_FACTOR ;
  reserved_addr.s_addr = ntohl(reserved_addr.s_addr);
  snprintf(eth0_mac_addr,18,"%02x:%02x:%02x:%02x:%02x:%02x",
           (int)(buffer.ifr_hwaddr.sa_data[0]),
           (int)(buffer.ifr_hwaddr.sa_data[1]),
           (int)(buffer.ifr_hwaddr.sa_data[2]),
           (int)(buffer.ifr_hwaddr.sa_data[3]),
           (int)(buffer.ifr_hwaddr.sa_data[4]),
           (int)(buffer.ifr_hwaddr.sa_data[5]));
  snprintf(RESERVED_IP,16,"%s",inet_ntoa(reserved_addr));
  strlcpy(command, "dnsmasq", MAX_COMMAND_STR_LEN);

  /* This part needs to be done only for AP interfaces. */
  for (int i=0,dev=this->ap_dev_num; i < this->sta_iface_index; i++, dev++)
  {
    if (this->cfg.lan_config.interface[i].enable)
    {
      if (this->cfg.lan_config.interface[i].enable_dhcpd)
      {
        strlcat(command, "  --dhcp-range=", MAX_COMMAND_STR_LEN);
        /* Insert a GetInterfaceName call here.*/
        snprintf(scratch_buf, 16, "eth%d,", dev);
        strlcat(command, scratch_buf, MAX_COMMAND_STR_LEN);
        start.s_addr = htonl(this->cfg.lan_config.interface[i].dhcp_start_address);
        strlcat(command, inet_ntoa(start), MAX_COMMAND_STR_LEN);
        strlcat(command, ",", MAX_COMMAND_STR_LEN);
        end.s_addr = htonl(this->cfg.lan_config.interface[i].dhcp_end_address);
        strlcat(command, inet_ntoa(end), MAX_COMMAND_STR_LEN);
        strlcat(command, ",", MAX_COMMAND_STR_LEN);
        strlcat(command, this->cfg.lan_config.interface[i].dhcp_lease_time, MAX_COMMAND_STR_LEN);
      }
    }
  }

  strlcat(command, " --dhcp-host=", MAX_COMMAND_STR_LEN);
  strlcat(command, eth0_mac_addr, MAX_COMMAND_STR_LEN);
  strlcat(command, ",", MAX_COMMAND_STR_LEN);
  strlcat(command, RESERVED_IP, MAX_COMMAND_STR_LEN);
  strlcat(command, ",infinite", MAX_COMMAND_STR_LEN);

  ds_system_call(command, strlen(command));

  return true;
}

boolean QCMAP_ConnectionManager::StopDHCPD()
{
  ds_system_call("killall -9 dnsmasq",
                 strlen("killall -9 dnsmasq"));

  return true;
}

boolean QCMAP_ConnectionManager::SetHostAPDConfig(int intf, char * cfg_pathname)
{
  /* Check to see if the file exists. */
  int fd;
  /* Check if the interface number given is valid */
  if (intf >= 0 && intf < 3 )
  {
    fd  = open(cfg_pathname, O_CREAT | O_EXCL);
    if (fd < 0)
    {
      /* If error, then file does exist. Copy to local config. */
      strlcpy(this->cfg.lan_config.interface[intf].path_to_hostapd_conf, cfg_pathname, QCMAP_CM_MAX_FILE_LEN);

      /* if it is OK, call write to XML */
      this->WriteConfigToXML();

      return true;
    }

    /* If no error, the file does not exist. Close the new file and delete it. */
    close(fd);
    unlink(cfg_pathname);
    return false;
  }
  return false;
}

boolean QCMAP_ConnectionManager::StartHostAPD(int interface)
{
  char command[MAX_COMMAND_STR_LEN];
  snprintf(command,MAX_COMMAND_STR_LEN,"hostapd -B %s -P /var/run/hostapd_pid%d",
  this->cfg.lan_config.interface[interface].path_to_hostapd_conf,interface);
  ds_system_call(command, strlen(command));
  return true;
}

boolean QCMAP_ConnectionManager::StopHostAPD(int intf)
{
  pid_t pid_hostapd;
  char  pid_file_path[QCMAP_CM_MAX_FILE_LEN];
  FILE *hostapd_pid;
  int pid_kill;
  snprintf(pid_file_path,QCMAP_CM_MAX_FILE_LEN,"/var/run/hostapd_pid%d", intf);
  hostapd_pid = fopen(pid_file_path,"r");
  if(hostapd_pid==NULL)
  {
    /*    What to do here? ps -A | grep hostapd     */
    ds_system_call("killall -9 hostapd",strlen("killall -9 hostapd"));
    return true;
  }
  fscanf(hostapd_pid, "%d", &pid_kill );
  fclose(hostapd_pid);
  pid_hostapd=pid_kill;
  kill(pid_hostapd,SIGTERM);
  return true;
}

void QCMAP_ConnectionManager::StartMcastDaemon(void)
{
  ds_system_call("pimd", strlen("pimd"));
  //call re-load to effectively apply pimd.conf values
  ReStartMcastDaemon();
}
void QCMAP_ConnectionManager::StopMcastDaemon(void)
{
  ds_system_call("killall -15 pimd", strlen("killall -15 pimd"));
}
void QCMAP_ConnectionManager::ReStartMcastDaemon(void)
{
  ds_system_call("pimd -l", strlen("pimd -l"));
}

boolean QCMAP_ConnectionManager::AddExtdFireWallEntry(qcmap_cm_extd_firewall_conf_t* extd_firewall_conf)
{
  int qcmap_cm_errno;
  int ret_val, i;
  int next_hdr_prot;

  for (i = 0; i < this->cfg.nat_config.num_extd_firewall_entries; i++)
  {
    if (memcmp(&(extd_firewall_conf->extd_firewall_entry.filter_spec), 
               &(this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec), 
               sizeof(ip_filter_type)) == 0)
    {
      /* we already added this one */
      return false;
    }
  }

  if (this->cfg.nat_config.num_extd_firewall_entries == QCMAP_MAX_FIREWALL_ENTRIES_V01)
  {
    /* we reach MAX entries */
    QCMAP_CM_LOG("Exceeds maximum number of firewall entries\n");
    return false;
  }

  if(!this->sta_connected)
  {
    /* call CM lib to add Firewall entry */
    ret_val = qcmap_cm_extd_firewall_op(this->qcmap_cm_handle, QCMAP_CM_ADD_EXTD_FIREWALL_ENTRY_RULE,
                                      extd_firewall_conf, &qcmap_cm_errno);

    if (ret_val != QCMAP_CM_SUCCESS)
      return false;
  }
  else
  {  /*Add Firewall entry on A5 only when firewall enabled*/
    if(this->cfg.nat_config.firewall_enabled)
    {
      if(!ModifyExtdFirewallEntryOnA5(&extd_firewall_conf->extd_firewall_entry,
                                      TRUE,
                                      this->cfg.nat_config.firewall_pkts_allowed))
      {
        QCMAP_CM_LOG("Error: Failed to add Firewall entry on A5\n");
        return false;
      }
    }
  }

  /* save into the config */
  memset(&(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry),
         0, sizeof(qcmap_cm_extd_firewall_entry_conf_t));

  if(!this->sta_connected)
   this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.firewall_handle =
     extd_firewall_conf->extd_firewall_entry.firewall_handle;

  this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.ip_vsn = 
    extd_firewall_conf->extd_firewall_entry.filter_spec.ip_vsn;

  if(extd_firewall_conf->extd_firewall_entry.filter_spec.ip_vsn == IP_V4)
  {
    next_hdr_prot = extd_firewall_conf->extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot;
    memcpy(&(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.ip_hdr.v4),
           &(extd_firewall_conf->extd_firewall_entry.filter_spec.ip_hdr.v4),
           sizeof(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.ip_hdr.v4));
  }
  else if(extd_firewall_conf->extd_firewall_entry.filter_spec.ip_vsn == IP_V6)
  {
    next_hdr_prot = extd_firewall_conf->extd_firewall_entry.filter_spec.ip_hdr.v6.next_hdr_prot;
    memcpy(&(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.ip_hdr.v6),
           &(extd_firewall_conf->extd_firewall_entry.filter_spec.ip_hdr.v6),
           sizeof(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.ip_hdr.v6));
  }

  switch(next_hdr_prot)
  {
  case PS_IPPROTO_TCP:
    memcpy(&(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.next_prot_hdr.tcp),
           &(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.tcp),
           sizeof(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.next_prot_hdr.tcp));
    break;

  case PS_IPPROTO_UDP:
    memcpy(&(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.next_prot_hdr.udp),
           &(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.udp),
           sizeof(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.next_prot_hdr.udp));
    break;

  case PS_IPPROTO_ICMP:
  case PS_IPPROTO_ICMP6:
    memcpy(&(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.next_prot_hdr.icmp),
           &(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.icmp),
           sizeof(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.next_prot_hdr.icmp));
    break;

  case PS_IPPROTO_ESP:
    memcpy(&(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.next_prot_hdr.esp),
           &(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.esp),
           sizeof(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.next_prot_hdr.esp));
    break;

  case PS_IPPROTO_TCP_UDP:
    memcpy(&(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range),
           &(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range),
           sizeof(this->cfg.nat_config.extd_firewall_entries[this->cfg.nat_config.num_extd_firewall_entries].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range));
    break;

  default:
    break;
  }
  this->cfg.nat_config.num_extd_firewall_entries++;

  /* if it is OK, call write to XML */
  this->WriteConfigToFirewallXML();

  return true;
}

boolean QCMAP_ConnectionManager::ModifyExtdFirewallEntryOnA5
(
  qcmap_cm_extd_firewall_entry_conf_t *firewall_entry,
  boolean enable,
  boolean pkts_allowed
)
{
  char command[MAX_COMMAND_STR_LEN],command_sec[MAX_COMMAND_STR_LEN];
  uint8 next_hdr_prot;
  char tmp_buf[50];
  struct in6_addr ip6_addr;
  char ip6_addr_buf[48];
  in_addr addr;
  in_addr subnet;
  char ip_addr[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char netmask_addr[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  /*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Verify ARGs */
  if( firewall_entry == NULL )
  {
    QCMAP_CM_LOG("NULL Extended firewall_entry\n");
    return false;
  }

  /* Adding firewall rules to the PREROUTING chain of the mangle table.
     This chain is traversed before the prerouting chain of the NAT table
     and so it is used to drop the packets matching the global port values
     in the firewall entries. */
  if(firewall_entry->filter_spec.ip_vsn == IP_V4)
  {
    if(enable)
      strlcpy(command, "iptables -t mangle -A PREROUTING ", MAX_COMMAND_STR_LEN);
    else
      strlcpy(command, "iptables -t mangle -D PREROUTING ", MAX_COMMAND_STR_LEN);

    strlcat(command, "-i eth0 ", MAX_COMMAND_STR_LEN);
    memset(tmp_buf,0,50);

    if(firewall_entry->filter_spec.ip_hdr.v4.field_mask & IPFLTR_MASK_IP4_SRC_ADDR)
    {
      addr.s_addr = htonl(firewall_entry->filter_spec.ip_hdr.v4.src.addr.ps_s_addr);
      subnet.s_addr = htonl(firewall_entry->filter_spec.ip_hdr.v4.src.subnet_mask.ps_s_addr);
      strlcpy(ip_addr, inet_ntoa(addr), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
      strlcpy(netmask_addr, inet_ntoa(subnet), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
      snprintf(tmp_buf, 37, "-s %s/%s ", ip_addr, netmask_addr);
      strlcat(command, tmp_buf, MAX_COMMAND_STR_LEN);
      memset(tmp_buf,0,50);
    }

    if(firewall_entry->filter_spec.ip_hdr.v4.field_mask & IPFLTR_MASK_IP4_DST_ADDR)
    {
      addr.s_addr = htonl(firewall_entry->filter_spec.ip_hdr.v4.dst.addr.ps_s_addr);
      subnet.s_addr = htonl(firewall_entry->filter_spec.ip_hdr.v4.dst.subnet_mask.ps_s_addr);
      strlcpy(ip_addr, inet_ntoa(addr), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
      strlcpy(netmask_addr, inet_ntoa(subnet), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
      snprintf(tmp_buf, 37, "-d %s/%s ", ip_addr, netmask_addr);
      strlcat(command, tmp_buf, MAX_COMMAND_STR_LEN);
      memset(tmp_buf,0,50);
    }

    if(firewall_entry->filter_spec.ip_hdr.v4.field_mask & IPFLTR_MASK_IP4_TOS)
    {
      QCMAP_CM_LOG("Not supporting TOS matching currently");
      #if 0
      snprintf(tmp_buf, 19, "-m tos --tos %d/%d ",
               firewall_entry->filter_spec.ip_hdr.v4.tos.val,
               firewall_entry->filter_spec.ip_hdr.v4.tos.mask);
      strlcat(command, tmp_buf, MAX_COMMAND_STR_LEN);
      memset(tmp_buf,0,50);
      #endif
    }

    next_hdr_prot = firewall_entry->filter_spec.ip_hdr.v4.next_hdr_prot;

    if(next_hdr_prot ==  PS_IPPROTO_ICMP)
    {
      strlcat(command, "-p icmp ", MAX_COMMAND_STR_LEN);
      if(firewall_entry->filter_spec.next_prot_hdr.icmp.field_mask &
                                                    IPFLTR_MASK_ICMP_MSG_TYPE )
      {
        snprintf(tmp_buf, 14, "--icmp-type %d",
                 firewall_entry->filter_spec.next_prot_hdr.icmp.type);
        strlcat(command, tmp_buf, MAX_COMMAND_STR_LEN);
        memset(tmp_buf,0,50);
      }

      if(firewall_entry->filter_spec.next_prot_hdr.icmp.field_mask &
                                                    IPFLTR_MASK_ICMP_MSG_CODE )
      {
        snprintf(tmp_buf, 4, "/%d ",
                 firewall_entry->filter_spec.next_prot_hdr.icmp.code);
        strlcat(command, tmp_buf, MAX_COMMAND_STR_LEN);
        memset(tmp_buf,0,50);
      }
    }
	/*Handling TCP,UDP,ESP,TCP_UDP will be done at once for v4 and v6*/
  }
  else //IPV6
  {
    if(enable)
      strlcpy(command, "ip6tables -t mangle -A PREROUTING ", MAX_COMMAND_STR_LEN);
    else
      strlcpy(command, "ip6tables -t mangle -D PREROUTING ", MAX_COMMAND_STR_LEN);

    strlcat(command, "-i eth0 ", MAX_COMMAND_STR_LEN);
    memset(tmp_buf,0,50);

    if(firewall_entry->filter_spec.ip_hdr.v6.field_mask & IPFLTR_MASK_IP6_SRC_ADDR)
    {
      memcpy(ip6_addr.s6_addr,
            firewall_entry->filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr8,
            QCMAP_IPV6_ADDR_LEN_V01*sizeof(uint8));
      snprintf(tmp_buf, 47,"-s %s/%d ",
               inet_ntop(AF_INET6,&ip6_addr,ip6_addr_buf,sizeof(ip6_addr_buf)),
               firewall_entry->filter_spec.ip_hdr.v6.src.prefix_len);
      strlcat(command, tmp_buf, MAX_COMMAND_STR_LEN);
      memset(tmp_buf,0,50);
      memset(&ip6_addr,0,sizeof(in6_addr));
      memset(ip6_addr_buf,0,sizeof(char)*48);
    }

    if(firewall_entry->filter_spec.ip_hdr.v6.field_mask & IPFLTR_MASK_IP6_DST_ADDR)
    {
      memcpy(ip6_addr.s6_addr,
             firewall_entry->filter_spec.ip_hdr.v6.dst.addr.in6_u.u6_addr8,
             QCMAP_IPV6_ADDR_LEN_V01*sizeof(uint8));
      snprintf(tmp_buf, 47,"-d %s/%d ",
               inet_ntop(AF_INET6,&ip6_addr,ip6_addr_buf,sizeof(ip6_addr_buf)),
               firewall_entry->filter_spec.ip_hdr.v6.dst.prefix_len);
      strlcat(command, tmp_buf, MAX_COMMAND_STR_LEN);
      memset(tmp_buf,0,50);
      memset(&ip6_addr,0,sizeof(in6_addr));
      memset(ip6_addr_buf,0,sizeof(char)*48);
    }

    if(firewall_entry->filter_spec.ip_hdr.v6.field_mask & IPFLTR_MASK_IP6_TRAFFIC_CLASS)
    {
      QCMAP_CM_LOG("Not supporting TOS matching currently");
      #if 0
      snprintf(tmp_buf, 19,"-m tos --tos %d/%d ",firewall_entry->filter_spec.ip_hdr.v6.trf_cls.val,
               firewall_entry->filter_spec.ip_hdr.v6.trf_cls.mask);
      strlcat(command, tmp_buf, MAX_COMMAND_STR_LEN);
      memset(tmp_buf,0,50);
      #endif
    }

    next_hdr_prot = firewall_entry->filter_spec.ip_hdr.v6.next_hdr_prot;

    if(next_hdr_prot == PS_IPPROTO_ICMP6)
    {
      strlcat(command, "-p icmpv6 ", MAX_COMMAND_STR_LEN);

      if(firewall_entry->filter_spec.next_prot_hdr.icmp.field_mask &
                      IPFLTR_MASK_ICMP_MSG_TYPE )
      {
        snprintf(tmp_buf, 18, "--icmpv6-type %d",firewall_entry->filter_spec.next_prot_hdr.icmp.type);
        strlcat(command, tmp_buf, MAX_COMMAND_STR_LEN);
        memset(tmp_buf,0,50);
      }

      if(firewall_entry->filter_spec.next_prot_hdr.icmp.field_mask &
                                             IPFLTR_MASK_ICMP_MSG_CODE )
      {
        snprintf(tmp_buf, 5, "/%d ",firewall_entry->filter_spec.next_prot_hdr.icmp.code);
        strlcat(command, tmp_buf, MAX_COMMAND_STR_LEN);
        memset(tmp_buf,0,50);
      }
    }
  }

  switch(next_hdr_prot)
  {
    case PS_IPPROTO_TCP:
      strlcat(command, "-p tcp ", MAX_COMMAND_STR_LEN);
      if(firewall_entry->filter_spec.next_prot_hdr.tcp.field_mask &
                                               IPFLTR_MASK_TCP_SRC_PORT )
      {
        snprintf(tmp_buf, 21, "--sport %d:%d ",
                 firewall_entry->filter_spec.next_prot_hdr.tcp.src.port,
                 firewall_entry->filter_spec.next_prot_hdr.tcp.src.range + firewall_entry->filter_spec.next_prot_hdr.tcp.src.port);
        strlcat(command, tmp_buf, MAX_COMMAND_STR_LEN);
        memset(tmp_buf,0,50);
      }
      if(firewall_entry->filter_spec.next_prot_hdr.tcp.field_mask &
                                               IPFLTR_MASK_TCP_DST_PORT )
      {
        snprintf(tmp_buf, 21, "--dport %d:%d ",
                 firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port,
                 firewall_entry->filter_spec.next_prot_hdr.tcp.dst.range + firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port);
        strlcat(command, tmp_buf, MAX_COMMAND_STR_LEN);
        memset(tmp_buf,0,50);
      }
      break;

    case PS_IPPROTO_UDP:
      strlcat(command, "-p udp ", MAX_COMMAND_STR_LEN);
      if(firewall_entry->filter_spec.next_prot_hdr.tcp.field_mask &
                                               IPFLTR_MASK_UDP_SRC_PORT )
      {
        snprintf(tmp_buf, 21, "--sport %d:%d ",
                 firewall_entry->filter_spec.next_prot_hdr.tcp.src.port,
			firewall_entry->filter_spec.next_prot_hdr.tcp.src.range + firewall_entry->filter_spec.next_prot_hdr.tcp.src.port);
        strlcat(command, tmp_buf, MAX_COMMAND_STR_LEN);
        memset(tmp_buf,0,50);
      }
      if(firewall_entry->filter_spec.next_prot_hdr.tcp.field_mask &
                                               IPFLTR_MASK_UDP_DST_PORT )
      {
        snprintf(tmp_buf, 21, "--dport %d:%d ",
                 firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port,
                 firewall_entry->filter_spec.next_prot_hdr.tcp.dst.range + firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port);
        strlcat(command, tmp_buf, MAX_COMMAND_STR_LEN);
        memset(tmp_buf,0,50);
      }
      break;

    case PS_IPPROTO_TCP_UDP:
      strlcpy(command_sec, command, MAX_COMMAND_STR_LEN);
      strlcat(command, "-p tcp ", MAX_COMMAND_STR_LEN);
      strlcat(command_sec, "-p udp ", MAX_COMMAND_STR_LEN);
      if(firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask &
                                               IPFLTR_MASK_TCP_UDP_SRC_PORT )
      {
        snprintf(tmp_buf, 21, "--sport %d:%d ",
                 firewall_entry->filter_spec.next_prot_hdr.tcp.src.port,
			firewall_entry->filter_spec.next_prot_hdr.tcp.src.range + firewall_entry->filter_spec.next_prot_hdr.tcp.src.port);
        strlcat(command, tmp_buf, MAX_COMMAND_STR_LEN);
        strlcat(command_sec, tmp_buf, MAX_COMMAND_STR_LEN);
        memset(tmp_buf,0,50);
      }
      if(firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask &
                                               IPFLTR_MASK_TCP_UDP_DST_PORT )
      {
        snprintf(tmp_buf, 21, "--dport %d:%d ",
                 firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port,
			firewall_entry->filter_spec.next_prot_hdr.tcp.dst.range + firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port);
        strlcat(command, tmp_buf, MAX_COMMAND_STR_LEN);
        strlcat(command_sec, tmp_buf, MAX_COMMAND_STR_LEN);
        memset(tmp_buf,0,50);
      }
      break;

    case PS_IPPROTO_ESP:
      QCMAP_CM_LOG("Not supporting ESP matching currently");
      #if 0
        strlcat(command, "-p esp ", MAX_COMMAND_STR_LEN);
        if(firewall_entry->filter_spec.next_prot_hdr.esp.field_mask & IPFLTR_MASK_ESP_SPI )
        {
          snprintf(tmp_buf,20,"--espspi %d ",firewall_entry->filter_spec.next_prot_hdr.esp.spi);
          strlcat(command, tmp_buf, MAX_COMMAND_STR_LEN);
          memset(tmp_buf,0,50);
        }
      #endif
      break;

    default:
      break;
    }

    snprintf(tmp_buf, 11, " -j %s ",pkts_allowed?"ACCEPT" : "DROP");
    strlcat(command, tmp_buf, MAX_COMMAND_STR_LEN);
    if(next_hdr_prot == PS_IPPROTO_TCP_UDP)
    {
      strlcat(command_sec, tmp_buf, MAX_COMMAND_STR_LEN);
      ds_system_call(command_sec, strlen(command_sec));
      QCMAP_CM_LOG("%s\n", command_sec);
    }

    memset(tmp_buf,0,50);

    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);
}

boolean QCMAP_ConnectionManager::GetFireWallHandleList(qcmap_cm_extd_firewall_conf_t* extd_firewall_conf)
{
  int qcmap_cm_errno;
  int ret_val;
  int i, index = 0;

  if(!this->sta_connected)
  {   /* call CM lib to get Firewall handle list */
    ret_val = qcmap_cm_extd_firewall_op(this->qcmap_cm_handle,
                                        QCMAP_CM_GET_FIREWALL_HANDLE_LIST,
                                    extd_firewall_conf, &qcmap_cm_errno);

    if (ret_val != QCMAP_CM_SUCCESS)
      return false;
  }
  else
  {
    if(this->cfg.nat_config.firewall_enabled)
    {
      /*We use index as handle in AP+STA mode*/
      for(i=0; i < this->cfg.nat_config.num_extd_firewall_entries; i++)
      {
        if(extd_firewall_conf->extd_firewall_handle_list.ip_family ==
           this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.\
                                                            filter_spec.ip_vsn)
        {
          extd_firewall_conf->extd_firewall_handle_list.handle_list[index] = i;
          index++;
        }
      }
      extd_firewall_conf->extd_firewall_handle_list.num_of_entries = index;
    }
    else //Return number of firewall entries as zero when firewall not enabled
      extd_firewall_conf->extd_firewall_handle_list.num_of_entries = 0;
  }

  return true;
}

boolean QCMAP_ConnectionManager::GetExtdFireWallEntry(qcmap_cm_extd_firewall_conf_t* extd_firewall_conf)
{
  int qcmap_cm_errno;
  int ret_val,i;
  uint8 next_hdr_prot;

  if(!this->sta_connected)
  {   /* call CM lib to get Firewall entry */
    ret_val = qcmap_cm_extd_firewall_op(this->qcmap_cm_handle,
                                        QCMAP_CM_GET_EXTD_FIREWALL_ENTRY_RULE,
                                    extd_firewall_conf, &qcmap_cm_errno);

  if (ret_val != QCMAP_CM_SUCCESS)
    return false;
  }
  else //In STA mode, use index as firewall handle
  {

    i = extd_firewall_conf->extd_firewall_entry.firewall_handle;

    extd_firewall_conf->extd_firewall_entry.filter_spec.ip_vsn =
      this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_vsn;

    if(extd_firewall_conf->extd_firewall_entry.filter_spec.ip_vsn == IP_V4)
    {
      memcpy(&(extd_firewall_conf->extd_firewall_entry.filter_spec.ip_hdr.v4),
             &(this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v4),
             sizeof(extd_firewall_conf->extd_firewall_entry.filter_spec.ip_hdr.v4));
      next_hdr_prot = extd_firewall_conf->extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot;
    }
    else if(extd_firewall_conf->extd_firewall_entry.filter_spec.ip_vsn == IP_V6)
    {
      memcpy(&(extd_firewall_conf->extd_firewall_entry.filter_spec.ip_hdr.v6),
             &(this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_hdr.v6),
             sizeof(extd_firewall_conf->extd_firewall_entry.filter_spec.ip_hdr.v6));
      next_hdr_prot = extd_firewall_conf->extd_firewall_entry.filter_spec.ip_hdr.v6.next_hdr_prot;
    }

    switch(next_hdr_prot)
    {
      case PS_IPPROTO_TCP:
        memcpy(&(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.tcp),
               &(this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp),
               sizeof(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.tcp));
        break;

       case PS_IPPROTO_UDP:
         memcpy(&(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.udp),
                &(this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.udp),
                sizeof(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.udp));
         break;

       case PS_IPPROTO_ICMP:
       case PS_IPPROTO_ICMP6:
         memcpy(&(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.icmp),
                &(this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.icmp),
                sizeof(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.icmp));
         break;

       case PS_IPPROTO_ESP:
         memcpy(&(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.esp),
                &(this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.esp),
                sizeof(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.esp));
         break;

       case PS_IPPROTO_TCP_UDP:
         memcpy(&(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range),
                &(this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range),
                sizeof(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range));
         break;

       default:
         break;
     }
  }

  return true;
}

boolean QCMAP_ConnectionManager::DeleteExtdFireWallEntry(qcmap_cm_extd_firewall_conf_t* extd_firewall_entry)
{
  int qcmap_cm_errno;
  int ret_val,i,j;

  if(!this->sta_connected)
    for (i = 0; i < this->cfg.nat_config.num_extd_firewall_entries; i++)
    {
      /* Do not compare the handle value, the last uint32 bytes of the struct. */
      if (extd_firewall_entry->extd_firewall_handle.handle ==
          this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.firewall_handle)
      {
        QCMAP_CM_LOG("Found matching handle %d and %d index is %d",
                     extd_firewall_entry->extd_firewall_handle.handle,
                     this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry.firewall_handle, i );
        break;
      }
    }
  else //In STA mode, index itself used as handle to delete entries
    i = extd_firewall_entry->extd_firewall_handle.handle;

  if (i >= this->cfg.nat_config.num_extd_firewall_entries)
  {
    /* we do not have this one  */
    QCMAP_CM_LOG("Did not find matching handle");
    return false;
  }
  
  QCMAP_CM_LOG("value of i is %d\n", i);

  if(!this->sta_connected)
  {  /* call CM lib to Delete FireWall entry */
    ret_val = qcmap_cm_extd_firewall_op(this->qcmap_cm_handle,
                                        QCMAP_CM_DELETE_EXTD_FIREWALL_ENTRY_RULE,
                                        extd_firewall_entry, &qcmap_cm_errno);

    if (ret_val != QCMAP_CM_SUCCESS)
      return false;
  }
  else
  {  /*Delete Firewall entry on A5 only when firewall enabled*/
    if(this->cfg.nat_config.firewall_enabled)
    {
      if(!ModifyExtdFirewallEntryOnA5(
            &this->cfg.nat_config.extd_firewall_entries[i].extd_firewall_entry,
            FALSE,
            this->cfg.nat_config.firewall_pkts_allowed))
        return false;
    }
  }

   /* save into the config */
  for (j = i; j < (this->cfg.nat_config.num_extd_firewall_entries - 1); j++)
  {
    memcpy(&(this->cfg.nat_config.extd_firewall_entries[j]), &(this->cfg.nat_config.extd_firewall_entries[j+1]), 
           sizeof(qcmap_cm_extd_firewall_conf_t));
  }
  this->cfg.nat_config.num_extd_firewall_entries--;

  /* if it is OK, call write to XML */
  this->WriteConfigToFirewallXML();

  return true;
}

boolean QCMAP_ConnectionManager::AddSNATEntryOnA5
(
  qcmap_cm_port_fwding_entry_conf_t* nat_entry
)
{
  char command[MAX_COMMAND_STR_LEN];
  struct in_addr addr;
  char tempIP[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
/*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Verify ARGs */
  if( nat_entry == NULL )
  {
    QCMAP_CM_LOG("NULL nat_entry\n");
    return false;
  }

  /* Convert Private IP to ASCII format */
  addr.s_addr = htonl(nat_entry->port_fwding_private_ip);
  strlcpy(tempIP, inet_ntoa(addr), sizeof(tempIP));

  /* Port value to be used only for TCP and UDP */
  if( nat_entry->port_fwding_protocol == IPPROTO_TCP ||
      nat_entry->port_fwding_protocol == IPPROTO_UDP )
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -t nat -A PREROUTING -p %d -i eth0 --dport %d -j DNAT --to-destination %s:%d",
              nat_entry->port_fwding_protocol,
              nat_entry->port_fwding_global_port,
              tempIP,
              nat_entry->port_fwding_private_port );
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);
  }
  else
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -t nat -A PREROUTING -p %d -i eth0 -j DNAT --to-destination %s",
              nat_entry->port_fwding_protocol, tempIP );
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);
  }

  return true;
}

boolean QCMAP_ConnectionManager::DeleteSNATEntryOnA5
(
  qcmap_cm_port_fwding_entry_conf_t* nat_entry
)
{
  char command[MAX_COMMAND_STR_LEN];
  struct in_addr addr;
  char tempIP[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
/*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Verify ARGs */
  if( nat_entry == NULL )
  {
    QCMAP_CM_LOG("NULL nat_entry\n");
    return false;
  }

  /* Convert Private IP to ASCII format */
  addr.s_addr = htonl(nat_entry->port_fwding_private_ip);
  strlcpy(tempIP, inet_ntoa(addr), sizeof(tempIP));

  /* Port value to be used only for TCP and UDP */
  if( nat_entry->port_fwding_protocol == IPPROTO_TCP ||
      nat_entry->port_fwding_protocol == IPPROTO_UDP )
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -t nat -D PREROUTING -p %d -i eth0 --dport %d -j DNAT --to-destination %s:%d",
              nat_entry->port_fwding_protocol,
              nat_entry->port_fwding_global_port,
              tempIP,
              nat_entry->port_fwding_private_port );
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);
  }
  else
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -t nat -D PREROUTING -p %d -i eth0 -j DNAT --to-destination %s",
              nat_entry->port_fwding_protocol, tempIP );
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);
  }

  return true;
}

boolean QCMAP_ConnectionManager::AddFirewallEntryOnA5
(
  qcmap_cm_firewall_entry_conf_t* firewall_entry
)
{
  char command[MAX_COMMAND_STR_LEN];
/*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Verify ARGs */
  if( firewall_entry == NULL )
  {
    QCMAP_CM_LOG("NULL firewall_entry\n");
    return false;
  }

  /* Specify port range only for TCP and UDP */
  if( firewall_entry->firewall_protocol == IPPROTO_TCP ||
      firewall_entry->firewall_protocol == IPPROTO_UDP )
  {
    /* Verify port range */
    if( firewall_entry->firewall_start_dest_port >
          firewall_entry->firewall_end_dest_port )
    {
      QCMAP_CM_LOG("Start port > End port\n");
      return false;
    }

    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -A FORWARD -i eth0 -p %d --dport %d:%d -j %s",
              firewall_entry->firewall_protocol,
              firewall_entry->firewall_start_dest_port,
              firewall_entry->firewall_end_dest_port,
              this->cfg.nat_config.firewall_pkts_allowed?"ACCEPT" : "DROP");
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);
  }
  else if(firewall_entry->firewall_protocol == PS_IPPROTO_TCP_UDP)
  {
    /* Verify port range */
    if( firewall_entry->firewall_start_dest_port >
          firewall_entry->firewall_end_dest_port )
    {
      QCMAP_CM_LOG("Start port > End port\n");
      return false;
    }
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -A FORWARD -i eth0 -p %d --dport %d:%d -j %s",
              IPPROTO_TCP,
              firewall_entry->firewall_start_dest_port,
              firewall_entry->firewall_end_dest_port,
              this->cfg.nat_config.firewall_pkts_allowed?"ACCEPT" : "DROP");
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);

    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -A FORWARD -i eth0 -p %d --dport %d:%d -j %s",
              IPPROTO_UDP,
              firewall_entry->firewall_start_dest_port,
              firewall_entry->firewall_end_dest_port,
              this->cfg.nat_config.firewall_pkts_allowed?"ACCEPT" : "DROP");
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);
  }
  else
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -A FORWARD -i eth0 -p %d -j %s",
              firewall_entry->firewall_protocol,
              this->cfg.nat_config.firewall_pkts_allowed?"ACCEPT" : "DROP");
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);
  }

  return true;
}

boolean QCMAP_ConnectionManager::DeleteFirewallEntryOnA5
(
  qcmap_cm_firewall_entry_conf_t* firewall_entry
)
{
  char command[MAX_COMMAND_STR_LEN];
/*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Verify ARGs */
  if( firewall_entry == NULL )
  {
    QCMAP_CM_LOG("NULL firewall_entry\n");
    return false;
  }

  /* Specify port range only for TCP and UDP */
  if( firewall_entry->firewall_protocol == IPPROTO_TCP ||
      firewall_entry->firewall_protocol == IPPROTO_UDP )
  {
    /* Verify port range */
    if( firewall_entry->firewall_start_dest_port >
          firewall_entry->firewall_end_dest_port )
    {
      QCMAP_CM_LOG("Start port > End port\n");
      return false;
    }

    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -D FORWARD -i eth0 -p %d --dport %d:%d -j %s",
              firewall_entry->firewall_protocol,
              firewall_entry->firewall_start_dest_port,
              firewall_entry->firewall_end_dest_port,
              this->cfg.nat_config.firewall_pkts_allowed?"ACCEPT" : "DROP");
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);
  }
  else if(firewall_entry->firewall_protocol == PS_IPPROTO_TCP_UDP)
  {
    /* Verify port range */
    if( firewall_entry->firewall_start_dest_port >
          firewall_entry->firewall_end_dest_port )
    {
      QCMAP_CM_LOG("Start port > End port\n");
      return false;
    }
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -D FORWARD -i eth0 -p %d --dport %d:%d -j %s",
              IPPROTO_TCP,
              firewall_entry->firewall_start_dest_port,
              firewall_entry->firewall_end_dest_port,
              this->cfg.nat_config.firewall_pkts_allowed?"ACCEPT" : "DROP");
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);

    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -D FORWARD -i eth0 -p %d --dport %d:%d -j %s",
              IPPROTO_UDP,
              firewall_entry->firewall_start_dest_port,
              firewall_entry->firewall_end_dest_port,
              this->cfg.nat_config.firewall_pkts_allowed?"ACCEPT" : "DROP");
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);
  }
  else
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -D FORWARD -i eth0 -p %d -j %s",
              firewall_entry->firewall_protocol,
              this->cfg.nat_config.firewall_pkts_allowed?"ACCEPT" : "DROP");
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);
  }

  return true;
}

boolean QCMAP_ConnectionManager::AddDMZOnA5
(
  uint32 dmzIP
)
{
  char command[MAX_COMMAND_STR_LEN];
  struct in_addr addr;
  char tempIP[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
/*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Verify DMZ IP */
  if( dmzIP == 0 )
  {
    QCMAP_CM_LOG("Invalid DMZ IP\n");
    return false;
  }

  /* Convert IP to ASCII format */
  addr.s_addr = htonl(dmzIP);
  strlcpy(tempIP, inet_ntoa(addr), sizeof(tempIP));

  snprintf( command, MAX_COMMAND_STR_LEN,
            "iptables -t nat -A PREROUTING -i eth0 -j DNAT --to-destination %s",
            tempIP );
  ds_system_call(command, strlen(command));
  QCMAP_CM_LOG("%s\n", command);

  return true;
}

boolean QCMAP_ConnectionManager::SetIPSECVpnPassThroughOnA5
(
  boolean enable
)
{
  char command[MAX_COMMAND_STR_LEN];

  /* Delete the existing rule. */
  snprintf( command, MAX_COMMAND_STR_LEN,
            "iptables -D FORWARD -p esp -i eth0 -j %s",
            (this->cfg.nat_config.enable_ipsec_vpn_pass_through) ?
            "ACCEPT" : "DROP" );
  ds_system_call(command, strlen(command));
  QCMAP_CM_LOG("%s\n", command);

  /* Add the new rule. */
  snprintf( command, MAX_COMMAND_STR_LEN,
            "iptables -A FORWARD -p esp -i eth0 -j %s",
            (enable) ?"ACCEPT" : "DROP" );
  ds_system_call(command, strlen(command));
  QCMAP_CM_LOG("%s\n", command);

  return true;
}

boolean QCMAP_ConnectionManager::SetL2TPVpnPassThroughOnA5
(
  boolean enable
)
{
  char command[MAX_COMMAND_STR_LEN];

  /* Delete the existing rule. */
  snprintf( command, MAX_COMMAND_STR_LEN,
            "iptables -D FORWARD -p esp -i eth0 -j %s",
            (this->cfg.nat_config.enable_l2tp_vpn_pass_through) ?
            "ACCEPT" : "DROP" );
  ds_system_call(command, strlen(command));
  QCMAP_CM_LOG("%s\n", command);

  /* Add the new rule. */
  snprintf( command, MAX_COMMAND_STR_LEN,
            "iptables -A FORWARD -p esp -i eth0 -j %s",
            (enable) ?"ACCEPT" : "DROP" );
  ds_system_call(command, strlen(command));
  QCMAP_CM_LOG("%s\n", command);

  return true;
}

boolean QCMAP_ConnectionManager::SetPPTPVpnPassThroughOnA5
(
  boolean enable
)
{
  char command[MAX_COMMAND_STR_LEN];

  /* Delete the existing rule. */
  snprintf( command, MAX_COMMAND_STR_LEN,
            "iptables -D FORWARD -p gre -i eth0 -j %s",
            (this->cfg.nat_config.enable_pptp_vpn_pass_through) ?
            "ACCEPT" : "DROP" );
  ds_system_call(command, strlen(command));
  QCMAP_CM_LOG("%s\n", command);

  /* Add the new rule. */
  snprintf( command, MAX_COMMAND_STR_LEN,
            "iptables -A FORWARD -p gre -i eth0 -j %s",
            (enable) ?"ACCEPT" : "DROP" );
  ds_system_call(command, strlen(command));
  QCMAP_CM_LOG("%s\n", command);

  return true;
}

boolean QCMAP_ConnectionManager::DeleteDMZOnA5
(
  uint32 dmzIP
)
{
  char command[MAX_COMMAND_STR_LEN];
  struct in_addr addr;
  char tempIP[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
/*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Verify DMZ IP */
  if( dmzIP == 0 )
  {
    QCMAP_CM_LOG("Invalid DMZ IP\n");
    return false;
  }

  /* Convert IP to ASCII format */
  addr.s_addr = htonl(dmzIP);
  strlcpy(tempIP, inet_ntoa(addr), sizeof(tempIP));

  snprintf( command, MAX_COMMAND_STR_LEN,
            "iptables -t nat -D PREROUTING -i eth0 -j DNAT --to-destination %s",
            tempIP );
  ds_system_call(command, strlen(command));
  QCMAP_CM_LOG("%s\n", command);

  return true;
}

/* Connect the Station interface to the specified hotspot.

   If wpa supplicant is enabled in config file, connection parameters
   specified in corresponding config file are used.
   Otherwise, open(non-secure) mode connection is attempted to the external
   hotspot SSID specified directly in the mobileap config file. */
boolean QCMAP_ConnectionManager::ConnectSTA
(
  qcmap_sta_connection_e conn_type,
  qcmap_sta_static_ip_config * static_ip_config
)
{
  char command[MAX_COMMAND_STR_LEN];
  char netMask[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  in_addr addr, staIp, staMask, staGwIp;
  boolean ret=TRUE;
/*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Verify STA iface index */
  if( this->sta_iface_index < 0 )
  {
    QCMAP_CM_LOG( "Error: Invalid STA iface index %d",
                  this->sta_iface_index );
    return false;
  }

  /* Check if supplicant enabled */
  if( this->cfg.lan_config.interface[this->sta_iface_index].enable_supplicant )
  {
    /* Connect STA to specified hotspot as per supplicant config */
    QCMAP_CM_LOG( "Supplicant enabled.\n"
                  "Connecting station iface eth0 with config as in: %s\n",
                  this->cfg.lan_config.interface[this->sta_iface_index].path_to_supplicant_conf);

    /* Validate config file path */
    if( strnlen(
      this->cfg.lan_config.interface[this->sta_iface_index].path_to_supplicant_conf,
      QCMAP_CM_MAX_FILE_LEN )
      == 0 )
    {
      QCMAP_CM_LOG("Error: External AP SSID length 0\n");
      return false;
    }

    /* Kill any running instance */
    snprintf( command, MAX_COMMAND_STR_LEN, "killall -15 wpa_supplicant" );
    ds_system_call(command, strlen(command));

    /* Run wpa_supplicant for sta interface with specified config */
    snprintf( command, MAX_COMMAND_STR_LEN, "wpa_supplicant -i eth0 -Dar6003 -B -c %s",
              this->cfg.lan_config.interface[this->sta_iface_index].path_to_supplicant_conf );
    ds_system_call(command, strlen(command));

    /* TODO: Wait for command completion. Then verify success. */
  }
  /* Supplicant disabled */
  else
  {
    /* Connect in open mode. */
    QCMAP_CM_LOG( "Supplicant disabled.\n"
                  "Connecting station iface eth0 to AP with SSID: %d %s\n",
                  this->sta_iface_index, this->cfg.lan_config.interface[this->sta_iface_index].external_ap_ssid );

    /* Validate SSID */
    if( strnlen(
      this->cfg.lan_config.interface[this->sta_iface_index].external_ap_ssid,
      QCMAP_CM_MAX_FILE_LEN )
      == 0 )
    {
      QCMAP_CM_LOG("Error: External AP SSID length 0\n");
      return false;
    }

    /* Connect to specified SSID */
    snprintf( command, MAX_COMMAND_STR_LEN, "iwconfig eth0 essid %s",
              this->cfg.lan_config.interface[this->sta_iface_index].external_ap_ssid );
    ds_system_call(command, strlen(command));

    /* TODO: Wait for command completion. Then verify success. */
  }

  /* Wait for estimated time */
  sleep(8);

  if ( conn_type == QCMAP_STA_CONNECTION_DYNAMIC )
  {
    QCMAP_CM_LOG("Bringing up STA iface eth0\n");
    /* Bringup interface */
    snprintf( command, MAX_COMMAND_STR_LEN, "ifconfig eth0 up" );
    ds_system_call(command, strlen(command));
    sleep(4);

    /* Disable IP forwarding before starting DHCP negotiation */
    QCMAP_CM_LOG("Disabling IP forwarding.\n");
    snprintf( command, MAX_COMMAND_STR_LEN,
              "echo 0 > /proc/sys/net/ipv4/ip_forward");
    ds_system_call(command, strlen(command));

    /* Kill any stale dhcpcd intance */
    QCMAP_CM_LOG("Killing previous dhcpcd process.\n");
    snprintf( command, MAX_COMMAND_STR_LEN, "killall -15 dhcpcd" );
    ds_system_call(command, strlen(command));
    snprintf( command, MAX_COMMAND_STR_LEN, "rm -rf /var/run/dhcpcd-eth0.pid" );
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);

    /* Obtain and assign IP address via dhcpcd daemon */
    QCMAP_CM_LOG("Running DHCP client on eth0\n");
    snprintf( command, MAX_COMMAND_STR_LEN, "dhcpcd eth0" );
    ds_system_call(command, strlen(command));
    sleep(5);

    /* Verify if IP address allocated and set successfully */
    /* DHCP Negotiation done. Re-enable IP forwarding. */
    QCMAP_CM_LOG("Re-enabling IP forwarding.\n");
    snprintf( command, MAX_COMMAND_STR_LEN,
              "echo 1 > /proc/sys/net/ipv4/ip_forward");
    ds_system_call(command, strlen(command));
  }
  else
  {
    QCMAP_CM_LOG("Bringing up STA iface eth0 with static ip configuration\n");
    staIp.s_addr = htonl(static_ip_config->ip_addr);
    staMask.s_addr = htonl(static_ip_config->netmask);
    staGwIp.s_addr = htonl(static_ip_config->gw_ip);
    /* Bring the interface UP. */
    memset(netMask,0,QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
    strlcpy(netMask, inet_ntoa(staMask), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
    snprintf( command, MAX_COMMAND_STR_LEN,
              "ifconfig eth0 %s netmask %s up", inet_ntoa(staIp),netMask);
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);
    /* Add the default route. */
    snprintf( command, MAX_COMMAND_STR_LEN,
              "route add default gw %s dev eth0", inet_ntoa(staGwIp));
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);
    /* Populate the /etc/resolv.conf file with DNS address provided. */
    addr.s_addr = htonl(static_ip_config->dns_addr);
    snprintf(command, MAX_COMMAND_STR_LEN,
             "echo 'nameserver %s' > /etc/resolv.conf", inet_ntoa(addr));
    ds_system_call(command, strlen(command));
    QCMAP_CM_LOG("%s\n", command);
  }
  /*
   * Just check whether the STA Interface is UP. No need to add the DNS address
   * to /etc/resolv.conf file. It will taken care by dhcpcd.
   */
  memset(this->staIpAddr, 0, sizeof(staIpAddr));
  memset(netMask,0,QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
  ret = GetStaIP(this->staIpAddr, sizeof(this->staIpAddr), netMask, sizeof(netMask));
  if ( TRUE == ret )
  {
    QCMAP_CM_LOG("STA Connected to external hotspot.\n");
    return true;
  }
  else
  {
    QCMAP_CM_LOG("Error: GetStaIP Failed. Bailing out.\n");
    return FALSE;
  }

}

/* Disconnect Station interface from the external hotspot */
boolean QCMAP_ConnectionManager::DisconnectSTA()
{
  char command[MAX_COMMAND_STR_LEN];
/*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  QCMAP_CM_LOG("Disconnecting STA\n");

  if( this->cfg.lan_config.interface[sta_iface_index].enable_supplicant )
  {
    QCMAP_CM_LOG("Killing suppicant process.\n");
    snprintf( command, MAX_COMMAND_STR_LEN, "killall -15 wpa_supplicant" );
    ds_system_call(command, strlen(command));
  }
  else
  {
    /* Disconnect STA in Open Mode. */
    snprintf( command, MAX_COMMAND_STR_LEN, "iwconfig eth0 essid off" );
    ds_system_call(command, strlen(command));
    snprintf( command, MAX_COMMAND_STR_LEN, "ifconfig eth0 down" );
    ds_system_call(command, strlen(command));
  }

  QCMAP_CM_LOG("Killing dhcpcd process.\n");
  snprintf( command, MAX_COMMAND_STR_LEN, "killall -15 dhcpcd" );
  ds_system_call(command, strlen(command));

  return true;
}

/* Find IP address assigned to the STA interface */
boolean QCMAP_ConnectionManager::GetStaIP(char *staIP, int staIPBufLen, char *netMask, int netMaskBufLen)
{
  int s, ret;
  struct ifreq buffer;
  struct sockaddr *sa;

  /* Open a socket */
  s = socket(PF_INET, SOCK_DGRAM, 0);
  if (s < 0)
  {
    QCMAP_CM_LOG("Unable to open socket to get IP address.\n");
    return false;
  }
  /* Set up the interface request buffer for eth0. */
  memset(&buffer, 0x00, sizeof(buffer));
  strlcpy(buffer.ifr_name, "eth0", IFNAMSIZ);

  /* Call the ioctl to get the address. */
  ret = ioctl(s, SIOCGIFADDR, &buffer);

  if (ret < 0)
  {
    QCMAP_CM_LOG("Unable to call ioctl to get IP address.\n");
    /* Close the socket handle. */
    close(s);
    return false;
  }

  /* Copy out the ip address for the interface. */
  sa = (struct sockaddr *)&(buffer.ifr_addr);
  strlcpy(staIP, inet_ntoa(((struct sockaddr_in *)sa)->sin_addr), staIPBufLen);

  /* Set up the interface request buffer for eth0. */
  memset(&buffer, 0x00, sizeof(buffer));
  strlcpy(buffer.ifr_name, "eth0", IFNAMSIZ);

  /* Call the ioctl to get the address. */
  ret = ioctl(s, SIOCGIFNETMASK, &buffer);

  if (ret < 0)
  {
    QCMAP_CM_LOG("Unable to call ioctl to get netmask.\n");
    /* Close the socket handle. */
    close(s);
    return false;
  }

  /* Copy out the netmask for the interface. */
  sa = (struct sockaddr *)&(buffer.ifr_netmask);
  strlcpy(netMask, inet_ntoa(((struct sockaddr_in *)sa)->sin_addr), netMaskBufLen);

  /* Close the socket handle. */
  close(s);

  return true;
}

boolean QCMAP_ConnectionManager::GetIPv4WWANNetworkConfiguration
(
  uint32 *public_ip,
  uint32 *primary_dns,
  uint32 *secondary_dns
)
{
  int qmi_error, qcmap_cm_errno = QCMAP_CM_SUCCESS;

  if (this->sta_connected)
  {
    QCMAP_CM_LOG("Currently in STA mode - cannot get WWAN Config\n");
    return false;
  }

  if (qcmap_cm_get_state() != QCMAP_CM_WAN_CONNECTED)
  {
    /* The WWAN is not connected, so it doesn't have an IP or DNS serers*/
    return false;
  }

  qmi_error = qcmap_cm_get_ipv4_net_conf(this->qcmap_cm_handle, public_ip, primary_dns, secondary_dns, &qcmap_cm_errno);

  if (qmi_error == QCMAP_CM_SUCCESS)
  {
    return true;
  }

  QCMAP_CM_LOG("Unable to get ipv4 net conf. errno %d\n", qcmap_cm_errno);

  return false;
}
boolean QCMAP_ConnectionManager::ChangeNatType
(
  qcmap_cm_nat_type nat_type
)
{
  int qcmap_cm_errno;
  int ret_val;
  qcmap_cm_nat_type cur_nat_type;
/*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Check if MobileAP is enabled */
  if (!this->qcmap_enable)
  {
    QCMAP_CM_LOG("Error: QCMAP not enabled.\n");
    return FALSE;
  }

  if (this->qcmap_nat_type_change_in_progress)
  {
    QCMAP_CM_LOG("Previous NAT Type change request in progress.\n");
    return FALSE;
  }

  pthread_mutex_lock(&this->cm_mutex);
  this->qcmap_nat_type_change_in_progress = TRUE;
  pthread_mutex_unlock(&this->cm_mutex);

  if((nat_type != QCMAP_CM_SYMMETRIC_NAT) &&
     (nat_type != QCMAP_CM_PORT_RESTRICTED_CONE_NAT))
  {
    QCMAP_CM_LOG("Invalid NAT type specified: %d", (int)nat_type);
    goto bail;
  }

  /* Check if Nat Type chosen is the one currently being used */
  if(this->GetNatType(&cur_nat_type))
  {
    if(cur_nat_type == nat_type)
    {
      QCMAP_CM_LOG("Error: Nat Type chosen = %s - Currently being used\n",
                    nat_type ? "PORT_RESTRICTED_CONE":"SYMMETRIC");

      pthread_mutex_lock(&this->cm_mutex);
      this->qcmap_nat_type_change_in_progress = FALSE;
      pthread_mutex_unlock(&this->cm_mutex);

      return TRUE;
    }
  }
  else
  {
    QCMAP_CM_LOG("Error retrieving NAT type\n");
    goto bail;
  }

  /* Remember previous NAT Type used */
  this->cfg.nat_config.prev_nat_type = this->cfg.nat_config.nat_type;
  this->cfg.nat_config.nat_type = nat_type;

  if(this->sta_connected)
  {
    return this->SetNatType();
  }

  /* Disable Autoconnect. Remember the previous state. */
  this->auto_connect = this->cfg.wan_config.auto_connect;

  ret_val = qcmap_cm_set_auto_connect(this->qcmap_cm_handle,
                                      &qcmap_cm_errno, false);
  if (ret_val != QCMAP_CM_SUCCESS)
  {
    QCMAP_CM_LOG("Disable Autoconnect failed. ret_val: %d, Err: %d\n",
                  ret_val, qcmap_cm_errno);
    this->cfg.nat_config.nat_type = this->cfg.nat_config.prev_nat_type;

    goto bail;
  }

  /* Disconnect backhaul. */
  QCMAP_CM_LOG("Disconnect Backhaul.\n");

  if ( false == DisconnectBackHaul() )
  {
    QCMAP_CM_LOG("Disconnect backhaul failed. Unable to set new NAT Type.\n");
    this->cfg.nat_config.nat_type = this->cfg.nat_config.prev_nat_type;

    /* Restore Autoconnect value. */
    ret_val = qcmap_cm_set_auto_connect(this->qcmap_cm_handle, &qcmap_cm_errno,
                                        this->auto_connect);
    if (ret_val != QCMAP_CM_SUCCESS)
    {
      QCMAP_CM_LOG("Restore Autoconnect failed!ret_val: %d, Error: %s\n",
                   ret_val, qcmap_cm_errno);
    }

    goto bail;
  }

  QCMAP_CM_LOG(" Please wait while the new NAT type is being set.\n");

  return TRUE;

  bail:
    pthread_mutex_lock(&this->cm_mutex);
    this->qcmap_nat_type_change_in_progress = FALSE;
    pthread_mutex_unlock(&this->cm_mutex);

    return FALSE;
}

boolean QCMAP_ConnectionManager::GetNatType
(
  qcmap_cm_nat_type *cur_nat_type
)
{
  int qcmap_cm_errno;
  int ret_val;

  if(this->sta_connected)
  {
    *cur_nat_type = this->cfg.nat_config.nat_type;
    return TRUE;
  }

  /* call CM lib to get NAT type */
  ret_val = qcmap_cm_nat_type_get(this->qcmap_cm_handle,
                                  cur_nat_type,
                                  &qcmap_cm_errno);

  if (ret_val != QCMAP_CM_SUCCESS)
    return false;

  return true;
}

boolean QCMAP_ConnectionManager::SetNatType()
{
  int qcmap_cm_errno;
  int ret_val;
  boolean ret = TRUE;
  /*------------------------------------------------------------------------*/

  if(this->sta_connected)
  {
    QCMAP_CM_LOG("STA mode enabled. Changing NAT type on A5\n");

    if(!this->EnableNATonA5())
    {
      QCMAP_CM_LOG("Error: Failed to change NAT type on A5\n");
      this->cfg.nat_config.nat_type = this->cfg.nat_config.prev_nat_type;
      ret = FALSE;
      goto bail;
    }

    goto bail;
  }

  /* Send NAT type change notification to modem */
  pthread_mutex_lock(&this->cm_mutex);

  ret_val = qcmap_cm_nat_type_set(this->qcmap_cm_handle,
                             this->cfg.nat_config.nat_type, &qcmap_cm_errno);

  pthread_mutex_unlock(&this->cm_mutex);

  if (ret_val != QCMAP_CM_SUCCESS)
  {
    QCMAP_CM_LOG("qcmap_cm_nat_type_set failed.\n");
	this->cfg.nat_config.nat_type = this->cfg.nat_config.prev_nat_type;
    ret = FALSE;
  }

  /* Connect Backhaul based on the previous state. */
  if ( this->auto_connect == true )
  {
    if ( false == ConnectBackHaul() )
    {
      QCMAP_CM_LOG("Connecting to backhaul fails. Try connecting later.\n");
    }
    else
    {
      QCMAP_CM_LOG("Connected to backhaul.\n");
    }

    /* Restore Autoconnect value. */
    ret_val = qcmap_cm_set_auto_connect(this->qcmap_cm_handle, &qcmap_cm_errno,
                                        this->auto_connect);
    if (ret_val != QCMAP_CM_SUCCESS)
    {
      QCMAP_CM_LOG("Restore Autoconnect failed!ret_val: %d, Error: %s\n",
                    ret_val, qcmap_cm_errno);
    }
  }

  bail:
    if(ret)
    {
      QCMAP_CM_LOG("QCMAP NAT type changed to %s\n",
                   this->cfg.nat_config.nat_type ? "PORT RESTRICTED CONE NAT":\
                                                   "SYMMETRIC NAT");
      this->WriteConfigToXML();
    }

    pthread_mutex_lock(&this->cm_mutex);
    this->qcmap_nat_type_change_in_progress = FALSE;
    pthread_mutex_unlock(&this->cm_mutex);

    return ret;

}

boolean QCMAP_ConnectionManager::SendWanConfig()
{
  qcmap_qti_msg_t qcmap_qti_msg;
  int msgqid;
  key_t key;

  QCMAP_CM_LOG("SendWanConfig()\n");

  /* Using Message Queues here to inform QTI about current QCMAP WWAN params*/

  /* A key is found using ftok() which takes a filename and a random
        character to generate a unique key*/
  if ((key = ftok("/usr/bin/QCMAP_ConnectionManager", QCMAP_QTI_KEY_ID)) == -1)
  {
    QCMAP_CM_LOG("SendWanConfig() : Error generating a key");
    return FALSE;
  }

  /* From the Key obtained, msg queue is created with rw-rw-rw- permissions */
  if ((msgqid = msgget(key, 0666 | IPC_CREAT)) == -1)
  {
    QCMAP_CM_LOG("SendWanConfig() : Error creating the message queue");
    return FALSE;
  }

  memset(&qcmap_qti_msg,0,sizeof(qcmap_qti_msg_t));

  qcmap_qti_msg.mtext[0] = this->cfg.wan_config.tech;
  qcmap_qti_msg.mtext[1] = this->cfg.wan_config.ip_family;
  qcmap_qti_msg.mtext[2] =
       this->cfg.wan_config.profile_id.v4.umts_profile_index;
  qcmap_qti_msg.mtext[3] =
       this->cfg.wan_config.profile_id.v4.cdma_profile_index;

  if((qcmap_qti_msg.mtext[1] == QCMAP_IP_V6_V01)||
     (qcmap_qti_msg.mtext[1] == QCMAP_IP_V4V6_V01))
  {
    qcmap_qti_msg.mtext[4] =
         this->cfg.wan_config.profile_id.v6.umts_profile_index;
    qcmap_qti_msg.mtext[5] =
         this->cfg.wan_config.profile_id.v6.cdma_profile_index;
  }
  /* The mtype variable will be useful in msgrcv() in QTI to retrieve the
     corresponding msg sent. A positive value for mtype indicates that 
     only the specific message with the same mtype is to be dequeued
     from the queue.*/
  qcmap_qti_msg.mtype = QCMAP_QTI_MTYPE_ID;

  /* Queue the message specifying the queue ID and the payload size */
  if( msgsnd(msgqid, (void *)&qcmap_qti_msg, QCMAP_QTI_MSG_SIZE, 0) == -1)
  {
    QCMAP_CM_LOG("SendWanConfig() : Error sending the message");
    return FALSE;
  }

  return TRUE;
}

void QCMAP_ConnectionManager::SetQtiState
(
  boolean enable
)
{
  /* This state variable tracks the current QTI link state.
     Currently being used to prevent QCMAP from triggering
     data call when QTI link is down in tethering only mode. */

  this->cfg.lan_config.qti_link_up = enable;
  return;
}

boolean QCMAP_ConnectionManager::GetQtiState()
{
  /* This state variable tracks the current QTI link state. */
  return this->cfg.lan_config.qti_link_up;
}
boolean QCMAP_ConnectionManager::GetIPV6PrefixInfo()
{

  struct sockaddr_nl  src_addr;
  struct sockaddr_nl dest_addr;
  struct iovec iov;
  struct sockaddr_nl sa;
  struct msghdr msg = { (void *)&sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
  struct nlmsghdr *nlh = NULL;
  struct sockaddr_in6 *sin6 = NULL;
  char devname[16];
  void *buf = NULL;
  qcmap_nl_getaddr_req_t req;
  int sock_fd;
  unsigned int buflen = 0;
  ssize_t recvsize;
  int ret = true;
  int qcmap_cm_error;

  if (qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_IP_V6_V01, devname, &qcmap_cm_error) !=
      QCMAP_CM_SUCCESS)
  {
    QCMAP_CM_LOG("Couldn't get ipv6 rmnet name. error %d\n", qcmap_cm_error);
    strlcpy(devname, QCMAP_V6_DEFAULT_DEVICE_NAME, 16);
  }

  memset(&(this->ipv6_prefix_info), 0, sizeof(qcmap_cm_nl_prefix_info_t));


  if ((sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0)
  {
    LOG_MSG_ERROR("GetIPV6PrefixInfo: socket() failed: %d\n", errno, 0, 0);
    ret = false;
    return ret;
  }

  /* Initialize the source address */
  memset(&src_addr, 0, sizeof(src_addr));

  #define QCMAP_NL_PID_MASK      (0x7FFFFFFF)
  #define QCMAP_NL_PID           (getpid() & QCMAP_NL_PID_MASK)
  src_addr.nl_family = AF_NETLINK;
  src_addr.nl_pid = QCMAP_NL_PID;
  src_addr.nl_groups = 0; /* Interested in unicast messages */

  /* Bind the socket to our source address */
  if (bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0)
  {
    LOG_MSG_ERROR("GetIPV6PrefixInfo: bind() failed: %d\n", errno, 0, 0);
    ret = false;
    goto bail;
  }

  /* Initialize destination address structure */
  memset(&dest_addr, 0, sizeof(dest_addr));

  dest_addr.nl_family = AF_NETLINK;
  dest_addr.nl_pid    = 0;  /* Addressed to kernel */
  dest_addr.nl_groups = 0;  /* This is a unicast message */

  /* Initialize the request message */
  memset(&req, 0, sizeof(req));

  /* Fill the netlink request message */
  req.nlh.nlmsg_len   = sizeof(req);
  req.nlh.nlmsg_pid   = 0;
  req.nlh.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST;
  req.nlh.nlmsg_type  = RTM_GETADDR;

  /* Set the ip family and interface index for which the addresses are requested */
  req.ifa.ifa_family = AF_INET6;

  LOG_MSG_INFO1("Get Prefix Info for interface %s ", devname, 0, 0);

  if ( sendto(sock_fd, (void*) &req, sizeof(req), 0,
               (struct sockaddr*) &dest_addr, sizeof(dest_addr)) < 0)
  {
    LOG_MSG_ERROR("GetIPV6PrefixInfo: bind() failed: %d\n", errno, 0, 0);
    ret = false;
    goto bail;
  }

  /* Max size of the netlink response message */
  #define QCMAP_NL_MAX_MSG_SIZE  (1024 * 4)
  /* Allocate and initialize buffer to read message */
  buf = calloc(1, NLMSG_SPACE(QCMAP_NL_MAX_MSG_SIZE));
  if (NULL == buf)
  {
    LOG_MSG_ERROR("GetIPV6PrefixInfo: memory alloc failure: %d\n", errno, 0, 0);
    ret = false;
    goto bail;
  }

  iov.iov_base = buf;
  iov.iov_len  = NLMSG_SPACE(QCMAP_NL_MAX_MSG_SIZE);

  /* Read message from kernel */
  if ((recvsize = recvmsg(sock_fd, &msg, 0)) < 0)
  {
    LOG_MSG_ERROR("GetIPV6PrefixInfo: memory alloc failure: %d\n", errno, 0, 0);
    ret = false;
    goto bail;
  }
  else
  {
    buflen = recvsize;
    LOG_MSG_INFO3("received response from kernel size=%d\n", buflen, 0, 0);
  }

  nlh = (struct nlmsghdr *)buf;

  /* Parse the message one header at a time */
  while (NLMSG_OK(nlh, buflen))
  {
    struct ifaddrmsg *ifa;
    struct rtattr *rta;
    int rtattrlen;
    struct ifreq ifr;

    ifa = (struct ifaddrmsg *)NLMSG_DATA(nlh);
    rta = (struct rtattr *)IFA_RTA(ifa);

    /* Make sure that the requested and received address family is the same */
    if ( AF_INET6 != ifa->ifa_family || RT_SCOPE_UNIVERSE != ifa->ifa_scope )
    {
      LOG_MSG_ERROR("GetIPV6PrefixInfo: ip family %d, Scope %d  don't match\n",
                    ifa->ifa_family, ifa->ifa_scope, 0);
      /* Advance to next header */
      nlh = NLMSG_NEXT(nlh, buflen);
      continue;
    }

    rtattrlen = IFA_PAYLOAD(nlh);

    /* Parse the RTM_GETADDR attributes */
    while (RTA_OK(rta, rtattrlen))
    {
      switch (rta->rta_type)
      {
        case IFA_ADDRESS:
           this->ipv6_prefix_info.prefix_len = ifa->ifa_prefixlen;
           sin6 = (struct sockaddr_in6 *)&this->ipv6_prefix_info.prefix_addr;
           memcpy(SASTORAGE_DATA(this->ipv6_prefix_info.prefix_addr),
                  RTA_DATA(rta),
                  sizeof(sin6->sin6_addr));
           this->ipv6_prefix_info.prefix_info_valid = TRUE;
           break;
        case IFA_CACHEINFO:
           memcpy( &this->ipv6_prefix_info.cache_info,
                   RTA_DATA(rta),
                   sizeof(this->ipv6_prefix_info.cache_info) );
           LOG_MSG_INFO2( "GetIPV6PrefixInfo: Address Cache Info - prefered=%d valid=%d\n",
                           this->ipv6_prefix_info.cache_info.ifa_prefered,
                           this->ipv6_prefix_info.cache_info.ifa_valid, 0);
           break;
        default:
           LOG_MSG_INFO3("GetIPV6PrefixInfo: rta_type=%x\n", rta->rta_type, 0, 0);
           break;
      }
      rta = RTA_NEXT(rta, rtattrlen);
    }

    /* Break out from here as by this point we would have found the address. */
    break;
  }

bail:
  close(sock_fd);

  if ( buf != NULL )
    free(buf);

  return ret;


}
void QCMAP_ConnectionManager::RemoveIPV6Address()
{
  char command[MAX_COMMAND_STR_LEN];
  unsigned char ip6_addr[MAX_IPV6_PREFIX+1];   // Getting IPv6 Address
  char devname[16];
  int qcmap_cm_error;

  if (!this->ipv6_prefix_info.prefix_info_valid )
  {
    LOG_MSG_ERROR("invalid Prefix Passed\n", 0, 0, 0);
    return;
  }
  if (qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_IP_V6_V01, devname, &qcmap_cm_error) !=
      QCMAP_CM_SUCCESS)
  {
    QCMAP_CM_LOG("Couldn't get ipv6 rmnet name. error %d\n", qcmap_cm_error);
    strlcpy(devname, QCMAP_V6_DEFAULT_DEVICE_NAME, 16);
  }
  memset(ip6_addr, 0, MAX_IPV6_PREFIX + 1);
  inet_ntop(AF_INET6,
            ((struct sockaddr_in6 *)&(this->ipv6_prefix_info.prefix_addr))->sin6_addr.s6_addr,
            ip6_addr, MAX_IPV6_PREFIX );
  snprintf(command, MAX_COMMAND_STR_LEN,
           "ip -6 addr del %s/%d dev %s",
           ip6_addr,this->ipv6_prefix_info.prefix_len,
           devname);
  ds_system_call( command, strlen(command));
  QCMAP_CM_LOG("%s\n", command);

  memset(&(this->ipv6_prefix_info), 0, sizeof(qcmap_cm_nl_prefix_info_t));

  return;
}
