/*====================================================

FILE:  QCMAP_ConnectionManager.cpp

SERVICES:
   QCMAP Connection Manager Implementation

=====================================================

  Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

=====================================================*/
/*===========================================================================
  EDIT HISTORY FOR MODULE

  Please notice that the changes are listed in reverse chronological order.

  when       who        what, where, why
  --------   ---        -------------------------------------------------------
  07/11/12   gk         9x25
  10/26/12   cp         Added support for Dual AP and different types of NAT.
  12/19/12   sb         Added support for RNDIS/ECM USB tethering.
  02/01/13   cp         Minor IPv6 changes.
  02/27/13   cp         Added support for deprecating of prefix when switching between
                        station mode and WWAN mode.
  03/12/13   sb         Minor hostapd fixes.
  04/09/13   cp         AR6003 changes.
  04/16/13   mp         Added support to get IPv6 WWAN/STA mode configuration.
  06/12/13   sg         Added DHCP Reservation feature
  09/17/13   at         Added support to Enable/Disable ALGs
  01/11/14   sr         Added support for connected Devices in SoftAP
  25/02/14   pm         Changes regarding Sta Assoc
  02/24/14   vm         Changes to Enable/Disable Station Mode to be in accordance
                        with IoE 9x15
  03/27/14   cp         Added support to DUN+SoftAP.
  06/06/14   rk         Added support DHCP option 26 for mtu in dnsmasq/dhcp server..
===========================================================================*/
#include <fstream>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
#include <ifaddrs.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */

#include "ds_string.h"
#include "ds_util.h"
#include "qcmap_cm_api.h"
#include "ds_qmi_qcmap_msgr.h"
#include "qualcomm_mobile_access_point_msgr_v01.h"
#include "QCMAP_ConnectionManager.h"

// Define some constants.
#define ETH_HDRLEN 14  // Ethernet header length
#define IP6_HDRLEN 40  // IPv6 header length
#define ICMP_HDRLEN 8  // ICMP header length for echo request, excludes data
#define ETH_P_IPV6 0x86DD

/* Default network config */
#define MODULE_NAME "qca6174"
#define AR6003 "ar6003"
#define AR6004 "ar6004"
#define WLAN_IFACE "wlan0"
#define ECM_IFACE "ecm0"
#define HOSTAPD_CFG_PATH "/etc/hostapd.conf"
#define HOSTAPD_PID_FILE "/etc/hostapd_ssid1.pid"

#define STA_MODE_HOSTAPD_CFG_PATH "/etc/sta_mode_hostapd.conf"
#define A5_IP_ADDR "192.168.225.1"
#define A5_SUB_NET_MASK "255.255.255.0"

#define A5_RMNET_SUB_NET_MASK "255.255.255.248"

#define HOSTAPD_CFG_PATH_SSID2 "/etc/hostapd-wlan1.conf"
#define HOSTAPD_SSID2_PID_FILE "/etc/hostapd_ssid2.pid"
#define A5_IP_ADDR_SSID2 "192.168.2.1"
#define A5_SUB_NET_MASK_SSID2 "255.255.255.0"

#define A5_RMNET_IP_ADDR "192.168.225.2"
#define Q6_RMNET_FACING_A5_IP_ADDR "192.168.225.3"
#define USB_RMNET_IP_ADDR "192.168.225.4"
#define Q6_RMNET_FACTING_USB_RMNET_IP_ADDR "192.168.225.5"
#define NAT_IP_ADDR "192.168.225.6"
#define MASK_NOT_PREF_1 "255.255.255.254"
#define MASK_NOT_PREF_2 "255.255.255.0"

#define RNDIS_ECM_GATEWAY_IP "192.168.225.5"
#define RNDIS_ECM_SUBNET_MASK "255.255.255.252"
#define RNDIS_ECM_USB_IP "192.168.225.6"

#define SUPPLICANT_CFG_PATH "/etc/wpa_supplicant.conf"
#define WPA_CTRL_PATH "/var/run/wpa_supplicant"
#define HOSTAPD_CTRL_PATH "/var/run/hostapd"
#define STA_INTERFACE_CTRL_PATH "/usr/bin/QCMAP_StaInterface"
#define HOSTNAME_PATH "/etc/hostname.txt"


#define AP_IFACE_LL_ADDR       "169.254.1.1"
#define GUEST_AP_IFACE_LL_ADDR "169.254.2.1"
#define USB_IFACE_LL_ADDR      "169.254.3.1"
#define LL_SUBNET_MASK         "255.255.255.0"
#define PPP_IFACE_LL_ADDR      "169.254.4.1"

/* Use a dummy MAC for storing DUN client information. */
#define PPP_CLIENT_MAC_ADDR "ff:ff:ff:ff:ff:ff"

#define MAX_CMD_SIZE 100
#define MAX_SCAN_SIZE 100

#define MIN_NOTIFY_INTERVAL 30
#define MAX_NOTIFY_INTERVAL 60000

#define DHCP_HOSTS_FILE "/etc/dhcp_hosts"
#define DHCP_RECORD_LENGTH 100

#define QCMAP_MULTIPLE_SYSTEM_CALL_FILE "/etc/qcmap_multiple_system_call.sh"
#define KERNEL_VERSION_LENGTH 16

#define QCMAP_FIREWALL_CONFIG "/etc/mobileap_firewall.xml"
#define QCMAP_ERI_CONFIG "/etc/mobileap_eri_config.bin"
#define FACTORY_CONF "/etc/factory_mobileap_cfg.xml"
#define TEMP_MOBILEAP_CFG "/etc/mobileap_cfg.tmp"
#define TEMP_FIREWALL_CFG "/etc/mobileap_firewall.tmp"

/* With size of 2K, maximum of 6 SIP server FQDNs can be accomodated.
Need this size to be increased if we need to accomodate more*/
#define MAX_DHCP_COMMAND_STR_LEN 2000
#define MTU_STRING_SIZE 5
#define DHCP_OPTION_MTU "26"
static FILE *dhcp_hosts_fp;
char mac_addr_string[QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01];

struct icmp6_ra_pkt {
  struct nd_router_advert router_adv;
  struct nd_opt_prefix_info prefix_info;
}__attribute__((__packed__));

/* Set Default Valid and Preferred Life Time Values. */
#define IPV6_DEFAULT_PREFERED_LIFETIME 2700;
#define IPV6_DEFAULT_VALID_LIFETIME 7200;
#define IPV6_MIN_PREFIX_LENGTH 64
#define IPV6_MAX_PREFIX_LENGTH 128
#define MAX_IPV6_PREFIX 40

/* Request message to send to the kernel to retrieve prefix info. */
typedef struct qcmap_nl_getaddr_req_s
{
  struct nlmsghdr   nlh;
  struct ifaddrmsg  ifa;
} qcmap_nl_getaddr_req_t;

/* WIFI iDriver/Firmware Init Delay Micro Seconds */
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
        __FUNCTION__               \
    )

std::string media_dir_blacklist[] = {
  "..",
  "./",
  "/bin",
  "/boot",
  "/cache",
  "/dev",
  "/lib",
  "/linuxrc",
  "/lost+found",
  "/proc",
  "/sbin",
  "/sys"
};

#define CARRY16(x) (((x) & 0x10000) ? 1 : 0)
/* Use this only when the containing type can't overflow due to the carry-overs */
#define CARRYS16(x) (((x) & (~0xffff)) >> 16)
#define IPTYPE_IPV6_ICMP        58

static uint16_t ones_comp_sum(uint16_t *ptr, size_t size)
{
  uint32_t ret = 0;
  size_t i;
  uint16_t *cur;
  for (cur = ptr, i = 0; i < size; i++) {
    ret += ntohs(cur[i]);
    ret = (ret + CARRY16(ret)) & 0xffff;
  }
  return ret;
}

uint16_t icmp6_checksum(struct ip6_hdr *ip6h, struct icmp6_header *icmp6h, uint16_t icmplen)
{
  uint32_t ret = 0;
  struct ip6_hdr tmph;

  tmph = *ip6h;
  tmph.ip6_nxt = IPTYPE_IPV6_ICMP;
  tmph.ip6_plen = icmplen;

  /* Checksum computation must contain a pseudo-header, comprised of:
   * - src and dst ip addresses
   * - payload length (32 bit value)
   * - next header
   */
  ret = ones_comp_sum((uint16_t *)&ip6h->ip6_src, sizeof(struct in6_addr)/sizeof(uint16_t));
  ret += ones_comp_sum((uint16_t *)&ip6h->ip6_dst, sizeof(struct in6_addr)/sizeof(uint16_t));
  ret += ntohs(ip6h->ip6_plen);

  if (icmplen != ntohs(ip6h->ip6_plen))
    LOG_MSG_INFO1("payload length %d does not match with icmplen %d", ip6h->ip6_plen, icmplen, 0);

  ret += (uint16_t) IPTYPE_IPV6_ICMP;
  ret += ones_comp_sum((uint16_t *)icmp6h, icmplen / sizeof(uint16_t));
  ret += CARRYS16(ret);

  return htons(~ret & 0xffff);
}


/*===========================================================================
  FUNCTION doesFileExist
==========================================================================*/
/*!
  @brief
  Function checks if file is present the the specified path.

@return
  boolean

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
boolean doesFileExist (char *filename)
{
  struct stat  statbuf;

  bzero( &statbuf, sizeof(statbuf) );
  if( (stat(filename, &statbuf) == 0) && (S_ISREG(statbuf.st_mode)))
    return true;
  else
    return false;
}

/*==========================================================
  FUNCTION    qcmap_match_mac_address
===========================================================*/
/*!
@brief
  Comparator function for matching MAC address

@parameters
  Two void pointers with the MAC addresses to be compared

@return
  0 - on a match
  1 - otherwise

  @note

 @ Dependencies
    - None

 @ Side Effects
    - None
*/
/*==========================================================*/
static long int
qcmap_match_mac_address
(
  const void *first,
  const void *second
)
{
  if( (first == NULL) || (second == NULL))
  {
    return 1;
  }
  uint8* msg_mac_addr = (uint8*)first;
  qcmap_cm_client_data_info_t* connectedDevicesList = NULL;
  connectedDevicesList = ( qcmap_cm_client_data_info_t*)second;

  return(memcmp( msg_mac_addr,
                 connectedDevicesList->mac_addr,
                 QCMAP_MSGR_MAC_ADDR_LEN_V01));
}

/*==========================================================
 FUNCTION    qcmap_match_device_type
===========================================================*/
/*!
@brief
  Comparator function for match ConnectedDevices device type

@parameters
  Two void pointers with the device type to be compared

@return
  0 - on a match
  1 - otherwise

@note

@ Dependencies
   - None

@ Side Effects
   - None
*/
/*==========================================================*/
static long int
qcmap_match_device_type
(
  const void *first,
  const void *second
)
{
  if( (first == NULL) || (second == NULL))
  {
    return 1;
  }
  qcmap_msgr_device_type_enum_v01* device_type = (qcmap_msgr_device_type_enum_v01*)first;
  qcmap_cm_client_data_info_t* connectedDevicesList = NULL;
  connectedDevicesList = ( qcmap_cm_client_data_info_t*)second;

  return((*device_type == connectedDevicesList->device_type) ? 0 : 1 );
}

/*==========================================================
  FUNCTION    qcmap_match_v6_iid
===========================================================*/
/*!
@brief
  Comparator function for matching IID

@parameters
  void *first - qcmap_cm_prefix_iid_info_t ptr
  void *second - qcmap_cm_client_data_info_t ptr to client info

@return
  QCMAP_CM_SUCCESS - on a match
  QCMAP_CM_ERROR - otherwise

  @note

 @ Dependencies
    - None

 @ Side Effects
    - None
*/
/*==========================================================*/
static long int
qcmap_match_v6_iid
(
  const void *first,
  const void *second
)
{
  qcmap_cm_client_data_info_t* connectedDevicesList = NULL;
  qcmap_cm_prefix_iid_info_t *prefix_iid_ptr = NULL;
  struct ps_in6_addr *client_iid_ptr = NULL;
  struct ps_in6_addr *client_prefix_ptr = NULL;

  if( (first == NULL) || (second == NULL))
  {
    return QCMAP_CM_ERROR;
  }

  prefix_iid_ptr = (qcmap_cm_prefix_iid_info_t *)first;
  connectedDevicesList = ( qcmap_cm_client_data_info_t*)second;

  if ( prefix_iid_ptr->iid_ptr == NULL )
  {
    LOG_MSG_ERROR("IID addr ptr is NULL,cannot match "
                  "IID's", 0, 0, 0);
    return QCMAP_CM_ERROR;
  }

  client_iid_ptr = (struct ps_in6_addr *)connectedDevicesList->link_local_v6_addr;

  /*Check whether IID's match*/
  if(!memcmp(&prefix_iid_ptr->iid_ptr->ps_s6_addr64[1],
             &client_iid_ptr->ps_s6_addr64[1],
             sizeof(uint64_t)))
  {
    LOG_MSG_INFO1("qcmap_match_v6_iid - IID match found\n", 0, 0, 0);
    return QCMAP_CM_SUCCESS;
  }
  else
  {
    //IID's do not match
    return QCMAP_CM_ERROR;
  }
}

/*=======================================================================
  FUNCTION    fetchHostNameAndLeasetime
=======================================================================*/
/*!
@brief
  Function to fetch the Device name  and lease time from
  dnsmasq.leases file

@parameters
  fd of the file to be parsed
  Device MAC address whose hostname & lease expiry has to be found
  char pointer to store the hostname of device
  uint32 pointer to hold the expiry time (in minutes) of the DHCP lease

@return
  void

@note

 @ Dependencies
    - None

 @ Side Effects
    - None
*/
/*==========================================================================*/
static void
fetchHostNameAndLeasetime
(
  FILE* fd,
  uint8* mac_addr,
  char* hostName,
  uint32* leaseExpiryTime
)
{
  if ((fd == NULL) || (mac_addr == NULL) ||(hostName == NULL) ||
      (leaseExpiryTime == NULL))
  {
    LOG_MSG_ERROR("Null arguements passed.\n",0,0,0);
    return;
  }
  char *expiryTime=NULL,*charMacAddr=NULL, *filehostName=NULL;
  char stringline[MAX_COMMAND_STR_LEN];
  char mac_addr_string[QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01];/*char array of mac addr*/
  int bufferSize = QCMAP_EPOCH_TIME_LEN +
                   QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01 +
                   QCMAP_MSGR_DEVICE_NAME_MAX_V01;
  int expTime = 0;
  time_t expTimeFormat;
  //Reset the file ptr to start of file
  if (fseek(fd,0,SEEK_SET) !=0)
  {
    LOG_MSG_ERROR("File pointer not reset to beginning of file\n",0,0,0);
    return;
  }

  ds_mac_addr_ntop(mac_addr, mac_addr_string);
  while(fgets( stringline, bufferSize, fd) != NULL)
  {
    expiryTime = strtok(stringline, " ");
    charMacAddr = strtok(NULL, " ");
    /* KW fix */
    if ( charMacAddr!=NULL && !strcmp(charMacAddr, mac_addr_string))
    {
      //If the Mac address matches, store the hostname
      filehostName = strtok(NULL, " ");
      if(filehostName)
        strlcpy( hostName,filehostName,QCMAP_MSGR_DEVICE_NAME_MAX_V01 );
      //Convert the expiry time from char format to interger
      expTime = ds_atoi((const)expiryTime);
      expTimeFormat = expTime;
      //Convert the time from Epoch format to readable format
      *leaseExpiryTime = (expTimeFormat - time(NULL))/60;
      return;
    }
  }
  return;
}

/*===========================================================================
  FUNCTION strtolower
==========================================================================*/
/*!
@brief
  Converts the string passed to lower case.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/

void  strtolower(uint8 *str)
{
  int counter = 0;
  char c;

  if( str!=NULL )
  {
    while ( str[counter] ) {
      c = str[counter];
      str[counter] = tolower(c);
      counter++;
    }
  }
}

/*===========================================================================
  FUNCTION check_non_empty_mac_addr
==========================================================================*/
/*!
@brief
  Check for empty mac address

@return
  boolean

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
boolean check_non_empty_mac_addr(uint8 *mac )
{
  memset(mac_addr_string,0,QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01);
  snprintf(mac_addr_string,QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01,"%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
  if ( strncmp(mac_addr_string,MAC_NULL_STR,QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01) == 0 )
     return false;
  else
    return true;
}

/*===========================================================================
  FUNCTION find_record_delete_ip
==========================================================================*/
/*!
@brief
  Search for a DHCP record based on IP and delete it from /etc/dhcp_hosts

@return
  boolean

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
boolean find_record_delete_ip(uint32_t client_reserved_ip)
{
  in_addr addr;
  char temp[DHCP_RECORD_LENGTH];
  boolean match_result = false;
  FILE *tmpfile = NULL;

  addr.s_addr = htonl(client_reserved_ip);
  tmpfile = fopen("/var/tmp_file","wb");
  if ( NULL == tmpfile)
  {
    LOG_MSG_ERROR("\nUnable to open /var/tmp_file file!!",0,0,0);
    return false;
  }
  while (fgets(temp,DHCP_RECORD_LENGTH,dhcp_hosts_fp)!= NULL )
  {
    if (( strstr(temp,inet_ntoa(addr)))!= NULL)
    {
      LOG_MSG_INFO1("IP-address: %s found!!\n",inet_ntoa(addr),0,0);
      match_result = true;
    }
    else
    {
      fputs(temp,tmpfile);
    }
  }
  if (match_result == false)
  {
    LOG_MSG_INFO1("IP-address: %s NOT found!!\n",inet_ntoa(addr),0,0);
  }
  fclose(tmpfile);
  fclose(dhcp_hosts_fp);
  sprintf(temp,"%s%s","mv /var/tmp_file ",DHCP_HOSTS_FILE);
  ds_system_call(temp,strlen(temp));
  return true;
}

/*===========================================================================
  FUNCTION sync_dhcp_hosts
==========================================================================*/
/*!
@brief
  Sync /etc/dhcp-hosts with the current DHCP reservation recordds
  on boot or when SetLANConfig API is called

@return
  boolean

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::sync_dhcp_hosts(void)
{
  in_addr addr;
  int i=0, j=0;
  qcmap_msgr_dhcp_reservation_v01 *dhcp_reserv_record = NULL;
  boolean mac_addr_non_empty = true;
  uint32_t client_ip;
  uint32 num_records = 0;
  /*-------------------------------------------------------------------------*/


  num_records = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].num_dhcp_reservation_records;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].num_dhcp_reservation_records = 0;
  for ( i = 0; i < num_records; i++ )
  {
    dhcp_reserv_record = &this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_reservation_records[i];
    client_ip = dhcp_reserv_record->client_reserved_ip;
    addr.s_addr = htonl(client_ip);
    if ( ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_start_address <=
                                                                           client_ip ) &&
         ( (this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_end_address - 1)   >=
                                                                           client_ip ) )
    {
      /*store the record*/
      memcpy(&this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_reservation_records[j++],\
             dhcp_reserv_record,sizeof(qcmap_msgr_dhcp_reservation_v01) );
      this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].num_dhcp_reservation_records++;

      /*sync /etc/dhcp-hosts file*/
      if ( dhcp_reserv_record->enable_reservation )
      {
        if ( dhcp_reserv_record->client_device_name[0] != '\0' )
        {
          fprintf(dhcp_hosts_fp,"%s,",dhcp_reserv_record->client_device_name);
        }
        /* mac address is not provided for USB client*/
        mac_addr_non_empty = check_non_empty_mac_addr(dhcp_reserv_record->client_mac_addr);
        if ( mac_addr_non_empty )
        {
          fprintf(dhcp_hosts_fp,"%s,",mac_addr_string);
        }
        fprintf(dhcp_hosts_fp,"%s\n",inet_ntoa(addr));
      }
    }
  }
}

/*===========================================================================
  FUNCTION QCMAP_ConnectionManager_callback
==========================================================================*/
/*!
@brief
  Handles the mobile ap events and invokes the callback function
  for the respective indications to be sent.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static void QCMAP_ConnectionManager_callback
(
  int               handle,               /* MobileAP Application id */
  qcmap_cm_event_e  event,                /* Type of MobileAP Event  */
  void             *qcmap_msgr_cb_user_data, /* Call back user data     */
  dsi_ce_reason_t   *callend_reason
)
{
  QCMAP_ConnectionManager *manager;
  qmi_qcmap_msgr_status_cb_data *cbPtr;
  qmi_qcmap_msgr_softap_handle_type *softApHandle = (qmi_qcmap_msgr_softap_handle_type *) qcmap_msgr_cb_user_data;
  if (softApHandle != NULL)
  {
    cbPtr = (qmi_qcmap_msgr_status_cb_data *) softApHandle->cb_ptr;
    manager = softApHandle->Mgr;
  } else {
     return;
  }

  qcmap_msgr_wwan_call_end_type_enum_v01 call_end_type = QCMAP_MSGR_WWAN_CALL_END_TYPE_INVALID_V01;
  int call_end_reason_code=0;
  int err_num=0;

  LOG_MSG_INFO1("QCMAP AP Handle %04X Event %04X\n", handle, event,0);

  /*If connect to backhaul failed then callend_reason would be filled
    with reason and call type, same is sent to clients in failure response*/
  if( callend_reason != NULL )
  {
    if( callend_reason->reason_type == DSI_CE_TYPE_MOBILE_IP )
      call_end_type = QCMAP_MSGR_WWAN_CALL_END_TYPE_MOBILE_IP_V01;
    else if( callend_reason->reason_type == DSI_CE_TYPE_INTERNAL )
      call_end_type = QCMAP_MSGR_WWAN_CALL_END_TYPE_INTERNAL_V01;
    else if( callend_reason->reason_type == DSI_CE_TYPE_CALL_MANAGER_DEFINED )
      call_end_type = QCMAP_MSGR_WWAN_CALL_END_TYPE_CALL_MANAGER_DEFINED_V01;
    else if( callend_reason->reason_type == DSI_CE_TYPE_3GPP_SPEC_DEFINED )
      call_end_type = QCMAP_MSGR_WWAN_CALL_END_TYPE_3GPP_SPEC_DEFINED_V01;
    else if( callend_reason->reason_type == DSI_CE_TYPE_PPP )
      call_end_type = QCMAP_MSGR_WWAN_CALL_END_TYPE_PPP_V01;
    else if( callend_reason->reason_type == DSI_CE_TYPE_EHRPD )
      call_end_type = QCMAP_MSGR_WWAN_CALL_END_TYPE_EHRPD_V01;
    else if( callend_reason->reason_type == DSI_CE_TYPE_IPV6 )
      call_end_type = QCMAP_MSGR_WWAN_CALL_END_TYPE_IPV6_V01;
    call_end_reason_code = callend_reason->reason_code;
  }

  switch ( event )
  {
  case QCMAP_CM_EVENT_ENABLED:
    LOG_MSG_INFO1("QCMAP_CM_EVENT_ENABLED\n",0,0,0);
    if (cbPtr && cbPtr->qcmap_cb)
    {
      cbPtr->qcmap_cb(cbPtr, QCMAP_MSGR_MOBILE_AP_STATUS_CONNECTED_V01);
    }
    /* Block WWAN access for the clients. */
    manager->BlockIPv4WWANAccess();
    manager->BlockIPv6WWANAccess();
    break;
  case QCMAP_CM_EVENT_WAN_CONNECTING:
    LOG_MSG_INFO1("QCMAP_CM_EVENT_WAN_CONNECTING\n",0,0,0);
    break;
  case QCMAP_CM_EVENT_WAN_CONNECTING_FAIL:
    LOG_MSG_INFO1("QCMAP_CM_EVENT_WAN_CONNECTING_FAIL\n",0,0,0);
    if (cbPtr && cbPtr->wwan_cb)
    {
     cbPtr->wwan_cb(cbPtr, QCMAP_MSGR_WWAN_STATUS_CONNECTING_FAIL_V01,
                     call_end_type,call_end_reason_code);
    }
    break;
  case QCMAP_CM_EVENT_WAN_IPv6_CONNECTING_FAIL:
    LOG_MSG_INFO1("QCMAP_CM_EVENT_WAN_IPv6_CONNECTING_FAIL\n",0,0,0);
    if (cbPtr && cbPtr->wwan_cb)
    {
      cbPtr->wwan_cb(cbPtr, QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING_FAIL_V01,call_end_type,call_end_reason_code);
    }
    break;
  case QCMAP_CM_EVENT_WAN_CONNECTED:
    LOG_MSG_INFO1("QCMAP_CM_EVENT_WAN_CONNECTED\n",0,0,0);
    /* There is no need to modify the netmask of WWAN interface. If we modify the
     * the netmask existing routes installed by the embedded applications
     * will be deleted and their data transfer will be affected.
     */
    manager->AddWWANIPv4SIPServerInfo();
    /* Now time to enable NAT, ALGs etc., on A5*/
    if ( !manager->InStaMode() && !manager->cradle_backhaul_connected )
    {
      manager->EnableNATonA5();
      manager->EnableDNS();
    }
    ds_system_call("echo QCMAP:WAN connected v4 > /dev/kmsg",\
                   strlen("echo QCMAP:WAN connected v4 > /dev/kmsg"));
    if (cbPtr && cbPtr->wwan_cb)
    {
      cbPtr->wwan_cb(cbPtr, QCMAP_MSGR_WWAN_STATUS_CONNECTED_V01,call_end_type,call_end_reason_code);
    }
    break;
  case QCMAP_CM_EVENT_WAN_IPv6_CONNECTED:
    LOG_MSG_INFO1("QCMAP_CM_EVENT_WAN_IPv6_CONNECTED\n",0,0,0);
    if ( !manager->InStaMode() && !manager->cradle_backhaul_connected )
    {
      manager->EnableIPV6Forwarding();
      manager->EnableIPV6Firewall();
    }
    ds_system_call("echo QCMAP:WAN connected v6 > /dev/kmsg",\
                   strlen("echo QCMAP:WAN connected v6 > /dev/kmsg"));
    if (cbPtr && cbPtr->wwan_cb)
    {
      cbPtr->wwan_cb(cbPtr, QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTED_V01,
                     call_end_type,call_end_reason_code);
    }
    break;
  case QCMAP_CM_EVENT_WAN_DISCONNECTED:
    LOG_MSG_INFO1("QCMAP_CM_EVENT_WAN_DISCONNECTED\n",0,0,0);
    manager->DeleteWWANIPv4SIPServerInfo();
    /* 9x25, Disable NAT on A5 */
    if ( !manager->InStaMode() && !manager->cradle_backhaul_connected )
    {
      manager->DisableNATonA5();
      manager->FlushIPV4Firewall();
    }
    if (cbPtr && cbPtr->wwan_cb)
    {
      cbPtr->wwan_cb(cbPtr, QCMAP_MSGR_WWAN_STATUS_DISCONNECTED_V01,
                     call_end_type,call_end_reason_code);
    }
    break;
  case QCMAP_CM_EVENT_WAN_IPv6_DISCONNECTED:
    LOG_MSG_INFO1("QCMAP_CM_EVENT_WAN_IPv6_DISCONNECTED\n",0,0,0);
    if ( !manager->InStaMode() && !manager->cradle_backhaul_connected )
    {
      manager->DisableIPV6Forwarding(true);
      manager->FlushIPV6Firewall();
    }
    if (cbPtr && cbPtr->wwan_cb)
    {
      cbPtr->wwan_cb(cbPtr, QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTED_V01,call_end_type,call_end_reason_code);
    }
    break;
  case QCMAP_CM_EVENT_DISABLED:
    LOG_MSG_INFO1("QCMAP_CM_EVENT_DISABLED\n",0,0,0);
    if (cbPtr && cbPtr->qcmap_cb)
    {
      cbPtr->qcmap_cb(cbPtr, QCMAP_MSGR_MOBILE_AP_STATUS_DISCONNECTED_V01);
    }
    pthread_mutex_lock(&manager->cm_mutex);
    manager->qcmap_tear_down_in_progress = false;
    manager->DisableHandle();
    pthread_cond_signal(&manager->cm_cond);
    pthread_mutex_unlock(&manager->cm_mutex);
    LOG_MSG_INFO1("Teardown flag=false\n",0,0,0);
    break;
  case QCMAP_CM_EVENT_STA_CONNECTED:
    LOG_MSG_INFO1("QCMAP_CM_EVENT_STA_CONNECTED\n",0,0,0);
    if (manager->sta_connected_event_received == true)
    {
      LOG_MSG_INFO1("QCMAP_CM_EVENT_STA_CONNECTED - Duplicate Event \n",0,0,0);
      break;
    }
    if (manager->IsSSRInProgress())
    {
      LOG_MSG_INFO1("Ignoring QCMAP_CM_EVENT_STA_CONNECTED event -- since it recieved during SSR",0,0,0);
      break;
    }
    manager->sta_connected_event_received = true;
    if (manager->IsAPSTABridgeActivated())
    {
      manager->ProcessStaAssocForWLANBridge(qcmap_msgr_cb_user_data);
    }
    else
    {
      manager->ProcessStaAssoc(qcmap_msgr_cb_user_data);
    }
    break;
  case QCMAP_CM_EVENT_STA_DISCONNECTED:
    LOG_MSG_INFO1("QCMAP_CM_EVENT_STA_DISCONNECTED\n",0,0,0);
    if (manager->sta_connected_event_received == false)
    {
      LOG_MSG_INFO1("QCMAP_CM_EVENT_STA_DISCONNECTED - Duplicate Event \n",0,0,0);
      break;
    }
    if (manager->IsSSRInProgress())
    {
      LOG_MSG_INFO1("Ignoring QCMAP_CM_EVENT_STA_DISCONNECTED event -- since it recieved during SSR",0,0,0);
      break;
    }
    manager->sta_connected_event_received = false;
    if (manager->IsAPSTABridgeActivated())
    {
      manager->ProcessStaDisAssocForWLANBridge();
    }
    else
    {
      manager->ProcessStaDisAssoc();
    }
    if (cbPtr && cbPtr->sta_cb)
    {
        cbPtr->sta_cb(cbPtr, QCMAP_MSGR_STATION_MODE_DISCONNECTED_V01);
    }
    break;
  case QCMAP_CM_EVENT_STA_ASSOCIATION_FAIL:
    LOG_MSG_INFO1("QCMAP_CM_EVENT_STA_ASSOCIATION_FAIL\n",0,0,0);
    manager->ProcessStaAssocFail(qcmap_msgr_cb_user_data);
    break;
  case QCMAP_CM_EVENT_STA_DHCP_IP_ASSIGNMENT_FAIL:
    LOG_MSG_INFO1("QCMAP_CM_EVENT_STA_DHCP_IP_ASSIGNMENT_FAIL\n",0,0,0);
    manager->ProcessStaDHCPIPFail(qcmap_msgr_cb_user_data);
  case QCMAP_CM_EVENT_CRADLE_CONNECTED:
    LOG_MSG_INFO1("QCMAP_CM_EVENT_CRADLE_CONNECTED\n",0,0,0);
    if (cbPtr && cbPtr->cradle_cb)
    {
        cbPtr->cradle_cb(cbPtr, QCMAP_MSGR_CRADLE_CONNECTED_V01);
    }
    break;
  case QCMAP_CM_EVENT_CRADLE_DISCONNECTED:
    LOG_MSG_INFO1("QCMAP_CM_EVENT_CRADLE_DISCONNECTED\n",0,0,0);
    if (cbPtr && cbPtr->cradle_cb)
    {
        cbPtr->cradle_cb(cbPtr, QCMAP_MSGR_CRADLE_DISCONNECTED_V01);
    }
    break;
  default:
    LOG_MSG_INFO1("QCMAP_CM_EVENT_UNKNOWN %08X\n", event,0,0);
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
  in_addr addr, usb_ip_addr;
  struct sigevent sev;
  uint8_t i;
  qcmap_msgr_lan_config_v01 lan_config;
  qcmap_msgr_station_mode_config_v01 station_config;
  qmi_error_type_v01 qmi_err_num;
  char command[MAX_COMMAND_STR_LEN];

  /*-----------------------------------------------------------------------------*/
  this->qcmap_enable = true;
  /* Populate default Config */
  memset(&this->cfg, 0, sizeof(this->cfg));
  this->qcmap_cm_handle = 0;
  this->enable_dns = true;
  this->wifi_mode = QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01;
  this->sta_associated = false;
  this->sta_connected = false;
  this->prefix_delegation_activated = false;
  this->cradle_backhaul_connected = false;
  this->cradle_connect_in_progress = false;
  this->qcmap_tear_down_in_progress = false;
  this->bridge_inited = false;
  this->activate_wlan_in_progress = false;
  this->sta_iface_index = QCMAP_LAN_INVALID_IFACE_INDEX;
  this->ap_dev_num = QCMAP_LAN_INVALID_IFACE_INDEX;
  memset(this->ppp_ipv6_addr, 0, QCMAP_MSGR_IPV6_ADDR_LEN_V01);
  memset(this->ppp_ipv6_iid, 0, QCMAP_MSGR_IPV6_ADDR_LEN_V01);
  pthread_mutex_init(&this->cm_mutex, NULL);
  pthread_cond_init(&this->cm_cond, NULL);
  /* Initialize LAN Config. */
  this->cfg.lan_config.wlan_mode = QCMAP_MSGR_WLAN_MODE_AP_V01;
  this->cfg.lan_config.priv_client_restart_wlan = false;
  this->cfg.nat_config.num_firewall_entries=0;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].devmode = QCMAP_CM_DEVMODE_AP;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].devmode = QCMAP_CM_DEVMODE_AP;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].access_profile = QCMAP_MSGR_PROFILE_INTERNET_ONLY_V01;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].devmode = QCMAP_CM_DEVMODE_STA;
  bzero(&this->cfg.lan_config.usb_conf,sizeof(this->cfg.lan_config.usb_conf));

  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].ll_ip_addr = ntohl(inet_addr(AP_IFACE_LL_ADDR));
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].ll_ip_addr = ntohl(inet_addr(GUEST_AP_IFACE_LL_ADDR));
  this->cfg.lan_config.usb_conf.gateway_addr = ntohl(inet_addr(USB_IFACE_LL_ADDR));
  this->cfg.lan_config.ll_subnet_mask = ntohl(inet_addr(LL_SUBNET_MASK));

  // Set AP+STA Mode to default Router
  this->cfg.lan_config.ap_sta_bridge_mode = false;
  this->ap_sta_bridge_mode_activated = false;
  this->prev_ap_sta_bridge_mode = false;
  memset(&this->ap_sta_bridge, 0, sizeof(qcmap_ap_sta_bridge_t));
  this->sta_connected_event_received = false;
  this->revert_ap_sta_router_mode = false;

  // Initialize M-DNS State
  this->mdns_state = QCMAP_MSGR_MDNS_MODE_DOWN_V01;
  // Initialize UPnP and DLNA states
  this->upnp_state = QCMAP_MSGR_UPNP_MODE_DOWN_V01;
  this->dlna_state = QCMAP_MSGR_DLNA_MODE_DOWN_V01;

  // Initialize mobileap,wlan bootup values
  this->cfg.bootup_config.enable_wlan_at_bootup =false;
  this->cfg.bootup_config.enable_mobileap_at_bootup =false;

  // Intialize IPv6 Prefix to Null
  memset(&this->ipv6_prefix_info, 0, sizeof(qcmap_cm_nl_prefix_info_t));
  this->ipv6_prefix_based_rules_added = false;

  //Initialize Connected Devices Info to NULL
  memset (&this->conn_device_info, 0, sizeof(qcmap_cm_connected_info_t));
  memset (&this->addrList, NULL, sizeof(qcmap_addr_info_list_t));

  /* Kill dnsmasq which would get started on boot up */
  ds_system_call("killall -9 dnsmasq",
                 strlen("killall -9 dnsmasq"));

  this->cfg.nat_config.firewall_enabled = false;
  this->cfg.nat_config.firewall_pkts_allowed = false;
  this->cfg.nat_config.dmz_ip = 0;

  this->set_activate_lan_in_progress(false);
  memset(&this->prev_lan_config,0,sizeof(qcmap_msgr_lan_config_v01));
  memset(&this->prev_station_mode_config,0,sizeof(qcmap_msgr_station_mode_config_v01));
  this->prev_guest_profile = QCMAP_MSGR_ACCESS_PROFILE_MIN_ENUM_VAL_V01;

  /* Initialize default values. */
  this->cfg.lan_config.enable_ipv4 = 1;
  this->cfg.lan_config.enable_ipv6 = 1;

  /* Intialize ipv4 address info. */
  this->ipv4_public_ip = 0;
  this->ipv4_default_gw_addr = 0;

  /* Initialize default timeout values. */
  this->cfg.nat_config.nat_entry_generic_timeout = QCMAP_NAT_ENTRY_DEFAULT_GENERIC_TIMEOUT;
  this->cfg.nat_config.nat_entry_icmp_timeout = QCMAP_NAT_ENTRY_DEFAULT_ICMP_TIMEOUT;
  this->cfg.nat_config.nat_entry_tcp_established_timeout = QCMAP_NAT_ENTRY_DEFAULT_TCP_TIMEOUT;
  this->cfg.nat_config.nat_entry_udp_timeout = QCMAP_NAT_ENTRY_DEFAULT_UDP_TIMEOUT;

  /* Set the timeout values. */
  this->SetNatTimeoutOnA5(QCMAP_MSGR_NAT_TIMEOUT_GENERIC_V01,
                          this->cfg.nat_config.nat_entry_generic_timeout, &qmi_err_num);
  this->SetNatTimeoutOnA5(QCMAP_MSGR_NAT_TIMEOUT_ICMP_V01,
                          this->cfg.nat_config.nat_entry_icmp_timeout, &qmi_err_num);
  this->SetNatTimeoutOnA5(QCMAP_MSGR_NAT_TIMEOUT_TCP_ESTABLISHED_V01,
                          this->cfg.nat_config.nat_entry_tcp_established_timeout, &qmi_err_num);
  this->SetNatTimeoutOnA5(QCMAP_MSGR_NAT_TIMEOUT_UDP_V01,
                          this->cfg.nat_config.nat_entry_udp_timeout, &qmi_err_num);

/* Initialize alg enable disable mask to be zero */
  this->alg_enabled_disabled_mask = 0;

  /* Initialize Remote USB IP */
  this->cfg.lan_config.usb_conf.remote_usb_ip4_address = 0;

  /* Initialize USB MAC address*/
  memset (this->cfg.lan_config.usb_conf.usb_mac_addr,0,
          QCMAP_MSGR_MAC_ADDR_LEN_V01);

  /* Read the configuration. */
  for( i=0; i<QCMAP_MSGR_MAX_FIREWALL_ENTRIES_V01; i++)
  {
    this->cfg.nat_config.extd_firewall_entries[i]=NULL;
    bzero(this->cfg.nat_config.del_command[i],MAX_COMMAND_STR_LEN);
  }

  bzero(&command,MAX_COMMAND_STR_LEN);

  if (xml_path)
  {
    strlcpy(this->xml_path, xml_path, QCMAP_CM_MAX_FILE_LEN);
    /* Read configuration from XML file. */
    if (ReadConfigFromXML())
    {
       /* If the factory configuration file doesnot exist, copy the primary
          configuration file to factory configuration */
      if( !doesFileExist( FACTORY_CONF ))
      {
        snprintf( command, MAX_COMMAND_STR_LEN,
          "cp %s %s ",this->xml_path,FACTORY_CONF);
        ds_system_call(command, strlen(command));
      }
      /*Convert string to lower case, to keep the comparision uniform */
      strtolower(this->cfg.lan_config.module);

      /*Check for the sta interface to be used   */
      strncpy(this->cfg.lan_config.sta_interface,WLAN_IFACE,strlen(WLAN_IFACE));

      /* Initialize the time id  to be handled during auto connect */
      sev.sigev_notify = SIGEV_SIGNAL;
      sev.sigev_signo = SIG;
      sev.sigev_value.sival_int = AUTO_CONNECT_V4;
      if(timer_create(CLOCKID, &sev, &(this->cfg.wan_config.timerid_v4))==-1)
      perror("timer_create failed ");
      sev.sigev_value.sival_ptr =NULL;
      this->cfg.wan_config.auto_connect_timer_running_v4=false;
      this->cfg.wan_config.auto_timer_value_v4=AUTO_CONNECT_TIMER;


      sev.sigev_value.sival_int = AUTO_CONNECT_V6;
      if(timer_create(CLOCKID, &sev, &(this->cfg.wan_config.timerid_v6))==-1)
      perror("timer_create failed ");
      sev.sigev_value.sival_ptr =NULL;
      this->cfg.wan_config.auto_connect_timer_running_v6=false;
      this->cfg.wan_config.auto_timer_value_v6=AUTO_CONNECT_TIMER;

     sev.sigev_value.sival_int = STA_ASSOC_FAIL;
     if (timer_create(CLOCKID, &sev, &(this->cfg.lan_config.timerid_sta_assoc))== -1)
     {
        LOG_MSG_INFO1(" Failed to create WLAN STA ASSOC timer",0,0,0);
        perror("Failed to create WLAN STA ASSOC timer");
     }
     this->cfg.lan_config.ap_sta_bridge_sta_assoc_running=false;

     sev.sigev_value.sival_int = STA_DHCP_FAIL;
     if (timer_create(CLOCKID, &sev, &(this->cfg.lan_config.timerid_sta_dhcp))== -1)
     {
       LOG_MSG_INFO1(" Failed to create WLAN STA DHCP timer",0,0,0);
       perror("Failed to create WLAN STA DHCP timer");
     }
     this->cfg.lan_config.ap_sta_bridge_sta_dhcp_running=false;

      /* DHCP config checks. */
      if((( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                             a5_ip_addr &
             this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                         sub_net_mask) !=
          ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                      dhcp_start_address &
             this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                          sub_net_mask))||
          ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                     dhcp_start_address <=
            this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                             a5_ip_addr))
      {
        this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
              dhcp_start_address =
        this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                       a5_ip_addr+MAX_WIFI_CLIENTS;
        addr.s_addr =
        htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                      dhcp_start_address );
        LOG_MSG_INFO1("SSID1 DHCP Start addr in XML is invalid."
                     "Setting it to default value %s\n",inet_ntoa(addr),0,0);
      }
      if((( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                             a5_ip_addr &
            this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                          sub_net_mask) !=
          ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                       dhcp_end_address &
            this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                          sub_net_mask))||
          ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                       dhcp_end_address <
            this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                      dhcp_start_address))
      {
        this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                        dhcp_end_address =
        this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                     dhcp_start_address + MAX_WIFI_CLIENTS;
        addr.s_addr =
        htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                         dhcp_end_address );
        LOG_MSG_INFO1("SSID1 DHCP End addr in XML is invalid."
                     "Setting it to default value %s\n",inet_ntoa(addr),0,0);
      }

      /*If dhcp lease time configured is less then 120 seconds, set it to default lease time */
      if(atoi(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_lease_time) < MIN_DHCP_LEASE )
      {
        strlcpy(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_lease_time,DHCP_LEASE_TIME,strlen(DHCP_LEASE_TIME));
      }
      /* sync /etc/dhcp-hosts with the existing DHCP Reservation records*/
      dhcp_hosts_fp = fopen(DHCP_HOSTS_FILE, "wb");
      if (NULL == dhcp_hosts_fp)
      {
          /* File could not be opened for writing/append*/
          LOG_MSG_ERROR("\n Failed to create /etc/dhcp_hosts!!",0,0,0);
      }
      else
      {
        this->sync_dhcp_hosts();
        fclose(dhcp_hosts_fp);
      }

      /*Check for address conflict between ap, guest ap and static sta configuration */
      lan_config.gw_ip = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].a5_ip_addr;
      lan_config.netmask = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].sub_net_mask;
      lan_config.enable_dhcp = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].enable_dhcpd;

      if( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].enable_dhcpd )
      {
        lan_config.dhcp_config.dhcp_start_ip = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_start_address;
        lan_config.dhcp_config.dhcp_end_ip = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_end_address;
      }

      station_config.conn_type = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].conn_type;
      if( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].conn_type ==
          QCMAP_MSGR_STA_CONNECTION_STATIC_V01 )
      {
        station_config.static_ip_config = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].static_ip_config;
      }

      if( CheckforAddrConflict( &lan_config,&station_config ) )
      {
        /*
           If there address conflict set STA mode to dynamic.
         */
        this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].conn_type = QCMAP_MSGR_STA_CONNECTION_DYNAMIC_V01;
      }

      LOG_MSG_INFO1("QCMAP Read XML OK\n",0,0,0);

      if(this->cfg.nat_config.firewall_config_file)
      {
        LOG_MSG_INFO1("\n Firewall XML file is %s \n",this->cfg.nat_config.firewall_config_file,0,0);

        if(ReadConfigFromFirewallXML())
        {
          LOG_MSG_INFO1("\n QCMAP Firewall XML read OK \n",0,0,0);
        } else
	{
		  LOG_MSG_INFO1("\n QCMAP Firewall XML read failed \n",0,0,0);
        }
      }
      else
      {
        LOG_MSG_INFO1("\n No firewall xml mentioned \n",0,0,0);
        this->cfg.nat_config.firewall_config_file[0] = '\0';
      }

      /* Set the timeout values. */
      this->SetNatTimeoutOnA5(QCMAP_MSGR_NAT_TIMEOUT_GENERIC_V01,
                              this->cfg.nat_config.nat_entry_generic_timeout, &qmi_err_num);
      this->SetNatTimeoutOnA5(QCMAP_MSGR_NAT_TIMEOUT_ICMP_V01,
                              this->cfg.nat_config.nat_entry_icmp_timeout, &qmi_err_num);
      this->SetNatTimeoutOnA5(QCMAP_MSGR_NAT_TIMEOUT_TCP_ESTABLISHED_V01,
                              this->cfg.nat_config.nat_entry_tcp_established_timeout, &qmi_err_num);
      this->SetNatTimeoutOnA5(QCMAP_MSGR_NAT_TIMEOUT_UDP_V01,
                              this->cfg.nat_config.nat_entry_udp_timeout, &qmi_err_num);

       // Start M-DNS daemon if its is configured in the .xml file
       // Enabling M-DNS based on the XML configuration
       if ((this->mdns_state == QCMAP_MSGR_MDNS_MODE_DOWN_V01) && this->cfg.srvc_config.mdns_config) {
         qmi_error_type_v01   qmi_err_num;
         EnableMDNS(&qmi_err_num);
       }

      this->WriteConfigToXML();

      /* Create the bridge Interface if not already created. */
      if ( !this->bridge_inited )
      {
        this->InitBridge();
      }
      return;
    }
    /* Read Config from XML failed. Use defaults. */
    LOG_MSG_INFO1("QCMAP Read XML failed.\n",0,0,0);
  }
  else
  {
    this->xml_path[0] = '\0';
  }

  /* Fill in the default config */
  /* Default mode - AP */
  /* Interface 1 - AP - enabled */
  this->cfg.nat_config.nat_type = QCMAP_MSGR_NAT_SYMMETRIC_NAT_V01; /*Symmetric NAT as default */

  strlcpy(this->cfg.lan_config.module, MODULE_NAME, QCMAP_CM_MAX_FILE_LEN);
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].enable = true;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].devmode = QCMAP_CM_DEVMODE_AP;
  strlcpy(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].path_to_hostapd_conf, HOSTAPD_CFG_PATH, QCMAP_CM_MAX_FILE_LEN);
  strlcpy(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].path_to_sta_mode_hostapd_conf, STA_MODE_HOSTAPD_CFG_PATH, QCMAP_CM_MAX_FILE_LEN);
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].a5_ip_addr = ntohl(inet_addr(A5_IP_ADDR));
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].sub_net_mask = ntohl(inet_addr(A5_SUB_NET_MASK));
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].enable_dhcpd = true;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_start_address = ntohl(inet_addr(A5_IP_ADDR)) + 20;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_end_address = ntohl(inet_addr(A5_IP_ADDR)) + 20 + MAX_WIFI_CLIENTS;
  strlcpy(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_lease_time,DHCP_LEASE_TIME, QCMAP_CM_MAX_FILE_LEN);
  strlcpy(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].path_to_supplicant_conf, "NA", QCMAP_CM_MAX_FILE_LEN);
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].conn_type = QCMAP_MSGR_STA_CONNECTION_ENUM_MIN_ENUM_VAL_V01;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].static_ip_config.dns_addr = 0;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].static_ip_config.gw_ip = 0;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].static_ip_config.netmask = 0;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].static_ip_config.ip_addr = 0;
  strlcpy( this->cfg.nat_config.firewall_config_file,QCMAP_FIREWALL_CONFIG,
         ( QCMAP_CM_MAX_FILE_LEN));
  strlcpy( this->cfg.wan_config.eri_config_file,QCMAP_ERI_CONFIG,
         ( QCMAP_CM_MAX_FILE_LEN));
  this->cfg.nat_config.enable_ipsec_vpn_pass_through = true;
  this->cfg.nat_config.enable_l2tp_vpn_pass_through = true;
  this->cfg.nat_config.enable_pptp_vpn_pass_through = true;
  this->cfg.nat_config.enable_alg_mask |= QCMAP_MSGR_MASK_SIP_ALG_V01;
  this->cfg.wan_config.default_sip_server_info.sip_serv_type = QCMAP_CM_SIP_SERVER_FQDN;
  strlcpy( this->cfg.wan_config.default_sip_server_info.sip_serv_info.sip_fqdn,
           "abcd.com", ( QCMAP_MSGR_PCSCF_FQDN_MAX_LENGTH_V01 ));
  /* Interface 2 - AP - disabled */
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].enable = false;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].devmode = QCMAP_CM_DEVMODE_AP;
  strlcpy(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].path_to_hostapd_conf, HOSTAPD_CFG_PATH_SSID2, QCMAP_CM_MAX_FILE_LEN);
  strlcpy(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].path_to_supplicant_conf, "NA", QCMAP_CM_MAX_FILE_LEN);
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].conn_type = QCMAP_MSGR_STA_CONNECTION_ENUM_MIN_ENUM_VAL_V01;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].static_ip_config.dns_addr = 0;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].static_ip_config.gw_ip = 0;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].static_ip_config.netmask = 0;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].static_ip_config.ip_addr = 0;


  /* Interface 3 - STA - disabled */
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].enable = false;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].devmode = QCMAP_CM_DEVMODE_STA;
  strlcpy(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].path_to_hostapd_conf, "NA", QCMAP_CM_MAX_FILE_LEN);
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].a5_ip_addr = 0;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].sub_net_mask = 0;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].enable_dhcpd = false;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].dhcp_start_address = 0;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].dhcp_end_address = 0;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].dhcp_lease_time[0] = '\0';
  strlcpy(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].path_to_supplicant_conf, SUPPLICANT_CFG_PATH, QCMAP_CM_MAX_FILE_LEN);
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].conn_type = QCMAP_MSGR_STA_CONNECTION_DYNAMIC_V01;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].static_ip_config.dns_addr = 0;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].static_ip_config.gw_ip = 0;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].static_ip_config.netmask = 0;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].static_ip_config.ip_addr = 0;

  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = SIG;
  sev.sigev_value.sival_ptr = &(this->cfg.wan_config.timerid_v4);
  if(timer_create(CLOCKID, &sev, &(this->cfg.wan_config.timerid_v4))==-1)
  perror("timer_create failed ");
  this->cfg.wan_config.auto_timer_value_v4=AUTO_CONNECT_TIMER;
  sev.sigev_value.sival_ptr =NULL;
  this->cfg.wan_config.auto_connect_timer_running_v4=false;
  sev.sigev_value.sival_ptr = &(this->cfg.wan_config.timerid_v6);
  if(timer_create(CLOCKID, &sev, &(this->cfg.wan_config.timerid_v6))==-1)
  perror("timer_create failed ");
  this->cfg.wan_config.auto_timer_value_v6=AUTO_CONNECT_TIMER;
  this->cfg.wan_config.auto_connect_timer_running_v6=false;
  this->cfg.wan_config.tech = QCMAP_WAN_TECH_ANY;
  this->cfg.wan_config.profile_id.v4.umts_profile_index =
               this->cfg.wan_config.profile_id.v6.umts_profile_index = 0;
  this->cfg.wan_config.profile_id.v4.cdma_profile_index =
               this->cfg.wan_config.profile_id.v6.cdma_profile_index = 0;
  strlcpy(this->cfg.lan_config.sta_interface,WLAN_IFACE,strlen(WLAN_IFACE));
  /* Create the bridge Interface if not already created. */
  if ( !this->bridge_inited )
  {
    this->InitBridge();
  }
  this->WriteConfigToXML();

  /* If the factory configuration file doesnot exist, copy the primary
     configuration file to factory configuration */
  if( !doesFileExist( FACTORY_CONF ))
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "cp %s %s ",this->xml_path,FACTORY_CONF);
    ds_system_call(command, strlen(command));
  }
  return;
}

QCMAP_ConnectionManager::~QCMAP_ConnectionManager()
{
  int i=0;
  qmi_error_type_v01 qmi_err_num;
  QCMAP_CM_LOG_FUNC_ENTRY();
  pthread_mutex_destroy(&this->cm_mutex);
  pthread_cond_destroy(&this->cm_cond);

  /* Free memory allocated for firewall rules */
  if(this->cfg.nat_config.firewall_enabled)
  {
    for (i = 0; i < this->cfg.nat_config.num_firewall_entries; i++)
    {
        if(this->cfg.nat_config.extd_firewall_entries[i]!=NULL)
        {
         free(this->cfg.nat_config.extd_firewall_entries[i]);
         this->cfg.nat_config.extd_firewall_entries[i]=NULL;
        }
    }
  }

  if(this->upnp_state == QCMAP_MSGR_UPNP_MODE_UP_V01)
  {
    InterimDisableUPNP(&qmi_err_num);
  }

  if(this->dlna_state == QCMAP_MSGR_DLNA_MODE_UP_V01)
  {
    InterimDisableDLNA(&qmi_err_num);
  }

  if(this->mdns_state == QCMAP_MSGR_MDNS_MODE_UP_V01)
  {
    DisableMDNS(&qmi_err_num);
  }
  /* Delete the Bridge Interface. */
  this->DelBridge();
}

/*===========================================================================
  FUNCTION CleanIPtables
==========================================================================*/
/*!
@brief
  Clean IPTables NAT, Filter, Mangle and Raw tables

@parameters
   void

@return
   void

@note

 - Dependencies
 - None

 - Side Effects
 - None
*/
/*=========================================================================*/
static void CleanIPtables(void)
{
  char command[MAX_COMMAND_STR_LEN];
  FILE *access_fd = NULL;
  LOG_MSG_INFO1("CleanIPtables()",0,0,0);

  access_fd = fopen(QCMAP_MULTIPLE_SYSTEM_CALL_FILE,"w");
  /* File could not be opened for writing/append*/
  if (access_fd == NULL)
  {
     LOG_MSG_ERROR("Cannot open %s file for writing",QCMAP_MULTIPLE_SYSTEM_CALL_FILE,0,0);
     return;
  }
  fprintf( access_fd,"#!/bin/sh \n");

  fprintf( access_fd,"iptables -t nat --flush \n");
  fprintf( access_fd,"iptables -t filter --flush \n");
  fprintf( access_fd,"iptables -t mangle --flush \n");
  fprintf( access_fd,"iptables -t raw --flush \n");
  fclose(access_fd);

  /* Changing permissions */
  snprintf( command, MAX_COMMAND_STR_LEN,
            "chmod 777 %s",QCMAP_MULTIPLE_SYSTEM_CALL_FILE);
  ds_system_call( command, strlen(command));
  snprintf( command, MAX_COMMAND_STR_LEN,QCMAP_MULTIPLE_SYSTEM_CALL_FILE);
  ds_system_call( command, strlen(command));
  snprintf( command, MAX_COMMAND_STR_LEN,
            "rm -f %s",QCMAP_MULTIPLE_SYSTEM_CALL_FILE);
  ds_system_call( command, strlen(command));
}

/*===========================================================================
 FUNCTION CleanEbtables
==========================================================================*/
/*!
@brief
  Clean EbTables NAT, Filter and broute tables

@parameters
  void

@return
  void

@note

 - Dependencies
 - None

 - Side Effects
 - None
*/
/*=========================================================================*/
static void CleanEbtables(void)
{
  char command[MAX_COMMAND_STR_LEN];
  FILE *access_fd = NULL;
  LOG_MSG_INFO1("CleanEbtables()",0,0,0);

  access_fd = fopen(QCMAP_MULTIPLE_SYSTEM_CALL_FILE,"w");
  /* File could not be opened for writing/append*/
  if (access_fd == NULL)
  {
     LOG_MSG_ERROR("Cannot open %s file for writing",QCMAP_MULTIPLE_SYSTEM_CALL_FILE,0,0);
     return;
  }
  fprintf( access_fd,"#!/bin/sh \n");

  fprintf( access_fd,"ebtables -t nat --flush \n");
  fprintf( access_fd,"ebtables -t filter --flush \n");
  fprintf( access_fd,"ebtables -t broute --flush \n");
  fclose(access_fd);

  /* Changing permissions */
  snprintf( command, MAX_COMMAND_STR_LEN,
            "chmod 777 %s",QCMAP_MULTIPLE_SYSTEM_CALL_FILE);
  ds_system_call( command, strlen(command));
  snprintf( command, MAX_COMMAND_STR_LEN,QCMAP_MULTIPLE_SYSTEM_CALL_FILE);
  ds_system_call( command, strlen(command));
  snprintf( command, MAX_COMMAND_STR_LEN,
            "rm -f %s",QCMAP_MULTIPLE_SYSTEM_CALL_FILE);
  ds_system_call( command, strlen(command));
}

/*===========================================================================
  FUNCTION Enable
==========================================================================*/
/*!
@brief
  Enables MobileAP based on the configuration.
  As a part of enable mobileap, registers mobileap event callback.
  It will bring up RmNet between Q6 and A5.
  If AutoConnect is enabled then initiates connect to backhaul.
  If station mode is configured then enables it.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::Enable(int *handle, void *cb_user_data, qmi_error_type_v01 *qmi_err_num)
{
  int qcmap_cm_errno;
  int rc = 0;
  struct timespec ts;

  ds_system_call("echo QCMAP:Enable mobileap > /dev/kmsg",
          strlen("echo QCMAP:Enable mobileap > /dev/kmsg"));
  QCMAP_CM_LOG_FUNC_ENTRY();

  ((qmi_qcmap_msgr_softap_handle_type *)cb_user_data)->Mgr = this;

  this->qcmap_cm_handle = *handle = qcmap_cm_enable(&this->cfg, QCMAP_ConnectionManager_callback, cb_user_data, &qcmap_cm_errno, qmi_err_num);


  if (this->qcmap_cm_handle <= 0)
  {
    LOG_MSG_ERROR("QCMAP Connection failed \n",0,0,0);
    return false;
  }

  LOG_MSG_INFO1("QCMAP Enabled this->qcmap_cm_handle=%x \n",this->qcmap_cm_handle,0,0);

  ds_system_call("echo QCMAP:Enable mobileap done > /dev/kmsg",
          strlen("echo QCMAP:Enable mobileap done > /dev/kmsg"));
  return true;
}


/*===========================================================================
  FUNCTION Disable
==========================================================================*/
/*!
@brief
  Disable Mobile ap will bring down the backhaul and wlan.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::Disable(int *err_num, qmi_error_type_v01 *qmi_err_num)
{
  int ret = 0, qcmap_cm_errno;

  LOG_MSG_INFO1("QCMAP_ConnectionManager::Disable",0,0,0);
  QCMAP_CM_LOG_FUNC_ENTRY();
  boolean retval;
  if (!this->qcmap_enable)
  {
    /* QCMAP is not enabled */
    LOG_MSG_ERROR("QCMAP not enabled\n",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    return false;
  }

  this->qcmap_tear_down_in_progress = true;
  pthread_cond_signal(&this->cm_cond);

  ret = qcmap_cm_disable(this->qcmap_cm_handle, &qcmap_cm_errno, qmi_err_num);

  if (ret < 0)
  {
    LOG_MSG_ERROR("QCMAP Disable fails: %d\n", qcmap_cm_errno,0,0);
    retval = false;
  }
  else
  {
    *err_num = qcmap_cm_errno;
    LOG_MSG_ERROR("QCMAP Disable in progress\n",0,0,0);
    retval = true;
  }

  if(this->wifi_mode != QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01)
  DisableWLAN(qmi_err_num);

  if (this->cradle_backhaul_connected)
    DisableCradle(qmi_err_num);

  return retval;
}

/*===========================================================================
  FUNCTION DisableHandle
==========================================================================*/
/*!
@brief
 Initialize mobileap handle to zero.

@return
  None.

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::DisableHandle()
{
  char command[MAX_COMMAND_STR_LEN];

  this->qcmap_cm_handle = 0;

  /* Flush the forward chain. This is needed to remove rule added
  * to block WWAN traffic.
  */
  snprintf( command, MAX_COMMAND_STR_LEN, "iptables --flush FORWARD");
  ds_system_call(command, strlen(command));

  snprintf( command, MAX_COMMAND_STR_LEN, "ip6tables --flush FORWARD");
  ds_system_call(command, strlen(command));

}

/*===========================================================================
  FUNCTION EnableIPV4
==========================================================================*/
/*!
@brief
  Enables IPV4 backhaul Functionality. If autoconnect is enabled,
  triggers the backhaul to get the IPv4 address.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::EnableIPV4(qmi_error_type_v01 *qmi_err_num)
{
  int ret = 0;
  struct timespec ts;
  boolean retval = true;

  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Cant continue if MobileAP is not enabled! */
  if (!this->qcmap_enable)
  {
    LOG_MSG_ERROR("Error: QCMAP not enabled.\n",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    return false;
  }

  if (this->cfg.lan_config.enable_ipv4)
  {
    /* QCMAP IPV4 is already enabled */
    LOG_MSG_ERROR("QCMAP IPV4 already enabled\n",0,0,0);
    return true;
  }

  /* Enable IPV4 so that backhaul will succeed. */
  this->cfg.lan_config.enable_ipv4 = true;

  /* Check for auto-connect and bring-up backhaul. */
  ret = qcmap_cm_enable_ipv4(this->qcmap_cm_handle, qmi_err_num);
  if ( ret < 0 )
  {
    LOG_MSG_ERROR("QCMAP IPV4 enable failed: %d\n", qmi_err_num, 0, 0);
    /* Restore the state. */
    this->cfg.lan_config.enable_ipv4 = false;
    retval = false;
  }

  if ( retval == true)
  {
    if ( this->sta_connected  ||
         (this->cfg.cradle_config.cradle_mode == QCMAP_MSGR_CRADLE_WAN_ROUTER_V01 &&
          this->cradle_backhaul_connected))
    {
      this->EnableNATonA5();
    }
    LOG_MSG_INFO1("QCMAP IPV4 Enabled\n",0,0,0);
    /* Save the Configuration. */
    this->WriteConfigToXML();
  }

  return retval;
}


/*===========================================================================
  FUNCTION DisableIPV4
==========================================================================*/
/*!
@brief
  Disables IPv4 Functionality.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::DisableIPV4(qmi_error_type_v01 *qmi_err_num)
{
  int ret = 0, qcmap_cm_errno = QCMAP_CM_ENOERROR;
  boolean retval = true;

  QCMAP_CM_LOG_FUNC_ENTRY();

  if (!this->qcmap_enable)
  {
    /* QCMAP is not enabled */
    LOG_MSG_ERROR("QCMAP not enabled\n",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    return false;
  }

  if (!this->cfg.lan_config.enable_ipv4)
  {
    /* QCMAP IPV4 is not enabled */
    LOG_MSG_ERROR("QCMAP IPV4 not enabled\n",0,0,0);
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    return true;
  }

  /* The logic here is to disconnect the existing V4 backhaul and set the
   * enable_ipv4 flag to false. This is required even when we are in STA mode.
   * Reason being if we don't bring down the call now and when we switch from
   * WLAN backhaul goes down we will still have IPV4 WWAN backhaul which should
   * not be the case.
   */

  /* Disconnect Backhaul. */
  ret = qcmap_cm_disable_ipv4(this->qcmap_cm_handle, &qcmap_cm_errno, qmi_err_num);

  if (ret == QCMAP_CM_SUCCESS ||
      (ret == QCMAP_CM_ERROR && qcmap_cm_errno == QCMAP_CM_EWOULDBLOCK))
  {
    /* we just disable the IPV4 forwarding. So that none of the clients
     * will be able to access IPV4 backhaul. This is needed for both
     * station mode and regular mode as when WWAN backhaul cb is
     * hit the enable_ipv4 will not be set and so below functionality
     * will not be executed.
     */
    this->DisableNATonA5();
    this->FlushIPV4Firewall();
    this->cfg.lan_config.enable_ipv4 = false;
    LOG_MSG_INFO1("QCMAP IPV4 Disabled\n",0,0,0);
    /* Save the Configuration. */
    this->WriteConfigToXML();
    return true;
  }
  else
  {
    LOG_MSG_INFO1( "QCMAP IPV4 Disable Fail %d: %d\n",
                  qcmap_cm_errno, qmi_err_num, 0);
    return false;
  }

}
/*===========================================================================
  FUNCTION IsInterfaceEnabled
==========================================================================*/
/*!
@brief
 This function returns if the interface is enabled.
@return
None
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
int8_t IsInterfaceEnabled(char *interface)
{
  struct ifreq ifr;
  int sock;
  sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sock < 0)
  {
    LOG_MSG_ERROR("%s: Unable to  socket %d", __func__, errno, 0);
    return -1;
  }
  memset(&ifr, 0, sizeof(ifr));
  strcpy(ifr.ifr_name, interface);
  if (ioctl(sock,SIOCGIFINDEX,&ifr) < 0)
  {
    return -1;
  }
  LOG_MSG_INFO1( "QCMAP ifr.ifr_ifindex =%d ",ifr.ifr_ifindex,0,0);
  close(sock);
  return ifr.ifr_ifindex;
}

/*===========================================================================
  FUNCTION InitBridge
==========================================================================*/
/*!
@brief
Initialize bridge interface.
@return
None
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::InitBridge()
{
  char command[MAX_COMMAND_STR_LEN];
  struct in_addr addr;
  char a5_ip[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char gw_ip[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char netmask[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  qmi_error_type_v01 qmi_err_num;
  int retry_count = 0;
  int ret;

  /* Create the Bridge Interface. */
  snprintf(command, MAX_COMMAND_STR_LEN, "brctl addbr %s", BRIDGE_IFACE);
  ds_system_call(command, strlen(command));

  /* Bring up and configure the bridge interface. */
  /* Configure Static IP*/
  if (this->IsAPSTABridgeActivated() == false)
  {
    addr.s_addr = htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].a5_ip_addr);
    strlcpy(a5_ip, inet_ntoa(addr), 16);
    addr.s_addr = htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].sub_net_mask);
    strlcpy(netmask, inet_ntoa(addr), 16);
    snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig %s %s netmask %s up", BRIDGE_IFACE, a5_ip, netmask);
  }
  else /*Get IP via DHCP*/
  {
    snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig %s up", BRIDGE_IFACE);
  }
  ds_system_call(command, strlen(command));

  //Enable the bridge forwarding for Ipv6
  snprintf(command, MAX_COMMAND_STR_LEN,
           "echo 1 > /proc/sys/net/ipv6/conf/bridge0/forwarding");
  ds_system_call(command, strlen(command));

  /*Check for rndis interface is enabled, if enabled bind it to bridge, so that dhcp request
    message are honoured earliest*/
  ret = IsInterfaceEnabled("rndis0");
  if( ret > 0 )
  {
    LOG_MSG_INFO1("rndis is enabled ",0,0,0);
    snprintf(command, MAX_COMMAND_STR_LEN, "brctl addif %s rndis0",
             BRIDGE_IFACE);
    ds_system_call(command, strlen(command));
  }
  else if ( ret == 0)
  {
    LOG_MSG_INFO1("rndis is disabled",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Unable to read rndis interface status",0,0,0);
  }

  /*Check for ecm interface is enabled, if enabled bind it to bridge, so that dhcp request
    message are honoured earliest*/
  ret = IsInterfaceEnabled("ecm0");
  if( ret > 0 && this->cfg.cradle_config.cradle_mode == QCMAP_MSGR_CRADLE_DISABLED_V01)
  {
    snprintf(command, MAX_COMMAND_STR_LEN, "brctl addif %s ecm0",
             BRIDGE_IFACE);
    ds_system_call(command, strlen(command));
  }
  else if ( ret == 0)
  {
    LOG_MSG_INFO1("ecm is disabled",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Unable to read ecm interface status",0,0,0);
  }
  /* Enable Proxy ARP. */
  snprintf( command, MAX_COMMAND_STR_LEN,
            "echo 1 > /proc/sys/net/ipv4/conf/%s/proxy_arp", BRIDGE_IFACE );
  ds_system_call( command, strlen(command));

/*-------------------------------------------------------------------------
  Delete the DNS masq leases which were allocated earlier. This is
  required because
  - the dnsmasq lease file stores the MAC address to IP address mapping
    and does not allocate the stored IP addresses to DHCP requests
    coming in with different MAC address
  - but with every reboot in case of USB the host uses a different MAC address
  - thus in order to assign the one constant IP address to the USB tethered
    device we delete this mapping.
 --------------------------------------------------------------------------*/
  snprintf(command, MAX_COMMAND_STR_LEN,
           "rm /var/lib/misc/dnsmasq.leases");
  ds_system_call(command, strlen(command));

  /* Restart DHCPD */
  StopDHCPD();
  if (this->IsAPSTABridgeActivated() == false)
  {
    StartDHCPD();
  }

  if (this->IsAPSTABridgeActivated() == false)
  {
    ds_system_call("echo QCMAP:DHCP server started > /dev/kmsg",
            strlen("echo QCMAP:DHCP server started > /dev/kmsg"));
  }

  /* Bring up rndis and ecm interfaces up on bootup,so that it improves ip
     address assignment time during bootup */
  snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig rndis0 up");
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig ecm0 up");
  ds_system_call(command, strlen(command));

  if ( (qcmap_cm_get_ipv6_state(&qmi_err_num) == QCMAP_CM_V6_WAN_CONNECTED) &&
       (this->IsAPSTABridgeActivated() == false) )
  {
    /* Just Restart RADISH. */
    this->StopRadish();
    this->StartRadish();
  }

  this->bridge_inited = TRUE;

  // Update Prev LAN Config with the currrent config with which the bridge is created
  prev_lan_config.gw_ip = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].a5_ip_addr;
  prev_lan_config.netmask = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].sub_net_mask;
  prev_lan_config.enable_dhcp = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].enable_dhcpd;
  prev_lan_config.dhcp_config.dhcp_start_ip = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_start_address;
  prev_lan_config.dhcp_config.dhcp_end_ip = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_end_address;
  prev_lan_config.dhcp_config.lease_time = (uint32)atoi(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_lease_time);

  /* Create a socket over the bridge interface, to send RA's for prefix deperecation. */
  while(retry_count < QCMAP_BRIDGE_MAX_RETRY )
  {
    this->bridge_sock = this->CreateRawSocket(BRIDGE_IFACE);
    if ( this->bridge_sock != QCMAP_CM_ERROR)
    {
      break;
    }
    LOG_MSG_ERROR("Error: Bridge Sock cannot be created: %d. retry=%d \n", errno, retry_count, 0);
    usleep(QCMAP_BRIDGE_MAX_TIMEOUT_MS);
    retry_count++;
  }

  if( retry_count == QCMAP_BRIDGE_MAX_RETRY )
  {
    LOG_MSG_ERROR("Reached maximum retry attempts for CreateRawSocket %d", retry_count, 0, 0);
  }

  snprintf(command, MAX_COMMAND_STR_LEN,
           "echo 1 > /sys/devices/virtual/net/%s/bridge/multicast_querier",BRIDGE_IFACE);
  ds_system_call(command, strlen(command));

  if (this->IsAPSTABridgeActivated() == false)
  {
    if(this->cfg.srvc_config.dlna_config)
    {
      if(!EnableDLNA(&qmi_err_num))
      {
        LOG_MSG_ERROR("Cannot enable DLNA, error: %d.", qmi_err_num, 0, 0);
      }
    }

    if(this->cfg.srvc_config.upnp_config)
    {
      if(!EnableUPNP(&qmi_err_num))
      {
        LOG_MSG_ERROR("Cannot enable UPnP, error: %d.", qmi_err_num, 0, 0);
      }
    }
  }
}

/*===========================================================================
  FUNCTION DelBridge
==========================================================================*/
/*!
@brief
Deletes the bridge interface.
@return
None
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::DelBridge()
{
  char command[MAX_COMMAND_STR_LEN];

  /* Flush the BROUTING chain. */
  snprintf(command, MAX_COMMAND_STR_LEN,
           "ebtables -t broute -D BROUTING -F");
  ds_system_call(command, strlen(command));

  /* Flush the Filter chain. */
  snprintf(command, MAX_COMMAND_STR_LEN,
           "ebtables -F");
  ds_system_call(command, strlen(command));

/*-------------------------------------------------------------------------
  Delete the DNS masq leases which were allocated earlier. This is
  required because
  - the dnsmasq lease file stores the MAC address to IP address mapping
    and does not allocate the stored IP addresses to DHCP requests
    coming in with different MAC address
  - but with every reboot in case of USB the host uses a different MAC address
  - thus in order to assign the one constant IP address to the USB tethered
    device we delete this mapping.
 --------------------------------------------------------------------------*/
  snprintf(command, MAX_COMMAND_STR_LEN,
           "rm /var/lib/misc/dnsmasq.leases");
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
           "echo 0 > /sys/devices/virtual/net/%s/bridge/multicast_querier",BRIDGE_IFACE);
  ds_system_call(command, strlen(command));

  /* Stop DHCP */
  StopDHCPD();
 /* Bring down the bridge interface. */
  snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig %s down", BRIDGE_IFACE);
  ds_system_call(command, strlen(command));

  /* Deletes the Bridge Interface. */
  snprintf(command, MAX_COMMAND_STR_LEN, "brctl delbr %s", BRIDGE_IFACE);
  ds_system_call(command, strlen(command));

  this->bridge_inited = false;

  if ( this->bridge_sock >=0 )
  {
    close(this->bridge_sock);
  }
}

int QCMAP_ConnectionManager::CreateRawSocket(char *dev_name)
{
  int sock;
  /* According to RFC Hop Limit must be set to 255 in order
   * to protect against off-link packets.
   */
  int hop_limit = 255;

  /* Create socket and set required options */
  sock = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
  if (sock < 0)
  {
    LOG_MSG_ERROR("%s: Unable to create socket %d", __func__, errno, 0);
    goto error;
  }

  if (setsockopt(sock,
                 IPPROTO_IPV6,
                 IPV6_MULTICAST_HOPS,
                 (char *)&hop_limit,
                 sizeof(hop_limit)) < 0)
  {
    LOG_MSG_ERROR("%s: Set sockopt failed %d", __func__, errno, 0);
    goto error;
  }

  if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, dev_name, strlen(dev_name)+1) < 0)
  {
    LOG_MSG_ERROR("%s: Unable to bind socket to interface %s", __func__, dev_name, 0);
    goto error;
  }

  return sock;

error:
  if (sock >=0)
  {
    close(sock);
  }
  return -1;
}

void QCMAP_ConnectionManager::DeleteDelegatedPrefix(uint8_t *ipv6_addr, qmi_error_type_v01 *qmi_err_num){
  struct in6_addr addr6;
  char straddr[INET6_ADDRSTRLEN];
  struct ps_in6_addr null_ipv6_address;
  memset(&null_ipv6_address, 0, sizeof(struct ps_in6_addr));

  if (memcmp (ipv6_addr, &null_ipv6_address, sizeof(struct in6_addr)))
  {
    LOG_MSG_ERROR("No IPv6 address to remove", *qmi_err_num, 0, 0);
    return;
  }

  memcpy(addr6.s6_addr, ipv6_addr, sizeof(addr6.s6_addr));
  ds_log_med("Remove IPv6 address: %s", inet_ntop(AF_INET6, &addr6, straddr, INET6_ADDRSTRLEN));

  if (prefix_delegation_activated &&
      !qcmap_cm_delete_ipv6_delegated_prefix(this->qcmap_cm_handle,
                                             true,
                                             ipv6_addr,
                                             qmi_err_num))
  {
    LOG_MSG_ERROR("Error: Unable flush prefix's %d", *qmi_err_num, 0, 0);
  }

  return;
}

boolean QCMAP_ConnectionManager::DeprecateClientRA(void){
  int                         i;
  ds_dll_el_t                 *node, *node2;
  qcmap_cm_nl_prefix_info_t   ipv6_prefix_info;
  qmi_error_type_v01          qmi_err_num;
  qcmap_cm_client_data_info_t* connectedDevicesList = NULL;
  qcmap_addr_info_list_t* addrList = &(this->addrList);
  struct ps_in6_addr null_ipv6_address;

  if(addrList->addrListHead == NULL)
  {
    LOG_MSG_ERROR("Error: List is empty", 0, 0, 0);
    return false;
  }

  node = addrList->addrListHead->next;

  if (node == NULL)
  {
    LOG_MSG_ERROR("Error: first element in list is empty", 0, 0, 0);
    return false;
  }

  memset(&null_ipv6_address, 0, sizeof(struct ps_in6_addr));

  while (node != NULL)
  {
    connectedDevicesList = ( qcmap_cm_client_data_info_t* )node->data;

    if (connectedDevicesList == NULL)
    {
      LOG_MSG_ERROR("connectedDevicesList is NULL", 0, 0, 0);
      node = node->next;
      continue;
    }

    if (memcmp (connectedDevicesList->ip_v6_addr, &null_ipv6_address, sizeof(struct in6_addr)))
    {

      memset(&ipv6_prefix_info, 0, sizeof(qcmap_cm_nl_prefix_info_t));
      ipv6_prefix_info.cache_info.ifa_prefered = IPV6_DEFAULT_PREFERED_LIFETIME;
      ipv6_prefix_info.cache_info.ifa_valid = IPV6_DEFAULT_VALID_LIFETIME;
      ipv6_prefix_info.prefix_len =  (unsigned char) IPV6_MIN_PREFIX_LENGTH;
      memcpy(((struct sockaddr_in6 *)&(ipv6_prefix_info.prefix_addr))->sin6_addr.s6_addr,
             connectedDevicesList->ip_v6_addr, sizeof(connectedDevicesList->ip_v6_addr));

      if(this->UpdatePrefix(&ipv6_prefix_info, true, true, connectedDevicesList->link_local_v6_addr) == -1)
      {
        LOG_MSG_ERROR("Error: Unable to deprecate prefix", 0, 0, 0);
        return false;
      }
    }
    else
      LOG_MSG_ERROR("Node does not have global v6 addr, skipping", 0, 0, 0);

    node = node->next;
  }

  //Remove Delegated Prefix's
  if (!qcmap_cm_delete_ipv6_delegated_prefix(this->qcmap_cm_handle, false, NULL, &qmi_err_num))
  {
    LOG_MSG_ERROR("Error: Unable flush prefix's %d", qmi_err_num, 0, 0);
  }

  return true;
}

boolean QCMAP_ConnectionManager::SendRS(void)
{
  ds_dll_el_t         *node;
  int                 qcmap_cm_error, qmi_error;
  int                 i, status, frame_length, sd, bytes, num_entries;
  struct ip6_hdr      iphdr;
  struct icmp6_hdr    icmphdr;
  struct sockaddr_ll  device;
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  qcmap_cm_client_data_info_t* connectedDevicesList = NULL;
  qcmap_addr_info_list_t* addrList = &(this->addrList);
  struct sockaddr_in6 dst_addr;
  struct ifreq ifr;
  uint8 *frame = NULL;

  if (!IsRadishRunning())
  {
    LOG_MSG_ERROR("Cannot send RS without Radish running",0,0,0);
    return false;
  }

  if(addrList->addrListHead != NULL && addrList->addrListHead->next != NULL)
  {
    if (qmi_error != QCMAP_CM_SUCCESS)
    {
      LOG_MSG_ERROR("Unable to get gateway address",0,0,0);
      return false;
    }

    //Get number of connected clients
    num_entries = this->conn_device_info.numOfNodes;
    node = addrList->addrListHead->next;

    //set mutlicast router destination
    memset(&dst_addr, 0, sizeof(dst_addr));
    inet_pton(AF_INET6, ROUTER_MULTICAST, &dst_addr.sin6_addr);
    dst_addr.sin6_family = AF_INET6;

    //open RAW socket for sending RS packet
    if ((sd = socket (AF_INET6, SOCK_RAW, IPPROTO_RAW)) < 0) {
        LOG_MSG_ERROR ("socket() failed to get socket descriptor for using ioctl() %d ", perror, 0, 0);
        return false;
      }

    //Send the RS packet to the rmnet iface
    memset(devname, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);
    if ( qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_MSGR_IP_FAMILY_V6_V01, devname, &qcmap_cm_error) !=
         QCMAP_CM_SUCCESS )
    {
      LOG_MSG_ERROR("Couldn't get ipv6 rmnet name. error %d\n", qcmap_cm_error,0,0);
      close(sd);
      return false;
    }

    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), devname);
    setsockopt(sd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr));

    //Main loop, send RS for each connected client
    for (i = 0; i < num_entries; i++)
    {
      frame = (uint8 *) malloc(FRAME_SIZE);
      if (!frame)
      {
        LOG_MSG_ERROR("No memory %d ", errno, 0, 0);
        close(sd);
        return;
      }

      connectedDevicesList = ( qcmap_cm_client_data_info_t* )node->data;

      // IPv6 header
      memset(&iphdr, 0, sizeof(iphdr));

      // IPv6 version (4 bits), Traffic class (8 bits), Flow label (20 bits)
      iphdr.ip6_flow = htonl ((MAC_SIZE << 28) | (0 << 20) | 0);

      // Payload length (16 bits): ICMP header + ICMP data
      iphdr.ip6_plen = htons (ICMP_HDRLEN);
      iphdr.ip6_nxt = IPPROTO_ICMPV6; //next header
      iphdr.ip6_hops = MAX_HOPS;

      // Source IPv6 address
      memcpy(iphdr.ip6_src.s6_addr, connectedDevicesList->link_local_v6_addr, sizeof(uint8_t)*QCMAP_MSGR_IPV6_ADDR_LEN_V01);

      // Destination IPv6 address
      if ((status = inet_pton (AF_INET6, ROUTER_MULTICAST, &(iphdr.ip6_dst))) != 1) {
        LOG_MSG_ERROR ("inet_pton failed", 0, 0, 0);
        free(frame);
        close(sd);
        return false;
      }

      // ICMP header
      memset(&icmphdr, 0, sizeof(icmphdr));

      icmphdr.icmp6_type = ND_ROUTER_SOLICIT;
      icmphdr.icmp6_code = MESSAGE_CODE;
      icmphdr.icmp6_cksum = ICMP_HEADER_CHKSUM;
      icmphdr.icmp6_cksum = icmp6_checksum(&iphdr, (struct icmp6_header*)&icmphdr, ICMP_HDRLEN);

      frame_length = IP6_HDRLEN + ICMP_HDRLEN;

      LOG_MSG_INFO1("Entered send_fake_rs at %d", __LINE__, 0, 0);

      // IPv6 header
      memcpy (frame, &iphdr, IP6_HDRLEN * sizeof (uint8));

      // ICMP header
      memcpy (frame + IP6_HDRLEN, &icmphdr, ICMP_HDRLEN * sizeof (uint8));

      // Send ethernet frame to socket.
      if ((bytes = sendto (sd, frame, frame_length * sizeof(uint8), 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr))) <= 0) {
          LOG_MSG_ERROR("Send to failed: %d ", errno, 0, 0);
          free(frame);
          close(sd);
          return false;
      }
      LOG_MSG_INFO1("Sent num_bytes: %d frame length: %d ", bytes, frame_length, 0);

      // Free allocated memory.
      free(frame);
      node = node->next;
    }
	close(sd);
  }
  else
  {
    LOG_MSG_ERROR("No connected devices", 0,0,0);
  }

  return true;
}

int QCMAP_ConnectionManager::UpdatePrefix(qcmap_cm_nl_prefix_info_t *ipv6_prefix_info, boolean deprecate, boolean send_ra, uint8_t *dest_v6_ip)
{
  struct icmp6_ra_pkt ra_pkt;
  struct nd_router_advert *router_adv = NULL;
  struct nd_opt_prefix_info *prefix_info = NULL;
  struct sockaddr_in6 dst_addr;
  struct ps_in6_addr *prefix_ptr, *addr_ptr, *addr_ptr1;
  int ret = -1;
  char ipaddr[MAX_IPV6_PREFIX + 1];
  char command[MAX_COMMAND_STR_LEN];
  int ppp_sockfd = 0;
  boolean is_multicast = false;


  if ( ipv6_prefix_info == NULL )
  {
    LOG_MSG_ERROR("NULL Prefix Passed\n", 0, 0, 0);
    return ret;
  }

  if ( this->bridge_sock < 0 )
  {
    LOG_MSG_ERROR("Error: Bridge Sock is not created\n", 0, 0, 0);
    return ret;
  }

  memset(&dst_addr, 0, sizeof(dst_addr));
  memset(&ra_pkt,0,sizeof(ra_pkt));

  router_adv = &(ra_pkt.router_adv);

  /* Initialize RA Packet. */
  router_adv->nd_ra_type = ND_ROUTER_ADVERT;
  router_adv->nd_ra_code = 0;
  router_adv->nd_ra_cksum = 0;
  router_adv->nd_ra_curhoplimit = 64;
  router_adv->nd_ra_flags_reserved = deprecate ? 0 : ND_RA_FLAG_OTHER;
  router_adv->nd_ra_router_lifetime = deprecate ? 0 : htons(0xffff);
  router_adv->nd_ra_reachable = 0;
  router_adv->nd_ra_retransmit = 0;

  prefix_info = &(ra_pkt.prefix_info);

  prefix_info->nd_opt_pi_type = ND_OPT_PREFIX_INFORMATION;
  prefix_info->nd_opt_pi_len = sizeof(struct nd_opt_prefix_info)/sizeof(uint64);
  prefix_info->nd_opt_pi_prefix_len = ipv6_prefix_info->prefix_len;
  prefix_info->nd_opt_pi_flags_reserved = ND_OPT_PI_FLAG_ONLINK | ND_OPT_PI_FLAG_AUTO;
  prefix_info->nd_opt_pi_valid_time = deprecate ? htonl(2) : htonl(ipv6_prefix_info->cache_info.ifa_valid);
  prefix_info->nd_opt_pi_preferred_time = deprecate ? htonl(1) : htonl(ipv6_prefix_info->cache_info.ifa_prefered);
  prefix_info->nd_opt_pi_reserved2 = 0;

  memcpy(prefix_info->nd_opt_pi_prefix.s6_addr,
         ((struct sockaddr_in6 *)&(ipv6_prefix_info->prefix_addr))->sin6_addr.s6_addr,
         sizeof(prefix_info->nd_opt_pi_prefix.s6_addr));

  /* Zero the bits which are greater than Prefix length. */
  prefix_ptr = (struct ps_in6_addr *)&prefix_info->nd_opt_pi_prefix;
  if ( prefix_info->nd_opt_pi_prefix_len <= IPV6_MIN_PREFIX_LENGTH )
  {
    /* Zero out the non prefix bits. */
    prefix_ptr->ps_s6_addr64[0] >>= (IPV6_MIN_PREFIX_LENGTH - prefix_info->nd_opt_pi_prefix_len);
    prefix_ptr->ps_s6_addr64[0] <<= (IPV6_MIN_PREFIX_LENGTH - prefix_info->nd_opt_pi_prefix_len);
    /* Zero out the IID part */
    prefix_ptr->ps_s6_addr64[1] = 0;
  }
  else
  {
    /* No need to touch the first 64 bits. Just Modify the  */
    /* Zero out the non prefix bits. */
    prefix_ptr->ps_s6_addr64[1] >>= (IPV6_MAX_PREFIX_LENGTH - prefix_info->nd_opt_pi_prefix_len);
    prefix_ptr->ps_s6_addr64[1] <<= (IPV6_MAX_PREFIX_LENGTH - prefix_info->nd_opt_pi_prefix_len);
  }

  if (dest_v6_ip != NULL)
    memcpy(dst_addr.sin6_addr.s6_addr, dest_v6_ip, sizeof(dst_addr.sin6_addr.s6_addr));
  else
  {
    inet_pton(AF_INET6, "ff02::1", &dst_addr.sin6_addr);
    is_multicast = true;
  }

  dst_addr.sin6_family = AF_INET6;

  if (send_ra)
  {
    LOG_MSG_ERROR("Forwarding RA\n", 0, 0, 0);
    if ((ret = sendto(this->bridge_sock,
               &ra_pkt,
               sizeof(ra_pkt),
               0,
               (struct sockaddr *)&dst_addr,
               sizeof(dst_addr))) < 0)
    {
      LOG_MSG_ERROR("Error: Cannot send RA: %d.\n", errno, 0, 0);
    }

    /* Check if PPP enabled and send the prefix. */
    addr_ptr = (struct ps_in6_addr *)dest_v6_ip;
    addr_ptr1 = (struct ps_in6_addr *)this->ppp_ipv6_iid;
    if ( this->cfg.lan_config.usb_conf.ppp_enabled &&
         ((is_multicast == true) ||
         !memcmp(&addr_ptr->ps_s6_addr64[1],
                 &addr_ptr1->ps_s6_addr64[1],
                 sizeof(uint64))))
    {
      LOG_MSG_ERROR("Forwarding RA to PPP interface\n", 0, 0, 0);
      /* Send the prefix over the PPP interface. */
      ppp_sockfd = this->CreateRawSocket(PPP_IFACE);

      if ( ppp_sockfd < 0 )
      {
        LOG_MSG_ERROR("Error: PPP Sock is not created\n", 0, 0, 0);
        return ret;
      }

      if ((ret = sendto(ppp_sockfd,
                        &ra_pkt,
                        sizeof(ra_pkt),
                        0,
                        (struct sockaddr *)&dst_addr,
                        sizeof(dst_addr))) < 0)
      {
        LOG_MSG_ERROR("Error: Cannot send RA to PPP interface: %d.\n", errno, 0, 0);
      }

      close(ppp_sockfd);
    }

  }
  /* Based on the deprecate flag decide whether to add/delete the prefix based route. */
  memset(ipaddr, 0, MAX_IPV6_PREFIX + 1);
  inet_ntop(AF_INET6, (struct in6_addr *)prefix_ptr, ipaddr, MAX_IPV6_PREFIX);
  if ( deprecate )
  {
    LOG_MSG_INFO1("\n Delete the prefix based route for inet6\n", 0, 0, 0);
    snprintf(command, MAX_COMMAND_STR_LEN,
             "ip -6 route del %s/%d", ipaddr, prefix_info->nd_opt_pi_prefix_len);
    ds_system_call(command, strlen(command));
  }
  else
  {
    LOG_MSG_INFO1("\n First Delete the prefix based route for WWAN Interface\n", 0, 0, 0);
    snprintf(command, MAX_COMMAND_STR_LEN,
             "ip -6 route del %s/%d", ipaddr, prefix_info->nd_opt_pi_prefix_len);
    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("\n Add the prefix based route for Bridge Interface\n", 0, 0, 0);
    snprintf(command, MAX_COMMAND_STR_LEN,
             "ip -6 route add %s/%d dev %s", ipaddr, prefix_info->nd_opt_pi_prefix_len, BRIDGE_IFACE);
    ds_system_call(command, strlen(command));
  }

  return ret;

}

/*===========================================================================
  FUNCTION EnableIPV6
==========================================================================*/
/*!
@brief
  Enables IPV6 Functionality. If autoconnect is enabled,
  triggers the backhaul to get the IPv6 address.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::EnableIPV6(qmi_error_type_v01 *qmi_err_num)
{
  int ret = 0;
  struct timespec ts;
  boolean retval = true;

  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Cant continue if MobileAP is not enabled! */
  if (!this->qcmap_enable)
  {
    LOG_MSG_ERROR("Error: QCMAP not enabled.\n",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    return false;
  }

  if (this->cfg.lan_config.enable_ipv6)
  {
    /* QCMAP IPV6 is already enabled */
    LOG_MSG_ERROR("QCMAP IPV6 already enabled\n",0,0,0);
    return true;
  }

  /* Enable IPV6 so that backhaul will succeed. */
  this->cfg.lan_config.enable_ipv6 = true;

  /* Check for auto-connect and bring-up backhaul. */
  ret = qcmap_cm_enable_ipv6(this->qcmap_cm_handle, qmi_err_num);
  if ( ret < 0 )
  {
    LOG_MSG_ERROR("QCMAP IPV6 Enable Failed: %d\n", qmi_err_num, 0, 0);
    /* Restore the state. */
    this->cfg.lan_config.enable_ipv6 = false;
    retval = false;
  }

  if ( retval == true )
  {
    if (this->sta_connected ||
        (this->cfg.cradle_config.cradle_mode == QCMAP_MSGR_CRADLE_WAN_ROUTER_V01 &&
         this->cradle_backhaul_connected))
    {
      this->EnableIPV6Forwarding();
      this->EnableIPV6Firewall();
    }
    LOG_MSG_INFO1("QCMAP IPV6 Enabled\n", 0, 0, 0);
    /* Save the Configuration. */
    this->WriteConfigToXML();
  }

  return retval;
}


/*===========================================================================
  FUNCTION DisableIPV6
==========================================================================*/
/*!
@brief
  Disables IPv6 Functionality.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::DisableIPV6(qmi_error_type_v01 *qmi_err_num)
{
  int ret = 0, qcmap_cm_errno = QCMAP_CM_ENOERROR;

  QCMAP_CM_LOG_FUNC_ENTRY();

  if (!this->qcmap_enable)
  {
    /* QCMAP is not enabled */
    LOG_MSG_ERROR("QCMAP not enabled\n",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    return false;
  }

  if (!this->cfg.lan_config.enable_ipv6)
  {
    /* QCMAP IPV6 is not enabled */
    LOG_MSG_ERROR("QCMAP IPV6 not enabled\n",0,0,0);
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    return true;
  }

  /* The logic here is to disconnect the existing V6 backhaul and set the
   * enable_ipv6 flag to false. This is required even when we are in STA mode.
   * Reason being if we don't bring down the call now and when we switch from
   * WLAN backhaul goes down we will still have IPV6 WWAN backhaul which should
   * not be the case.
   */
  /* Disconnect Backhaul. */
  ret = qcmap_cm_disable_ipv6(this->qcmap_cm_handle, &qcmap_cm_errno, qmi_err_num);

  if (ret == QCMAP_CM_SUCCESS ||
      (ret == QCMAP_CM_ERROR && qcmap_cm_errno == QCMAP_CM_EWOULDBLOCK))
  {
    /* we just disable the IPV6 forwarding. So that none of the clients
     * will be able to access IPV6 backhaul. This is needed for both
     * station mode and regular mode as when WWAN backhaul cb is
     * hit the enable_ipv6 will not be set and so below functionality
     * will not be executed.
     */
    this->DisableIPV6Forwarding(true);
    this->FlushIPV6Firewall();
    this->cfg.lan_config.enable_ipv6 = false;
    LOG_MSG_INFO1("QCMAP IPV6 Disabled\n", 0, 0, 0);
    /* Save the Configuration. */
    this->WriteConfigToXML();
    return true;
  }
  else
  {
    LOG_MSG_INFO1( "QCMAP IPV6 Disable Fail %d: %d\n",
                  qcmap_cm_errno, qmi_err_num, 0);
    return false;
  }

}

/*===========================================================================
  FUNCTION CheckHostapd
==========================================================================*/
/*!
@brief
  Return true if the hostapd is running.

@parameters
  char *process

@return
  true  - process killed
  flase - process running after waiting 1 second

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean CheckHostapd(int omit_pid)
{
  char process[MAX_COMMAND_STR_LEN];
  int i = 0;
  FILE *cmd;
  char pid_s[MAX_CMD_SIZE];
  int pid;

  memset(process, 0, MAX_COMMAND_STR_LEN);

  if ( omit_pid == 0 )
  {
    snprintf(process, MAX_COMMAND_STR_LEN, "pidof hostapd");
  }
  else
  {
    snprintf(process, MAX_COMMAND_STR_LEN, "pidof -o %d hostapd", omit_pid);
  }
  cmd = popen(process, "r");
  pid = 0;
  memset(pid_s, 0, MAX_CMD_SIZE);
  fgets(pid_s, MAX_CMD_SIZE, cmd);
  pid = atoi(pid_s);
  LOG_MSG_INFO1("\n pid =%d \n", pid, 0, 0);
  pclose(cmd);
  /* If pid is zero we break from while*/
  if(pid != 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}

/*===========================================================================
  FUNCTION IsHostapdRunning
==========================================================================*/
/*!
@brief
  Checks if the hostapd is running.

@parameters
  void

@return
  true  - on Success
  false - on Failure
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::IsHostapdRunning(int iface)
{
  FILE *fp = NULL;
  int ap_pid = 0;

  switch ( iface )
  {
    case QCMAP_MSGR_INTF_AP_INDEX:
     {
        LOG_MSG_INFO1("Primary AP mode %d",iface,0,0);
        /* Only instance of hostapd will be running for the first time */
        if ( CheckHostapd(0) )
        {
          LOG_MSG_INFO1("Hostapd is Running", 0, 0, 0);
          return true;
        }
        else
        {
          return false;
        }
     }
     break;
    case QCMAP_MSGR_INTF_GUEST_AP_INDEX:
     {
        LOG_MSG_INFO1("Guest AP mode %d",iface,0,0);
        /* Get the Primary AP PID. */
        fp = fopen(HOSTAPD_PID_FILE, "r");

        if ( fp == NULL )
        {
          LOG_MSG_ERROR("Error opening hostapd pid file: %d.\n", errno, 0, 0);
        }
        else if (fscanf(fp, "%d", &ap_pid) != 1)
        {
          LOG_MSG_ERROR("Error reading hostapd file: %d.\n", errno, 0, 0);
          ap_pid = 0;
          fclose(fp);
        }
        else
        {
          LOG_MSG_INFO1("Primary AP PID = %d",ap_pid,0,0);
          fclose(fp);
        }

        if ( ap_pid == 0 )
        {
          LOG_MSG_ERROR("Hostapd for AP is not yet started", 0, 0, 0);
        }

        if ( CheckHostapd(ap_pid) )
        {
          LOG_MSG_INFO1("Guest AP mode Hostapd is Running", 0, 0, 0);
          return true;
        }
        else
        {
         return false;
        }
     }
     break;
    default:
          LOG_MSG_ERROR("No Hostapd of this mode %d", iface, 0, 0);
     break;
  }
  return true;
}

/*===========================================================================
  FUNCTION  EnableWLAN
==========================================================================*/
/*!
@brief
  Enables the wlan based on the wlan chip and  mode configured.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::EnableWLAN( qmi_error_type_v01 *qmi_err_num,
                                             boolean enable_wlan_onbootup,
                                             boolean privileged_client )
{

  char devmode[16] = { '\0' };
  QCMAP_CM_LOG_FUNC_ENTRY();

  ds_system_call("echo QCMAP:Start WLAN Enable > /dev/kmsg",\
                  strlen("echo QCMAP:Start WLAN Enable > /dev/kmsg"));

  if( privileged_client &&
      !this->cfg.lan_config.priv_client_restart_wlan )
  {
    LOG_MSG_ERROR("\n Privileged can't enable wlan\n", 0, 0, 0);
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    return false;
  }

 /* If wlan is enabled during bootup, we don't need to check mobileap state*/
  if ( !enable_wlan_onbootup && this->qcmap_cm_handle <= 0 )
  {
    LOG_MSG_ERROR("\n MobileAP not enabled\n", 0, 0, 0);
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    return false;
  }

  if ( this->wifi_mode >= QCMAP_MSGR_WLAN_MODE_AP_V01 &&
       this->wifi_mode <= QCMAP_MSGR_WLAN_MODE_AP_STA_V01 )
  {
    LOG_MSG_ERROR("\n WLAN is already enabled. Current Mode:- %d\n",
                   this->wifi_mode ,0, 0);
    return true;
  }


  /* STA mode is tied to mobile ap handle as all the NAT configuration is
   * tied to mobile ap handle. */
  if ( ( this->cfg.lan_config.wlan_mode == QCMAP_MSGR_WLAN_MODE_AP_STA_V01 ) &&
       ( this->qcmap_tear_down_in_progress )
     )
  {
    LOG_MSG_ERROR("Cannot enable STA mode when mobileap is in not enabled.", 0, 0, 0);
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    return false;
  }

  if (this->cfg.lan_config.ap_sta_bridge_mode &&
      this->cfg.lan_config.wlan_mode == QCMAP_MSGR_WLAN_MODE_AP_STA_V01 &&
      (this->cradle_backhaul_connected || this->cradle_connect_in_progress))
  {
    LOG_MSG_ERROR("Cannot enable AP-STA Bridge mode when in Cradle.", 0, 0, 0);
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    return false;
  }

  /* Intialize. */
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].enable = true;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].enable = false;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].enable = false;

  /* Set STA iface index in interface array in lan_cfg */
  this->sta_iface_index = QCMAP_MSGR_INTF_STATION_INDEX;
  /* Set AP device index. */
  this->ap_dev_num = 0;

  /* Verify that the devmode is supported. Allowed ones are:
     AP, AP+AP, AP+STA. */
  switch ( this->cfg.lan_config.wlan_mode )
  {
    case QCMAP_MSGR_WLAN_MODE_AP_V01:
    {
      this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].enable = true;
      strlcpy(devmode, "ap", sizeof(devmode));
      break;
    }
    case QCMAP_MSGR_WLAN_MODE_AP_AP_V01:
    {
      this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].enable = true;
      this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].enable = true;
      strlcpy(devmode, "ap,ap", sizeof(devmode));
      break;
    }
    case QCMAP_MSGR_WLAN_MODE_AP_STA_V01:
    {
      this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].enable = true;
      this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].enable = true;
      strlcpy(devmode, "sta,ap", sizeof(devmode));
      this->ap_dev_num = 1;
      /* Disable WLAN-IPA interface if AP+STA Bridge Configuration is Active */
      /* Ensure that we only move to bridge mode if revert flag is false*/
      if ((this->cfg.lan_config.ap_sta_bridge_mode) && (this->revert_ap_sta_router_mode == false))
      {
         ds_system_call("sed -i s/gIPAConfig=0x1f/gIPAConfig=0x00/w /lib/firmware/wlan/qca_cld/WCNSS_qcom_cfg.ini",
                         strlen("sed -i s/gIPAConfig=0x1f/gIPAConfig=0x00/w /lib/firmware/wlan/qca_cld/WCNSS_qcom_cfg.ini"));
         this->ap_sta_bridge_mode_activated = true;
      }
      break;
    }
    default:
    {
      /* Incorrect WLAN Mode. Restore it to default mode and continue. */
      LOG_MSG_ERROR("Incorrect WLAN Mode: %d, Start with default Mode.\n",
                    this->cfg.lan_config.wlan_mode, 0, 0);
      this->cfg.lan_config.wlan_mode = QCMAP_MSGR_WLAN_MODE_AP_V01;
      this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].enable = true;
      strlcpy(devmode, "ap", sizeof(devmode));
      break;
    }
  }

  /* Update Current WIFI mode. */
  this->wifi_mode = this->cfg.lan_config.wlan_mode;

  /* Reset the revert ap_sta router mode flag to ensure that next Enable/Disable/Bootup
     AP-STA is started with bridge mode*/
  this->revert_ap_sta_router_mode = false;

  LOG_MSG_INFO1( "ap_dev_num: %d\n",this->ap_dev_num, 0, 0);

  // Enable the wlan module
  EnableWLANModule(devmode,enable_wlan_onbootup);

  if(!this->activate_wlan_in_progress && !this->IsAPSTABridgeActivated())
  {
    if(this->cfg.srvc_config.dlna_config)
    {
      if(!EnableDLNA(qmi_err_num))
      {
        LOG_MSG_ERROR("Cannot enable DLNA, error: %d.", *qmi_err_num, 0, 0);
      }
    }

    if(this->cfg.srvc_config.upnp_config)
    {
      if(!EnableUPNP(qmi_err_num))
      {
        LOG_MSG_ERROR("Cannot enable UPnP, error: %d.", *qmi_err_num, 0, 0);
      }
    }
  }
  /* Reset priviledge client flag. Priviledge client needs to first disable
   * wlan, inorder to request enable wlan */
  if( privileged_client )
  {
    this->cfg.lan_config.priv_client_restart_wlan = false;
    LOG_MSG_INFO1("EnableWLAN is called from a privilaged client",0,0,0);
  }

  ds_system_call("echo QCMAP:WLAN Enabled > /dev/kmsg",\
                  strlen("echo QCMAP:WLAN Enabled > /dev/kmsg"));

  return true;
}

/*===========================================================================
  FUNCTION  EnableWLANModule
==========================================================================*/
/*!
@brief
  This function is invoked, to bringup the wlan module.
  This function will load the appropriate driver and brings up the wlan in
  the mode configured. Enables dhcpd service for the wlan interfaces.

@return
  void

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

void QCMAP_ConnectionManager::EnableWLANModule(char *wlanmode,boolean enable_wlan_onbootup)
{
  char command[MAX_COMMAND_STR_LEN];
  struct in_addr addr;
  char devmode[16] = { '\0' };
  char a5_ip[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char gw_ip[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char netmask[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char subnet[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  int s, i, qcmap_cm_error, dev;
  struct ifreq buffer;
  uint32_t time_out = (WIFI_DEV_INIT_DELAYS_MAX/WIFI_DEV_INIT_DELAY);
  int j = 0;
  int retry = 0;
  int ret = 0;

  strncpy(devmode,wlanmode,sizeof(devmode));

  /* This function stops all currently running HostAPD daemons.
   * Since hostapd is not running on bootup, to improve the wlan
   * service availablity, we are spending time stopping hostapd */
  if(!enable_wlan_onbootup)
  {
    StopHostAPD();
  }

  /* Enable IP forwarding */
  ds_system_call("echo 1 > /proc/sys/net/ipv4/ip_forward",
                 strlen("echo 1 > /proc/sys/net/ipv4/ip_forward"));
  /* Run the WLAN Script to load the WLAN driver. */
  snprintf(command, MAX_COMMAND_STR_LEN,"/etc/init.d/wlan start %s",
           devmode);
  LOG_MSG_INFO1("Bringup WLAN",0,0,0);

  ds_system_call(command, strlen(command));

  for ( i = 0, dev = 0; i < QCMAP_MAX_NUM_INTF; i++ )
  {
    if ( !this->cfg.lan_config.interface[i].enable )
      continue;
    snprintf(command, MAX_COMMAND_STR_LEN, "/proc/sys/net/ipv4/conf/wlan%d",dev++);
    while ( j++ < time_out )
    {
      std::ifstream ifile( command );
      if ( ifile )
      {
        break;
      }
      usleep( WIFI_DEV_INIT_DELAY );
    }
  }

  LOG_MSG_INFO1("Wlan card brought up in mode %d.\n", this->wifi_mode,0,0);
  /* Bring up interfaces.
     This part needs to be done only for AP interfaces.
     Proceed only upto STA iface index since STA is always last in the
     list (ap; ap,ap; ap,sta; ap,ap,sta supported). */
  for ( i=0, dev = this->ap_dev_num; i < this->sta_iface_index; i++, dev++ )
  {
    if ( !this->cfg.lan_config.interface[i].enable )
      break;
    /* Start the HostAP daemon. */
    StartHostAPD(i, dev);

    if(!IsHostapdRunning(i))
    {
       LOG_MSG_INFO1("hostapd is not running, restart hostapd", 0, 0, 0);
      /* In 11AC mode, hostapd started for guest AP mode is failing, because
         hostapd of AP mode is not completed its processing which involves
         some processing in wlan. So a delay of 500msec is required before
         starting hostapd for Guest ap. The delay allows
         hostapd of ap mode to be completed */
        usleep(QCMAP_HOSTAPD_DELAY_MS);
        StartHostAPD(i, dev);
    }
    /* Add the interfaces to the bridge. */
    snprintf(command, MAX_COMMAND_STR_LEN, "brctl addif %s wlan%d",
           BRIDGE_IFACE, dev);
    /* Associating wlan to bridge fails, if wlan interface is still in scan mode,
       hence retrying with a delay */
    retry = 0;
    while(retry < QCMAP_ASSOC_WLAN_TO_BRIDGE_MAX_RETRIES)
    {
      ret = system(command);
      if(ret < 0)
      {
        LOG_MSG_ERROR("system: bridge wlan association failed ", 0, 0, 0);
      }
      else
      {
        if( WIFEXITED(ret) && (WEXITSTATUS(ret) == 0))
        {
          LOG_MSG_INFO1("system: bridge wlan association passed ", 0, 0, 0);
          break;
        }
      }
      LOG_MSG_ERROR("system: Retry bridge wlan association", 0, 0, 0);
      usleep(QCMAP_ASSOC_WLAN_TO_BRIDGE_TIMEOUT_US);
      retry++;
    }
    /* Config WLAN interface IP */
    addr.s_addr = htonl(this->cfg.lan_config.interface[i].ll_ip_addr);
    strlcpy(a5_ip, inet_ntoa(addr), 16);
    addr.s_addr = htonl(this->cfg.lan_config.ll_subnet_mask);
    strlcpy(netmask, inet_ntoa(addr), 16);
    snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig wlan%d %s netmask %s up", dev, a5_ip, netmask);
    ds_system_call(command, strlen(command));
    addr.s_addr = htonl(this->cfg.lan_config.interface[i].ll_ip_addr & this->cfg.lan_config.ll_subnet_mask);
    strlcpy(subnet, inet_ntoa(addr), 16);
    snprintf(command, MAX_COMMAND_STR_LEN, "route del -net %s netmask %s dev wlan%d", subnet, netmask, dev);
    ds_system_call(command, strlen(command));
    /* First delete the link-local route. */
    snprintf(command, MAX_COMMAND_STR_LEN, "ip -6 route del fe80::/64 dev wlan%d",
           dev);
    ds_system_call(command, strlen(command));
  }

  /* Enable Station Mode. */
  this->EnableStaMode();

  /* Install access restrictions for the Second SSID if the profile is
     INTERNETONLY.*/
  if ( (this->wifi_mode == QCMAP_MSGR_WLAN_MODE_AP_AP_V01) &&
       (this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].access_profile == QCMAP_MSGR_PROFILE_INTERNET_ONLY_V01) )
  {
    /* Disable InterBSS in WLAN Driver.. */
    this->InstallGuestAPAccessRules();
  }

  // Store the current active configuration in the backup DS
  this->prev_guest_profile = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].access_profile;
  this->prev_station_mode_config.conn_type = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].conn_type;
  memcpy(&this->prev_station_mode_config.static_ip_config,&this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].static_ip_config,sizeof(qcmap_msgr_sta_static_ip_config_v01));
  this->prev_ap_sta_bridge_mode = this->cfg.lan_config.ap_sta_bridge_mode;

  /* Set IPv6 */
  if(this->cfg.lan_config.enable_ipv6)
  {
    snprintf(command, MAX_COMMAND_STR_LEN,
             "echo 2 > /proc/sys/net/ipv6/conf/%s/accept_ra", ECM_IFACE);
    ds_system_call(command, strlen(command));
  }
}

/*===========================================================================
  FUNCTION DeleteWLANConnectedDevicesClients
==========================================================================*/
/*!
@brief
  Delete all entries in Connected Devices module related to WLAN.

@return

@note

 - Dependencies
 - None

 - Side Effects
 - None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::DeleteWLANConnectedDevicesClients(qcmap_msgr_device_type_enum_v01 device_type)
{
  ds_dll_el_t * node = NULL;
  qcmap_cm_client_data_info_t* list_data = NULL;
  qcmap_addr_info_list_t* addrList = &(this->addrList);
  struct in_addr addr;
  char command[MAX_COMMAND_STR_LEN];
  qmi_error_type_v01 qmi_err_num;

  LOG_MSG_INFO1("DeleteWLANConnectedDevicesClients() with device_type %d",device_type,0,0);

  if(addrList->addrListHead == NULL)
  {
    LOG_MSG_INFO1("DeleteWLANConnectedDevicesClients - Head Null -> No entries to delete \n",0,0,0);
    return;
  }

  // Delete devices from the connected device list
  node = ds_dll_delete( addrList->addrListHead, &(addrList->addrListTail),
                        (void*)&device_type, qcmap_match_device_type);
  while(node)
  {
    list_data = (qcmap_cm_client_data_info_t*)node->data;
    // This case should not exist since we are getting the node based on node->data match in ds_dll_delete
    if (list_data == NULL)
    {
      LOG_MSG_ERROR("DeleteWLANConnectedDevicesClients - The device information node is NULL\n",0,0,0);
    }
    else
    {
      this->DeleteDelegatedPrefix(list_data->ip_v6_addr, &qmi_err_num);
      addr.s_addr = list_data->ip_addr;
      ds_log_med("DeleteWLANConnectedDevicesClients-- Deleting Client %s from device connected list with type %d\n",inet_ntoa(addr),device_type);
      /* Delete connection track entries for wlan client */
      if ( addr.s_addr )
      {
        snprintf(command,MAX_COMMAND_STR_LEN, "conntrack -D --orig-src %s",inet_ntoa(addr));
        ds_system_call(command, strlen(command));
        snprintf(command,MAX_COMMAND_STR_LEN, "conntrack -D --reply-src %s",inet_ntoa(addr));
        ds_system_call(command, strlen(command));
      }
      // Free the device information structure
      ds_free (list_data);
      list_data = NULL;
    }
    //Free the memory of the linked list node
    ds_dll_free(node);
    node = NULL;
    this->conn_device_info.numOfNodes--;
    node = ds_dll_delete( addrList->addrListHead, &(addrList->addrListTail),
                               (void*)&device_type, qcmap_match_device_type);
  }
}

/*===========================================================================
  FUNCTION DisableWLAN
==========================================================================*/
/*!
@brief
  Brings down the wlan and stops the dhcp server.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::DisableWLAN( qmi_error_type_v01 *qmi_err_num,
                                              boolean privileged_client )
{
  char command[MAX_COMMAND_STR_LEN];
  struct in_addr addr;
  char a5_ip[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char netmask[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  int qcmap_cm_error;
  qcmap_msgr_wlan_mode_enum_v01     prev_wifi_mode;
  qcmap_msgr_device_type_enum_v01 device_type = QCMAP_MSGR_DEVICE_TYPE_ENUM_MIN_ENUM_VAL_V01;
  ds_dll_el_t * node = NULL;
  ds_dll_el_t * nodeF = NULL;
  qcmap_cm_client_data_info_t* connectedDevicesList = NULL;

  if ( this->wifi_mode == QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01 )
  {
    LOG_MSG_ERROR("\n WLAN is already disabled.\n",0,0,0);
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    return true;
  }

  /* Stop services before bringing down bridge so byebye messages are sent */
  if( !(GetUSBEnable()) && !this->activate_wlan_in_progress)
  {
    if(this->cfg.srvc_config.upnp_config)
    {
      LOG_MSG_INFO1("Interim disable UPnP daemon", 0, 0, 0);
      InterimDisableUPNP(qmi_err_num);
    }
    if(this->cfg.srvc_config.dlna_config)
    {
      LOG_MSG_INFO1("Interim disable DLNA daemon", 0, 0, 0);
      InterimDisableDLNA(qmi_err_num);
    }
  }

  /* Priviledge client needs to disable wlan and only than, enable wlan is
   * permitted */
  if( privileged_client )
  {
    this->cfg.lan_config.priv_client_restart_wlan = true;
    /* Reset the revert ap_sta_router_mode flag to ensure the for SSR
       we revert back to AP-BRIDGE mode*/
    this->revert_ap_sta_router_mode = false;
    LOG_MSG_INFO1("DisableWLAN is called from a privilaged client",0,0,0);
  }

  /*Stop all instances of hostapd_cli */
  this->StopHostapdCli(QCMAP_MSGR_PRIMARY_AND_GUEST_AP_V01);

  /* Shut down interfaces.
     Proceed only upto STA iface index since STA is always last in the
     list (ap; ap,ap; ap,sta; ap,ap,sta supported).*/
  StopHostAPD();

  /* Disable station mode iface. */
  this->DisableStaMode();

  this->DisableWLANModule();

  /* AP-STA Bridge Mode cleanup*/
  if (this->IsAPSTABridgeActivated())
  {
     ds_system_call("sed -i s/gIPAConfig=0x00/gIPAConfig=0x1f/w /lib/firmware/wlan/qca_cld/WCNSS_qcom_cfg.ini",
                     strlen("sed -i s/gIPAConfig=0x00/gIPAConfig=0x1f/w /lib/firmware/wlan/qca_cld/WCNSS_qcom_cfg.ini"));
     CleanEbtables();
     CleanIPtables();
     /* Stopping all timers if they were not stopped */
     stop_wlan_sta_timer(STA_ASSOC_FAIL);
     stop_wlan_sta_timer(STA_DHCP_FAIL);
  }

  /* Delete all the WLAN clients from the Connected Devices List */
  if (this->wifi_mode == QCMAP_MSGR_WLAN_MODE_AP_V01)
  {
    device_type = QCMAP_MSGR_DEVICE_TYPE_PRIMARY_AP_V01;
    this->DeleteWLANConnectedDevicesClients(device_type);
  }
  else if (this->wifi_mode ==  QCMAP_MSGR_WLAN_MODE_AP_AP_V01)
  {
    device_type = QCMAP_MSGR_DEVICE_TYPE_PRIMARY_AP_V01;
    this->DeleteWLANConnectedDevicesClients(device_type);
    device_type = QCMAP_MSGR_DEVICE_TYPE_GUEST_AP_V01;
    this->DeleteWLANConnectedDevicesClients(device_type);
  }
  else if (this->wifi_mode == QCMAP_MSGR_WLAN_MODE_AP_STA_V01)
  {
    device_type = QCMAP_MSGR_DEVICE_TYPE_GUEST_AP_V01;
    this->DeleteWLANConnectedDevicesClients(device_type);
  }
  /* Save the previous wifi mode. */
  prev_wifi_mode = this->wifi_mode;

  /* Reset wifi mode */
  this->wifi_mode = QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01;
  /* Reset STA mode specific parameters */
  this->sta_connected = false;
  this->ap_sta_bridge_mode_activated = false;
  this->sta_iface_index = QCMAP_LAN_INVALID_IFACE_INDEX;

  /* Try switching to backhaul if previous mode is AP-STA. */
  if (!this->cradle_backhaul_connected)
  {
    if ( prev_wifi_mode == QCMAP_MSGR_WLAN_MODE_AP_STA_V01 )
    {
      // Clean up stale entries if previous mode was AP-STA, i.e. WLAN0 was backhaul
      this->FlushIPV4Firewall();
      this->FlushIPV6Firewall();
      if ( qcmap_cm_get_state(qmi_err_num) == QCMAP_CM_WAN_CONNECTED )
      {
        this->EnableNATonA5();
        this->EnableDNS();
      }
      if ( qcmap_cm_get_ipv6_state(qmi_err_num) == QCMAP_CM_V6_WAN_CONNECTED )
      {
        this->EnableIPV6Forwarding();
        this->EnableIPV6Firewall();
      }
    }
    else
    {
      if ( qcmap_cm_get_ipv6_state(qmi_err_num) == QCMAP_CM_V6_WAN_CONNECTED )
      {
        /* Just Restart RADISH. */
        this->StopRadish();
        this->StartRadish();
      }
    }
  }

  /* Set all the WLAN enabled boolean values to false */
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].enable = false;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].enable = false;
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].enable = false;

  return true;
}


/*===========================================================================
  FUNCTION DisableWLANModule
==========================================================================*/
/*!
@brief
  Brings down the for wlan chip  and
  deletes the related routing informations.

@return
  void

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::DisableWLANModule(void)
{
   char command[MAX_COMMAND_STR_LEN];
  struct in_addr addr;
  char a5_ip[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char netmask[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  int qcmap_cm_error;
  boolean usb_enable=false;

 /*-----------------------------------------------------------------------
    Delete iptables rules.
  -----------------------------------------------------------------------*/
  /* Delete access restrictions for the Second SSID if the profile is
     INTERNETONLY.*/
  if ( this->wifi_mode == QCMAP_MSGR_WLAN_MODE_AP_AP_V01 &&
       ((this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].access_profile == QCMAP_MSGR_PROFILE_INTERNET_ONLY_V01) ||
       (this->prev_guest_profile == QCMAP_MSGR_PROFILE_INTERNET_ONLY_V01)) )
  {
    this->DeleteGuestAPAccessRules();
  }

  /* Delete the interfaces from bridge. */
  snprintf(command, MAX_COMMAND_STR_LEN, "brctl delif %s wlan%d",
           BRIDGE_IFACE, this->ap_dev_num);
  ds_system_call(command, strlen(command));
  if ( this->wifi_mode == QCMAP_MSGR_WLAN_MODE_AP_AP_V01 )
  {
    snprintf(command, MAX_COMMAND_STR_LEN, "brctl delif %s wlan%d",
             BRIDGE_IFACE, this->ap_dev_num+1);
    ds_system_call(command, strlen(command));
  }

  /* If bridge mode is activated Delete sta interface from bridge */
  /* If USB was enabled restart USB*/
  /* Restart DHCP On bridge, assign IP Address to Bridge*/
  if (this->IsAPSTABridgeActivated())
  {
    snprintf(command, MAX_COMMAND_STR_LEN, "brctl delif %s %s",
             BRIDGE_IFACE, this->cfg.lan_config.sta_interface);
    ds_system_call(command, strlen(command));
    if ( GetUSBEnable() && !IsSSRInProgress())
    {
      usb_enable=true;
      ds_system_call("echo 0 > /sys/class/android_usb/android0/enable ",strlen("echo 0 > /sys/class/android_usb/android0/enable "));
    }

    // Assign IP to bridge
    snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig %s down", BRIDGE_IFACE);
    ds_system_call(command, strlen(command));
    addr.s_addr = htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].a5_ip_addr);
    strlcpy(a5_ip, inet_ntoa(addr), 16);
    addr.s_addr = htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].sub_net_mask);
    strlcpy(netmask, inet_ntoa(addr), 16);
    snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig %s %s netmask %s up", BRIDGE_IFACE, a5_ip, netmask);
    ds_system_call(command, strlen(command));
    // Restart DHCP Server
    snprintf(command, MAX_COMMAND_STR_LEN, "rm /var/lib/misc/dnsmasq.leases");
    ds_system_call(command, strlen(command));
    this->StopDHCPD();
    this->StartDHCPD();

    /* Start USB*/
    if (usb_enable)
      ds_system_call("echo 1 > /sys/class/android_usb/android0/enable ",strlen("echo 1 > /sys/class/android_usb/android0/enable "));
  }

  snprintf(command, MAX_COMMAND_STR_LEN,"/etc/init.d/wlan stop");
  LOG_MSG_INFO1("Bring down WLAN ",0,0,0);
  ds_system_call(command, strlen(command));

  return;
}

void QCMAP_ConnectionManager::GetIPV6PrefixInfo(char *devname,
                                                qcmap_cm_nl_prefix_info_t   *ipv6_prefix_info)
{
  struct sockaddr_nl  src_addr;
  struct sockaddr_nl dest_addr;
  struct iovec iov;
  struct sockaddr_nl sa;
  struct msghdr msg = { (void *)&sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
  struct nlmsghdr *nlh = NULL;
  struct sockaddr_in6 *sin6 = NULL;
  void *buf = NULL;
  qcmap_nl_getaddr_req_t req;
  int ret = -1;
  int sock_fd;
  unsigned iface_idx;
  unsigned int buflen = 0;
  ssize_t recvsize;


  if ((sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0)
  {
    LOG_MSG_ERROR("GetIPV6PrefixInfo: socket() failed: %d\n", errno, 0, 0);
    return -1;
  }

  /* Initialize the source address */
  memset(&src_addr, 0, sizeof(src_addr));

  src_addr.nl_family = AF_NETLINK;
  src_addr.nl_pid = 0;
  src_addr.nl_groups = 0; /* Interested in unicast messages */

  /* Bind the socket to our source address */
  if (bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0)
  {
    LOG_MSG_ERROR("GetIPV6PrefixInfo: bind() failed: %d\n", errno, 0, 0);
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
  iface_idx = IsInterfaceEnabled(devname);
  if (!iface_idx)
  {
    LOG_MSG_ERROR("Couldn't find interface %s Error:%d", devname, errno, 0);
    goto bail;
  }

  LOG_MSG_INFO1("Get Prefix Info for interface %s with ID: %d", devname, iface_idx, 0);

  if ( sendto(sock_fd, (void*) &req, sizeof(req), 0,
               (struct sockaddr*) &dest_addr, sizeof(dest_addr)) < 0)
  {
    LOG_MSG_ERROR("GetIPV6PrefixInfo: bind() failed: %d\n", errno, 0, 0);
    goto bail;
  }

/* Max size of the netlink response message */
#define QCMAP_NL_MAX_MSG_SIZE  (1024 * 4)
  /* Allocate and initialize buffer to read message */
  buf = calloc(1, NLMSG_SPACE(QCMAP_NL_MAX_MSG_SIZE));
  if (NULL == buf)
  {
    LOG_MSG_ERROR("GetIPV6PrefixInfo: memory alloc failure: %d\n", errno, 0, 0);
    goto bail;
  }

  iov.iov_base = buf;
  iov.iov_len  = NLMSG_SPACE(QCMAP_NL_MAX_MSG_SIZE);

  /* Read message from kernel */
  if ((recvsize = recvmsg(sock_fd, &msg, 0)) < 0)
  {
    LOG_MSG_ERROR("GetIPV6PrefixInfo: memory alloc failure: %d\n", errno, 0, 0);
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
    if ( AF_INET6 != ifa->ifa_family || RT_SCOPE_UNIVERSE != ifa->ifa_scope ||
         ifa->ifa_index != iface_idx)
    {
      LOG_MSG_ERROR("GetIPV6PrefixInfo: ip family %d, Scope %d or Index %d don't match\n",
                    ifa->ifa_family, ifa->ifa_scope, ifa->ifa_index);
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
           ipv6_prefix_info->prefix_len = ifa->ifa_prefixlen;
           sin6 = (struct sockaddr_in6 *)&ipv6_prefix_info->prefix_addr;
           memcpy(SASTORAGE_DATA(ipv6_prefix_info->prefix_addr),
                  RTA_DATA(rta),
                  sizeof(sin6->sin6_addr));
           ipv6_prefix_info->prefix_info_valid = true;
           break;
        case IFA_CACHEINFO:
           memcpy( &ipv6_prefix_info->cache_info,
                   RTA_DATA(rta),
                   sizeof(ipv6_prefix_info->cache_info) );
           LOG_MSG_INFO2( "GetIPV6PrefixInfo: Address Cache Info - prefered=%d valid=%d\n",
                           ipv6_prefix_info->cache_info.ifa_prefered,
                           ipv6_prefix_info->cache_info.ifa_valid, 0);
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

  return;


}

/*===========================================================================
  FUNCTION EnableIPV6Forwarding
==========================================================================*/
/*!
@brief
  Enables Ipv6 forwarding and starts radish.

@return
  void

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::EnableIPV6Forwarding()
{
  char command[MAX_COMMAND_STR_LEN];
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  int qcmap_cm_error;
  unsigned char ip6_addr[MAX_IPV6_PREFIX+1];   // Getting IPv6 Address
  memset(ip6_addr, 0, MAX_IPV6_PREFIX + 1);
  qcmap_nl_addr_t nl_addr;
  struct in6_addr all_nodes_addr = { 0xff,2,0,0,0,0,0,0,0,0,0,0,0,0,0,1 };

  LOG_MSG_INFO1("Entering IPV6 Forwarding",0,0,0);
  /* Cant continue if MobileAP is not enabled! */
  if (!this->qcmap_enable)
  {
    LOG_MSG_ERROR("Error: QCMAP not enabled.\n",0,0,0);
    return false;
  }

  memset(devname, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);

  if ( this->cfg.lan_config.enable_ipv6 )
  {
    if ( !this->sta_connected && !this->cradle_backhaul_connected )
    {
      if ( qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_MSGR_IP_FAMILY_V6_V01, devname, &qcmap_cm_error) !=
           QCMAP_CM_SUCCESS )
      {
        LOG_MSG_ERROR("Couldn't get ipv6 rmnet name. error %d\n", qcmap_cm_error,0,0);
        return false;
      }
    }
    else if ( this->cradle_backhaul_connected )
    {
      strlcpy(devname, ECM_IFACE, QCMAP_MSGR_INTF_LEN);
    }
    else if ( this->sta_connected )
    {
      if (this->IsAPSTABridgeActivated())
         strlcpy(devname,BRIDGE_IFACE, QCMAP_MSGR_INTF_LEN);
      else
         strlcpy(devname,this->cfg.lan_config.sta_interface, QCMAP_MSGR_INTF_LEN);
    }

    /* Allow IPV6 WWAN access when we are not in station mode. */
    if ( !this->sta_connected && !this->cradle_backhaul_connected )
    {
      this->AllowIPv6WWANAccess();
    }

    /* Remove the prefix based v6 rule since we might get a new prefix */
    if ( (this->wifi_mode == QCMAP_MSGR_WLAN_MODE_AP_AP_V01) &&
         (this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].access_profile == QCMAP_MSGR_PROFILE_INTERNET_ONLY_V01) &&
         (this->ipv6_prefix_based_rules_added))
    {
       inet_ntop(AF_INET6,((struct sockaddr_in6 *)&(ipv6_prefix_info.prefix_addr))->sin6_addr.s6_addr, ip6_addr, MAX_IPV6_PREFIX);
       snprintf( command, MAX_COMMAND_STR_LEN,
                "ebtables -D INPUT -i wlan%d -p IPv6 --ip6-destination %s/%d -j DROP",
                 this->ap_dev_num+1,ip6_addr,ipv6_prefix_info.prefix_len);
       ds_system_call( command, strlen(command));
       snprintf( command, MAX_COMMAND_STR_LEN,
                "ebtables -D OUTPUT -o wlan%d -p IPv6 --ip6-source %s/%d -j DROP",
                 this->ap_dev_num+1,ip6_addr,ipv6_prefix_info.prefix_len);
       ds_system_call( command, strlen(command));
       this->ipv6_prefix_based_rules_added = false;
    }

    LOG_MSG_INFO1("Setting prefix delegation mode %d",this->cfg.wan_config.prefix_delegation,0,0);
    prefix_delegation_activated = this->cfg.wan_config.prefix_delegation;

    LOG_MSG_INFO1("\nSetting forwarding for ipv6",0,0,0);
    if (!prefix_delegation_activated || this->sta_connected || this->cradle_backhaul_connected)
    {
      snprintf(command, MAX_COMMAND_STR_LEN,
               "echo 2 > /proc/sys/net/ipv6/conf/%s/accept_ra", devname);
      ds_system_call(command, strlen(command));
      ds_system_call("echo 2 > /proc/sys/net/ipv6/conf/all/proxy_ndp",
                      strlen("echo 2 > /proc/sys/net/ipv6/conf/all/proxy_ndp"));
      snprintf(command, MAX_COMMAND_STR_LEN,
               "echo 0 > /proc/sys/net/ipv6/conf/%s/accept_ra_prefix_route", devname);
      ds_system_call(command, strlen(command));
    }
    ds_system_call("echo 2 > /proc/sys/net/ipv6/conf/all/forwarding",
                    strlen("echo 2 > /proc/sys/net/ipv6/conf/all/forwarding"));

    /* Deleting the default route. */
    snprintf(command, MAX_COMMAND_STR_LEN,
             "ip -6 route del default dev %s metric 256", devname);
    ds_system_call(command, strlen(command));

    /* Add the default route with lower metric so that the packets
     *  will follow this route.
     */
    LOG_MSG_INFO1("\n Adding route for inet6 ::/0\n",0,0,0);
    snprintf(command, MAX_COMMAND_STR_LEN,
             "ip -6 route add default dev %s metric 256", devname);
    ds_system_call(command, strlen(command));

    /* Get the global ipv6 address. */
    memset(&this->ipv6_prefix_info, 0, sizeof(qcmap_cm_nl_prefix_info_t));
    this->ipv6_prefix_info.cache_info.ifa_prefered = IPV6_DEFAULT_PREFERED_LIFETIME;
    this->ipv6_prefix_info.cache_info.ifa_valid = IPV6_DEFAULT_VALID_LIFETIME;
    this->GetIPV6PrefixInfo(devname, &this->ipv6_prefix_info);

    if ( this->ipv6_prefix_info.prefix_info_valid == true &&
         (!prefix_delegation_activated || this->sta_connected || this->cradle_backhaul_connected))
    {
      this->UpdatePrefix(&this->ipv6_prefix_info, false, true, NULL);

      memset(&nl_addr, 0 ,sizeof(nl_addr));
      memcpy(nl_addr.ra_ipv6_dst_addr, all_nodes_addr.s6_addr, QCMAP_MSGR_IPV6_ADDR_LEN_V01);
      memcpy(nl_addr.ip_v6_addr,
             ((struct sockaddr_in6 *)&(this->ipv6_prefix_info.prefix_addr))->sin6_addr.s6_addr,
             QCMAP_MSGR_IPV6_ADDR_LEN_V01);
      nl_addr.isValidIPv6address = true;
      this->UpdateGlobalV6addr(&nl_addr, true);
    }

    /* Restart Radish. */
    this->StopRadish();
    this->StartRadish();

    if (prefix_delegation_activated && !(this->sta_connected || this->cradle_backhaul_connected))
    {
      this->SendRS();
    }

    /* Add the prefix based v6 rule since we might get a new prefix */
    if ( (this->wifi_mode == QCMAP_MSGR_WLAN_MODE_AP_AP_V01) &&
         (this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].access_profile == QCMAP_MSGR_PROFILE_INTERNET_ONLY_V01) )
    {
      if (!this->ipv6_prefix_based_rules_added && this->ipv6_prefix_info.prefix_info_valid )
      {
         /* Get new IPv6 Address  */
         memset(ip6_addr, 0, MAX_IPV6_PREFIX + 1);
         inet_ntop(AF_INET6,((struct sockaddr_in6 *)&(this->ipv6_prefix_info.prefix_addr))->sin6_addr.s6_addr, ip6_addr, MAX_IPV6_PREFIX );
         snprintf( command, MAX_COMMAND_STR_LEN,
                  "ebtables -A INPUT -i wlan%d -p IPv6 --ip6-destination %s/%d -j DROP",
                  this->ap_dev_num+1,ip6_addr,ipv6_prefix_info.prefix_len);
         ds_system_call( command, strlen(command));
         snprintf( command, MAX_COMMAND_STR_LEN,
                  "ebtables -A OUTPUT -o wlan%d -p IPv6 --ip6-source %s/%d -j DROP",
                  this->ap_dev_num+1,ip6_addr,ipv6_prefix_info.prefix_len);
         ds_system_call( command, strlen(command));
         this->ipv6_prefix_based_rules_added = true;
      }
    }
  }

  return true;
}

/*===========================================================================
  FUNCTION DisableIPV6Forwarding
==========================================================================*/
/*!
@brief
  This function will disable Ipv6 forwarding and stops radish service.

@return
  void

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::DisableIPV6Forwarding(boolean send_ra)
{
  char command[MAX_COMMAND_STR_LEN];
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  int qcmap_cm_error;

  LOG_MSG_INFO1("Disable IPV6 Forwarding",0,0,0);

  /* Cant continue if MobileAP is not enabled! */
  if ( !this->qcmap_enable )
  {
    LOG_MSG_ERROR("Error: QCMAP not enabled.\n",0,0,0);
    return false;
  }

  if ( this->cfg.lan_config.enable_ipv6 )
  {

    /* Stop Radish. */
    this->StopRadish();

    /* Install the rule to block WWAN access. */
    this->BlockIPv6WWANAccess();

    /* Get device name */
    if ( this->sta_connected || this->cradle_backhaul_connected )
    {
      if( this->cradle_backhaul_connected )
      {
        strlcpy(devname, ECM_IFACE, QCMAP_MSGR_INTF_LEN);
      }
      else
      {
        strlcpy(devname,this->cfg.lan_config.sta_interface, QCMAP_MSGR_INTF_LEN);
      }
    }

    /* Deprecate old prefix. */
    if ( this->ipv6_prefix_info.prefix_info_valid == true )
    {
      if (prefix_delegation_activated)
        this->DeprecateClientRA();
      else
        this->UpdatePrefix(&this->ipv6_prefix_info, true, send_ra, NULL);
    }
    else
    {
       /* In case of STA Mode, try to get the address again and deprecate the prefix. */
       if ( this->sta_connected || this->cradle_backhaul_connected )
       {
         memset(&this->ipv6_prefix_info, 0, sizeof(qcmap_cm_nl_prefix_info_t));
         this->ipv6_prefix_info.cache_info.ifa_prefered = IPV6_DEFAULT_PREFERED_LIFETIME;
         this->ipv6_prefix_info.cache_info.ifa_valid = IPV6_DEFAULT_VALID_LIFETIME;
         this->GetIPV6PrefixInfo(devname, &this->ipv6_prefix_info);
         if ( this->ipv6_prefix_info.prefix_info_valid == true )
         {
           this->UpdatePrefix(&this->ipv6_prefix_info, true, send_ra, NULL);
         }
       }
    }

    /* Delete PPP IPV6 route if present. */
    this->DeletePPPIPv6Route();

    LOG_MSG_INFO1("\nDisabling forwarding for ipv6",0,0,0);
    ds_system_call("echo 0 > /proc/sys/net/ipv6/conf/all/forwarding",
                    strlen("echo 0 > /proc/sys/net/ipv6/conf/all/forwarding"));
    ds_system_call("echo 0 > /proc/sys/net/ipv6/conf/all/proxy_ndp",
                   strlen("echo 0 > /proc/sys/net/ipv6/conf/all/proxy_ndp"));

    LOG_MSG_INFO1("\n Delete default route for inet6 ::/0\n",0,0,0);
    snprintf(command, MAX_COMMAND_STR_LEN,
             "ip -6 route del default metric 256");
    ds_system_call(command, strlen(command));

    if ( this->sta_connected || this->cradle_backhaul_connected )
    {
      snprintf(command, MAX_COMMAND_STR_LEN,
               "echo 0 > /proc/sys/net/ipv6/conf/%s/accept_ra", devname);
      ds_system_call(command, strlen(command));
    }

    //Enable the bridge forwarding for Ipv6
    snprintf(command, MAX_COMMAND_STR_LEN,
             "echo 1 > /proc/sys/net/ipv6/conf/bridge0/forwarding");
    ds_system_call(command, strlen(command));

    /*Reset the Global Ipv6 address from the connected devices information*/
    if (GetNumofConnectedDevices() > 0)
    {
      if (!(DeleteGlobalipv6AddressInfo()))
      {
        LOG_MSG_ERROR("Could not delete Global IPv6 address from Connected"
                      "devices information", 0, 0, 0);
      }
    }
  }

  return true;
}

/*===========================================================================
  FUNCTION StorePPPIPv6IID
==========================================================================*/
/*!
@brief
  Adds the destination based route to ppp interface.

@parameters
  uint8_t ppp_ip[]

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::StorePPPIPv6IID
(
  uint8_t            ppp_ip[]
)
{
  int qmi_error;
  unsigned char ip6_addr[MAX_IPV6_PREFIX+1];   // Getting IPv6 Address
  memset(ip6_addr, 0, MAX_IPV6_PREFIX+1);
  char command[MAX_COMMAND_STR_LEN];
  struct ps_in6_addr *prefix_ptr, *global_prefix_ptr;
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  int qcmap_cm_error;
  struct in6_addr null_ipv6_address;
  qmi_error_type_v01 qmi_err_num;
  qcmap_nl_addr_t qcmap_nl_buffer;
  qcmap_nl_addr_t nl_addr;

  memset(&null_ipv6_address, 0, sizeof(struct in6_addr));

  if (!this->cfg.lan_config.usb_conf.ppp_enabled)
  {
    /* QCMAP PPP not enabled */
    LOG_MSG_ERROR("QCMAP PPP not enabled\n",0,0,0);
    return;
  }

  /* Update the txqueuelength for PPP interface. */
  snprintf( command, MAX_COMMAND_STR_LEN,
            "ifconfig %s txqueuelen 1000",
            PPP_IFACE);
  ds_system_call( command, strlen(command));

  /* Restart UPNP/DLNA */
  if(this->cfg.srvc_config.dlna_config)
  {
    if(!EnableDLNA(&qmi_err_num))
    {
      LOG_MSG_ERROR("Cannot enable DLNA, error: %d.", qmi_err_num, 0, 0);
    }
  }

  if(this->cfg.srvc_config.upnp_config)
  {
    if(!EnableUPNP(&qmi_err_num))
    {
      LOG_MSG_ERROR("Cannot enable UPnP, error: %d.", qmi_err_num, 0, 0);
    }
  }

  if (!memcmp ((struct in6_addr *)ppp_ip, &null_ipv6_address, sizeof(struct in6_addr)))
  {
    /* QCMAP PPP NULL address recieved */
    LOG_MSG_ERROR("QCMAP PPP NULL address recieved\n",0,0,0);
    return;
  }

  if (!this->cfg.lan_config.enable_ipv6)
  {
    /* QCMAP IPV6 is not enabled */
    LOG_MSG_ERROR("QCMAP IPV6 not enabled\n",0,0,0);
    return;
  }

  {
    /* Store the IID. */
    memset(this->ppp_ipv6_iid, 0, QCMAP_MSGR_IPV6_ADDR_LEN_V01);
    memcpy(this->ppp_ipv6_iid, ppp_ip, QCMAP_MSGR_IPV6_ADDR_LEN_V01);

    /* Update the connected devices information. */
    memset(&qcmap_nl_buffer, 0, sizeof(qcmap_nl_buffer));
    qcmap_nl_buffer.isValidIPv4address = true;
    qcmap_nl_buffer.isValidIPv6address = true;
    qcmap_nl_buffer.ip_addr = htonl(this->cfg.lan_config.ppp_reserved_ip);
    memcpy(qcmap_nl_buffer.ip_v6_addr, ppp_ip, QCMAP_MSGR_IPV6_ADDR_LEN_V01);
    //Convert the MAC from char to hex
    if(!ds_mac_addr_pton((char*)PPP_CLIENT_MAC_ADDR, qcmap_nl_buffer.mac_addr))
    {
      LOG_MSG_ERROR("StorePPPIPv6IID - Error in MAC address conversion",
                    0,0,0);
    }
    this->SetUSBMac(qcmap_nl_buffer.mac_addr);
    if (!(this->MatchMacAddrInList(&(qcmap_nl_buffer))))
    {
      LOG_MSG_INFO1("No match found for the USB MAC,"
                    "so add a linked list node\n",0, 0, 0);
      if (!(this->AddNewDeviceEntry(
           (void*)qcmap_nl_buffer.mac_addr,
           QCMAP_MSGR_DEVICE_TYPE_USB_V01,
           &qcmap_nl_buffer.ip_addr,
           qcmap_nl_buffer.ip_v6_addr,
           qcmap_nl_buffer.isValidIPv4address,
           qcmap_nl_buffer.isValidIPv6address)))
      {
        LOG_MSG_ERROR("Error in adding a new device entry ",
                      0, 0, 0);
      }
      else
      {
        if (qcmap_nl_send_getneigh_event() != QCMAP_NL_SUCCESS)
          LOG_MSG_ERROR("Error sending GETNEIGH message",0,0,0);
      }
    }
  }

  if (!memcmp ((struct in6_addr *)this->ppp_ipv6_addr, &null_ipv6_address, sizeof(struct in6_addr)))
  {
    /* QCMAP PPP Global IPV6 address not yet recieved. Silently return. */
    return;
  }

  /* Add the IPV6 route. If not done already. */
  memset(&nl_addr, 0 ,sizeof(nl_addr));
  memcpy(nl_addr.ra_ipv6_dst_addr, this->ppp_ipv6_iid, QCMAP_MSGR_IPV6_ADDR_LEN_V01);
  memcpy(nl_addr.ip_v6_addr,
         this->ppp_ipv6_addr,
         QCMAP_MSGR_IPV6_ADDR_LEN_V01);
  nl_addr.isValidIPv6address = true;
  this->UpdateGlobalV6addr(&nl_addr);

  return;
}

/*===========================================================================
  FUNCTION DeletePPPIPv6Route
==========================================================================*/
/*!
@brief
  Deletes the destination based route to ppp interface.

@parameters
  None

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::DeletePPPIPv6Route
(
)
{
  int qmi_error;
  unsigned char ip6_addr[MAX_IPV6_PREFIX+1];   // Getting IPv6 Address
  memset(ip6_addr, 0, MAX_IPV6_PREFIX+1);
  char command[MAX_COMMAND_STR_LEN];
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  int qcmap_cm_error;
  qmi_error_type_v01 qmi_err_num;

  if (!this->cfg.lan_config.usb_conf.ppp_enabled)
  {
    /* QCMAP PPP not enabled */
    LOG_MSG_ERROR("QCMAP PPP not enabled\n",0,0,0);
    return;
  }

  if (!this->cfg.lan_config.enable_ipv6)
  {
    /* QCMAP IPV6 is not enabled */
    LOG_MSG_ERROR("QCMAP IPV6 not enabled\n",0,0,0);
    return;
  }

  /* Delete the destination based route. */
  inet_ntop(AF_INET6,(struct in6_addr *)this->ppp_ipv6_addr, ip6_addr, MAX_IPV6_PREFIX);
  snprintf( command, MAX_COMMAND_STR_LEN,
            "ip -6 route del %s dev %s", ip6_addr, PPP_IFACE);
  ds_system_call( command, strlen(command));

  /* Reset the address. */
  memset(this->ppp_ipv6_addr, 0, QCMAP_MSGR_IPV6_ADDR_LEN_V01);

  return;
}


/*===========================================================================
  FUNCTION StartRadish
==========================================================================*/
/*!
@brief
  Starts Radish with appropriate intrerface, this will ebanle Ipv6 multicast
  forwarding.

@return
  void

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::StartRadish()
{
  char command[MAX_COMMAND_STR_LEN];
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  char usb_intf_name [DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  int qcmap_cm_error;

  LOG_MSG_INFO1("\n Starting radish \n",0,0,0);

  if ( !this->sta_connected && !this->cradle_backhaul_connected )
  {
    if ( qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_MSGR_IP_FAMILY_V6_V01, devname, &qcmap_cm_error) !=
         QCMAP_CM_SUCCESS )
    {
      LOG_MSG_ERROR("Couldn't get ipv6 rmnet name. error %d\n", qcmap_cm_error,0,0);
      return;
    }
  }
  else if ( this->cradle_backhaul_connected )
  {
    strlcpy(devname, ECM_IFACE, QCMAP_MSGR_INTF_LEN);
  }
  else if (this->sta_connected)
  {
    strlcpy(devname, this->cfg.lan_config.sta_interface, QCMAP_MSGR_INTF_LEN);
  }

  if ( this->cfg.lan_config.usb_conf.ppp_enabled == true )
  {
    snprintf(command, MAX_COMMAND_STR_LEN,
             "radish -k -b %s -i %s -x -i %s -x -i %s -x > /dev/null 2>&1 &",
             devname, devname, BRIDGE_IFACE, PPP_IFACE);
  }
  else
  {
    snprintf(command, MAX_COMMAND_STR_LEN,
             "radish -k -b %s -i %s -x -i %s -x > /dev/null 2>&1 &",
             devname, devname, BRIDGE_IFACE);
  }
  ds_system_call(command, strlen(command));
}

/*===========================================================================
  FUNCTION StopRadish
==========================================================================*/
/*!
@brief
  This function will stop Radish.

@return
  void

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::StopRadish()
{
  LOG_MSG_INFO1("\n Stopping radish \n",0,0,0);
  ds_system_call("killall -9 radish", strlen("killall -9 radish"));
}


/*===========================================================================
  FUNCTION IsRadishRunning
==========================================================================*/
/*!
@brief
  Checks to see if radish is running, waits up to 1 second

@parameters
  char *process

@return
  true  - process is running
  flase - process is not running after waiting 1 second

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::IsRadishRunning()
{
  int i = 0;
  FILE *cmd;
  char pid_s[MAX_CMD_SIZE];
  int pid;
  const char process[] = "pidof radish";

  while (i++ < 5)
  {
    usleep(200000);
    cmd = popen(process, "r");
    pid = 0;
    memset(pid_s, 0, MAX_CMD_SIZE);

    fgets(pid_s, MAX_CMD_SIZE, cmd);
    pid = atoi(pid_s);
    pclose(cmd);
    if(pid != 0)
    {
      return true;
    }
  }

  LOG_MSG_ERROR("Radish is not running after waiting 1 second\n", 0, 0, 0);
  return false;
}

/*===========================================================================
  FUNCTION CheckforAddrConflict
==========================================================================*/
/*!
@brief
  This function will check if ap, guest ap and sta are in different subnets.
  To avoid address conflicts.

@return
  true  - on Success
  false - on Failure
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::CheckforAddrConflict( qcmap_msgr_lan_config_v01 *lan_config,
                                                       qcmap_msgr_station_mode_config_v01 *station_config )
{

  /* Check for LAN Configuration for address conflicts */
  if ( lan_config != NULL )
  {
   /* Check with sta configuratin for address conflicts */
    if ( station_config !=NULL )
    {
      if ( ( station_config->conn_type == QCMAP_MSGR_STA_CONNECTION_STATIC_V01 ) &&
           ( ( lan_config->gw_ip & lan_config->netmask ) ==
             ( station_config->static_ip_config.gw_ip & station_config->static_ip_config.netmask )))
      {
        return true;
      }
    }
    /* Check with default sta configuratin for address conflicts */
    else if ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].conn_type == QCMAP_MSGR_STA_CONNECTION_STATIC_V01 )
    {
      if ( ( lan_config->gw_ip & lan_config->netmask ) ==
           ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].static_ip_config.gw_ip &
             this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].static_ip_config.netmask ))
      {
        LOG_MSG_INFO1("\n LAN configured address overlapped with STA IP \n",0,0,0);
        return true;
      }
    }
  }

  /* Check for sta Configuratin for address conflicts with AP and Guest AP mode*/
  if ( station_config != NULL && (station_config->conn_type == QCMAP_MSGR_STA_CONNECTION_STATIC_V01 ))
  {
     if ( ( station_config->static_ip_config.gw_ip & station_config->static_ip_config.netmask) ==
           ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].a5_ip_addr &
            this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].sub_net_mask ))
     {
        LOG_MSG_INFO1("\n STA configured address overlapped with LAN IP\n",0,0,0);
        return true;
     }
  }
  return false;
}

/*===========================================================================
  FUNCTION SetWLANConfig
==========================================================================*/
/*!
@brief
  This function will set the wlan configuration.

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetWLANConfig( qcmap_msgr_wlan_mode_enum_v01 wlan_mode,
                                               qcmap_msgr_access_profile_v01  guest_ap_acess_profile,
                                               qcmap_msgr_station_mode_config_v01 *station_config,
                                               qmi_error_type_v01 *qmi_err_num )
{
  in_addr addr;
  // Initialize QMI error
  *qmi_err_num = QMI_ERR_NONE_V01;
  qcmap_cm_access_profile_type new_cfg_access_profile;
  /* check for address conflicts before setting the wlan configuration */
  if( CheckforAddrConflict( NULL,station_config ))
  {
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
    return false;
  }

  if ( wlan_mode >= QCMAP_MSGR_WLAN_MODE_AP_V01 &&
       wlan_mode <= QCMAP_MSGR_WLAN_MODE_AP_STA_V01 )
  {
    this->cfg.lan_config.wlan_mode = wlan_mode;
  }

  if ( guest_ap_acess_profile != QCMAP_MSGR_ACCESS_PROFILE_MIN_ENUM_VAL_V01) {
      this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].access_profile = guest_ap_acess_profile;
  }

  if ( station_config != NULL ) {
    this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].conn_type = station_config->conn_type;
    if ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].conn_type ==
                                        QCMAP_MSGR_STA_CONNECTION_STATIC_V01 )
    {
      this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].static_ip_config = station_config->static_ip_config;
    }
    this->cfg.lan_config.ap_sta_bridge_mode = station_config->ap_sta_bridge_mode;
  }

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();
  return true;

}

/*===========================================================================
  FUNCTION GetWLANConfig
==========================================================================*/
/*!
@brief
  This function will get the wlan configuration.

@return
  true  - on Success
  false - on Failure
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetWLANConfig(
                                             qcmap_msgr_wlan_mode_enum_v01 *wlan_mode,
                                             qcmap_msgr_access_profile_v01 *guest_access_profile,
                                             qcmap_msgr_station_mode_config_v01 *station_config,
                                             qmi_error_type_v01 *qmi_err_num)
{
  if ( wlan_mode == NULL || guest_access_profile == NULL || station_config == NULL )
  {
    LOG_MSG_ERROR("NULL params.\n",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01 ;
    return false;
  }

  *wlan_mode = this->cfg.lan_config.wlan_mode;

  *guest_access_profile = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].access_profile;

  /* Populate Station Configuration. */
  station_config->conn_type = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].conn_type;
  station_config->static_ip_config = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].static_ip_config;
  station_config->ap_sta_bridge_mode = this->cfg.lan_config.ap_sta_bridge_mode;

  return true;
}

/*===========================================================================
  FUNCTION SetLANConfig
==========================================================================*/
/*!
@brief
  This function will set the lan configuration.

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetLANConfig(qcmap_msgr_lan_config_v01 *lan_config,
                                               qmi_error_type_v01 *qmi_err_num )
{
  in_addr addr;
  // Initialize QMI error
  *qmi_err_num = QMI_ERR_NONE_V01;

  /* sync /etc/dhcp-hosts with the existing DHCP Reservation records*/
  dhcp_hosts_fp = fopen(DHCP_HOSTS_FILE, "wb");
  if (NULL == dhcp_hosts_fp)
  {
      /* File could not be opened for writing/append*/
      LOG_MSG_ERROR("\n Failed to create /etc/dhcp_hosts!!",0,0,0);
      *qmi_err_num = QMI_ERR_INTERNAL_V01;
      return false;
  }

  /* check for address conflicts before setting the wlan configuration */
  if( CheckforAddrConflict(lan_config, NULL ))
  {
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
    return false;
  }

  if (lan_config != NULL)
  {
    this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].a5_ip_addr = lan_config->gw_ip;
    this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].sub_net_mask = lan_config->netmask;
    this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].enable_dhcpd = lan_config->enable_dhcp;

     // Checking DHCP Configuration.
    if ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].enable_dhcpd )
    {
       this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_start_address = lan_config->dhcp_config.dhcp_start_ip;
       this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_end_address = lan_config->dhcp_config.dhcp_end_ip;
       memset(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_lease_time, 0, QCMAP_CM_MAX_FILE_LEN);
       snprintf( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_lease_time, QCMAP_CM_MAX_FILE_LEN, "%d",
       lan_config->dhcp_config.lease_time);

       /* DHCP config checks. */
       if ((( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                             a5_ip_addr &
              this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                         sub_net_mask) !=
            ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                      dhcp_start_address &
              this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                          sub_net_mask))||
           ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                        dhcp_start_address <=
             this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                        a5_ip_addr))
      {
        this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
              dhcp_start_address =
        this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                       a5_ip_addr+MAX_WIFI_CLIENTS;
        addr.s_addr =
        htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                      dhcp_start_address );
        LOG_MSG_INFO1("SSID1 DHCP Start addr provide is invalid."
                     "Setting it to default value %s\n",inet_ntoa(addr),0,0);
      }

      if ((( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                             a5_ip_addr &
             this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                          sub_net_mask) !=
           ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                       dhcp_end_address &
             this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                          sub_net_mask))||
           ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                       dhcp_end_address <
             this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                      dhcp_start_address))
      {
        this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                        dhcp_end_address =
        this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                     dhcp_start_address + MAX_WIFI_CLIENTS;
        addr.s_addr =
        htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                                         dhcp_end_address );
        LOG_MSG_INFO1("SSID1 DHCP End addr provided is invalid."
                     "Setting it to default value %s\n",inet_ntoa(addr),0,0);
      }
    }
  }
  /*sync the /etc/dhcp-hosts file now*/
  this->sync_dhcp_hosts();
  if ( dhcp_hosts_fp )
  {
    fclose(dhcp_hosts_fp);
  }

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();
  return true;

}

/*===========================================================================
  FUNCTION GetLANConfig
==========================================================================*/
/*!
@brief
  This function will get the lan configuration.

@return
  true  - on Success
  false - on Failure
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetLANConfig(qcmap_msgr_lan_config_v01 *lan_config,
                                             qmi_error_type_v01 *qmi_err_num)
{
  if ( lan_config == NULL )
  {
    LOG_MSG_ERROR("NULL params.\n",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01 ;
    return false;
  }

  /* Populate AP Configuration. */
  lan_config->gw_ip = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].a5_ip_addr;
  lan_config->netmask = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].sub_net_mask;
  lan_config->enable_dhcp = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].enable_dhcpd;

  if ( lan_config->enable_dhcp == TRUE )
  {
    lan_config->dhcp_config.dhcp_start_ip = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_start_address;
    lan_config->dhcp_config.dhcp_end_ip = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_end_address;
    lan_config->dhcp_config.lease_time = (uint32)atoi(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_lease_time);
  }

  return true;
}

/*===========================================================================
  FUNCTION GetWLANStatus
==========================================================================*/
/*!
@brief
  Gets the wlan configured mode.

@parameters
  qcmap_msgr_wlan_mode_enum_v01 *wlan_mode

@return
  true  - on Success
  false - on Failure
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetWLANStatus
(
  qcmap_msgr_wlan_mode_enum_v01 *wlan_mode,
  qmi_error_type_v01 *qmi_err_num
)
{
  if ( wlan_mode == NULL )
  {
    LOG_MSG_ERROR("NULL params.\n",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01 ;
    return false;
  }

  *wlan_mode = this->wifi_mode;
  return true;
}

/*===========================================================================
  FUNCTION ActivateWLAN
==========================================================================*/
/*!
@brief
  Brings up the lan with the latest wlan configuration, wlan mode and access profile.

@parameters
  void

@return
  true  - on Success
  false - on Failure
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::ActivateWLAN
(
  qmi_error_type_v01 *qmi_err_num
)
{
   LOG_MSG_INFO1("QCMAP_ConnectionManager::ActivateWLAN",0,0,0);
  /* STA mode is tied to mobile ap handle as all the NAT configuration is
   * tied to mobile ap handle. */
  if ( ( this->cfg.lan_config.wlan_mode == QCMAP_MSGR_WLAN_MODE_AP_STA_V01 ) &&
       ( this->qcmap_tear_down_in_progress || this->qcmap_cm_handle <= 0 ) )
  {
    LOG_MSG_ERROR("Cannot enable STA mode when mobileap is in not enabled.", 0, 0, 0);
    *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
    return false;
  }

  // If Access profile has changed and we are in AP-AP mode or previously we were in ap-ap mode, then, we will delete or add the ebtables rules.
  if (IsGuestProfileUpdated() && (this->wifi_mode  == QCMAP_MSGR_WLAN_MODE_AP_AP_V01)) {
      UpdateAccessProfileRules();
      this->prev_guest_profile = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].access_profile;
  }

  // We need to disable WLAN, re-enable it, if either WLAN MODE or Access Profile or Station Mode config has changed
  if ( (IsWlanModeUpdated() && (this->wifi_mode != QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01)) ||
        (IsStaCfgUpdated() && (this->wifi_mode  == QCMAP_MSGR_WLAN_MODE_AP_STA_V01)) ||
        ((this->revert_ap_sta_router_mode == true) && (this->wifi_mode  == QCMAP_MSGR_WLAN_MODE_AP_STA_V01)) )
  {
    /* Disable WLAN. */
    this->activate_wlan_in_progress = true;
    this->DisableWLAN(qmi_err_num);
  }

  /* Enable WLAN. */
  this->EnableWLAN(qmi_err_num);

  this->activate_wlan_in_progress = false;
  return true;
}

/*===========================================================================
  FUNCTION ActivateHostapdConfig
==========================================================================*/
/*!
@brief
  Restarts Hostapd with the latest configuration.

@parameters
  void

@return
  true  - on Success
  false - on Failure
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::ActivateHostapdConfig
(
  qcmap_msgr_activate_hostapd_ap_enum_v01 ap_type,
  qcmap_msgr_activate_hostapd_action_enum_v01 action_type,
  qmi_error_type_v01 *qmi_err_num
)
{
  FILE *fp = NULL;
  int ap_pid =0, guest_ap_pid = 0;

   LOG_MSG_INFO1("QCMAP_ConnectionManager::ActivateHostapdConfig ap_type = %d, action = %d",ap_type,action_type,0);

  if ( this->wifi_mode == QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01 )
  {
    LOG_MSG_ERROR("Cannot activate hostapd when WLAN is disabled.", 0, 0, 0);
    *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
    return false;
  }

  if ( ap_type < QCMAP_MSGR_PRIMARY_AP_V01 ||
       ap_type > QCMAP_MSGR_PRIMARY_AND_GUEST_AP_V01 )
  {
    LOG_MSG_ERROR("Cannot activate hostapd incorrect ap_type: %d.",
                  ap_type, 0, 0);
    *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
    return false;
  }

  if ( action_type < QCMAP_MSGR_HOSTAPD_START_V01 ||
       action_type > QCMAP_MSGR_HOSTAPD_RESTART_V01 )
  {
    LOG_MSG_ERROR("Cannot activate hostapd incorrect action_type: %d.",
                  action_type, 0, 0);
    *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
    return false;
  }

  if ( ( ap_type == QCMAP_MSGR_PRIMARY_AND_GUEST_AP_V01 ||
       ap_type == QCMAP_MSGR_GUEST_AP_V01 ) &&
       this->wifi_mode != QCMAP_MSGR_WLAN_MODE_AP_AP_V01 )
  {
    LOG_MSG_ERROR("Cannot activate hostapd when AP-AP is not enabled: %d.",
                  this->wifi_mode, 0, 0);
    *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
    return false;
  }

   /* Get the Primary AP PID. */
  fp = fopen(HOSTAPD_PID_FILE, "r");
  if ( fp == NULL )
  {
    LOG_MSG_ERROR("Error opening hostapd pid file: %d.\n", errno, 0, 0);
  }
  else if (fscanf(fp, "%d", &ap_pid) != 1) {
    LOG_MSG_ERROR("Error reading hostapd file: %d.\n", errno, 0, 0);
    ap_pid = 0;
    fclose(fp);
  }
  else
  {
    LOG_MSG_INFO1("Primary AP PID = %d",ap_pid,0,0);
    fclose(fp);
  }

  /* Get the Guest AP PID. */
  if ( this->wifi_mode == QCMAP_MSGR_WLAN_MODE_AP_AP_V01 )
  {
    fp = fopen(HOSTAPD_SSID2_PID_FILE, "r");
    if ( fp == NULL )
    {
      LOG_MSG_ERROR("Error opening Guest AP hostapd pid file: %d.\n", errno, 0, 0);
    }
    else if (fscanf(fp, "%d", &guest_ap_pid) != 1)
    {
      LOG_MSG_ERROR("Error reading Guest AP hostapd file: %d.\n", errno, 0, 0);
      guest_ap_pid = 0;
      fclose(fp);
    }
    else
    {
      LOG_MSG_INFO1("Guest AP PID = %d",guest_ap_pid,0,0);
      fclose(fp);
    }
  }

  if ( action_type == QCMAP_MSGR_HOSTAPD_START_V01 )
  {
    return ActivateHostapdActionStart(ap_type, ap_pid, guest_ap_pid, qmi_err_num);
  }
  else  if ( action_type == QCMAP_MSGR_HOSTAPD_STOP_V01 )
  {
    return ActivateHostapdActionStop(ap_type, ap_pid, guest_ap_pid, qmi_err_num);
  }
  else  if ( action_type == QCMAP_MSGR_HOSTAPD_RESTART_V01 )
  {
    return ActivateHostapdActionRestart(ap_type, ap_pid, guest_ap_pid, qmi_err_num);
  }

  return true;
}

/*===========================================================================
  FUNCTION ActivateHostapdActionStart
==========================================================================*/
/*!
@brief
  Starts Hostapd with the latest configuration.

@parameters
  void

@return
  true  - on Success
  false - on Failure
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::ActivateHostapdActionStart
(
  qcmap_msgr_activate_hostapd_ap_enum_v01 ap_type,
  int ap_pid,
  int guest_ap_pid,
  qmi_error_type_v01 *qmi_err_num
)
{
  char command[MAX_COMMAND_STR_LEN];
  struct in_addr addr;
  int i, dev;
  char netmask[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char subnet[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  FILE *fp = NULL;

   LOG_MSG_INFO1("QCMAP_ConnectionManager::ActivateHostapdActionStart",0,0,0);

  switch ( ap_type )
  {
    case QCMAP_MSGR_PRIMARY_AP_V01:
    {
      /* Start Hostapd for Primary AP. */
      if ( ap_pid != 0 )
      {
        LOG_MSG_ERROR("Hostapd already running for Primary AP. PID : %d.",
                      ap_pid, 0, 0);
        *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
        return false;
      }
      StartHostAPD(QCMAP_MSGR_INTF_AP_INDEX, this->ap_dev_num);
      addr.s_addr = htonl(this->cfg.lan_config.ll_subnet_mask);
      strlcpy(netmask, inet_ntoa(addr), 16);
      addr.s_addr = htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].ll_ip_addr & this->cfg.lan_config.ll_subnet_mask);
      strlcpy(subnet, inet_ntoa(addr), 16);
      snprintf(command, MAX_COMMAND_STR_LEN, "route del -net %s netmask %s dev wlan%d", subnet, netmask, this->ap_dev_num);
      ds_system_call(command, strlen(command));
    }
    break;
    case QCMAP_MSGR_GUEST_AP_V01:
    {
       /* Start Hostapd for GuestAP. */
      if ( guest_ap_pid != 0 )
      {
        LOG_MSG_ERROR("Hostapd already running for Guest AP. PID : %d.",
                      guest_ap_pid, 0, 0);
        *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
        return false;
      }
      StartHostAPD(QCMAP_MSGR_INTF_GUEST_AP_INDEX, this->ap_dev_num);
      addr.s_addr = htonl(this->cfg.lan_config.ll_subnet_mask);
      strlcpy(netmask, inet_ntoa(addr), 16);
      addr.s_addr = htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].ll_ip_addr & this->cfg.lan_config.ll_subnet_mask);
      strlcpy(subnet, inet_ntoa(addr), 16);
      snprintf(command, MAX_COMMAND_STR_LEN, "route del -net %s netmask %s dev wlan%d", subnet, netmask, this->ap_dev_num+1);
      ds_system_call(command, strlen(command));
    }
    break;
    case QCMAP_MSGR_PRIMARY_AND_GUEST_AP_V01:
    {
      if ( ap_pid != 0 && guest_ap_pid != 0 )
      {
        LOG_MSG_ERROR("Hostapd already running for Both AP's. PID1 : %d and PID2 : %d.",
                      ap_pid, guest_ap_pid, 0);
        *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
        return false;
      }

      /* Start HostAP for both the AP's.
         This part needs to be done only for AP interfaces.
         Proceed only upto STA iface index since STA is always last in the
         list (ap; ap,ap; ap,sta; ap,ap,sta supported). */
      for ( i=0, dev = this->ap_dev_num; i < this->sta_iface_index; i++, dev++ )
      {
        if ( !this->cfg.lan_config.interface[i].enable )
          break;
        /* Start only the non-running hostapd process. */
        if ( i == QCMAP_MSGR_INTF_AP_INDEX && ap_pid != 0 )
          continue;
        else if ( i == QCMAP_MSGR_INTF_GUEST_AP_INDEX && guest_ap_pid != 0 )
          continue;
        /* Start the HostAP daemon. */
        StartHostAPD(i, dev);
        addr.s_addr = htonl(this->cfg.lan_config.ll_subnet_mask);
        strlcpy(netmask, inet_ntoa(addr), 16);
        addr.s_addr = htonl(this->cfg.lan_config.interface[i].ll_ip_addr & this->cfg.lan_config.ll_subnet_mask);
        strlcpy(subnet, inet_ntoa(addr), 16);
        snprintf(command, MAX_COMMAND_STR_LEN, "route del -net %s netmask %s dev wlan%d", subnet, netmask, dev);
        ds_system_call(command, strlen(command));
      }
    }
    break;
  }

  return true;
}

/*===========================================================================
  FUNCTION ActivateHostapdActionStop
==========================================================================*/
/*!
@brief
  Stops Hostapd for the required AP.

@parameters
  void

@return
  true  - on Success
  false - on Failure
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::ActivateHostapdActionStop
(
  qcmap_msgr_activate_hostapd_ap_enum_v01 ap_type,
  int ap_pid,
  int guest_ap_pid,
  qmi_error_type_v01 *qmi_err_num
)
{
  char command[MAX_COMMAND_STR_LEN];
  struct in_addr addr;
  int i, dev;
  char netmask[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char subnet[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  FILE *fp = NULL;
  qcmap_msgr_device_type_enum_v01 device_type = QCMAP_MSGR_DEVICE_TYPE_ENUM_MIN_ENUM_VAL_V01;

   LOG_MSG_INFO1("QCMAP_ConnectionManager::ActivateHostapdActionStop",0,0,0);

  switch ( ap_type )
  {
    case QCMAP_MSGR_PRIMARY_AP_V01:
    {
      if ( ap_pid == 0 )
      {
        LOG_MSG_ERROR("Hostapd already stopped for Primary AP. PID : %d.",
                      ap_pid, 0, 0);
        *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
        return false;
      }
      this->StopHostapdCli(ap_type);
      /* Stop the hostapd for Primary AP. */
      snprintf(command, MAX_COMMAND_STR_LEN, "kill %d", ap_pid);
      ds_system_call(command, strlen(command));
      if ( !IsHostapdkilled(guest_ap_pid) )
      {
        LOG_MSG_ERROR("Another instance of hostapd is running", 0, 0, 0);
        *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
        return false;
      }
      if (this->wifi_mode == QCMAP_MSGR_WLAN_MODE_AP_STA_V01)
      {
        device_type = QCMAP_MSGR_DEVICE_TYPE_GUEST_AP_V01;
        this->DeleteWLANConnectedDevicesClients(device_type);
      }
      else
      {
        device_type = QCMAP_MSGR_DEVICE_TYPE_PRIMARY_AP_V01;
        this->DeleteWLANConnectedDevicesClients(device_type);
      }
    }
    break;
    case QCMAP_MSGR_GUEST_AP_V01:
    {
      if ( guest_ap_pid == 0 )
      {
        LOG_MSG_ERROR("Hostapd already stopped for Guest AP. PID : %d.",
                      guest_ap_pid, 0, 0);
        *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
        return false;
      }
      this->StopHostapdCli(ap_type);
      /* Stop the hostapd for Guest AP. */
      snprintf(command, MAX_COMMAND_STR_LEN, "kill %d", guest_ap_pid);
      ds_system_call(command, strlen(command));
      if ( !IsHostapdkilled(ap_pid) )
      {
        LOG_MSG_ERROR("Another instance of hostapd is running", 0, 0, 0);
        *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
        return false;
      }
      device_type = QCMAP_MSGR_DEVICE_TYPE_GUEST_AP_V01;
      this->DeleteWLANConnectedDevicesClients(device_type);
    }
    break;
    case QCMAP_MSGR_PRIMARY_AND_GUEST_AP_V01:
    {
      if ( ap_pid == 0 && guest_ap_pid == 0 )
      {
        LOG_MSG_ERROR("Hostapd already stopped for Both AP's",
                      0, 0, 0);
        *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
        return false;
      }
      this->StopHostapdCli(ap_type);
      /* Stop the Hostapd for both the AP's. */
      StopHostAPD();

      if ( !IsHostapdkilled(0) )
      {
        LOG_MSG_ERROR("Another instance of hostapd is running", 0, 0, 0);
        *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
        return false;
      }
      device_type = QCMAP_MSGR_DEVICE_TYPE_PRIMARY_AP_V01;
      this->DeleteWLANConnectedDevicesClients(device_type);
      device_type = QCMAP_MSGR_DEVICE_TYPE_GUEST_AP_V01;
      this->DeleteWLANConnectedDevicesClients(device_type);
    }
    break;
  }

  return true;
}

/*===========================================================================
  FUNCTION ActivateHostapdActionRestart
==========================================================================*/
/*!
@brief
  Restarts Hostapd with the latest configuration.

@parameters
  void

@return
  true  - on Success
  false - on Failure
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::ActivateHostapdActionRestart
(
  qcmap_msgr_activate_hostapd_ap_enum_v01 ap_type,
  int ap_pid,
  int guest_ap_pid,
  qmi_error_type_v01 *qmi_err_num
)
{
  char command[MAX_COMMAND_STR_LEN];
  struct in_addr addr;
  int i, dev;
  char netmask[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char subnet[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  FILE *fp = NULL;
  qcmap_msgr_device_type_enum_v01 device_type = QCMAP_MSGR_DEVICE_TYPE_ENUM_MIN_ENUM_VAL_V01;

  LOG_MSG_INFO1("QCMAP_ConnectionManager::ActivateHostapdActionRestart", 0, 0, 0);

  switch ( ap_type )
  {
    case QCMAP_MSGR_PRIMARY_AP_V01:
    {
      this->StopHostapdCli(ap_type);
      /* Stop the process only if it is not already killed. */
      if ( ap_pid != 0 )
      {
        snprintf(command, MAX_COMMAND_STR_LEN, "kill %d", ap_pid);
        ds_system_call(command, strlen(command));
      }
      if ( !IsHostapdkilled(guest_ap_pid) )
      {
        LOG_MSG_ERROR("Another instance of hostapd is running", 0, 0, 0);
        *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
        return false;
      }
      if (this->wifi_mode == QCMAP_MSGR_WLAN_MODE_AP_STA_V01)
      {
        device_type = QCMAP_MSGR_DEVICE_TYPE_GUEST_AP_V01;
        this->DeleteWLANConnectedDevicesClients(device_type);
      }
      else
      {
        device_type = QCMAP_MSGR_DEVICE_TYPE_PRIMARY_AP_V01;
        this->DeleteWLANConnectedDevicesClients(device_type);
      }
       /* Restart Hostapd for Primary AP. */
      StartHostAPD(QCMAP_MSGR_INTF_AP_INDEX, this->ap_dev_num);
      addr.s_addr = htonl(this->cfg.lan_config.ll_subnet_mask);
      strlcpy(netmask, inet_ntoa(addr), 16);
      addr.s_addr = htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].ll_ip_addr & this->cfg.lan_config.ll_subnet_mask);
      strlcpy(subnet, inet_ntoa(addr), 16);
      snprintf(command, MAX_COMMAND_STR_LEN, "route del -net %s netmask %s dev wlan%d", subnet, netmask, this->ap_dev_num);
      ds_system_call(command, strlen(command));
    }
    break;
    case QCMAP_MSGR_GUEST_AP_V01:
    {
      this->StopHostapdCli(ap_type);

      /* Stop the process only if it not already killed. */
      if ( guest_ap_pid != 0 )
      {
        snprintf(command, MAX_COMMAND_STR_LEN, "kill %d", guest_ap_pid);
        ds_system_call(command, strlen(command));
      }
      if ( !IsHostapdkilled(ap_pid) )
      {
        LOG_MSG_ERROR("Another instance of hostapd is running", 0, 0, 0);
        *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
        return false;
      }
      device_type = QCMAP_MSGR_DEVICE_TYPE_GUEST_AP_V01;
      this->DeleteWLANConnectedDevicesClients(device_type);
       /* Restart Hostapd for GuestAP. */
      StartHostAPD(QCMAP_MSGR_INTF_GUEST_AP_INDEX, this->ap_dev_num);
      addr.s_addr = htonl(this->cfg.lan_config.ll_subnet_mask);
      strlcpy(netmask, inet_ntoa(addr), 16);
      addr.s_addr = htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].ll_ip_addr & this->cfg.lan_config.ll_subnet_mask);
      strlcpy(subnet, inet_ntoa(addr), 16);
      snprintf(command, MAX_COMMAND_STR_LEN, "route del -net %s netmask %s dev wlan%d", subnet, netmask, this->ap_dev_num+1);
      ds_system_call(command, strlen(command));
    }
    break;
    case QCMAP_MSGR_PRIMARY_AND_GUEST_AP_V01:
    {
      this->StopHostapdCli(ap_type);

      /* Stop Hostapd. */
      StopHostAPD();

      if ( !IsHostapdkilled(0) )
      {
        LOG_MSG_ERROR("Another instance of hostapd is running", 0, 0, 0);
        *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
        return false;
      }
      device_type = QCMAP_MSGR_DEVICE_TYPE_PRIMARY_AP_V01;
      this->DeleteWLANConnectedDevicesClients(device_type);
      device_type = QCMAP_MSGR_DEVICE_TYPE_GUEST_AP_V01;
      this->DeleteWLANConnectedDevicesClients(device_type);
      /* Bring up interfaces.
         This part needs to be done only for AP interfaces.
         Proceed only upto STA iface index since STA is always last in the
         list (ap; ap,ap; ap,sta; ap,ap,sta supported). */
      for ( i=0, dev = this->ap_dev_num; i < this->sta_iface_index; i++, dev++ )
      {
        if ( !this->cfg.lan_config.interface[i].enable )
          break;
        /* Start the HostAP daemon. */
        StartHostAPD(i, dev);
        addr.s_addr = htonl(this->cfg.lan_config.ll_subnet_mask);
        strlcpy(netmask, inet_ntoa(addr), 16);
        addr.s_addr = htonl(this->cfg.lan_config.interface[i].ll_ip_addr & this->cfg.lan_config.ll_subnet_mask);
        strlcpy(subnet, inet_ntoa(addr), 16);
        snprintf(command, MAX_COMMAND_STR_LEN, "route del -net %s netmask %s dev wlan%d", subnet, netmask, dev);
        ds_system_call(command, strlen(command));
      }
    }
    break;
  }

  return true;
}

/*===========================================================================
  FUNCTION ActivateSupplicantConfig
==========================================================================*/
/*!
@brief
  Restarts Supplicant with the latest configuration.

@parameters
  void

@return
  true  - on Success
  false - on Failure
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::ActivateSupplicantConfig
(
  qmi_error_type_v01 *qmi_err_num
)
{
  char command[MAX_COMMAND_STR_LEN];
  char temp_command[MAX_COMMAND_STR_LEN];

   LOG_MSG_INFO1("QCMAP_ConnectionManager::ActivateSupplicantConfig",0,0,0);

  if ( this->wifi_mode != QCMAP_MSGR_WLAN_MODE_AP_STA_V01 ||
       this->sta_iface_index != QCMAP_MSGR_INTF_STATION_INDEX )
  {
    LOG_MSG_ERROR("\n WLAN is in incorrect Mode: %d.\n", this->wifi_mode, 0, 0);
    *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
    return false;
  }

  /* Restart Supplicant as per supplicant config */
  LOG_MSG_INFO1("Restarting station iface %s with config as in: %s\n",this->cfg.lan_config.sta_interface,
                this->cfg.lan_config.interface[this->sta_iface_index].path_to_supplicant_conf,0);

  /* Kill any running instance */
  snprintf( command, MAX_COMMAND_STR_LEN, "killall -15 wpa_supplicant" );
  ds_system_call(command, strlen(command));

  if(!IsWpaSupplicantkilled())
  {
    /* Kill wpa_supplicant forcefully. */
    snprintf( command, MAX_COMMAND_STR_LEN, "killall -9 wpa_supplicant" );
    ds_system_call(command, strlen(command));
  }
  /* Run wpa_supplicant for sta interface with specified config */
  if( strncmp(this->cfg.lan_config.module,AR6003,strlen(AR6003)) == 0 )
  {
    snprintf( command, MAX_COMMAND_STR_LEN, "wpa_supplicant -i %s -Dar6003 -B -c %s",this->cfg.lan_config.sta_interface,
              this->cfg.lan_config.interface[this->sta_iface_index].path_to_supplicant_conf );
  }
  else
  {
    snprintf( command, MAX_COMMAND_STR_LEN, "wpa_supplicant -i %s -Dnl80211 -B -c %s",this->cfg.lan_config.sta_interface,
              this->cfg.lan_config.interface[this->sta_iface_index].path_to_supplicant_conf );
  }

  /* If debug mode is enabled add concat dbg cmd */
  if (this->cfg.debug_config.supplicant_debug)
  {
    snprintf(temp_command, MAX_COMMAND_STR_LEN, "%s %s", command, this->cfg.debug_config.supplicant_dbg_cmd);
    memcpy(command, temp_command, MAX_COMMAND_STR_LEN);
  }

  ds_system_call(command, strlen(command));

   /* Kill any wpa_cli running instance */
  snprintf( command, MAX_COMMAND_STR_LEN, "killall -15 wpa_cli" );
  ds_system_call(command, strlen(command));

  if(!IsWpaClikilled())
  {
    /* Kill wpa_cli forcefully. */
    snprintf( command, MAX_COMMAND_STR_LEN, "killall -9 wpa_cli" );
    ds_system_call(command, strlen(command));
  }

  /* Run wpa_cli */
  snprintf( command, MAX_COMMAND_STR_LEN, "wpa_cli -i %s -p %s -B -a /usr/bin/QCMAP_StaInterface",this->cfg.lan_config.sta_interface,
            WPA_CTRL_PATH);
  ds_system_call(command, strlen(command));

  return true;
}

/*===========================================================================
  FUNCTION ActivateLAN
==========================================================================*/
/*!
@brief
  Brings up the lan with the latest wlan configuration and mode.

@parameters
  admin - If set to true forcefully re-activate LAN, even if no change in cfg.

@return
  true  - on Success
  false - on Failure
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::ActivateLAN
(
  qmi_error_type_v01 *qmi_err_num,
  boolean admin
)
{
   LOG_MSG_INFO1("QCMAP_ConnectionManager::ActivateLAN",0,0,0);
   boolean usb_enable = false;
   boolean wlan_enable = false;
   char usb_intf_name [DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
   char command[MAX_COMMAND_STR_LEN];
   char a5_ip[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
   char netmask[QCMAP_LAN_MAX_IPV4_ADDR_SIZE], subnet[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
   char usb_gw_ip[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
   struct in_addr addr;

   // Wlan related addresses
   addr.s_addr = htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].ll_ip_addr);
   strlcpy(a5_ip, inet_ntoa(addr), 16);
   addr.s_addr = htonl(this->cfg.lan_config.ll_subnet_mask);
   strlcpy(netmask, inet_ntoa(addr), 16);

   // USB related link local address
  addr.s_addr = htonl(this->cfg.lan_config.usb_conf.gateway_addr);
  strlcat(usb_gw_ip, inet_ntoa(addr), 16);

   // Sanity check to ensure that we only proceed if either the mode or AP configuration has changed
   // Added check to ensure when in AP-STA bridge mode; ActivateLAN is also called from DHCP Fail Case
  if (!IsLanCfgUpdated() && !admin) {
    LOG_MSG_INFO1(" There is no change in the LAN Config",0,0,0);
    return true;
  }
  bzero(usb_intf_name,sizeof(usb_intf_name));
  // AP Configuration has changed and Bridge interface was up, need to apply the new configuration.
  // This will also take care if the mode has changed
  if (this->bridge_inited) {
     // Bring down USB TE
     if ( GetUSBEnable()) {
         this->set_activate_lan_in_progress(true);
         usb_enable = true;
         ds_system_call("echo 0 > /sys/class/android_usb/android0/enable ",strlen("echo 0 > /sys/class/android_usb/android0/enable "));
         if ( this->cfg.lan_config.usb_conf.link_type == QCMAP_QTI_USB_LINK_RNDIS ) {
             strlcpy(usb_intf_name, "rndis0",sizeof(usb_intf_name));
         }  else if ( this->cfg.lan_config.usb_conf.link_type == QCMAP_QTI_USB_LINK_ECM ) {
             strlcpy(usb_intf_name, "ecm0",sizeof(usb_intf_name));
         } else {
            LOG_MSG_ERROR("Incorrect USB LINK Detected Link Type = %x", this->cfg.lan_config.usb_conf.link_type,0,0);
         }
         snprintf( command, MAX_COMMAND_STR_LEN, "ifconfig %s down",usb_intf_name);
         ds_system_call(command, strlen(command));
         snprintf( command, MAX_COMMAND_STR_LEN, "brctl delif %s %s",BRIDGE_IFACE,usb_intf_name);
         ds_system_call(command, strlen(command));
     }
     else if ( this->cfg.lan_config.usb_conf.ppp_enabled )
     {
       /* Kill the PPP Daemon. Right now there is no way we can make PPP to renegotiate. */
       ds_system_call("killall -TERM pppd", strlen("killall -TERM pppd"));
     }

     // Disable WLAN if we are not in STA mode and WLAN is enabled
     // Also Disable WLAN if we are in STA mode and admin=true; DHCP Fail case
     if ( (this->wifi_mode  == QCMAP_MSGR_WLAN_MODE_AP_V01) ||
          (this->wifi_mode  == QCMAP_MSGR_WLAN_MODE_AP_AP_V01) ||
          (admin == true)) {
        wlan_enable = true;
        this->DisableWLAN(qmi_err_num);
     } else if (this->wifi_mode  == QCMAP_MSGR_WLAN_MODE_AP_STA_V01) {
        wlan_enable = true;
        /* Disable WLAN and remove wlan from bridge Iface */
        snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig wlan%d down",this->ap_dev_num);
        ds_system_call(command, strlen(command));
        StopHostAPD();
        snprintf( command, MAX_COMMAND_STR_LEN, "brctl delif %s wlan%d",BRIDGE_IFACE,this->ap_dev_num);
        ds_system_call(command,strlen(command));
     }

     // Disable Bridge force-fully.
     if (this->bridge_inited) {
        LOG_MSG_INFO1(" Delete the bridge interface. forcefully \n",0,0,0);
        this->DelBridge();
     }

     this->InitBridge();

     // Enable WLAN
     if (wlan_enable) {
        if ((this->wifi_mode  != QCMAP_MSGR_WLAN_MODE_AP_STA_V01) ||
            (admin == true)) {
           this->EnableWLAN(qmi_err_num);
        } else {
           // Start Hostapd and add wlan to bridge iface
           StartHostAPD(QCMAP_MSGR_INTF_AP_INDEX, this->ap_dev_num);
           snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig wlan%d %s netmask %s up",this->ap_dev_num, a5_ip, netmask);
           ds_system_call(command, strlen(command));
           addr.s_addr = htonl(this->cfg.lan_config.interface[this->ap_dev_num].ll_ip_addr & this->cfg.lan_config.ll_subnet_mask);
           strlcpy(subnet, inet_ntoa(addr), 16);
           snprintf(command, MAX_COMMAND_STR_LEN, "route del -net %s netmask %s dev wlan%d", subnet, netmask, this->ap_dev_num);
           ds_system_call(command, strlen(command));
           snprintf( command, MAX_COMMAND_STR_LEN, "brctl addif %s wlan%d",BRIDGE_IFACE,this->ap_dev_num);
           ds_system_call(command,strlen(command));
        }
     }

     // Enable USB if it was enabled before
     if (usb_enable) {
       ds_system_call("echo 1 > /sys/class/android_usb/android0/enable ",strlen("echo 1 > /sys/class/android_usb/android0/enable "));
       snprintf( command, MAX_COMMAND_STR_LEN, "ifconfig %s %s netmask %s up",usb_intf_name,usb_gw_ip,netmask);
       ds_system_call(command, strlen(command));
       snprintf( command, MAX_COMMAND_STR_LEN, "brctl addif %s %s",BRIDGE_IFACE,usb_intf_name);
       ds_system_call(command, strlen(command));
     }
  }

  return true;
}

/*===========================================================================
  FUNCTION EnableStaMode
==========================================================================*/
/*!
@brief
  Enables station mode, as a part of this brings up wpa_supplicant.
  wpa_supplicant runs in the background and acts as the backend component controlling
  the wireless connection.

@parameters
  void

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::EnableStaMode()
{
  char command[MAX_COMMAND_STR_LEN];
  char temp_command[MAX_COMMAND_STR_LEN];
  char netmask[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char subnet[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char a5_ip[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  struct in_addr addr;

  LOG_MSG_INFO1("Entering EnableStaMode",0,0,0);

  /* Cant continue if MobileAP is not enabled! */
  if ( !this->qcmap_enable )
  {
    LOG_MSG_ERROR("Error: QCMAP not enabled.\n",0,0,0);
    return false;
  }

  if ( this->wifi_mode == QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01 )
  {
    LOG_MSG_ERROR("\n WLAN is in disabled state.\n",0,0,0);
    return false;
  }

  if ( this->wifi_mode != QCMAP_MSGR_WLAN_MODE_AP_STA_V01 ||
       this->sta_iface_index != QCMAP_MSGR_INTF_STATION_INDEX )
  {
    LOG_MSG_ERROR("\n WLAN is in incorrect Mode.\n",0,0,0);
    return false;
  }

  /* Connect STA to specified hotspot as per supplicant config */
  LOG_MSG_INFO1("Connecting station iface %s with config as in: %s\n",this->cfg.lan_config.sta_interface,
                this->cfg.lan_config.interface[this->sta_iface_index].path_to_supplicant_conf,0);

  /* Validate config file path */
  if( strnlen( this->cfg.lan_config.interface[this->sta_iface_index].path_to_supplicant_conf,
               QCMAP_CM_MAX_FILE_LEN ) == 0 )
  {
    LOG_MSG_ERROR("Error: External AP SSID length 0\n", 0, 0, 0);
    return false;
  }

  /* Enable 4Addr Scheme and start Associtation timer if AP-STA is configured in Bridge Mode*/
  /* Stop DHCP Server and briing up WLAN-STA interface with Guest-LL configuration*/
  /* We will restart dnsmasq as a dhcprelay on we get IP address from EXTAP */
  /* Add WLAN-STA interface to Linux Bridge*/
  if (this->IsAPSTABridgeActivated())
  {
    ds_system_call("iw dev wlan0 set 4addr on",strlen("iw dev wlan0 set 4addr on"));
    this->StopDHCPD();
    /* Stop Radish */
    StopRadish();
    addr.s_addr = htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].ll_ip_addr);
    strlcpy(a5_ip, inet_ntoa(addr), 16);
    addr.s_addr = htonl(this->cfg.lan_config.ll_subnet_mask);
    strlcpy(netmask, inet_ntoa(addr), 16);
    snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig %s %s netmask %s up",this->cfg.lan_config.sta_interface, a5_ip, netmask);
    ds_system_call(command, strlen(command));
    addr.s_addr = htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_GUEST_AP_INDEX].ll_ip_addr & this->cfg.lan_config.ll_subnet_mask);
    strlcpy(subnet, inet_ntoa(addr), 16);
    snprintf(command, MAX_COMMAND_STR_LEN, "route del -net %s netmask %s dev %s", subnet, netmask, this->cfg.lan_config.sta_interface);
    ds_system_call(command, strlen(command));
    snprintf(command, MAX_COMMAND_STR_LEN, "ip -6 route del fe80::/64 dev %s",this->cfg.lan_config.sta_interface);
    ds_system_call(command, strlen(command));
  }


  /* Kill any running instance */
  snprintf( command, MAX_COMMAND_STR_LEN, "killall -15 wpa_supplicant" );
  ds_system_call(command, strlen(command));
  snprintf( command, MAX_COMMAND_STR_LEN, "killall -15 wpa_cli" );
  ds_system_call(command, strlen(command));

  /* Run wpa_supplicant for sta interface with specified config */
  if( strncmp(this->cfg.lan_config.module,AR6003,strlen(AR6003)) == 0 )
  {
    snprintf( command, MAX_COMMAND_STR_LEN, "wpa_supplicant -i %s -Dar6003 -B -c %s",this->cfg.lan_config.sta_interface,
              this->cfg.lan_config.interface[this->sta_iface_index].path_to_supplicant_conf );
  }
  else
  {
    if (this->IsAPSTABridgeActivated())
    {
       snprintf( command, MAX_COMMAND_STR_LEN, "wpa_supplicant -b %s -i %s -Dnl80211 -B -c %s", BRIDGE_IFACE,
                 this->cfg.lan_config.sta_interface, this->cfg.lan_config.interface[this->sta_iface_index].path_to_supplicant_conf);
    } else {
       snprintf( command, MAX_COMMAND_STR_LEN, "wpa_supplicant -i %s -Dnl80211 -B -c %s",this->cfg.lan_config.sta_interface,
                 this->cfg.lan_config.interface[this->sta_iface_index].path_to_supplicant_conf );
    }
  }

  /* Start STA Association timer if we are in AP-STA Bridge Mode*/
  if (this->IsAPSTABridgeActivated())
  {
    start_wlan_sta_timer(STA_ASSOC_FAIL);
    LOG_MSG_INFO1("AP+STA BRIDGE Mode Activated",0,0,0);
  }
  else
  {
    LOG_MSG_INFO1("AP+STA ROUTER Mode Activated",0,0,0);
  }

  /* If debug mode is enabled add concat dbg cmd */
  if (this->cfg.debug_config.supplicant_debug)
  {
    snprintf(temp_command, MAX_COMMAND_STR_LEN, "%s %s", command, this->cfg.debug_config.supplicant_dbg_cmd);
    memcpy(command, temp_command, MAX_COMMAND_STR_LEN);
  }

  ds_system_call(command, strlen(command));

  /* Run wpa_cli */
  snprintf( command, MAX_COMMAND_STR_LEN, "wpa_cli -i %s -p %s -B -a /usr/bin/QCMAP_StaInterface",this->cfg.lan_config.sta_interface,
            WPA_CTRL_PATH);
  ds_system_call(command, strlen(command));

  return true;

}

/*===========================================================================
  FUNCTION DisableStaMode
==========================================================================*/
/*!
@brief
  Disables station mode, as a part of this disables NAT.

@parameters
  void

@return
  true  - on Success
  false - on Failure
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::DisableStaMode()
{
  char command[MAX_COMMAND_STR_LEN];

  LOG_MSG_INFO1("Entering DisableStaMode",0,0,0);

  /* Cant continue if MobileAP is not enabled! */
  if (!this->qcmap_enable)
  {
    LOG_MSG_ERROR("Error: QCMAP not enabled.\n",0,0,0);
    return false;
  }

  if ( this->wifi_mode == QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01 )
  {
    LOG_MSG_ERROR("\n WLAN is in disabled state.\n",0,0,0);
    return false;
  }

  if ( this->wifi_mode != QCMAP_MSGR_WLAN_MODE_AP_STA_V01 ||
       this->sta_iface_index != QCMAP_MSGR_INTF_STATION_INDEX )
  {
    LOG_MSG_ERROR("\n WLAN is in incorrect Mode.\n",0,0,0);
    return false;
  }

  if (!this->cradle_backhaul_connected)
  {
    /* Disable NAT on A5. */
    this->DisableNATonA5();

    /* Disable IPV6 Forwarding. */
    this->DisableIPV6Forwarding(true);
  }

  /* Disconnect STA from hotspot. */
  this->DisconnectSTA();

  /* Reset STA connected and associated flags. */
  this->sta_connected = false;
  this->sta_associated = false;

  return true;

}

/*===========================================================================
  FUNCTION InStaMode
==========================================================================*/
/*!
@brief
  Indicate if station mode is enabled or disabled.

@parameters
  void

@return
  true  - if station mode is on
  false - if station mode is not configured.
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::InStaMode()
{
  if ( this->sta_connected )
  {
    return true;
  }
  else
  {
    return false;
  }
}


/*===========================================================================
 FUNCTION EnterDefaultMACRulesForBridgeMode
==========================================================================*/
/*!
@brief
  Clean EbTables and IPtables default rules for AP-STA Bridge Mode,
  related to MAC address

@parameters
  void

@return
   void

@note

  - Dependencies
  - None

  - Side Effects
  - None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::EnterDefaultMACRulesForBridgeMode(void)
{
  char command[MAX_COMMAND_STR_LEN];
  LOG_MSG_INFO1("EnterDefaultRulesForBridgeMode()",0,0,0);

  /* Enter Rules to Perform MAC-SNAT to ensure WIFI bridge mode works fine with 3-addr scheme */
  snprintf(command, MAX_COMMAND_STR_LEN,"ebtables -t nat -A POSTROUTING -o %s -j snat --to-source %02x:%02x:%02x:%02x:%02x:%02x",
                   this->cfg.lan_config.sta_interface, this->ap_sta_bridge.sta_mac[0],this->ap_sta_bridge.sta_mac[1],this->ap_sta_bridge.sta_mac[2],
                   this->ap_sta_bridge.sta_mac[3], this->ap_sta_bridge.sta_mac[4], this->ap_sta_bridge.sta_mac[5]);
  ds_system_call(command, strlen(command));
  snprintf(command, MAX_COMMAND_STR_LEN,"ebtables -t nat -A POSTROUTING -o wlan%d -j snat --to-source %02x:%02x:%02x:%02x:%02x:%02x",
                   this->ap_dev_num, this->ap_sta_bridge.ap_mac[0],this->ap_sta_bridge.ap_mac[1],this->ap_sta_bridge.ap_mac[2],
                   this->ap_sta_bridge.ap_mac[3], this->ap_sta_bridge.ap_mac[4], this->ap_sta_bridge.ap_mac[5]);
  ds_system_call(command, strlen(command));
}

/*===========================================================================
  FUNCTION SendRSOnBridgeIface
==========================================================================*/
/*!
@brief
  Generate a RS Message on bridge iface.

@parameters
  void

@return
  void

@note

 - Dependencies
 - None

 - Side Effects
 - None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::SendRSOnBridgeIface(void)
{
  int ret;
  struct nd_router_solicit rs;
  struct sockaddr_in6 dst_addr;
  LOG_MSG_INFO1("SendRSOnBridgeIface",0,0,0);

  memset(&dst_addr, 0, sizeof(dst_addr));
  memset(&rs, 0, sizeof(nd_router_solicit));

  // Populate icmp6_hdr portion of router solicit struct.
  rs.nd_rs_hdr.icmp6_type = ND_ROUTER_SOLICIT;  // 133 (RFC 4861)
  rs.nd_rs_hdr.icmp6_code = 0;              // zero for router solicitation (RFC 4861)
  rs.nd_rs_hdr.icmp6_cksum = htons(0);      // zero when calculating checksum

  inet_pton(AF_INET6, "ff02::1", &dst_addr.sin6_addr);
  dst_addr.sin6_family = AF_INET6;

  if ((ret = sendto(this->bridge_sock, &rs, sizeof(nd_router_solicit), 0,
                           (struct sockaddr *)&dst_addr,sizeof(dst_addr))) < 0)
  {
    LOG_MSG_ERROR("Error: Cannot send RS on Bridge IFace error: %d.\n", errno, 0, 0);
  }
}

/*===========================================================================
  FUNCTION EnterDefaultIPRulesForBridgeMode
==========================================================================*/
/*!
@brief
  Clean EbTables and IPtables default IP based for rules AP-STA Bridge Mode

@parameters
  void

@return
  void

@note

 - Dependencies
 - None

 - Side Effects
 - None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::EnterDefaultIPRulesForBridgeMode(void)
{
  char command[MAX_COMMAND_STR_LEN];
  char usb_intf_name [DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  char netmask [QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  LOG_MSG_INFO1("EnterDefaultIPRulesForBridgeMode()",0,0,0);

  /* Since we want bridge as a proxy for STA iface. Ensure all arp-requests are replied by Bridge and not forwarded*/
  snprintf(command, MAX_COMMAND_STR_LEN,"ebtables -t filter -A FORWARD -i %s -p ARP -j DROP", this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));
  snprintf(command, MAX_COMMAND_STR_LEN,"ebtables -t filter -A FORWARD -o %s -p ARP -j DROP", this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));

  /* Ensure DHCP-ARP DAD works fine */
  snprintf(command, MAX_COMMAND_STR_LEN,"ebtables -t nat -I PREROUTING -p arp --arp-ip-src=0.0.0.0 --arp-opcode Request -j ACCEPT");
  ds_system_call(command, strlen(command));

  /* Ensure we dont reply to gratitious ARP requests */
  snprintf(command, MAX_COMMAND_STR_LEN,"ebtables -t nat -I PREROUTING -p arp --arp-opcode Request --arp-gratuitous -j ACCEPT");
  ds_system_call(command, strlen(command));

  strlcpy(netmask, inet_ntoa(this->ap_sta_bridge.bridge_netmask), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);

  /* Default Subnet Based ARP Reply Rules for WLAN and USB Interface*/
  snprintf(command, MAX_COMMAND_STR_LEN,
                   "ebtables -t nat -A PREROUTING -i wlan%d -p arp --arp-ip-src=%s/%s --arp-opcode Request -j arpreply --arpreply-mac %02x:%02x:%02x:%02x:%02x:%02x --arpreply-target ACCEPT",
                   this->ap_dev_num, inet_ntoa(this->ap_sta_bridge.bridge_def_gw), netmask, this->ap_sta_bridge.bridge_mac[0], this->ap_sta_bridge.bridge_mac[1],
                   this->ap_sta_bridge.bridge_mac[2], this->ap_sta_bridge.bridge_mac[3], this->ap_sta_bridge.bridge_mac[4], this->ap_sta_bridge.bridge_mac[5]);
  ds_system_call(command, strlen(command));
  if ( GetUSBEnable())
  {
    if ( this->cfg.lan_config.usb_conf.link_type == QCMAP_QTI_USB_LINK_RNDIS )
    {
      strlcpy(usb_intf_name, "rndis0",16);
    }
    else if ( this->cfg.lan_config.usb_conf.link_type == QCMAP_QTI_USB_LINK_ECM )
    {
      strlcpy(usb_intf_name, "ecm0",16);
    }
    else
    {
      LOG_MSG_ERROR("Incorrect USB LINK Detected Link Type = %x", this->cfg.lan_config.usb_conf.link_type,0,0);
      return;
    }
    snprintf(command, MAX_COMMAND_STR_LEN,
                     "ebtables -t nat -A PREROUTING -i %s -p arp --arp-ip-src=%s/%s --arp-opcode Request -j arpreply --arpreply-mac %02x:%02x:%02x:%02x:%02x:%02x --arpreply-target ACCEPT",
                     usb_intf_name, inet_ntoa(this->ap_sta_bridge.bridge_def_gw), netmask, this->ap_sta_bridge.bridge_mac[0], this->ap_sta_bridge.bridge_mac[1],
                     this->ap_sta_bridge.bridge_mac[2], this->ap_sta_bridge.bridge_mac[3], this->ap_sta_bridge.bridge_mac[4], this->ap_sta_bridge.bridge_mac[5]);
    ds_system_call(command, strlen(command));
  }
}

/*===========================================================================
  FUNCTION GetHWAddr
==========================================================================*/
/*!
@brief
  Get Mac address for an interface

@parameters
   Char * to store retrieved MAC Address
   devname name of the iface

@return
   QCMAP_CM_SUCCESS
   QCMAP_CM_ERROR
@note

  - Dependencies
  - None

  - Side Effects
 - None
*/
/*=========================================================================*/
static int GetHWAddr(char *mac,char* devname)
{
  int s, ret;
  struct ifreq buffer;
  struct sockaddr *sa;

  LOG_MSG_INFO1("GetHWAddr",0,0,0);

  if ( mac == NULL || devname == NULL )
  {
    LOG_MSG_ERROR("Null arguements passed.\n",0,0,0);
    return QCMAP_CM_ERROR;
  }

  /* Open a socket */
  s = socket(PF_INET, SOCK_DGRAM, 0);
  if ( s < 0 )
  {
    LOG_MSG_ERROR("Unable to open socket to get IP address.\n",0,0,0);
    return false;
  }
  /* Set up the interface request buffer for wlan0. */
  memset(&buffer, 0x00, sizeof(buffer));
  strlcpy(buffer.ifr_name, devname, IFNAMSIZ);

  /* Call the ioctl to get the address. */
  ret = ioctl(s, SIOCGIFHWADDR, &buffer);

  if ( ret < 0 )
  {
    LOG_MSG_ERROR("Unable to call ioctl to get MAC address.\n",0,0,0);
    /* Close the socket handle. */
    close(s);
    return false;
  }

  /* Copy out the ip address for the interface. */
  sa = (struct sockaddr *)&(buffer.ifr_addr);
  memcpy(mac, buffer.ifr_hwaddr.sa_data, ETH_ALEN);

  ds_log_med("HW Address for dev %s is %02x:%02x:%02x:%02x:%02x:%02x \n",
                  devname, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  close(s);
  return QCMAP_CM_SUCCESS;
}

/*===========================================================================
 FUNCTION ProcessStaAssocForWLANBridge
==========================================================================*/
/*!
@brief
  Brings up the station mode interface.

@parameters
  cb_user_data

@return
  void
@note

 - Dependencies
 - None

 - Side Effects
 - None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::ProcessStaAssocForWLANBridge(void *cb_user_data)
{
  char command[MAX_COMMAND_STR_LEN];
  in_addr addr, staIp, staMask, staGwIp;
  boolean ret = false;
  qmi_error_type_v01 qmi_err_num;
  int i = 0;
  char usb_intf_name [DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  char ap_intf_name [DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  char usb_gw_ip[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char netmask[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char subnet[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];

  LOG_MSG_INFO1("Entering ProcessStaAssocForWLANBridge",0,0,0);

  /* Cant continue if MobileAP is not enabled! */
  if (!this->qcmap_enable)
  {
    LOG_MSG_ERROR("Error: QCMAP not enabled.\n",0,0,0);
    return;
  }

  if ( this->wifi_mode == QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01 )
  {
    LOG_MSG_ERROR("\n WLAN is in disabled state.\n",0,0,0);
    return;
  }

  if ( this->wifi_mode != QCMAP_MSGR_WLAN_MODE_AP_STA_V01 ||
       this->sta_iface_index != QCMAP_MSGR_INTF_STATION_INDEX )
  {
    LOG_MSG_ERROR("\n WLAN is in incorrect Mode.\n",0,0,0);
    return;
  }

  // USB related link local address
  addr.s_addr = htonl(this->cfg.lan_config.usb_conf.gateway_addr);
  strlcat(usb_gw_ip, inet_ntoa(addr), 16);
  addr.s_addr = htonl(this->cfg.lan_config.ll_subnet_mask);
  strlcpy(netmask, inet_ntoa(addr), 16);

  /* Stop WLAN-STA Assoc Timer*/
  stop_wlan_sta_timer(STA_ASSOC_FAIL);

  /* Switch to STA interface. */
  if ( qcmap_cm_get_state(&qmi_err_num) == QCMAP_CM_WAN_CONNECTED )
  {
    this->DisableNATonA5();
    this->FlushIPV4Firewall();
  }
  if ( qcmap_cm_get_ipv6_state(&qmi_err_num) == QCMAP_CM_V6_WAN_CONNECTED )
  {
    this->DisableIPV6Forwarding(true);
    this->FlushIPV6Firewall();
  }

  /* Clean All Firewall/Filtering Rules */
  CleanIPtables();
  CleanEbtables();

  ds_system_call(command, strlen(command));
  snprintf(command, MAX_COMMAND_STR_LEN, "brctl addif %s %s",
           BRIDGE_IFACE, this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));

  snprintf(ap_intf_name, (DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2), "wlan%d", this->ap_dev_num);
  /*Get all HW Address */
  if ((GetHWAddr(ap_sta_bridge.bridge_mac, BRIDGE_IFACE) != QCMAP_CM_SUCCESS) ||
      (GetHWAddr(ap_sta_bridge.ap_mac, ap_intf_name) != QCMAP_CM_SUCCESS)     ||
      (GetHWAddr(ap_sta_bridge.sta_mac, this->cfg.lan_config.sta_interface) != QCMAP_CM_SUCCESS))
  {
    LOG_MSG_ERROR("MAC Address Get returned error on AP-STA Bridge Mode. Cannot proceed further \n",0,0,0);
    return;
  }

  /* Enter Generic rules Ebtables/IPtables rules for AP-STA in Bridge mode*/
  this->EnterDefaultMACRulesForBridgeMode();

  /* Delete old IP address and default route*/
  snprintf(command, MAX_COMMAND_STR_LEN, "ip addr flush dev %s", BRIDGE_IFACE);
  ds_system_call(command, strlen(command));

  /* Connect to the hotspot. */
  if ( this->cfg.lan_config.interface[this->sta_iface_index].conn_type ==
       QCMAP_MSGR_STA_CONNECTION_DYNAMIC_V01 )
  {
    LOG_MSG_INFO1("Dynamic IP Assignment Settings for %s\n",this->cfg.lan_config.sta_interface,0,0);

    /* Kill any stale dhcpcd intance */
    LOG_MSG_INFO1("Killing previous dhcpcd process.\n",0,0,0);
    snprintf( command, MAX_COMMAND_STR_LEN, "killall -15 dhcpcd" );
    ds_system_call(command, strlen(command));
    /* Clean the PIDs */
    snprintf( command, MAX_COMMAND_STR_LEN, "rm -rf /var/run/dhcpcd-%s.pid",this->cfg.lan_config.sta_interface);
    ds_system_call(command, strlen(command));
    snprintf( command, MAX_COMMAND_STR_LEN, "rm -rf /var/run/dhcpcd-%s.pid",BRIDGE_IFACE);
    ds_system_call(command, strlen(command));
    /* Clean the leases */
    snprintf( command, MAX_COMMAND_STR_LEN, "rm -rf /var/db/dhcpcd-%s.lease",this->cfg.lan_config.sta_interface);
    ds_system_call(command, strlen(command));
    snprintf( command, MAX_COMMAND_STR_LEN, "rm -rf /var/db/dhcpcd-%s.lease",BRIDGE_IFACE);
    ds_system_call(command, strlen(command));

    /* Obtain and assign IP address via dhcpcd daemon */
    LOG_MSG_INFO1("Running DHCP client on %s\n",BRIDGE_IFACE,0,0);
    snprintf( command, MAX_COMMAND_STR_LEN, "dhcpcd %s -t 0 -o domain_name_servers --noipv4ll -b",BRIDGE_IFACE);
    ds_system_call(command, strlen(command));

    /* Start DHCP Timer */
    if (QCMAP_CM_SUCCESS == start_wlan_sta_timer(STA_DHCP_FAIL))
    {
      LOG_MSG_INFO1("Timer Started for STA DHCP IP assignment",0,0,0);
    }
    else
    {
      LOG_MSG_ERROR("Starting DHCP IP Assignment Timer Failed",0,0,0);
    }
    this->sta_associated = true;
  }
  else /* If Static Configuration Provided */
  {
    if (cb_user_data == NULL)
    {
      LOG_MSG_ERROR("ProcessStaAssoc: NULL handle passed",0,0,0);
      return;
    }
    LOG_MSG_INFO1("Bringing up STA iface %s with static ip configuration\n",BRIDGE_IFACE,0,0);
    staIp.s_addr = htonl(this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.ip_addr);
    staMask.s_addr = htonl(this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.netmask);
    staGwIp.s_addr = htonl(this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.gw_ip);
    /* Bring the interface UP. */
    memset(netmask,0,QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
    strlcpy(netmask, inet_ntoa(staMask), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
    snprintf( command, MAX_COMMAND_STR_LEN,
              "ifconfig %s %s netmask %s up",BRIDGE_IFACE, inet_ntoa(staIp), netmask);
    ds_system_call(command, strlen(command));
    /* Add the default route. */
    snprintf( command, MAX_COMMAND_STR_LEN,
              "route add default gw %s dev %s", inet_ntoa(staGwIp),BRIDGE_IFACE);
    ds_system_call(command, strlen(command));
    /* Populate the /etc/resolv.conf file with DNS address provided. */
    addr.s_addr = htonl(this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.dns_addr);
    snprintf(command, MAX_COMMAND_STR_LEN,
             "echo 'nameserver %s' > /etc/resolv.conf", inet_ntoa(addr));
    ds_system_call(command, strlen(command));
    this->sta_associated = true;
    this->ProcessAddrAssign(cb_user_data,BRIDGE_IFACE);
  }
  return;
}

/*===========================================================================
  FUNCTION ProcessStaAssoc
==========================================================================*/
/*!
@brief
  Brings up the station mode interface.

@parameters
  cb_user_data

@return
  void
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

void QCMAP_ConnectionManager::ProcessStaAssoc(void *cb_user_data)
{
  char command[MAX_COMMAND_STR_LEN];
  char netMask[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  in_addr addr, staIp, staMask, staGwIp;
  boolean ret = false;
  qmi_error_type_v01 qmi_err_num;
  int i = 0;

  LOG_MSG_INFO1("Entering ProcessStaAssoc",0,0,0);

  /* Cant continue if MobileAP is not enabled! */
  if (!this->qcmap_enable)
  {
    LOG_MSG_ERROR("Error: QCMAP not enabled.\n",0,0,0);
    return;
  }

  if ( this->wifi_mode == QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01 )
  {
    LOG_MSG_ERROR("\n WLAN is in disabled state.\n",0,0,0);
    return;
  }

  if ( this->wifi_mode != QCMAP_MSGR_WLAN_MODE_AP_STA_V01 ||
       this->sta_iface_index != QCMAP_MSGR_INTF_STATION_INDEX )
  {
    LOG_MSG_ERROR("\n WLAN is in incorrect Mode.\n",0,0,0);
    return;
  }
  /* First delete duplicate rule if any. */
  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -D FORWARD -i %s  -o %s -j DROP", BRIDGE_IFACE,
            this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -A FORWARD -i %s  -o %s -j DROP", BRIDGE_IFACE,
            this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));

  /* Connect to the hotspot. */
  if ( this->cfg.lan_config.interface[this->sta_iface_index].conn_type ==
       QCMAP_MSGR_STA_CONNECTION_DYNAMIC_V01 )
  {
    LOG_MSG_INFO1("Bringing up STA iface %s\n",this->cfg.lan_config.sta_interface,0,0);
    /* Bringup interface */
    snprintf( command, MAX_COMMAND_STR_LEN, "ifconfig %s up",this->cfg.lan_config.sta_interface);
    ds_system_call(command, strlen(command));

    /* Kill any stale dhcpcd intance */
    LOG_MSG_INFO1("Killing previous dhcpcd process.\n",0,0,0);
    snprintf( command, MAX_COMMAND_STR_LEN, "kill -15 $(cat /var/run/dhcpcd-%s.pid)", this->cfg.lan_config.sta_interface);
    ds_system_call(command, strlen(command));

    snprintf( command, MAX_COMMAND_STR_LEN, "rm -rf /var/run/dhcpcd-%s.pid",this->cfg.lan_config.sta_interface);
    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("%s\n", command,0,0);

    /* Obtain and assign IP address via dhcpcd daemon */
    LOG_MSG_INFO1("Running DHCP client on %s\n",this->cfg.lan_config.sta_interface,0,0);
    snprintf( command, MAX_COMMAND_STR_LEN, "dhcpcd %s -t 0 -o domain_name_servers --noipv4ll -b",this->cfg.lan_config.sta_interface);
    ds_system_call(command, strlen(command));

    this->sta_associated = true;
  }

  else
  {
    if (cb_user_data == NULL)
    {
      LOG_MSG_ERROR("ProcessStaAssoc: NULL handle passed",0,0,0);
      return;
    }
    LOG_MSG_INFO1("Bringing up STA iface %s with static ip configuration\n",this->cfg.lan_config.sta_interface,0,0);
    staIp.s_addr = htonl(this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.ip_addr);
    staMask.s_addr = htonl(this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.netmask);
    staGwIp.s_addr = htonl(this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.gw_ip);
    /* Bring the interface UP. */
    memset(netMask,0,QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
    strlcpy(netMask, inet_ntoa(staMask), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
    snprintf( command, MAX_COMMAND_STR_LEN,
              "ifconfig %s %s netmask %s up",this->cfg.lan_config.sta_interface, inet_ntoa(staIp), netMask);
    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("%s\n", command,0,0);
    /* Add the default route, if not in cralde mode. */
    if (!this->cradle_backhaul_connected)
    {
      snprintf( command, MAX_COMMAND_STR_LEN,
                "route add default gw %s dev %s", inet_ntoa(staGwIp),this->cfg.lan_config.sta_interface);
      ds_system_call(command, strlen(command));
      LOG_MSG_INFO1("%s\n", command,0,0);
    }
    /* Populate the /etc/resolv.conf file with DNS address provided. */
    addr.s_addr = htonl(this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.dns_addr);
    snprintf(command, MAX_COMMAND_STR_LEN,
             "echo 'nameserver %s' > /etc/resolv.conf", inet_ntoa(addr));
    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("%s\n", command,0,0);

    this->sta_associated = true;
    this->ProcessAddrAssign(cb_user_data,this->cfg.lan_config.sta_interface);
  }
  return;
}

/*===========================================================================
 FUNCTION ProcessStaAssocFail
==========================================================================*/
/*!
@brief
  Process Association Fail timer and provide indication to clients.
  This event only occurs for AP-STA bridge mode.

@parameters
  cb_user_data

@return
  void
@note

 - Dependencies
 - None

 - Side Effects
 - None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::ProcessStaAssocFail(void *cb_user_data)
{
  qmi_qcmap_msgr_softap_handle_type *softApHandle = NULL;
  qmi_qcmap_msgr_status_cb_data *cbPtr = NULL;
  qmi_error_type_v01 qmi_err_num;

  LOG_MSG_INFO1("QCMAP_ConnectionManager::ProcessStaAssocFail()",0,0,0);

  softApHandle = (qmi_qcmap_msgr_softap_handle_type *) cb_user_data;
  if (softApHandle != NULL)
  {
    cbPtr = (qmi_qcmap_msgr_status_cb_data *) softApHandle->cb_ptr;
  }
  else
  {
    LOG_MSG_ERROR("ProcessStaAssocFail() : NULL handle passed",0,0,0);
    return;
  }

  /*Come back to AP+STA Router mode mode*/
  LOG_MSG_INFO1("Going back to AP+STA Router Mode since Association failed in AP+STA Bridge mode",0,0,0);
  this->revert_ap_sta_router_mode = true;
  if(this->ActivateWLAN(&qmi_err_num) == false)
  {
    LOG_MSG_ERROR("Activate WLAN Failed",0,0,0);
  }

  /* Switch to backhaul mode. */
  if ( qcmap_cm_get_state(&qmi_err_num) == QCMAP_CM_WAN_CONNECTED && !this->cradle_backhaul_connected )
  {
    this->EnableNATonA5();
    this->EnableDNS();
  }
  if ( qcmap_cm_get_ipv6_state(&qmi_err_num) == QCMAP_CM_V6_WAN_CONNECTED && !this->cradle_backhaul_connected )
  {
    this->EnableIPV6Forwarding();
    this->EnableIPV6Firewall();
  }

  if (cbPtr && cbPtr->sta_cb)
  {
    LOG_MSG_INFO1(" Sending STA Connected Indication to Client. ",0,0,0);
    cbPtr->sta_cb(cbPtr, QCMAP_MSGR_STATION_MODE_ASSOCIATION_FAIL_V01);
  }
  return;
}

/*===========================================================================
 FUNCTION ProcessStaDHCPIPFail
==========================================================================*/
/*!
@brief
 Process DHCP IP Assignmentment Fail timer for AP-STA bridge mode.
 Send an indication to client

@parameters
 cb_user_data

@return
  void

@note

 - Dependencies
 - None

 - Side Effects
 - None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::ProcessStaDHCPIPFail(void *cb_user_data)
{
  qmi_qcmap_msgr_softap_handle_type *softApHandle = NULL;
  qmi_qcmap_msgr_status_cb_data *cbPtr = NULL;
  qmi_error_type_v01 qmi_err_num;
  in_addr addr, staIp, staMask, staGwIp;
  char netMask[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char command[MAX_COMMAND_STR_LEN];

  LOG_MSG_INFO1("QCMAP_ConnectionManager::ProcessStaDHCPIPFail()",0,0,0);

  softApHandle = (qmi_qcmap_msgr_softap_handle_type *) cb_user_data;
  if (softApHandle != NULL)
  {
    cbPtr = (qmi_qcmap_msgr_status_cb_data *) softApHandle->cb_ptr;
  }
  else
  {
    LOG_MSG_ERROR(" ProcessStaDHCPIPFail() : NULL handle passed",0,0,0);
    return;
  }

  /* Check if Static IP is configured */
  if ((this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.ip_addr == 0) ||
     (this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.netmask == 0) ||
     (this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.gw_ip == 0))
  {
    LOG_MSG_ERROR("Static IP configuration for WLAN STA mode not correctly configured, hence moving back to AP+STA Router Mode",0,0,0);
    this->revert_ap_sta_router_mode = true;
    if ((this->ActivateLAN(&qmi_err_num, true) == false)) // Forcefully reactivate LAN
      LOG_MSG_ERROR("Failed to Re-Activate LAN on ProcessStaDHCPIPFail()",0,0,0);

    /* Switch to backhaul mode. */
    if ( qcmap_cm_get_state(&qmi_err_num) == QCMAP_CM_WAN_CONNECTED )
    {
      this->EnableNATonA5();
      this->EnableDNS();
    }
    if ( qcmap_cm_get_ipv6_state(&qmi_err_num) == QCMAP_CM_V6_WAN_CONNECTED )
    {
      this->EnableIPV6Forwarding();
      this->EnableIPV6Firewall();
    }
  }
  else /* If Static IP is configured then configure Bridge with static IP */
  {
    LOG_MSG_INFO1("Bringing up Bridge iface %s with static ip configuration\n",BRIDGE_IFACE,0,0);
    staIp.s_addr = htonl(this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.ip_addr);
    staMask.s_addr = htonl(this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.netmask);
    staGwIp.s_addr = htonl(this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.gw_ip);
    /* Bring the interface UP. */
    memset(netMask,0,QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
    strlcpy(netMask, inet_ntoa(staMask), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
    snprintf( command, MAX_COMMAND_STR_LEN,
              "ifconfig %s %s netmask %s up",BRIDGE_IFACE, inet_ntoa(staIp), netMask);
    ds_system_call(command, strlen(command));
    /* Add the default route. */
    snprintf( command, MAX_COMMAND_STR_LEN,
             "route add default gw %s dev %s", inet_ntoa(staGwIp),BRIDGE_IFACE);
    ds_system_call(command, strlen(command));
    /* Populate the /etc/resolv.conf file with DNS address provided. */
    addr.s_addr = htonl(this->cfg.lan_config.interface[this->sta_iface_index].static_ip_config.dns_addr);
    snprintf(command, MAX_COMMAND_STR_LEN,
            "echo 'nameserver %s' > /etc/resolv.conf", inet_ntoa(addr));
    ds_system_call(command, strlen(command));
    this->sta_associated = true;
    this->ProcessAddrAssign(cb_user_data,BRIDGE_IFACE);
  }

  if (cbPtr && cbPtr->sta_cb)
  {
    LOG_MSG_INFO1(" Sending STA Connected Indication to Client. ",0,0,0);
    cbPtr->sta_cb(cbPtr, QCMAP_MSGR_STATION_MODE_DHCP_IP_ASSIGNMENT_FAIL_V01);
  }
  return;
}

/*===========================================================================
  FUNCTION ProcessAddrAssign
==========================================================================*/
/*!
@brief
  Process the address assignment from Sta

@parameters
  cb_user_data

@return
  void
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

void QCMAP_ConnectionManager::ProcessAddrAssign(void *cb_user_data, char *devname)
{
  char command[MAX_COMMAND_STR_LEN];
  in_addr staIp, staMask, staGwIp;
  in_addr_t default_gw_addr=0;
  boolean ret = false;
  qmi_error_type_v01 qmi_err_num;
  qmi_qcmap_msgr_softap_handle_type *softApHandle = NULL;
  qmi_qcmap_msgr_status_cb_data *cbPtr = NULL;
  char gw[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char iface_ip[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];

  memset(gw, 0, QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
  memset(iface_ip, 0, QCMAP_LAN_MAX_IPV4_ADDR_SIZE);

  ds_log_med("Entering ProcessAddrAssign for further handling for dev %s",devname);

  if ( cb_user_data == NULL)
  {
    LOG_MSG_ERROR("ProcessAddrAssign : NULL arguments passed",0,0,0);
    return;
  }

  softApHandle = (qmi_qcmap_msgr_softap_handle_type *) cb_user_data;
  if (softApHandle != NULL)
  {
    cbPtr = (qmi_qcmap_msgr_status_cb_data *) softApHandle->cb_ptr;
  }
  else
  {
    LOG_MSG_ERROR("ProcessAddrAssign : NULL handle passed",0,0,0);
    return;
  }

  if(!this->sta_associated)
  {
    LOG_MSG_INFO1("ProcessAddrAssign : Exiting STA Disassociated",0,0,0);
    return;
  }

  if(this->sta_connected)
  {
    LOG_MSG_INFO1("ProcessAddrAssign: Exiting: Duplicate function Call for IPv4",0,0,0);
    return;
  }

  if ( this->wifi_mode != QCMAP_MSGR_WLAN_MODE_AP_STA_V01 )
  {
    LOG_MSG_ERROR(" WLAN is in incorrect Mode.",0,0,0);
    return;
  }

  ret = GetIP(&staIp.s_addr, &staMask.s_addr, devname);
  if ( true == ret )
  {
    ds_log_med("STA Connected to external hotspot with IP %s\n",inet_ntoa(staIp));

    if (this->IsAPSTABridgeActivated() && (strncmp(devname,BRIDGE_IFACE,strlen(BRIDGE_IFACE)) == 0))
    {
      LOG_MSG_INFO1("Stopping DHCP IP Assignment Timer for WLAN STA",0,0,0);
      stop_wlan_sta_timer(STA_DHCP_FAIL);
      memcpy(&this->ap_sta_bridge.bridge_ip, &staIp, sizeof(in_addr));
      memcpy(&this->ap_sta_bridge.bridge_netmask, &staMask, sizeof(in_addr));
    }

    /* Logic to get the gateway IP is using the default route.
     * The idea is to write the gateway ip into a file and then read it in
     * EnableNATonA5() to add the default route through the Gateway IP.
     */
    LOG_MSG_INFO1("ProcessAddrAssign: Processing Address Assoc for IPv4",0,0,0);
    snprintf( command, MAX_COMMAND_STR_LEN,
             "ip route show | grep default | grep via| grep %s|awk '{print $3}' > /etc/gateway.txt",devname);
    ds_system_call(command, strlen(command));

    this->sta_connected = true;

    //skip if cradle backhaul is connected
    if (!this->cradle_backhaul_connected)
    {
      /*If in bridge mode, cleanup is done as path of STA_CONNECTED */
      if (this->IsAPSTABridgeActivated() == false)
      {
        /* Switch to STA interface. */
        if ( qcmap_cm_get_state(&qmi_err_num) == QCMAP_CM_WAN_CONNECTED )
        {
          this->DisableNATonA5();
          this->FlushIPV4Firewall();
        }
        if ( qcmap_cm_get_ipv6_state(&qmi_err_num) == QCMAP_CM_V6_WAN_CONNECTED )
        {
          this->DisableIPV6Forwarding(true);
          this->FlushIPV6Firewall();
        }
        this->EnableNATonA5();

        this->EnableIPV6Forwarding();
        this->EnableIPV6Firewall();

        /* Enable IPv4 MCAST */
        LOG_MSG_INFO1(" Start PIMD to enable MCAST forwarding for AP-STA \n",0,0,0);
        StartMcastDaemon();
      }
      else /* Brige Mode Activated. Start DHCP Relay */
      {
        StopDHCPD();
        if (false == this->GetGatewayIP(&default_gw_addr))
        {
           LOG_MSG_ERROR("Unable to get the Gateway IP",0,0,0);
        }
        else
        {
          staGwIp.s_addr = default_gw_addr;
          strlcpy(iface_ip, inet_ntoa(staIp), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
          strlcpy(gw, inet_ntoa(staGwIp), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
          memcpy(&this->ap_sta_bridge.bridge_def_gw, &staGwIp, sizeof(in_addr));
          ds_log_med("STA Connected to external hotspot with IP %s\n",iface_ip);
          ds_log_med("STA Connected to external hotspot with gateway IP %s\n",gw);
          snprintf(command,MAX_COMMAND_STR_LEN,"dnsmasq -i %s -I lo -z --dhcp-relay=%s,%s,%s --dhcp-proxy",BRIDGE_IFACE, iface_ip, gw, BRIDGE_IFACE);
          ds_system_call(command, strlen(command));
          this->RestartUSBForBridgeMode();
          this->EnterDefaultIPRulesForBridgeMode();
        }
      }
    }
  }
  else
  {
    LOG_MSG_INFO1("Error: GetStaIP Failed. Bailing out.\n",0,0,0);
    return;
  }
  /* First delete duplicate rule if any. */
  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -D FORWARD -i %s -o %s -j DROP", BRIDGE_IFACE,
            this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));

  if (cbPtr && cbPtr->sta_cb)
  {
    LOG_MSG_INFO1(" Sending STA Connected Indication to Client. ",0,0,0);
    cbPtr->sta_cb(cbPtr, QCMAP_MSGR_STATION_MODE_CONNECTED_V01);
  }
  return;
}

/*===========================================================================
 FUNCTION RestartUSBForBridgeMode
==========================================================================*/
/*!
 @brief
  Restart USB Link to the state that it was before ProcessAssocSTAForBridge()
  was called

@parameters

@return
  void
@note

 - Dependencies
 - None

 - Side Effects
 - None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::RestartUSBForBridgeMode(void)
{
  char command[MAX_COMMAND_STR_LEN];
  in_addr addr;
  boolean ret = false;
  char usb_intf_name [DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  char usb_gw_ip[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char netmask[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char subnet[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  boolean usb_enable=false;

  // USB related link local address
  addr.s_addr = htonl(this->cfg.lan_config.usb_conf.gateway_addr);
  strlcat(usb_gw_ip, inet_ntoa(addr), 16);
  addr.s_addr = htonl(this->cfg.lan_config.ll_subnet_mask);
  strlcpy(netmask, inet_ntoa(addr), 16);

  LOG_MSG_INFO1("RestoreUSBForBridgeMode()",0,0,0);

  /* Restart USB */
  if ( GetUSBEnable())
  {
    if ( this->cfg.lan_config.usb_conf.link_type == QCMAP_QTI_USB_LINK_RNDIS )
    {
      strlcpy(usb_intf_name, "rndis0",16);
    }
    else if ( this->cfg.lan_config.usb_conf.link_type == QCMAP_QTI_USB_LINK_ECM )
    {
      strlcpy(usb_intf_name, "ecm0",16);
    }
    else
    {
      LOG_MSG_ERROR("Incorrect USB LINK Detected Link Type = %x", this->cfg.lan_config.usb_conf.link_type,0,0);
    }
    snprintf( command, MAX_COMMAND_STR_LEN, "brctl delif %s %s",BRIDGE_IFACE,usb_intf_name);
    ds_system_call(command, strlen(command));
    ds_system_call("echo 0 > /sys/class/android_usb/android0/enable ",strlen("echo 0 > /sys/class/android_usb/android0/enable "));
    usleep(500000);

    /* Delay for 1 secs since USB UP/DOWN in immediate instance causes issues*/
    ds_system_call("echo 1 > /sys/class/android_usb/android0/enable ",strlen("echo 1 > /sys/class/android_usb/android0/enable "));
    snprintf( command, MAX_COMMAND_STR_LEN, "ifconfig %s %s netmask %s up",usb_intf_name,usb_gw_ip,netmask);
    ds_system_call(command, strlen(command));
    /* Delete LL Address */
    addr.s_addr = htonl(this->cfg.lan_config.usb_conf.gateway_addr & this->cfg.lan_config.ll_subnet_mask);
    strlcpy(subnet, inet_ntoa(addr), 16);
    snprintf(command, MAX_COMMAND_STR_LEN, "route del -net %s netmask %s dev %s", subnet, netmask, usb_intf_name);
    ds_system_call(command, strlen(command));
    snprintf(command, MAX_COMMAND_STR_LEN, "ip -6 route del fe80::/64 dev %s", usb_intf_name);
    ds_system_call(command, strlen(command));
  }
}

/*===========================================================================
  FUNCTION ProcessCradleAddrAssign
==========================================================================*/
/*!
@brief
  Process the address assignment from Cradle

@parameters
  cb_user_data

@return
  void
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

void QCMAP_ConnectionManager::ProcessCradleAddrAssign(void *cb_user_data)
{
  char command[MAX_COMMAND_STR_LEN];
  in_addr craIp, staMask;
  boolean ret = false;
  qmi_error_type_v01 qmi_err_num;
  qmi_qcmap_msgr_softap_handle_type *softApHandle = NULL;
  qmi_qcmap_msgr_status_cb_data *cbPtr = NULL;
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  int qcmap_cm_error;

  memset(devname, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);

  LOG_MSG_INFO1("Entering ProcessCradleAddrAssign for further handling",0,0,0);

  if ( cb_user_data == NULL)
  {
    LOG_MSG_ERROR("ProcessCradleAddrAssign : NULL arguments passed",0,0,0);
    return;
  }

  if (this->cfg.cradle_config.cradle_mode == QCMAP_MSGR_CRADLE_DISABLED_V01)
  {
    LOG_MSG_ERROR("Not in cradle mode",0,0,0);
    return;
  }

  softApHandle = (qmi_qcmap_msgr_softap_handle_type *) cb_user_data;
  if (softApHandle != NULL)
  {
    cbPtr = (qmi_qcmap_msgr_status_cb_data *) softApHandle->cb_ptr;
  }
  else
  {
    LOG_MSG_ERROR("ProcessCradleAddrAssign : NULL handle passed",0,0,0);
    return;
  }

  ret = GetIP(&craIp.s_addr, &staMask.s_addr, ECM_IFACE);
  if ( true == ret )
  {
    LOG_MSG_INFO1("Device Connected to cradle.\n",0,0,0);

    /* Logic to get the gateway IP is using the default route.
     * The idea is to write the gateway ip into a file and then read it in
     * EnableNATonA5() to add the default route through the Gateway IP.
     */
    LOG_MSG_INFO1("ProcessCradleAddrAssign: Processing Address Assoc for IPv4",0,0,0);
    snprintf( command, MAX_COMMAND_STR_LEN,
             "ip route show | grep default | grep via| grep ecm0|awk '{print $3}' > /etc/gateway.txt");
    ds_system_call(command, strlen(command));

    /* Switch to Cradle interface. */
    if ( qcmap_cm_get_state(&qmi_err_num) == QCMAP_CM_WAN_CONNECTED || this->sta_connected )
    {
      this->DisableNATonA5();
      this->FlushIPV4Firewall();
    }
    if ( qcmap_cm_get_ipv6_state(&qmi_err_num) == QCMAP_CM_V6_WAN_CONNECTED || this->sta_connected )
    {
      this->DisableIPV6Forwarding(true);
      this->FlushIPV6Firewall();
    }

    this->cradle_backhaul_connected = true;

    this->EnableNATonA5();
    this->BlockIPv4WWANAccessCradle();

    this->EnableIPV6Forwarding();
    this->EnableIPV6Firewall();
    this->BlockIPv6WWANAccessCradle();

    /* Enable IPv4 MCAST */
    LOG_MSG_INFO1(" Restart PIMD to enable MCAST forwarding for Cradle \n",0,0,0);
    StopMcastDaemon();
    StartMcastDaemon();
  }
  else
  {
    LOG_MSG_INFO1("Error: GetIP Failed. Bailing out.\n",0,0,0);
    return;
  }

  if (cbPtr && cbPtr->cradle_cb)
  {
    LOG_MSG_INFO1(" Sending Cradle Connected Indication to Client. ",0,0,0);
    cbPtr->cradle_cb(cbPtr, QCMAP_MSGR_CRADLE_CONNECTED_V01);
  }
  return;
}

/*===========================================================================
  FUNCTION ProcessStaDisAssoc
==========================================================================*/
/*!
@brief
 Brings down station mode interfaces.

@parameters
  void

@return
  void
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::ProcessStaDisAssoc()
{
  char command[MAX_COMMAND_STR_LEN];
  qmi_error_type_v01 qmi_err_num;
  LOG_MSG_INFO1("Entering ProcessStaDisAssoc",0,0,0);

  /* Cant continue if MobileAP is not enabled! */
  if (!this->qcmap_enable)
  {
    LOG_MSG_ERROR("Error: QCMAP not enabled.\n",0,0,0);
    return;
  }

  if ( this->wifi_mode == QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01 )
  {
    LOG_MSG_ERROR("\n WLAN is in disabled state.\n",0,0,0);
    return;
  }

  if ( this->wifi_mode != QCMAP_MSGR_WLAN_MODE_AP_STA_V01 ||
       this->sta_iface_index != QCMAP_MSGR_INTF_STATION_INDEX )
  {
    LOG_MSG_ERROR("\n WLAN is in incorrect Mode.\n",0,0,0);
    return;
  }

  if (!this->cradle_backhaul_connected)
  {
    /* Disable NAT on A5. and clean IPV4 firewall */
    this->DisableNATonA5();
    this->FlushIPV4Firewall();

    /* Disable IPV6 Forwarding. and firewall */
    this->DisableIPV6Forwarding(true);
    this->FlushIPV6Firewall();
  }

  /*Reset STA Associated flag*/
  this->sta_associated = false;

  /* Reset STA connected flag. */
  this->sta_connected = false;

  /* Kill DHCPCD Process. */
  snprintf( command, MAX_COMMAND_STR_LEN, "kill -15 $(cat /var/run/dhcpcd-%s.pid)", this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));

  if (!this->cradle_backhaul_connected)
  {
    /* Switch to backhaul mode. */
    if ( qcmap_cm_get_state(&qmi_err_num) == QCMAP_CM_WAN_CONNECTED )
    {
      this->EnableNATonA5();
      this->EnableDNS();
    }
    if ( qcmap_cm_get_ipv6_state(&qmi_err_num) == QCMAP_CM_V6_WAN_CONNECTED )
    {
      this->EnableIPV6Forwarding();
      this->EnableIPV6Firewall();
    }

    /* Disable IPv4 MCAST */
    LOG_MSG_INFO1(" Stop PIMD to disable MCAST forwarding on STA interface.\n",0,0,0);
    StopMcastDaemon();
  }
}

/*===========================================================================
 FUNCTION ProcessStaDisAssocForWLANBridge
==========================================================================*/
/*!
@brief
  Brings down station mode interfaces when we are in AP-STA mode

@parameters
 void

@return
  void
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::ProcessStaDisAssocForWLANBridge()
{
  char command[MAX_COMMAND_STR_LEN];
  qmi_error_type_v01 qmi_err_num;
  LOG_MSG_INFO1("Entering ProcessStaDisAssocForWLANBridge",0,0,0);

  /* Cant continue if MobileAP is not enabled! */
  if (!this->qcmap_enable)
  {
    LOG_MSG_ERROR("Error: QCMAP not enabled.\n",0,0,0);
    return;
  }

  if ( this->wifi_mode == QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01 )
  {
    LOG_MSG_ERROR("\n WLAN is in disabled state.\n",0,0,0);
    return;
  }

  if ( this->wifi_mode != QCMAP_MSGR_WLAN_MODE_AP_STA_V01 ||
       this->sta_iface_index != QCMAP_MSGR_INTF_STATION_INDEX )
  {
    LOG_MSG_ERROR("\n WLAN is in incorrect Mode.\n",0,0,0);
    return;
  }

  /* Clean IPtables and Ebtables */
  CleanIPtables();
  CleanEbtables();

  /* Moving back to AP-STA Router Mode */
  LOG_MSG_ERROR("Moving Back to AP+STA Router Mode, so webserver is accessable",0,0,0);
  this->revert_ap_sta_router_mode = true;
  if ((this->ActivateLAN(&qmi_err_num, true) == false)) // Forcefully reactivate LAN
      LOG_MSG_ERROR("Failed to Re-Activate LAN",0,0,0);

  /*Reset STA Associated flag*/
  this->sta_associated = false;

  /* Reset STA connected flag. */
  this->sta_connected = false;

  if (!this->cradle_backhaul_connected)
  {
    /* Kill DHCPCD Process. */
    snprintf( command, MAX_COMMAND_STR_LEN, "killall -15 dhcpcd" );
    ds_system_call(command, strlen(command));

    /* Switch to backhaul mode. */
    if ( qcmap_cm_get_state(&qmi_err_num) == QCMAP_CM_WAN_CONNECTED )
    {
      this->EnableNATonA5();
      this->EnableDNS();
    }
    if ( qcmap_cm_get_ipv6_state(&qmi_err_num) == QCMAP_CM_V6_WAN_CONNECTED )
    {
      this->EnableIPV6Forwarding();
      this->EnableIPV6Firewall();
    }
  }
}

/*===========================================================================
  FUNCTION ProcessCradleDisAssoc
==========================================================================*/
/*!
@brief
  Bring down the Cradle iface

@parameters
  void

@return
  void
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

void QCMAP_ConnectionManager::ProcessCradleDisAssoc(void *cb_user_data)
{
  char command[MAX_COMMAND_STR_LEN];
  qmi_error_type_v01 qmi_err_num;
  qmi_qcmap_msgr_softap_handle_type *softApHandle = NULL;
  qmi_qcmap_msgr_status_cb_data *cbPtr = NULL;

  LOG_MSG_INFO1("Entering ProcessCradleDisAssoc",0,0,0);

  if ( cb_user_data == NULL)
  {
    LOG_MSG_ERROR("ProcessAddrAssign : NULL arguments passed",0,0,0);
    return;
  }

  /* Cant continue if MobileAP is not enabled! */
  if (!this->qcmap_enable)
  {
    LOG_MSG_ERROR("Error: QCMAP not enabled.\n",0,0,0);
    return;
  }

  if ( !this->cradle_backhaul_connected )
  {
    LOG_MSG_ERROR("\n Cradle is not connected.\n",0,0,0);
    return;
  }

  softApHandle = (qmi_qcmap_msgr_softap_handle_type *) cb_user_data;
  if (softApHandle != NULL)
  {
    cbPtr = (qmi_qcmap_msgr_status_cb_data *) softApHandle->cb_ptr;
  }
  else
  {
    LOG_MSG_ERROR("ProcessAddrAssign : NULL handle passed",0,0,0);
    return;
  }

  /* Disable NAT on A5 and clean IPV4 firewall */
  this->DisableNATonA5();
  this->FlushIPV4Firewall();

  /* Disable IPV6 Forwarding and firewall */
  this->DisableIPV6Forwarding(true);
  this->FlushIPV6Firewall();

  /*Reset Cradle conencted flag*/
  this->cradle_backhaul_connected = false;

  /* stop dhcp client on ecm0 */
  LOG_MSG_INFO1("Killing dhcpcd process.\n",0,0,0);
  snprintf(command, MAX_COMMAND_STR_LEN, "kill -15 $(cat /var/run/dhcpcd-%s.pid)", ECM_IFACE);
  ds_system_call(command, strlen(command));
  snprintf(command, MAX_COMMAND_STR_LEN, "rm -rf /var/run/dhcpcd-%s.pid", ECM_IFACE);
  ds_system_call(command, strlen(command));

  /* Switch to STA/WAN mode if present */
  if (this->sta_connected)
  {
    this->EnableNATonA5();

    this->EnableIPV6Forwarding();
    this->EnableIPV6Firewall();

    /* Enable IPv4 MCAST */
    LOG_MSG_INFO1(" Restart PIMD to enable MCAST forwarding for AP-STA \n",0,0,0);
    StopMcastDaemon();
    StartMcastDaemon();
  }
  else if (qcmap_cm_get_state(&qmi_err_num) == QCMAP_CM_WAN_CONNECTED ||
           qcmap_cm_get_ipv6_state(&qmi_err_num) == QCMAP_CM_V6_WAN_CONNECTED)
  {
    if ( qcmap_cm_get_state(&qmi_err_num) == QCMAP_CM_WAN_CONNECTED )
    {
      this->EnableNATonA5();
      this->EnableDNS();
    }
    if ( qcmap_cm_get_ipv6_state(&qmi_err_num) == QCMAP_CM_V6_WAN_CONNECTED )
    {
      this->EnableIPV6Forwarding();
      this->EnableIPV6Firewall();
    }
  }

  /* Disable IPv4 MCAST */
  LOG_MSG_INFO1(" Stop PIMD to disable MCAST forwarding on STA interface.\n",0,0,0);
  StopMcastDaemon();

  if (cbPtr && cbPtr->cradle_cb)
  {
    LOG_MSG_INFO1(" Sending STA Connected Indication to Client. ",0,0,0);
    cbPtr->cradle_cb(cbPtr, QCMAP_MSGR_CRADLE_DISCONNECTED_V01);
  }
  return;
}


/*===========================================================================
  FUNCTION EnableMiscNATTasks
==========================================================================*/
/*!
@brief
  Adds SNAT, DMZ, VPN passthrough and firewall configurations.

@parameters
  void

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::EnableMiscNATTasks()
{
  int i = 0, ret = QCMAP_CM_SUCCESS;
  qmi_error_type_v01 qmi_err_num;

  qcmap_backhaul_interface_type interface;
  qcmap_msgr_firewall_entry_conf_t *firewall_entry = NULL;

  QCMAP_CM_LOG_FUNC_ENTRY();
  /* Config SNAT */
  for (i = 0; i < this->cfg.nat_config.num_port_fwding_entries; i++)
  {
      if ( this->cfg.nat_config.port_fwding_entries[i].port_fwding_protocol != PS_IPPROTO_TCP_UDP )
      {
        AddSNATEntryOnA5(&this->cfg.nat_config.port_fwding_entries[i], &qmi_err_num);
      }
      else
      {
        this->cfg.nat_config.port_fwding_entries[i].port_fwding_protocol = PS_IPPROTO_TCP;
        AddSNATEntryOnA5(&this->cfg.nat_config.port_fwding_entries[i], &qmi_err_num);
        this->cfg.nat_config.port_fwding_entries[i].port_fwding_protocol = PS_IPPROTO_UDP;
        AddSNATEntryOnA5(&this->cfg.nat_config.port_fwding_entries[i], &qmi_err_num);
        this->cfg.nat_config.port_fwding_entries[i].port_fwding_protocol = PS_IPPROTO_TCP_UDP;
      }
  }

  /* Config DMZ IP.*/
  if ( this->cfg.nat_config.dmz_ip != 0 )
  {
    AddDMZOnA5(this->cfg.nat_config.dmz_ip, &qmi_err_num);
  }

   // Adding default firewall rules
   interface = TYPE_IPV4;
   if (!SetDefaultFirewallRule(interface)) {
      LOG_MSG_ERROR("Default firewall rules not added for IPV4",0,0,0);
   }

  // Adding Firewall rules for IPV4
  if (this->cfg.nat_config.firewall_enabled) {
    for (i = 0; i < this->cfg.nat_config.num_firewall_entries; i++) {
       firewall_entry = &cfg.nat_config.extd_firewall_entries[i]->extd_firewall_entry;
       if (firewall_entry && firewall_entry->filter_spec.ip_vsn == IP_V4 ) {
           SetFirewallV4(this->cfg.nat_config.extd_firewall_entries[i],true, &qmi_err_num);
       }
    }
  }

  /* Configure VPN PassThrough. */
  SetIPSECVpnPassThroughOnA5(this->cfg.nat_config.enable_ipsec_vpn_pass_through, &qmi_err_num);
  SetL2TPVpnPassThroughOnA5(this->cfg.nat_config.enable_l2tp_vpn_pass_through, &qmi_err_num);
  SetPPTPVpnPassThroughOnA5(this->cfg.nat_config.enable_pptp_vpn_pass_through, &qmi_err_num);

  /* Configure Webserver WWAN Access. */
  SetWebserverWWANAccessOnA5(this->cfg.nat_config.enable_webserver_wwan_access, &qmi_err_num);
  /* Enable ALGs from XML file */
  if ( this->cfg.nat_config.enable_alg_mask & QCMAP_MSGR_MASK_RTSP_ALG_V01 )
  {
    LOG_MSG_INFO1("QCMAP_ConnectionManager::EnableRTSPAlg() Called",0,0,0)
    ret = EnableRTSPAlg(&qmi_err_num);
    if(ret != true)
    {
      LOG_MSG_ERROR("Fail to enable RTSP ALG, Ret value = %d error: %d.\n",
                     ret, qmi_err_num, 0);
    }
  }

  /*SIP Alg is enabled by default in kernel. So we to enable or disable based on
    the config explicitly*/
  if ( this->cfg.nat_config.enable_alg_mask & QCMAP_MSGR_MASK_SIP_ALG_V01 )
  {
    LOG_MSG_INFO1("QCMAP_ConnectionManager::EnableSIPAlg() Called",0,0,0)
    ret = EnableSIPAlg(&qmi_err_num);
    if(ret != true)
    {
      LOG_MSG_ERROR("Fail to enable SIP ALG, Ret value = %d error: %d.\n",
                     ret, qmi_err_num, 0);
    }
  }
  else
  {
    LOG_MSG_INFO1("QCMAP_ConnectionManager::DisableSIPAlg() Called",0,0,0)
    ret = DisableSIPAlg(&qmi_err_num);
    if(ret != true)
    {
      LOG_MSG_ERROR("Fail to Disable SIP ALG, Ret value%d Error: %d\n",
                     ret, qmi_err_num, 0);
    }
  }
}


/*===========================================================================
  FUNCTION EnableNATonA5
==========================================================================*/
/*!
@brief
  Enables NAT on A5 based on the NAT type configured.

@parameters
  void

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::EnableNATonA5()
{
  char command[MAX_COMMAND_STR_LEN];
  struct in_addr addr;
  int i = 0;
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  char publicIpAddr[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char gwIP[QCMAP_LAN_MAX_IPV4_ADDR_SIZE], subnetIP[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char netMaskIP[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  int qcmap_cm_error;
  in_addr_t public_ip = 0, sec_dns_addr = 0, default_gw_addr = 0, net_mask = 0;
  uint32 pri_dns_addr = 0;
  qmi_error_type_v01 qmi_err_num;
/*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Cant continue if MobileAP is not enabled! */
  if (!this->qcmap_enable)
  {
    LOG_MSG_ERROR("Error: QCMAP not enabled.\n",0,0,0);
    return false;
  }

  /* Check for STA mode, before we start setting up NAT on A5 */
  memset(devname, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);
  memset(publicIpAddr, 0, QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
  memset(gwIP, 0, QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
  memset(subnetIP, 0, QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
  memset(netMaskIP, 0, QCMAP_LAN_MAX_IPV4_ADDR_SIZE);

  if ( !this->sta_connected && !this->cradle_backhaul_connected)
  {

    if ( qcmap_cm_get_dev_name(this->qcmap_cm_handle,
                               QCMAP_MSGR_IP_FAMILY_V4_V01,
                               devname, &qcmap_cm_error) != QCMAP_CM_SUCCESS )
    {
      LOG_MSG_ERROR("Couldn't get rmnet name. error %d\n", qcmap_cm_error,0,0);
      return false;
    }


    if( QCMAP_CM_SUCCESS !=
        qcmap_cm_get_ipv4_net_conf(this->qcmap_cm_handle,
                                   &public_ip, &pri_dns_addr, &sec_dns_addr,
                                   &default_gw_addr, &qmi_err_num) )
    {
      LOG_MSG_ERROR("Couldn't get public ip address. error %d\n", qmi_err_num,0,0);
      return false;
    }
    this->cfg.lan_config.um_iface_ip_addr_facing_a5 = default_gw_addr;
    net_mask = htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].sub_net_mask);
  }
  else
  {
    if( this->cradle_backhaul_connected )
    {
      strlcpy(devname, ECM_IFACE, QCMAP_MSGR_INTF_LEN);
    }
    else if( this->sta_connected )
    {
      strlcpy(devname, this->cfg.lan_config.sta_interface, QCMAP_MSGR_INTF_LEN);
    }

    if ( false == this->GetIP(&public_ip, &net_mask, devname) )
    {
      LOG_MSG_ERROR("Unable to get the IP address",0,0,0);
      return false;
    }
    if ( false == this->GetGatewayIP(&default_gw_addr) )
    {
      LOG_MSG_ERROR("Unable to get the Gateway IP",0,0,0);
      /* continue by adding just the default route. */
    }
  }

  addr.s_addr = public_ip;
  strlcpy(publicIpAddr, inet_ntoa(addr), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
  addr.s_addr = default_gw_addr;
  strlcpy(gwIP, inet_ntoa(addr), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
  addr.s_addr = public_ip & net_mask;
  strlcpy(subnetIP, inet_ntoa(addr), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
  addr.s_addr = net_mask;
  strlcpy(netMaskIP, inet_ntoa(addr), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);

  /* Store ipv4 address info. */
  this->ipv4_public_ip = public_ip;
  this->ipv4_default_gw_addr = default_gw_addr;

  /* Delete the default route to backhaul. */
  snprintf( command, MAX_COMMAND_STR_LEN, "route del default dev %s", devname);
  ds_system_call(command, strlen(command));
  LOG_MSG_INFO1("%s\n", command,0,0);

  /* Delete the subnet base route */
  snprintf( command, MAX_COMMAND_STR_LEN, "route del -net %s netmask %s dev %s", subnetIP, netMaskIP, devname);
  ds_system_call(command, strlen(command));
  LOG_MSG_INFO1("%s\n", command,0,0);

  /* Clear the existing connection track entries. */
  /* Delete the conntrack connections based on source NAT and destination NAT flags. */
  snprintf( command, MAX_COMMAND_STR_LEN, "conntrack -D -n");
  ds_system_call(command, strlen(command));

  snprintf( command, MAX_COMMAND_STR_LEN, "conntrack -D -g");
  ds_system_call(command, strlen(command));

  /*-----------------------------------------------------------------------
    To allow embedded call on same PDN, traffic from MobileAP clients should
    not be forwarded to WWAN interface. So we install a firewall rule in
    DisableNATOnA5 to not to forward traffic from bridge interface to WWAN
    interface. So if we flush the rules here even that entry will be deleted.
    To avoid it we do the flushing only when in WWANmode. When we are swithing
    between WWAN and station mode, we anyway first flush all the existing
    iptables rules in DisableNATOnA5. So not doing it for station mode should
    affect anything.
  -----------------------------------------------------------------------*/
  if ( !this->sta_connected )
  {
    snprintf( command, MAX_COMMAND_STR_LEN, "iptables --flush FORWARD");
    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("%s\n", command,0,0);
  }

  snprintf( command, MAX_COMMAND_STR_LEN, "iptables -t mangle --flush");
  ds_system_call(command, strlen(command));
  LOG_MSG_INFO1("%s\n", command,0,0);

  snprintf( command, MAX_COMMAND_STR_LEN, "iptables --table nat --flush");
  ds_system_call(command, strlen(command));
  LOG_MSG_INFO1("%s\n", command,0,0);

  /* Check for enable ipv4 must be here to remove the default routes which
     DHCPCD adds */
  if ( !this->cfg.lan_config.enable_ipv4 )
  {
    LOG_MSG_ERROR("Error: IPV4 not enabled. Not continuing further\n",0,0,0);
    return false;
  }

  /*-----------------------------------------------------------------------
    Set up forwarding and masquerading.
  -----------------------------------------------------------------------*/
  switch (this->cfg.nat_config.nat_type)
  {
     case QCMAP_MSGR_NAT_FULL_CONE_NAT_V01:
     {
       snprintf( command, MAX_COMMAND_STR_LEN,
                 "iptables -t nat -A POSTROUTING -o %s -j MASQUERADE --random", devname );
       ds_system_call(command, strlen(command));
       LOG_MSG_INFO1("%s\n", command,0,0);
       snprintf( command, MAX_COMMAND_STR_LEN,
                 "iptables -t nat -A PREROUTING -i %s -j NATTYPE --mode dnat --type 1", devname );
       ds_system_call(command, strlen(command));
       LOG_MSG_INFO1("%s\n", command,0,0);
       snprintf( command, MAX_COMMAND_STR_LEN,
                 "iptables -A FORWARD -o %s -j NATTYPE --mode forward_out --type 1", devname );
       ds_system_call(command, strlen(command));
       LOG_MSG_INFO1("%s\n", command,0,0);
       snprintf( command, MAX_COMMAND_STR_LEN,
                 "iptables -A FORWARD -i %s -j NATTYPE --mode forward_in --type 1", devname );
       ds_system_call(command, strlen(command));
       LOG_MSG_INFO1("%s\n", command,0,0);
       break;
     }
     case QCMAP_MSGR_NAT_SYMMETRIC_NAT_V01:
     {
       snprintf( command, MAX_COMMAND_STR_LEN,
                 "iptables -t nat -A POSTROUTING -o %s -j MASQUERADE --random", devname );
       ds_system_call(command, strlen(command));
       LOG_MSG_INFO1("%s\n", command,0,0);
       break;
     }
     case QCMAP_MSGR_NAT_PORT_RESTRICTED_CONE_NAT_V01:
     {
       snprintf( command, MAX_COMMAND_STR_LEN,
                 "iptables -t nat -A POSTROUTING -o %s -j SNAT --to-source %s",
                 devname, publicIpAddr);
       ds_system_call(command, strlen(command));
       LOG_MSG_INFO1("%s\n", command,0,0);
       break;
     }
     case QCMAP_MSGR_NAT_ADDRESS_RESTRICTED_NAT_V01:
     {
       snprintf( command, MAX_COMMAND_STR_LEN,
                 "iptables -t nat -A POSTROUTING -o %s -j MASQUERADE --random", devname );
       ds_system_call(command, strlen(command));
       LOG_MSG_INFO1("%s\n", command,0,0);
       snprintf( command, MAX_COMMAND_STR_LEN,
                 "iptables -t nat -A PREROUTING -i %s -j NATTYPE --mode dnat --type 2", devname );
       ds_system_call(command, strlen(command));
       LOG_MSG_INFO1("%s\n", command,0,0);
       snprintf( command, MAX_COMMAND_STR_LEN,
                 "iptables -A FORWARD -o %s -j NATTYPE --mode forward_out --type 2", devname );
       ds_system_call(command, strlen(command));
       LOG_MSG_INFO1("%s\n", command,0,0);
       snprintf( command, MAX_COMMAND_STR_LEN,
                 "iptables -A FORWARD -i %s -j NATTYPE --mode forward_in --type 2", devname );
       ds_system_call(command, strlen(command));
       LOG_MSG_INFO1("%s\n", command,0,0);
       break;
     }
  }

  /*-----------------------------------------------------------------------
    Enable packet forwarding
  -----------------------------------------------------------------------*/
  snprintf( command, MAX_COMMAND_STR_LEN,
            "echo 1 > /proc/sys/net/ipv4/ip_forward");
  ds_system_call(command, strlen(command));
  LOG_MSG_INFO1("%s\n", command,0,0);

  /*-----------------------------------------------------------------------
    Enable ARP Proxy on WAN interface
  -----------------------------------------------------------------------*/
  snprintf( command, MAX_COMMAND_STR_LEN,
            "echo 1 > /proc/sys/net/ipv4/conf/%s/proxy_arp", devname );
  ds_system_call(command, strlen(command));
  LOG_MSG_INFO1("%s\n", command,0,0);

  LOG_MSG_INFO1(" NAT Enabled.\n",0,0,0);

  /* Drop unstatefull tcp packets */
  snprintf( command, MAX_COMMAND_STR_LEN,
            "iptables -t filter -A FORWARD -i %s -p tcp -m state --state INVALID -j DROP",
            BRIDGE_IFACE );
  ds_system_call(command, strlen(command));

  /*-----------------------------------------------------------------------
    Install the subnet route..
  -----------------------------------------------------------------------*/
  snprintf( command, MAX_COMMAND_STR_LEN, "route add -net %s netmask %s dev %s", subnetIP, netMaskIP, devname);
  ds_system_call(command, strlen(command));
  LOG_MSG_INFO1("%s\n", command,0,0);

  /*-----------------------------------------------------------------------
    Install the default route..
  -----------------------------------------------------------------------*/
  if ( default_gw_addr != 0 )
  {
    snprintf( command, MAX_COMMAND_STR_LEN, "route add default gw %s dev %s",
              gwIP ,devname);
  }
  else
  {
    snprintf( command, MAX_COMMAND_STR_LEN, "route add default dev %s",
              devname);
  }
  ds_system_call(command, strlen(command));
  LOG_MSG_INFO1("%s\n", command,0,0);

  this->EnableMiscNATTasks();

  return true;
}

/*===========================================================================
  FUNCTION DisableNATonA5
==========================================================================*/
/*!
@brief
  This function will delete NAT entries on A5.

@parameters
  void

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::DisableNATonA5()
{
  char command[MAX_COMMAND_STR_LEN];
  struct in_addr addr;
  int i = 0 , ret = QCMAP_CM_SUCCESS;
  char publicIpAddr[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char gwIP[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  int qcmap_cm_error;
  in_addr_t public_ip = 0, sec_dns_addr = 0, default_gw_addr = 0, net_mask = 0;
  uint32 pri_dns_addr = 0;
  qmi_error_type_v01 qmi_err_num;
/*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Cant continue if SoftAP is not enabled! */
  if (!this->qcmap_enable)
  {
    LOG_MSG_ERROR("Error: QCMAP not enabled.\n",0,0,0);
    return false;
  }

  if ( !this->cfg.lan_config.enable_ipv4 )
  {
    LOG_MSG_ERROR("Error: IPV4 not enabled.\n",0,0,0);
    return false;
  }
  /*-----------------------------------------------------------------------
    Delete iptables entries and flush tables.
    -----------------------------------------------------------------------*/
  snprintf( command, MAX_COMMAND_STR_LEN, "iptables --flush FORWARD");
  ds_system_call(command, strlen(command));

  snprintf( command, MAX_COMMAND_STR_LEN, "iptables --flush INPUT");
  ds_system_call(command, strlen(command));

  snprintf( command, MAX_COMMAND_STR_LEN, "iptables --table nat --flush");
  ds_system_call(command, strlen(command));

  /* Install the rule to block WWAN access. */
  this->BlockIPv4WWANAccess();

  /*-----------------------------------------------------------------------
    Disable ALGs with enable bit set in XML.
  -----------------------------------------------------------------------*/
  if (this->cfg.nat_config.enable_alg_mask & QCMAP_MSGR_MASK_RTSP_ALG_V01)
  {
    LOG_MSG_INFO1("QCMAP_ConnectionManager::DisableRTSPAlg() Called",0,0,0)
    ret = DisableRTSPAlg(&qmi_err_num);
    if(ret != true)
    {
      LOG_MSG_ERROR("Fail to Disable RTSP ALG, Ret value%d Error: %d\n",
                     ret, qmi_err_num, 0);
    }
  }

  if (this->cfg.nat_config.enable_alg_mask & QCMAP_MSGR_MASK_SIP_ALG_V01)
  {
    LOG_MSG_INFO1("QCMAP_ConnectionManager::DisableSIPAlg() Called",0,0,0)
    ret = DisableSIPAlg(&qmi_err_num);
    if(ret != true)
    {
      LOG_MSG_ERROR("Fail to Disable SIP ALG, Ret value%d Error: %d\n",
                     ret, qmi_err_num, 0);
    }
  }

  /* Check for STA mode, before we start setting up NAT on A5 */
  memset(publicIpAddr, 0, QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
  memset(gwIP, 0, QCMAP_LAN_MAX_IPV4_ADDR_SIZE);

  /* Gateway address of a Connected Interface, set in function EnableNatonA5 */
  if(this->ipv4_default_gw_addr != 0)
  {
    addr.s_addr = this->ipv4_default_gw_addr;
    strlcpy(gwIP, inet_ntoa(addr), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
    /* First delete the default route. */
    snprintf( command, MAX_COMMAND_STR_LEN, "route del default gw %s", gwIP);
    ds_system_call(command, strlen(command));
  }
  else
  {
    snprintf( command, MAX_COMMAND_STR_LEN, "route del default gw 0.0.0.0");
    ds_system_call(command, strlen(command));
  }

  /* Delete the conntrack connections based on source NAT and destination NAT flags. */
  snprintf( command, MAX_COMMAND_STR_LEN, "conntrack -D -n");
  ds_system_call(command, strlen(command));

  snprintf( command, MAX_COMMAND_STR_LEN, "conntrack -D -g");
  ds_system_call(command, strlen(command));

  /* Clear the ipv4 address info since disconnecting and the same
     will be reset in EnableNATonA5*/
  this->ipv4_public_ip = 0;
  this->ipv4_default_gw_addr = 0;

  return true;
}

/*===========================================================================
  FUNCTION BlockIPv4WWANAccess
==========================================================================*/
/*!
@brief
  This function blocks IPv4 WWAN access for MobileAP clients.

@parameters
  void

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::BlockIPv4WWANAccess()
{
  char command[MAX_COMMAND_STR_LEN];

/*===========================================================================
  Allow access between Bridge and Bridge.
==========================================================================*/
  /* First delete duplicate rule if any. */
  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -D FORWARD -i %s -o %s -j ACCEPT", BRIDGE_IFACE, BRIDGE_IFACE);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -A FORWARD -i %s -o %s -j ACCEPT", BRIDGE_IFACE, BRIDGE_IFACE);
  ds_system_call(command, strlen(command));

/*===========================================================================
  Allow access between Bridge and PPP.
==========================================================================*/
  /* First delete duplicate rule if any. */
  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -D FORWARD -i %s -o %s -j ACCEPT", BRIDGE_IFACE, PPP_IFACE);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -A FORWARD -i %s -o %s -j ACCEPT", BRIDGE_IFACE, PPP_IFACE);
  ds_system_call(command, strlen(command));

  /* First delete duplicate rule if any. */
  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -D FORWARD -i %s -o %s -j ACCEPT", PPP_IFACE, BRIDGE_IFACE);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -A FORWARD -i %s -o %s -j ACCEPT", PPP_IFACE, BRIDGE_IFACE);
  ds_system_call(command, strlen(command));

/*===========================================================================
  Block WWAN access to MobileAP clients.
==========================================================================*/
  /* First delete duplicate rule if any. */
  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -D FORWARD -i %s ! -o %s -j DROP", BRIDGE_IFACE,
            this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));
  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -D FORWARD -i %s ! -o %s -j DROP", BRIDGE_IFACE,
            ECM_IFACE);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -A FORWARD -i %s ! -o %s -j DROP", BRIDGE_IFACE,
            this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));

/*===========================================================================
  Block WWAN access to PPP client.
==========================================================================*/
  /* First delete duplicate rule if any. */
  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -D FORWARD -i %s ! -o %s -j DROP", PPP_IFACE,
            this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));
  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -D FORWARD -i %s ! -o %s -j DROP", PPP_IFACE,
            ECM_IFACE);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -A FORWARD -i %s ! -o %s -j DROP", PPP_IFACE,
            this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));
}

/*===========================================================================
  FUNCTION BlockIPv4WWANAccessCradle
==========================================================================*/
/*!
@brief
  This function blocks IPv4 WWAN access for MobileAP clients when cradle is
  enabled.  All clients will only be able to connect to backhaul via the
  cradle.  This is needed so rmnet/wlan0 (AP-STA) do not have access to cradle.

@parameters
  void

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::BlockIPv4WWANAccessCradle()
{
  char command[MAX_COMMAND_STR_LEN];

/*===========================================================================
  Block WWAN access to MobileAP clients.
==========================================================================*/
  /* First delete duplicate rule if any. */
  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -D FORWARD -i %s ! -o %s -j DROP", BRIDGE_IFACE,
            this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -D FORWARD -i %s ! -o %s -j DROP", BRIDGE_IFACE,
            ECM_IFACE);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -A FORWARD -i %s ! -o %s -j DROP", BRIDGE_IFACE,
            ECM_IFACE);
  ds_system_call(command, strlen(command));

/*===========================================================================
  Block WWAN access to PPP client.
==========================================================================*/
  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -D FORWARD -i %s ! -o %s -j DROP", PPP_IFACE,
            this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));
  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -D FORWARD -i %s ! -o %s -j DROP", PPP_IFACE,
            ECM_IFACE);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "iptables -A FORWARD -i %s ! -o %s -j DROP", PPP_IFACE,
            ECM_IFACE);
  ds_system_call(command, strlen(command));
}

/*===========================================================================
  FUNCTION BlockIPv6WWANAccess
==========================================================================*/
/*!
@brief
  This function blocks IPv6 WWAN access for MobileAP clients.

@parameters
  void

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::BlockIPv6WWANAccess()
{
  char command[MAX_COMMAND_STR_LEN];
  int qcmap_cm_error=0;

/*===========================================================================
  Allow access between Bridge and Bridge.
==========================================================================*/
  /* First delete duplicate rule if any. */
  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -D FORWARD -i %s -o %s -j ACCEPT", BRIDGE_IFACE, BRIDGE_IFACE);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -A FORWARD -i %s -o %s -j ACCEPT", BRIDGE_IFACE, BRIDGE_IFACE);
  ds_system_call(command, strlen(command));

/*===========================================================================
  Allow access between Bridge and PPP.
==========================================================================*/
  /* First delete duplicate rule if any. */
  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -D FORWARD -i %s -o %s -j ACCEPT", BRIDGE_IFACE, PPP_IFACE);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -A FORWARD -i %s -o %s -j ACCEPT", BRIDGE_IFACE, PPP_IFACE);
  ds_system_call(command, strlen(command));

  /* First delete duplicate rule if any. */
  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -D FORWARD -i %s -o %s -j ACCEPT", PPP_IFACE, BRIDGE_IFACE);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -A FORWARD -i %s -o %s -j ACCEPT", PPP_IFACE, BRIDGE_IFACE);
  ds_system_call(command, strlen(command));

/*===========================================================================
  Block WWAN access to MobileAP clients.
==========================================================================*/
  /* First delete duplicate rule if any. */
  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -D FORWARD -i %s ! -o %s -j DROP", BRIDGE_IFACE,
            this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));
  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -D FORWARD -i %s ! -o %s -j DROP", BRIDGE_IFACE,
            ECM_IFACE);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -A FORWARD -i %s ! -o %s -j DROP", BRIDGE_IFACE,
            this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));

/*===========================================================================
  Block WWAN access to PPP client.
==========================================================================*/
  /* First delete duplicate rule if any. */
  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -D FORWARD -i %s ! -o %s -j DROP", PPP_IFACE,
            this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));
  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -D FORWARD -i %s ! -o %s -j DROP", PPP_IFACE,
            ECM_IFACE);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -A FORWARD -i %s ! -o %s -j DROP", PPP_IFACE,
            this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));

}

/*===========================================================================
  FUNCTION BlockIPv6WWANAccessCradle
==========================================================================*/
/*!
@brief
  This function blocks IPv6 WWAN access for MobileAP clients when in cradle
  mode. All clients will only be able to connect to backhaul via the
  cradle.  This is needed so rmnet/wlan0 (AP-STA) do not have access to cradle.

@parameters
  void

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::BlockIPv6WWANAccessCradle()
{
  char command[MAX_COMMAND_STR_LEN];

/*===========================================================================
  Block WWAN access to MobileAP clients.
==========================================================================*/
  /* First delete duplicate rule if any. */
  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -D FORWARD -i %s ! -o %s -j DROP", BRIDGE_IFACE,
            this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));
  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -D FORWARD -i %s ! -o %s -j DROP", BRIDGE_IFACE,
            ECM_IFACE);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -A FORWARD -i %s ! -o %s -j DROP", BRIDGE_IFACE,
            ECM_IFACE);
  ds_system_call(command, strlen(command));

/*===========================================================================
  Block WWAN access to PPP client.
==========================================================================*/
  /* First delete duplicate rule if any. */
  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -D FORWARD -i %s ! -o %s -j DROP", PPP_IFACE,
            this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));
  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -D FORWARD -i %s ! -o %s -j DROP", PPP_IFACE,
            ECM_IFACE);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -A FORWARD -i %s ! -o %s -j DROP", PPP_IFACE,
            ECM_IFACE);
  ds_system_call(command, strlen(command));
}

/*===========================================================================
  FUNCTION AllowIPv6WWANAccess
==========================================================================*/
/*!
@brief
  This function allows IPv6 WWAN access for MobileAP clients.

@parameters
  void

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::AllowIPv6WWANAccess()
{
  char command[MAX_COMMAND_STR_LEN];
  int qcmap_cm_error=0;

  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -D FORWARD -i %s -o %s -j ACCEPT", BRIDGE_IFACE, BRIDGE_IFACE);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -D FORWARD -i %s -o %s -j ACCEPT", BRIDGE_IFACE, PPP_IFACE);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -D FORWARD -i %s -o %s -j ACCEPT", PPP_IFACE, BRIDGE_IFACE);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -D FORWARD -i %s ! -o %s -j DROP", BRIDGE_IFACE,
            this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -D FORWARD -i %s ! -o %s -j DROP", BRIDGE_IFACE, ECM_IFACE);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -D FORWARD -i %s ! -o %s -j DROP", PPP_IFACE,
            this->cfg.lan_config.sta_interface);
  ds_system_call(command, strlen(command));

  snprintf(command, MAX_COMMAND_STR_LEN,
            "ip6tables -D FORWARD -i %s ! -o %s -j DROP", PPP_IFACE, ECM_IFACE);
  ds_system_call(command, strlen(command));
}

/*===========================================================================
  FUNCTION ReadConfigFromXML
==========================================================================*/
/*!
@brief
  Reads the mobileap configuration file.

@parameters
  void

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
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

/*===========================================================================
  FUNCTION ReadConfigFromFirewallXML
==========================================================================*/
/*!
@brief
  Read firewall rules from firewall configuration file.

@parameters
  void

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::ReadConfigFromFirewallXML()
{

  if ( ( this->cfg.nat_config.firewall_config_file ) && ( this->cfg.nat_config.firewall_config_file[0] != '\0' ) &&
       ( qcmap_cm_read_firewall_xml(this->cfg.nat_config.firewall_config_file, &this->cfg) == QCMAP_CM_SUCCESS ))
  {
    return true;
  }
  else
    return false;
}


/*===========================================================================
  FUNCTION WriteConfigToXML
==========================================================================*/
/*!
@brief
  Writes the mobileap configuration file with the latest configuration values.

@parameters
  void

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::WriteConfigToXML()
{
  char command[MAX_COMMAND_STR_LEN];

  bzero(&command,MAX_COMMAND_STR_LEN);
  if ( ( this->xml_path ) && ( this->xml_path[0] != '\0' ) &&
       ( qcmap_cm_write_xml( TEMP_MOBILEAP_CFG, &this->cfg ) == QCMAP_CM_SUCCESS ))
  {
    /* First update the configuration to the temporary config file, on success
       copy the configuration file to the primary configuration file*/
    snprintf( command, MAX_COMMAND_STR_LEN,"mv %s %s ",TEMP_MOBILEAP_CFG ,this->xml_path);
    ds_system_call(command, strlen(command));
    return true;
  }
  else
    return false;
}


/*===========================================================================
  FUNCTION WriteConfigToFirewallXML
==========================================================================*/
/*!
@brief
  Writes the latest firewall configuration values.

@parameters
  void

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::WriteConfigToFirewallXML()
{
  char command[MAX_COMMAND_STR_LEN];

  bzero(&command,MAX_COMMAND_STR_LEN);
  if ((this->cfg.nat_config.firewall_config_file) && (this->cfg.nat_config.firewall_config_file[0] != '\0') &&
      (qcmap_cm_write_firewall_xml(TEMP_FIREWALL_CFG,&this->cfg) == QCMAP_CM_SUCCESS))
  {
    snprintf( command, MAX_COMMAND_STR_LEN,"mv %s %s ",TEMP_FIREWALL_CFG ,this->cfg.nat_config.firewall_config_file);
    ds_system_call(command, strlen(command));
    return true;
  }
  else
    return false;
}


/*===========================================================================
  FUNCTION ConnectBackHaul
==========================================================================*/
/*!
@brief
  Tries to connect to backhaul. As a part of connect to backhaul checks for the
  network service and nas configuration.
  If the network is in roaming mode,  connects to backhaul based on the roaming flag configuration.

@parameters
  qcmap_msgr_wwan_call_type_v01 call_type
  int                           *err_num

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::ConnectBackHaul(qcmap_msgr_wwan_call_type_v01 call_type, int *err_num, qmi_error_type_v01 *qmi_err_num)
{
  int qcmap_cm_errno;
  int ret_val;
  int rc = 0;
  struct timespec ts;
  in_addr_t public_ip = 0,  sec_dns_addr = 0, default_gw_addr = 0;
  uint32 pri_dns_addr = 0;
  char command[MAX_COMMAND_STR_LEN];
  QCMAP_CM_LOG_FUNC_ENTRY();

  LOG_MSG_INFO1("In ConnectBackHaul %x Call Type:%d\n",
                this->qcmap_cm_handle, call_type, 0);
  ret_val = qcmap_cm_connect_backhaul(this->qcmap_cm_handle, call_type, &qcmap_cm_errno, qmi_err_num);

  if (ret_val == QCMAP_CM_SUCCESS)
  {
    *err_num = qcmap_cm_errno;
    return true;
  }

  if (ret_val == QCMAP_CM_ERROR && qcmap_cm_errno == QCMAP_CM_EWOULDBLOCK)
  {
    LOG_MSG_INFO1("QCMAP WAN Connecting Inprogress \n",0,0,0);
    return true;
  }
  else
  {
    LOG_MSG_INFO1( "QCMAP WAN Connecting Fail, ret_val %d qcmap_cm_errno %d\n",
                  ret_val, qcmap_cm_errno,0 );
    return false;
  }
}

/*===========================================================================
  FUNCTION EnableDNS
==========================================================================*/
/*!
@brief
  Adds the IP addresses of nameservers available, received from network.

@parameters
  void

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::EnableDNS()
{
  int qcmap_cm_error;
  int ret_val;
  int rc = 0;
  struct timespec ts;
  in_addr_t public_ip = 0,  sec_dns_addr = 0, default_gw_addr = 0;
  uint32 pri_dns_addr = 0;
  struct in_addr addr;
  char command[MAX_COMMAND_STR_LEN];
  qmi_error_type_v01 qmi_err_num;

  if ( qcmap_cm_get_ipv4_net_conf(this->qcmap_cm_handle, &public_ip, &pri_dns_addr,
       &sec_dns_addr, &default_gw_addr, &qmi_err_num) != QCMAP_CM_SUCCESS )
  {
    LOG_MSG_ERROR("Error in qcmap_cm_get_ipv4_net_conf %d\n", qmi_err_num,0,0);
  }

  if ( !pri_dns_addr && !sec_dns_addr )
  {
    LOG_MSG_INFO1("QCMAP DNS not enabled \n",0,0,0);
    return false;
  }

  if ( pri_dns_addr )
  {
    addr.s_addr = pri_dns_addr;
    snprintf(command, MAX_COMMAND_STR_LEN, "echo 'nameserver %s' > /etc/resolv.conf", inet_ntoa(addr));
    LOG_MSG_INFO1("QCMAP PRI DNS %s\n", inet_ntoa(addr),0,0);
    ds_system_call(command, strlen(command));
  }

  if ( sec_dns_addr )
  {
    addr.s_addr = sec_dns_addr;
    snprintf(command, MAX_COMMAND_STR_LEN, "echo 'nameserver %s' >> /etc/resolv.conf", inet_ntoa(addr));
    LOG_MSG_INFO1("QCMAP SEC DNS %s\n", inet_ntoa(addr),0,0);
    ds_system_call(command, strlen(command));
  }

  LOG_MSG_INFO1("QCMAP DNS Enabled \n",0,0,0);
  return true;
}

/*===========================================================================
  FUNCTION DisconnectBackHaul
==========================================================================*/
/*!
@brief
  Brings down the bachaul service.

@parameters
  qcmap_msgr_wwan_call_type_v01 call_type
  int                           *err_num

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::DisconnectBackHaul(qcmap_msgr_wwan_call_type_v01 call_type,int *err_num, qmi_error_type_v01 *qmi_err_num)
{
  int qcmap_cm_errno;
  int ret_val;

  QCMAP_CM_LOG_FUNC_ENTRY();

  ret_val = qcmap_cm_disconnect_backhaul(this->qcmap_cm_handle, call_type, &qcmap_cm_errno, qmi_err_num);

  if (ret_val == QCMAP_CM_SUCCESS)
  {
    this->enable_dns = true;
    *err_num = qcmap_cm_errno;
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


/*===========================================================================
  FUNCTION SetAutoconnect
==========================================================================*/
/*!
@brief
  This function will enable the autoconnect flag.
  If the mobileap and auto connect is enabled, connect to backhaul is triggered.
  In case connect to backhaul fails, then a autoconnect timer is triggered. After the expiry of
  autoconnect timer a new request to connect to backhaul is initiated.

@parameters
  boolean enable

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetAutoconnect(boolean enable, qmi_error_type_v01 *qmi_err_num)
{
  int qcmap_cm_errno;
  int ret_val;
  boolean timer_running;

  /* Config Auto Connect */
  ret_val = qcmap_cm_set_auto_connect(this->qcmap_cm_handle, &qcmap_cm_errno, enable, qmi_err_num);
  if (ret_val != QCMAP_CM_SUCCESS)
    return false;

  this->cfg.wan_config.auto_connect = enable;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

/*===========================================================================
  FUNCTION GetAutoconnect
==========================================================================*/
/*!
@brief
  Returns autoconnect flag value.

@parameters
  value

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::GetAutoconnect()
{
  return this->cfg.wan_config.auto_connect;
}

/*===========================================================================
  FUNCTION SetRoaming
==========================================================================*/
/*!
@brief
  This function will set the roaming flag value. Enabling the roaming flag
  will enable connecting to a Network in roaming mode.

@parameters
  boolean enable

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::SetRoaming(boolean enable, qmi_error_type_v01 *qmi_err_num)
{
  int ret_val;

  /* Config Auto Connect */
  ret_val = qcmap_cm_set_roaming(this->qcmap_cm_handle, qmi_err_num, enable);
  if (ret_val != QCMAP_CM_SUCCESS)
    return false;

  this->cfg.wan_config.roaming = enable;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}


/*===========================================================================
  FUNCTION GetRoaming
==========================================================================*/
/*!
@brief
  Returns roaming flag value.

@parameters
  void

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::GetRoaming()
{
 return this->cfg.wan_config.roaming;
}

/*===========================================================================
  FUNCTION GetWWANStatistics
==========================================================================*/
/*!
@brief
  Used to Get the WWAN data transfer statistics.

@parameters
  qcmap_msgr_ip_family_enum_v01       ip_family
  qcmap_msgr_wwan_statistics_type_v01 *wwan_stats
  int                                 *p_error

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetWWANStatistics(
                                   qcmap_msgr_ip_family_enum_v01 ip_family,
                                   qcmap_msgr_wwan_statistics_type_v01 *wwan_stats,
                                   qmi_error_type_v01 *qmi_err_num)
{
  int ret_val;

  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Get WWAN Statistics */
  ret_val = qcmap_cm_get_wwan_statistics(this->qcmap_cm_handle,
                                         ip_family,
                                         wwan_stats,
                                         qmi_err_num);
  if ( ret_val != QCMAP_CM_SUCCESS )
    return false;

  return true;
}


/*===========================================================================
  FUNCTION ResetWWANStatistics
==========================================================================*/
/*!
@brief
  Resets WWAN data transfer statistics to zero.

@parameters
  qcmap_msgr_ip_family_enum_v01       ip_family
  int                                 *p_error

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::ResetWWANStatistics(
                                   qcmap_msgr_ip_family_enum_v01 ip_family,
                                   qmi_error_type_v01 *qmi_err_num)
{
  int ret_val;

  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Reset WWAN Statistics */
  ret_val = qcmap_cm_reset_wwan_statistics(this->qcmap_cm_handle,
                                           ip_family,
                                           qmi_err_num);
  if ( ret_val != QCMAP_CM_SUCCESS )
    return false;

  return true;
}

/*===========================================================================
  FUNCTION StartDHCPD
==========================================================================*/
/*!
@brief
  This function will start the dhcp service with the configured values
  for the enabled interfaces.

@parameters
  void

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::StartDHCPD()
{
  char command[MAX_COMMAND_STR_LEN];
  char scratch_buf[MAX_COMMAND_STR_LEN];
  in_addr start, end, subnet_mask, gw_addr;

  ds_system_call("echo QCMAP:Start DHCP server > /dev/kmsg",
          strlen("echo QCMAP:Start DHCP server > /dev/kmsg"));

  bzero(command, MAX_COMMAND_STR_LEN);
  if (this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].enable_dhcpd)
  {
    strlcpy(command, "dnsmasq", MAX_COMMAND_STR_LEN);
    snprintf(scratch_buf, MAX_COMMAND_STR_LEN, " -i %s -I lo -z", BRIDGE_IFACE);
    strlcat(command, scratch_buf, MAX_COMMAND_STR_LEN);

    strlcat(command, "  --dhcp-range=", MAX_COMMAND_STR_LEN);
    /* Insert a GetInterfaceName call here.*/
    snprintf(scratch_buf, MAX_COMMAND_STR_LEN, "%s,", BRIDGE_IFACE);

    strlcat(command, scratch_buf, MAX_COMMAND_STR_LEN);

    start.s_addr = htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_start_address);
    strlcat(command, inet_ntoa(start), MAX_COMMAND_STR_LEN);

    strlcat(command, ",", MAX_COMMAND_STR_LEN);

    /* Reserve IP for PPP only if we have more than 1 IP available. */
    if ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_end_address >
         this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_start_address)
    {
      end.s_addr =
             htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_end_address - 1);
      this->cfg.lan_config.ppp_reserved_ip =
                   this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_end_address;
    }
    else
    {
      LOG_MSG_INFO1("No IP reserved for DUN client", 0, 0, 0);
      end.s_addr =
             htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_end_address);
      this->cfg.lan_config.ppp_reserved_ip = 0;
    }

    strlcat(command, inet_ntoa(end), MAX_COMMAND_STR_LEN);

    strlcat(command, ",", MAX_COMMAND_STR_LEN);

    subnet_mask.s_addr = htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].sub_net_mask);
    strlcat(command, inet_ntoa(subnet_mask), MAX_COMMAND_STR_LEN);

    strlcat(command, ",", MAX_COMMAND_STR_LEN);

    strlcat(command, this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_lease_time, MAX_COMMAND_STR_LEN);
    /*if dhcp-hostsfile exits append this to the dnsmasq option*/
    dhcp_hosts_fp = fopen(DHCP_HOSTS_FILE, "rb");
    /* File could not be opened for writing/append*/
    if (NULL == dhcp_hosts_fp)
    {
        LOG_MSG_INFO1("\nFile /etc/dhcp_hosts does not exists!!",0,0,0);
    }
    else
    {

        strlcat(command, " --dhcp-hostsfile=", MAX_COMMAND_STR_LEN);
        /* Insert the DHCP hosts file.*/
        snprintf(scratch_buf, MAX_COMMAND_STR_LEN, "%s", DHCP_HOSTS_FILE);
        strlcat(command, scratch_buf, MAX_COMMAND_STR_LEN);
        fclose(dhcp_hosts_fp);
    }

    /* DNS Gateway address, to used in DHCP ACK message*/
    strlcat(command, " --dhcp-option-force=6,", MAX_COMMAND_STR_LEN);
    gw_addr.s_addr = htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].a5_ip_addr);
    strlcat(command, inet_ntoa(gw_addr), MAX_COMMAND_STR_LEN);

    /* Copy DHCP command. Used when LAN config has not been re-activated
       with changed values, but few other components of DHCPD change. */
    strlcpy(this->dhcp_command_str, command, MAX_COMMAND_STR_LEN);

    /*Update DHCPD daemon with network SIP server, MTU info */
    UpdateDHCPDNetworkInfo();
  }

  ds_system_call("echo QCMAP:DHCP server started > /dev/kmsg",
          strlen("echo QCMAP:DHCP server started > /dev/kmsg"));
  return true;
}

/*===========================================================================
  FUNCTION GetWWANMTUInfo
==========================================================================*/
/*!
@brief
  This function will get mtu size from wwan ipv4 interface .

@parameters
  int* mtu_wwan_ipv4

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetWWANMTUInfo(int* mtu_wwan_ipv4)
{
  int socket_mtu, af = AF_INET;
  struct ifreq ifr;
  int qcmap_cm_error;
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];

  /* get mtu value for rmnet of ipv4 wwan backhaul starts */
  /* Get Device Name */
  /* Send the RS packet to the rmnet iface */
  memset(devname, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);
  if(qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_MSGR_IP_FAMILY_V4_V01,
     devname, &qcmap_cm_error) != QCMAP_CM_SUCCESS )
  {
    LOG_MSG_ERROR("Couldn't get ipv4 rmnet name. error %d\n", qcmap_cm_error,0,0);
    return false;
  }

  memset(&ifr, 0, sizeof(ifr));
  snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), devname);
  /* get mtu from found device for ipv4 starts */
  if((socket_mtu = socket(af, SOCK_DGRAM, 0)) < 0)
  {
    LOG_MSG_ERROR("Couldn't create socket to get mtu of ipv4 wwan err %d\n", errno,0,0);
    return false;
  }
  ifr.ifr_addr.sa_family = AF_INET;
  if (ioctl(socket_mtu, SIOCGIFMTU, &ifr) < 0)
  {
    LOG_MSG_ERROR("Couldn't get mtu from ipv4 wwan iface. err %d\n", errno,0,0);
    close(socket_mtu);
    return false;
  }
  LOG_MSG_INFO1("Successfully obtained MTU of wwan IPV4 iface  %d",  ifr.ifr_mtu, 0, 0);
  close(socket_mtu);
  /* get mtu from found device for ipv4 wwan iface ends */
  if(ifr.ifr_mtu > 0 )
  {
    *mtu_wwan_ipv4=(int) ifr.ifr_mtu;
    return true;
  }
  else
  {
    LOG_MSG_ERROR("Couldn't get .mtu i.e not got it from wwan  %d\n", ifr.ifr_mtu,0,0);
    return false;
  }
}
/*===========================================================================
  FUNCTION UpdateDHCPDNetworkInfo
==========================================================================*/
/*!
@brief
  This function will re-start the dhcpd service with the configured SIP server, mtu
  information.

@parameters
  void

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::UpdateDHCPDNetworkInfo()
{
  char dhcp_command[MAX_DHCP_COMMAND_STR_LEN];
  in_addr pcscf_addr;
  int mtu_size_wwan_ipv4;
  char mtu_string[MTU_STRING_SIZE];
  memset(dhcp_command, 0, MAX_DHCP_COMMAND_STR_LEN);
  strlcpy(dhcp_command,this->dhcp_command_str, MAX_DHCP_COMMAND_STR_LEN);
  /* if either sta backhaul connected or cradle backhaul connected ignore mtu wwanipv4*/
  if ( (!this->sta_connected) && (!this->cradle_backhaul_connected) )
  {
    if( GetWWANMTUInfo(&mtu_size_wwan_ipv4) == true )
    {
      /* add mtu wwan ipv4 as dhcp option 26 starts */
      if(mtu_size_wwan_ipv4 > 0 )
      {
        strlcat(dhcp_command, " --dhcp-option-force=", MAX_DHCP_COMMAND_STR_LEN);
        strlcat(dhcp_command, DHCP_OPTION_MTU, MAX_DHCP_COMMAND_STR_LEN);
        strlcat(dhcp_command, ",", MAX_DHCP_COMMAND_STR_LEN);
        memset(mtu_string, 0, sizeof(mtu_string));
        snprintf(mtu_string,sizeof(mtu_string),"%d",(int) mtu_size_wwan_ipv4);
        strlcat(dhcp_command, mtu_string, MAX_DHCP_COMMAND_STR_LEN);
      }
      else
      {
        LOG_MSG_ERROR("Couldn't set .mtu i.e not got it from wwan  %d\n", mtu_size_wwan_ipv4,0,0);
      }
      /* add mtu wwan ipv4 as dhcp option 26 ends */
    }
  }
  strlcat(dhcp_command, " --dhcp-option-force=120", MAX_DHCP_COMMAND_STR_LEN);

  if (this->cfg.wan_config.default_sip_server_info.sip_serv_type == QCMAP_CM_SIP_SERVER_ADDR)
  {
    strlcat(dhcp_command, ",", MAX_DHCP_COMMAND_STR_LEN);
    pcscf_addr.s_addr = this->cfg.wan_config.default_sip_server_info.sip_serv_info.sip_serv_ip;
    strlcat(dhcp_command,
            inet_ntoa(pcscf_addr),
            MAX_DHCP_COMMAND_STR_LEN);
  }
  else if (this->cfg.wan_config.default_sip_server_info.sip_serv_type == QCMAP_CM_SIP_SERVER_FQDN)
  {
    strlcat(dhcp_command, ",", MAX_DHCP_COMMAND_STR_LEN);
    strlcat(dhcp_command, "\"", MAX_DHCP_COMMAND_STR_LEN);
    strlcat(dhcp_command,
            this->cfg.wan_config.default_sip_server_info.sip_serv_info.sip_fqdn,
            MAX_DHCP_COMMAND_STR_LEN);
    strlcat(dhcp_command, "\"", MAX_DHCP_COMMAND_STR_LEN);
  }

  LOG_MSG_INFO1("%d network assigned PCSCF info",
                 this->cfg.wan_config.sip_server_list_count, 0, 0);
  if (this->cfg.wan_config.sip_server_list_count > 0)
  {
    for (int i = 0; i < this->cfg.wan_config.sip_server_list_count; i++)
    {
      strlcat(dhcp_command, ",", MAX_DHCP_COMMAND_STR_LEN);
      if (this->cfg.wan_config.sip_server_info_list[i].sip_serv_type == QCMAP_CM_SIP_SERVER_ADDR)
      {
        LOG_MSG_INFO1("Adding network assigned PCSCF address", 0, 0, 0);
        pcscf_addr.s_addr = this->cfg.wan_config.sip_server_info_list[i].sip_serv_info.sip_serv_ip;
        strlcat(dhcp_command,
                inet_ntoa(pcscf_addr),
                (MAX_DHCP_COMMAND_STR_LEN - strlen(dhcp_command)));
      }
      else if (this->cfg.wan_config.sip_server_info_list[i].sip_serv_type == QCMAP_CM_SIP_SERVER_FQDN)
      {
        strlcat(dhcp_command, "\"", MAX_DHCP_COMMAND_STR_LEN);
        strlcat(dhcp_command,
                this->cfg.wan_config.sip_server_info_list[i].sip_serv_info.sip_fqdn,
                MAX_DHCP_COMMAND_STR_LEN);
        strlcat(dhcp_command, "\"", MAX_DHCP_COMMAND_STR_LEN);
      }
    }
  }

  ds_system_call("killall -9 dnsmasq", strlen("killall -9 dnsmasq"));

  ds_system_call(dhcp_command, strlen(dhcp_command));

  return true;
}

/*===========================================================================
  FUNCTION StopDHCPD
==========================================================================*/
/*!
@brief
  Stops the dhcp services started.

@parameters
  void

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::StopDHCPD()
{
  char command[MAX_COMMAND_STR_LEN];

   snprintf(command,MAX_COMMAND_STR_LEN, " killall -9 dnsmasq");
   ds_system_call(command, strlen(command));

  /* Clear off the reservatiosn flag. */
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_reservations_updated = FALSE;

  return true;
}

/*===========================================================================
  FUNCTION StartHostAPD
==========================================================================*/
/*!
@brief
  Starts hostapd service with the appropriate configuration
  files based on the wlan mode configured.

@parameters
  int interface

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::StartHostAPD(int interface, int wlan_dev_index)
{
  char command[MAX_COMMAND_STR_LEN];
  char temp_command[MAX_COMMAND_STR_LEN];
  qcmap_msgr_activate_hostapd_ap_enum_v01 ap_type;

  ds_system_call("echo QCMAP:Start Hostapd > /dev/kmsg",\
                  strlen("echo QCMAP:Start Hostapd > /dev/kmsg"));

  snprintf(command, MAX_COMMAND_STR_LEN, "sh /etc/cld-hostapd-wlan%d.sh",
           wlan_dev_index);
  ds_system_call(command, strlen(command));

  if ( interface == QCMAP_MSGR_INTF_AP_INDEX &&
       this->wifi_mode == QCMAP_MSGR_WLAN_MODE_AP_STA_V01 )
  {
    snprintf(command, MAX_COMMAND_STR_LEN, "hostapd -B %s -P %s -e %s ",
             this->cfg.lan_config.interface[interface].path_to_sta_mode_hostapd_conf,
             HOSTAPD_PID_FILE,
             this->cfg.lan_config.interface[interface].path_to_hostapd_entropy);

    ap_type = QCMAP_MSGR_PRIMARY_AP_V01;
  }
  else if ( interface == QCMAP_MSGR_INTF_AP_INDEX )
  {
    snprintf(command, MAX_COMMAND_STR_LEN, "hostapd -B %s -P %s -e %s ",
             this->cfg.lan_config.interface[interface].path_to_hostapd_conf,
             HOSTAPD_PID_FILE,
             this->cfg.lan_config.interface[interface].path_to_hostapd_entropy);

    ap_type = QCMAP_MSGR_PRIMARY_AP_V01;
  }
  else
  {
    snprintf(command, MAX_COMMAND_STR_LEN, "hostapd -B %s -P %s -e %s ",
             this->cfg.lan_config.interface[interface].path_to_hostapd_conf,
             HOSTAPD_SSID2_PID_FILE,
             this->cfg.lan_config.interface[interface].path_to_hostapd_entropy);

    ap_type = QCMAP_MSGR_GUEST_AP_V01;
  }

  /* If debug mode is enabled add concat dbg cmd  */
  if (this->cfg.debug_config.hostapd_debug)
  {
    snprintf(temp_command,
             MAX_COMMAND_STR_LEN, "%s %s", command,
             this->cfg.debug_config.hostapd_dbg_cmd);
    memcpy(command, temp_command, MAX_COMMAND_STR_LEN);
  }

  ds_system_call(command, strlen(command));

  //Start Hostapd_cli on the interface
  this->StartHostapdCli(ap_type);

  ds_system_call("echo QCMAP:Started Hostapd > /dev/kmsg",\
                  strlen("echo QCMAP:Started Hostapd > /dev/kmsg"));
  return true;
}

/*===========================================================================
  FUNCTION StopHostAPD
==========================================================================*/
/*!
@brief
  Stops hostapd service.

@parameters
  int interface

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::StopHostAPD(void)
{
  FILE *fp = NULL;
  int ap_pid = 0, guest_ap_pid = 0;
  char command[MAX_COMMAND_STR_LEN];

  ds_system_call("killall hostapd",
                 strlen("killall hostapd"));

  switch (this->wifi_mode)
  {
    case QCMAP_MSGR_WLAN_MODE_AP_V01:
    case QCMAP_MSGR_WLAN_MODE_AP_STA_V01:
         /* Only one instance of hostapd is running */
         if (!IsHostapdkilled(0) )
         {
            LOG_MSG_INFO1("Forcefully terminating hostapd", 0, 0, 0);
            ds_system_call("killall -9 hostapd", strlen("killall -9 hostapd"));
         }
         snprintf( command, MAX_COMMAND_STR_LEN,"rm %s",HOSTAPD_PID_FILE);
         ds_system_call(command, strlen(command));
         break;

    case QCMAP_MSGR_WLAN_MODE_AP_AP_V01:
     /* Get the Primary AP PID. */
         fp = fopen(HOSTAPD_PID_FILE, "r");
         if ( fp == NULL )
         {
           LOG_MSG_ERROR("Error opening hostapd pid file: %d.\n", errno, 0, 0);
         }
         else if (fscanf(fp, "%d", &ap_pid) != 1)
         {
           LOG_MSG_ERROR("primary AP hostapd is stopped: %d.\n", errno, 0, 0);
           ap_pid = 0;
           fclose(fp);
         }
         else
         {
           LOG_MSG_INFO1("Primary AP PID = %d",ap_pid,0,0);
           fclose(fp);
         }

         /* Get the Guest AP PID. */
         fp = fopen(HOSTAPD_SSID2_PID_FILE, "r");
         if ( fp == NULL )
         {
           LOG_MSG_ERROR("Error opening Guest AP hostapd pid file: %d.\n", errno, 0, 0);
         }
         else if (fscanf(fp, "%d", &guest_ap_pid) != 1)
         {
           LOG_MSG_ERROR("Guest AP hostapd is stopped: %d.\n", errno, 0, 0);
           guest_ap_pid = 0;
           fclose(fp);
         }
         else
         {
           LOG_MSG_INFO1("Guest AP PID = %d",guest_ap_pid,0,0);
           fclose(fp);
         }
         /* Check if Primary ap hostapd is killed */
         if(guest_ap_pid !=0 )
         {
           if ( !IsHostapdkilled(guest_ap_pid) )
           {
             LOG_MSG_INFO1("Forcefully terminating hostapd", 0, 0, 0);
             ds_system_call("killall -9 hostapd", strlen("killall -9 hostapd"));
           }
         }
         snprintf( command, MAX_COMMAND_STR_LEN,"rm %s",HOSTAPD_PID_FILE);
         ds_system_call(command, strlen(command));
         /* Check if Guest ap hostapd is killed */
         if(ap_pid !=0 )
         {
           if (!IsHostapdkilled(ap_pid) )
           {
             LOG_MSG_INFO1("Forcefully terminating hostapd", 0, 0, 0);
             ds_system_call("killall -9 hostapd", strlen("killall -9 hostapd"));
           }
         }
         snprintf( command, MAX_COMMAND_STR_LEN,"rm %s",HOSTAPD_SSID2_PID_FILE);
         ds_system_call(command, strlen(command));
         break ;

   default:
         break;
  }
  return true;
}


/*===========================================================================
  FUNCTION StartMcastDaemon
==========================================================================*/
/*!
@brief
  Starts pimd service, which provides protocol independent multicast service.

@parameters
  void

@return
  void

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::StartMcastDaemon(void)
{
  ds_system_call("pimd", strlen("pimd"));
}

/*===========================================================================
  FUNCTION StopMcastDaemon
==========================================================================*/
/*!
@brief
  Stops pimd service.

@parameters
  void

@return
  void
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::StopMcastDaemon(void)
{
  ds_system_call("killall -15 pimd", strlen("killall -15 pimd"));
}

/*===========================================================================
  FUNCTION AddSNATEntryOnA5
==========================================================================*/
/*!
@brief
  Prepares  command based on the nat configuration to add SNAT
  entries to kernel and execute the same.

@parameters
  qcmap_cm_port_fwding_entry_conf_t *nat_entry

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::AddSNATEntryOnA5
(
  qcmap_cm_port_fwding_entry_conf_t* nat_entry,
  qmi_error_type_v01 *qmi_err_num
)
{
  char command[MAX_COMMAND_STR_LEN], devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  struct in_addr addr;
  int qcmap_cm_error;
  char tempIP[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char tmp[MAX_COMMAND_STR_LEN];
  /*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  memset(devname, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);

  /* Verify ARGs */
  if( nat_entry == NULL )
  {
    LOG_MSG_ERROR("NULL nat_entry\n",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
    return false;
  }

  if ( !this->sta_connected && !this->cradle_backhaul_connected )
  {
    if ( qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_MSGR_IP_FAMILY_V4_V01, devname, &qcmap_cm_error) != QCMAP_CM_SUCCESS )
    {
      LOG_MSG_ERROR("Couldn't get rmnet name. error %d\n", qcmap_cm_error,0,0);
      *qmi_err_num = QMI_ERR_INTERFACE_NOT_FOUND_V01;
      return false;
    }
  }
  else if ( this->cradle_backhaul_connected )
  {
    strlcpy(devname, ECM_IFACE, QCMAP_MSGR_INTF_LEN);
  }
  else if ( this->sta_connected )
  {
    strlcpy(devname, this->cfg.lan_config.sta_interface, QCMAP_MSGR_INTF_LEN);
  }
  bzero(command,MAX_COMMAND_STR_LEN);
  bzero(tmp,MAX_COMMAND_STR_LEN);

  if( ( this->cfg.nat_config.nat_type == QCMAP_MSGR_NAT_ADDRESS_RESTRICTED_NAT_V01 )||
      ( this->cfg.nat_config.nat_type == QCMAP_MSGR_NAT_FULL_CONE_NAT_V01 ))
  {
    /*The below rule will add SNAT rules after the first rule in PREROUTING chain
      which will make sure that ARC nat rule will be at the begining of the
      PREROUTING chain rules */
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -t nat -I PREROUTING 2 -i %s ",devname);
  }
  else
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -t nat -I PREROUTING -i %s ",devname);
  }

  if ( nat_entry->port_fwding_protocol!=0 )
  {
    snprintf(tmp,MAX_COMMAND_STR_LEN," -p %d ",nat_entry->port_fwding_protocol);
    strcat(command,tmp);
  }

  if ( nat_entry->port_fwding_global_port!=0 )
  {
    bzero(tmp,MAX_COMMAND_STR_LEN);
    snprintf( tmp, MAX_COMMAND_STR_LEN," --dport %d ",nat_entry->port_fwding_global_port);
    strcat(command,tmp);
  }

  if ( nat_entry->port_fwding_private_ip!=0 )
  {
    strcat(command," -j DNAT ");
    bzero(tmp,MAX_COMMAND_STR_LEN);
    addr.s_addr = htonl(nat_entry->port_fwding_private_ip);
    strlcpy(tempIP, inet_ntoa(addr), sizeof(tempIP));
    snprintf(tmp, MAX_COMMAND_STR_LEN," --to-destination %s",tempIP);
    strcat(command,tmp);
    if ( nat_entry->port_fwding_private_port!=0 )
    {
      bzero(tmp,MAX_COMMAND_STR_LEN);
      snprintf( tmp, MAX_COMMAND_STR_LEN,":%d ",nat_entry->port_fwding_private_port);
      strcat(command,tmp);
    }
  }
  ds_system_call(command, strlen(command));
  LOG_MSG_INFO1("%s\n", command,0,0);
  return true;
}

/*===========================================================================
  FUNCTION DeleteSNATEntryOnA5
==========================================================================*/
/*!
@brief
  Prepares command based on the nat entry to be deleted.

@parameters
  qcmap_cm_port_fwding_entry_conf_t *nat_entry

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::DeleteSNATEntryOnA5
(
  qcmap_cm_port_fwding_entry_conf_t* nat_entry,
  qmi_error_type_v01 *qmi_err_num
)
{
  char command[MAX_COMMAND_STR_LEN], devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  struct in_addr addr;
  int qcmap_cm_error;
  char tmp[MAX_COMMAND_STR_LEN];
  char tempIP[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
/*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  memset(devname, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);

  /* Verify ARGs */
  if( nat_entry == NULL )
  {
    LOG_MSG_ERROR("NULL nat_entry\n",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
    return false;
  }

  if( !this->sta_connected && !this->cradle_backhaul_connected )
  {
    if ( qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_MSGR_IP_FAMILY_V4_V01, devname, &qcmap_cm_error) != QCMAP_CM_SUCCESS )
    {
      LOG_MSG_ERROR("Couldn't get rmnet name. error %d\n", qcmap_cm_error,0,0);
      *qmi_err_num = QMI_ERR_INTERFACE_NOT_FOUND_V01;
      return false;
    }
  }
  else if ( this->cradle_backhaul_connected )
  {
    strlcpy(devname, ECM_IFACE, QCMAP_MSGR_INTF_LEN);
  }
  else if ( this->sta_connected )
  {
    strlcpy(devname, this->cfg.lan_config.sta_interface, QCMAP_MSGR_INTF_LEN);
  }
  bzero(command,MAX_COMMAND_STR_LEN);
  bzero(tmp,MAX_COMMAND_STR_LEN);
  snprintf( command, MAX_COMMAND_STR_LEN,
      "iptables -t nat -D PREROUTING -i %s ",devname);

  if( nat_entry->port_fwding_protocol!=0 )
  {
    snprintf(tmp,MAX_COMMAND_STR_LEN," -p %d ",nat_entry->port_fwding_protocol);
    strcat(command,tmp);
  }

  if( nat_entry->port_fwding_global_port !=0 )
  {
    bzero(tmp,MAX_COMMAND_STR_LEN);
    snprintf( tmp, MAX_COMMAND_STR_LEN," --dport %d ",nat_entry->port_fwding_global_port);
    strcat(command,tmp);
  }

  if( nat_entry->port_fwding_private_ip !=0 )
  {
    strcat(command," -j DNAT ");
    bzero(tmp,MAX_COMMAND_STR_LEN);
    addr.s_addr = htonl(nat_entry->port_fwding_private_ip);
    strlcpy(tempIP, inet_ntoa(addr), sizeof(tempIP));
    snprintf(tmp, MAX_COMMAND_STR_LEN," --to-destination %s",tempIP);
    strcat(command,tmp);
    if( nat_entry->port_fwding_private_port !=0 )
    {
      bzero(tmp,MAX_COMMAND_STR_LEN);
      snprintf( tmp, MAX_COMMAND_STR_LEN,":%d ",nat_entry->port_fwding_private_port);
      strcat(command,tmp);
    }
  }
  ds_system_call(command, strlen(command));
  LOG_MSG_INFO1("%s\n", command,0,0);
  return true;
}

/*===========================================================================
  FUNCTION AddDMZOnA5
==========================================================================*/
/*!
@brief
  Adds DMZ IP address. Adding DMZ ipaddress will allow incomming packets,
  which are not intended to be received by softap clients to be
  captured on DMZ interface.

@parameters
  uint32       dmzIP

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::AddDMZOnA5
(
  uint32 dmzIP,
 qmi_error_type_v01 *qmi_err_num
)
{
  char command[MAX_COMMAND_STR_LEN], devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  struct in_addr addr;
  int qcmap_cm_error;
  char tempIP[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
/*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  memset(devname, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);

  /* Verify DMZ IP */
  if( dmzIP == 0 )
  {
    LOG_MSG_ERROR("Invalid DMZ IP\n",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
    return false;
  }

  /* Convert IP to ASCII format */
  addr.s_addr = htonl(dmzIP);
  strlcpy(tempIP, inet_ntoa(addr), sizeof(tempIP));

  if ( this->cradle_backhaul_connected )
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -t nat -A PREROUTING -i %s -j DNAT --to-destination %s", ECM_IFACE, tempIP );
  }
  else if( this->sta_connected )
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -t nat -A PREROUTING -i %s -j DNAT --to-destination %s",this->cfg.lan_config.sta_interface,
              tempIP );
  }
  else
  {
    if ( qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_MSGR_IP_FAMILY_V4_V01, devname, &qcmap_cm_error) != QCMAP_CM_SUCCESS )
    {
      LOG_MSG_ERROR("Couldn't get rmnet name. error %d\n", qcmap_cm_error,0,0);
      *qmi_err_num = QMI_ERR_INTERFACE_NOT_FOUND_V01;
      return false;
    }
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -t nat -A PREROUTING -i %s -j DNAT --to-destination %s",
              devname,
              tempIP );
  }

  ds_system_call(command, strlen(command));
  LOG_MSG_INFO1("%s\n", command,0,0);

  return true;
}

/*===========================================================================
  FUNCTION AddDMZ
==========================================================================*/
/*!
@brief
  Configures DMZ ipaddress and updates the configuration file
  with the DMZ IP .

@parameters
  uint32       dmzIP

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::AddDMZ(uint32 dmz_ip, qmi_error_type_v01 *qmi_err_num)
{

  // Check if the DMZ IP is already existing
  if (cfg.nat_config.dmz_ip) {
      LOG_MSG_ERROR("DMZ is already configured",0,0,0);
      *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
      return false;
  }

  this->cfg.nat_config.dmz_ip = dmz_ip;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  if( !AddDMZOnA5(dmz_ip, qmi_err_num) )
  {
     return false;
  }
  return true;
}

/*===========================================================================
  FUNCTION DeleteDMZOnA5
==========================================================================*/
/*!
@brief
  Ths is function will remove the DMZ ipaddress entry.

@parameters
  uint32       dmzIP

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::DeleteDMZOnA5
(
  uint32 dmzIP,
  qmi_error_type_v01 *qmi_err_num
)
{
  char command[MAX_COMMAND_STR_LEN], devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  struct in_addr addr;
  char tempIP[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  int qcmap_cm_error;
/*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  memset(devname, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);
  /* Verify DMZ IP */
  if( dmzIP == 0 )
  {
    LOG_MSG_ERROR("Invalid DMZ IP\n",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
    return false;
  }

  /* Convert IP to ASCII format */
  addr.s_addr = htonl(dmzIP);
  strlcpy(tempIP, inet_ntoa(addr), sizeof(tempIP));

  if ( this->cradle_backhaul_connected )
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -t nat -D PREROUTING -i %s -j DNAT --to-destination %s", ECM_IFACE, tempIP );
  }
  else if ( this->sta_connected )
  {
  snprintf( command, MAX_COMMAND_STR_LEN,
            "iptables -t nat -D PREROUTING -i %s -j DNAT --to-destination %s",this->cfg.lan_config.sta_interface,
            tempIP );
  }
  else
  {
    if ( qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_MSGR_IP_FAMILY_V4_V01, devname, &qcmap_cm_error) != QCMAP_CM_SUCCESS)
    {
      LOG_MSG_ERROR("Couldn't get rmnet name. error %d\n", qcmap_cm_error,0,0);
      *qmi_err_num = QMI_ERR_INTERFACE_NOT_FOUND_V01;
      return false;
    }
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -t nat -D PREROUTING -i %s -j DNAT --to-destination %s",
              devname,
              tempIP );
  }

  ds_system_call(command, strlen(command));
  LOG_MSG_INFO1("%s\n", command,0,0);

  return true;
}

/*===========================================================================
  FUNCTION DeleteDMZ
==========================================================================*/
/*!
@brief
  Will disable DMZ feature.

@parameters
  void

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::DeleteDMZ(qmi_error_type_v01 *qmi_err_num)
{

  if ( !DeleteDMZOnA5(this->cfg.nat_config.dmz_ip, qmi_err_num) )
  {
    if( *qmi_err_num != QMI_ERR_INTERFACE_NOT_FOUND_V01 )
    {
      return false;
    }
  }

  this->cfg.nat_config.dmz_ip = 0;
  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  if( *qmi_err_num == QMI_ERR_INTERFACE_NOT_FOUND_V01 )
  {
    return false;
  }

  return true;
}

/*===========================================================================
  FUNCTION GetDMZ
==========================================================================*/
/*!
@brief
  Gets the DMZ ipaddress configured.

@parameters
  uint32_t *dmz_ip

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetDMZ(uint32_t *dmz_ip, qmi_error_type_v01 *qmi_err_num)
{

  if ( this->cfg.nat_config.dmz_ip == 0 )
  {
    LOG_MSG_ERROR("\nDMZ IP not configured!!",0,0,0);
    *qmi_err_num = QMI_ERR_DISABLED_V01;
    return false;
  }

  *dmz_ip = this->cfg.nat_config.dmz_ip;

  return true;
}

/*===========================================================================
  FUNCTION GetStaIP
==========================================================================*/
/*!
@brief
  Gets IP address and netmask assigned to the STA interface.

@parameters
  uint32 *staIP
  uint32 *netMask
  char* devname

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetIP(uint32 *retIP, uint32 *netMask, char *iface)
{
  int s, ret;
  struct ifreq buffer;
  struct sockaddr *sa;

  if ( retIP == NULL || netMask == NULL )
  {
    LOG_MSG_ERROR("Null arguements passed.\n",0,0,0);
    return false;
  }

  /* Open a socket */
  s = socket(PF_INET, SOCK_DGRAM, 0);
  if ( s < 0 )
  {
    LOG_MSG_ERROR("Unable to open socket to get IP address.\n",0,0,0);
    return false;
  }
  /* Set up the interface request buffer for wlan0. */
  memset(&buffer, 0x00, sizeof(buffer));
  strlcpy(buffer.ifr_name, iface, IFNAMSIZ);

  /* Call the ioctl to get the address. */
  ret = ioctl(s, SIOCGIFADDR, &buffer);

  if ( ret < 0 )
  {
    LOG_MSG_ERROR("Unable to call ioctl to get IP address.\n",0,0,0);
    /* Close the socket handle. */
    close(s);
    return false;
  }

  /* Copy out the ip address for the interface. */
  sa = (struct sockaddr *)&(buffer.ifr_addr);
  *retIP = ((struct sockaddr_in *)sa)->sin_addr.s_addr;

  /* Set up the interface request buffer for wlan0. */
  memset(&buffer, 0x00, sizeof(buffer));
  strlcpy(buffer.ifr_name, iface, IFNAMSIZ);

  /* Call the ioctl to get the address. */
  ret = ioctl(s, SIOCGIFNETMASK, &buffer);

  if ( ret < 0 )
  {
    LOG_MSG_ERROR("Unable to call ioctl to get netmask.\n",0,0,0);
    /* Close the socket handle. */
    close(s);
    return false;
  }

  /* Copy out the netmask for the interface. */
  sa = (struct sockaddr *)&(buffer.ifr_netmask);
  *netMask = ((struct sockaddr_in *)sa)->sin_addr.s_addr;

  /* Close the socket handle. */
  close(s);

  return true;
}

/*===========================================================================
  FUNCTION GetGatewayIP
==========================================================================*/
/*!
@brief
  Gets the Gateway IP assigned to the STA interface.

@parameters
  uint32 *gwIP

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetGatewayIP(uint32 *gwIP)
{
  FILE *fp = NULL;
  char pattern[] = "via"; /* Gateway IP will be followed by Via. */
  char gatewayIP[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  in_addr addr;

  if ( gwIP == NULL )
  {
     LOG_MSG_ERROR("NULL Args.\n", 0,0,0);
     return false;
  }

  memset(gatewayIP, 0, QCMAP_LAN_MAX_IPV4_ADDR_SIZE);

  fp = fopen("/etc/gateway.txt", "r");

  if ( fp == NULL )
  {
     LOG_MSG_ERROR("Error opening Gateway IP file: %d.\n", errno, 0, 0);
     return false;
  }

  if (fscanf(fp, "%s", gatewayIP) != 1) {
    LOG_MSG_ERROR("Error reading Getway IP file: %d.\n", errno, 0, 0);
    fclose(fp);
    return false;
  }

  memset(&addr,0,sizeof(in_addr));
  if (inet_aton(gatewayIP, &addr))
  {
    *gwIP= addr.s_addr;
  }
  else
  {
    LOG_MSG_ERROR("Invalid Getway IP: %d.\n", errno,0,0);
    fclose(fp);
    return false;
  }

  ds_log_med("STA Gateway IP: 0x%s\n", inet_ntoa(addr));

  fclose(fp);
  return true;
}


/*===========================================================================
  FUNCTION DisconnectSTA
==========================================================================*/
/*!
@brief
  Disconnects Station interface from the external hotspot.

@parameters
  void

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::DisconnectSTA()
{
  char command[MAX_COMMAND_STR_LEN];
/*-------------------------------------------------------------------------*/
  QCMAP_CM_LOG_FUNC_ENTRY();

  LOG_MSG_INFO1("Disconnecting STA\n",0,0,0);

  /* Kill WPA Supplicant Process. */
  snprintf( command, MAX_COMMAND_STR_LEN, "killall -15 wpa_supplicant" );
  ds_system_call(command, strlen(command));

  if(!IsWpaSupplicantkilled())
  {
    LOG_MSG_ERROR("Forcefull terminating wpa_supplicant", 0, 0, 0);
    bzero( command, MAX_COMMAND_STR_LEN );
    snprintf( command, MAX_COMMAND_STR_LEN, "killall -9 wpa_supplicant" );
    ds_system_call(command, strlen(command));
  }

  /* Kill WPA Cli Process. */
  snprintf( command, MAX_COMMAND_STR_LEN, "killall -15 wpa_cli" );
  ds_system_call(command, strlen(command));

  if (!IsWpaClikilled())
  {
    /* Kill wpa_cli forcefully. */
    snprintf( command, MAX_COMMAND_STR_LEN, "killall -9 wpa_cli" );
    ds_system_call(command, strlen(command));
  }

  /* Kill DHCPCD Process. */
  snprintf( command, MAX_COMMAND_STR_LEN, "kill -15 $(cat /var/run/dhcpcd-%s.pid)",
            this->cfg.lan_config.sta_interface );
  ds_system_call(command, strlen(command));

  /* Reset the STA Connected Event recieved*/
  this->sta_connected_event_received = false;

  return true;
}

/*===========================================================================
  FUNCTION GetNetworkConfig
==========================================================================*/
/*!
@brief
  Gets the network configured value from WWAN or external hotspot.

@parameters
  in_addr_t  *public_ip,
  uint32     *primary_dns,
  in_addr_t  *secondary_dns,
  int        *err_num

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetNetworkConfig
(
  in_addr_t *public_ip,
  uint32 *primary_dns,
  in_addr_t *secondary_dns,
  qmi_error_type_v01 *qmi_err_num
)
{
  int qmi_error;
  uint32 default_gw = 0;
  in_addr_t netmask = 0;
  FILE *file_ptr=NULL;
  char temp_str[16]="";
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  char char_match[] = "nameserver "; //search for this pattern in file
  boolean pri_dns_found = FALSE; // seperates out primary DNS and secondary DNS

  memset(devname, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);

  if (!this->cfg.lan_config.enable_ipv4)
  {
    /* QCMAP IPV4 is not enabled */
    LOG_MSG_ERROR("QCMAP IPV4 not enabled\n",0,0,0);
    *qmi_err_num = QMI_ERR_NO_NETWORK_FOUND_V01;
    return false;
  }

  if(this->sta_connected || this->cradle_backhaul_connected)
  {
    if(this->cradle_backhaul_connected)
      strlcpy(devname, ECM_IFACE, QCMAP_MSGR_INTF_LEN);
    else
      strlcpy(devname, this->cfg.lan_config.sta_interface, QCMAP_MSGR_INTF_LEN);
    /* Device in Station mode - get config assigned by external hotspot */
    if (this->IsAPSTABridgeActivated())
    {
      strlcpy(devname, BRIDGE_IFACE, strlen(BRIDGE_IFACE)+1);
    }
    else
    {
      strlcpy(devname,this->cfg.lan_config.sta_interface, QCMAP_MSGR_INTF_LEN);
    }

    /* Device in Station mode - get config assigned by external hotspot */
    if (!this->GetIP(public_ip, &netmask, devname))
    {
      LOG_MSG_ERROR("Unable to get the IP address",0,0,0);
      *qmi_err_num = QMI_ERR_INTERNAL_V01;
      return false;
    }

    file_ptr = fopen("/etc/resolv.conf","r");
    if(file_ptr == NULL)
    {
      *qmi_err_num = QMI_ERR_INTERNAL_V01;
      LOG_MSG_ERROR("Error in getting IPv4 net config %d\n", *qmi_err_num,0,0);
      return false;
    }
    while((fgets(temp_str,strlen(char_match)+1,file_ptr)!=NULL))
    {
      if(strncmp(temp_str,char_match,strlen(char_match))==0)
      {
        fscanf(file_ptr, "%s", temp_str);
        if(!pri_dns_found)
        {
          inet_aton(temp_str, (in_addr *)primary_dns);
          pri_dns_found = TRUE;
        }
        else
        {
          inet_aton(temp_str, (in_addr *)secondary_dns);
          break; // found sec dns also - no more looping
        }
      }
    }
    fclose(file_ptr);
  }

  else
  {
    if ( qcmap_cm_get_state(qmi_err_num) != QCMAP_CM_WAN_CONNECTED )
    {
      /* The WWAN is not connected, so it doesn't have an IP or DNS serers*/
      *qmi_err_num = QMI_ERR_NO_NETWORK_FOUND_V01;
      return false;
    }

    qmi_error = qcmap_cm_get_ipv4_net_conf(this->qcmap_cm_handle,
                                           public_ip, primary_dns,
                                           secondary_dns,
                                           &default_gw,
                                           qmi_err_num);

    if ( qmi_error == QCMAP_CM_SUCCESS )
    {
      return true;
    }
  }

  if( (*public_ip == 0) && (*primary_dns == 0) && (*secondary_dns == 0))
  {
    *qmi_err_num = QMI_ERR_INTERNAL_V01;
    LOG_MSG_ERROR("Error in getting IPv4 net config %d\n", *qmi_err_num,0,0);
    return false;
  }

  return true;
}


/*===========================================================================
  FUNCTION GetIPv6NetworkConfig
==========================================================================*/
/*!
@brief
  Gets the IPv6 network configured value.

@parameters
  uint8_t public_ip[]
  uint8_t primary_dns[]
  uint8_t secondary_dns[]
  qmi_error_type_v01 *qmi_err_num

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetIPv6NetworkConfig
(
  uint8_t            public_ip[],
  uint8_t            primary_dns[],
  uint8_t            secondary_dns[],
  qmi_error_type_v01 *qmi_err_num
)
{
  int qmi_error;
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];

  if (!this->cfg.lan_config.enable_ipv6)
  {
    /* QCMAP IPV6 is not enabled */
    LOG_MSG_ERROR("QCMAP IPV6 not enabled\n",0,0,0);
    *qmi_err_num = QMI_ERR_NO_NETWORK_FOUND_V01;
    return false;
  }

  if(this->sta_connected || this->cradle_backhaul_connected)
  {
    if(this->cradle_backhaul_connected)
      strlcpy(devname, ECM_IFACE, QCMAP_MSGR_INTF_LEN);
    else
      strlcpy(devname, this->cfg.lan_config.sta_interface, QCMAP_MSGR_INTF_LEN);
    memset(&this->ipv6_prefix_info, 0, sizeof(qcmap_cm_nl_prefix_info_t));
    this->GetIPV6PrefixInfo(devname, &this->ipv6_prefix_info);
    if ( this->ipv6_prefix_info.prefix_info_valid == true )
    {
      memcpy((in6_addr *)public_ip,
            ((struct sockaddr_in6 *)&(this->ipv6_prefix_info.prefix_addr))\
                                                  ->sin6_addr.s6_addr,
            sizeof(in6_addr));
    }
    else
    {
      *qmi_err_num = QMI_ERR_INTERNAL_V01;
      LOG_MSG_ERROR("Error in getting v6 station "
                    "mode config %d\n", *qmi_err_num,0,0);
      return false;
    }
  }
  else
  {
    if ( qcmap_cm_get_ipv6_state(qmi_err_num) != QCMAP_CM_V6_WAN_CONNECTED )
    {
      /* The WWAN is not connected, so no IPv6 prefix or DNS servers */
      *qmi_err_num = QMI_ERR_NO_NETWORK_FOUND_V01;
      return false;
    }

    qmi_error = qcmap_cm_get_ipv6_net_conf(this->qcmap_cm_handle,
                                           public_ip, primary_dns,
                                           secondary_dns,
                                           qmi_err_num);

    if ( qmi_error == QCMAP_CM_SUCCESS )
    {
      return true;
    }
  }

  if(PS_IN6_IS_ADDR_UNSPECIFIED(public_ip) &&
     PS_IN6_IS_ADDR_UNSPECIFIED(primary_dns) &&
     PS_IN6_IS_ADDR_UNSPECIFIED(secondary_dns))
  {
    *qmi_err_num = QMI_ERR_INTERNAL_V01;
    LOG_MSG_ERROR("Error in getting IPv6 WWAN config %d\n", *qmi_err_num,0,0);
    return false;
  }

  return true;
}


/*===========================================================================
  FUNCTION AddStaticNatEntry
==========================================================================*/
/*!
@brief
  Checks nat entry for the redundancy and if unique adds a new snat entry.

@parameters
  qcmap_cm_port_fwding_entry_conf_t* nat_entry

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::AddStaticNatEntry(qcmap_cm_port_fwding_entry_conf_t* nat_entry, qmi_error_type_v01 *qmi_err_num)
{
  int qcmap_cm_errno;
  int ret_val, i;

  QCMAP_CM_LOG_FUNC_ENTRY();
  for ( i = 0; i < this->cfg.nat_config.num_port_fwding_entries; i++ )
  {
    if ( ( nat_entry->port_fwding_global_port == this->cfg.nat_config.port_fwding_entries[i].port_fwding_global_port) &&
         ( nat_entry->port_fwding_protocol == this->cfg.nat_config.port_fwding_entries[i].port_fwding_protocol ))
    {
      /* we already added this one */
      LOG_MSG_ERROR("\nSNAT Entry already present!!",0,0,0);
      *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
      return false;
    }
  }

  if ( this->cfg.nat_config.num_port_fwding_entries == QCMAP_MSGR_MAX_SNAT_ENTRIES_V01 )
  {
    /* we reach MAX entries */
    LOG_MSG_ERROR("\nReached Max limit on SNAT Entries!!",0,0,0);
    *qmi_err_num = QMI_ERR_INSUFFICIENT_RESOURCES_V01;
    return false;
  }

  /* save into the config */
  memcpy(&this->cfg.nat_config.port_fwding_entries[this->cfg.nat_config.num_port_fwding_entries],
         nat_entry, sizeof(qcmap_cm_port_fwding_entry_conf_t));
  this->cfg.nat_config.num_port_fwding_entries++;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  if ( nat_entry->port_fwding_protocol != PS_IPPROTO_TCP_UDP )
  {
    if( !AddSNATEntryOnA5(nat_entry, qmi_err_num) )
    {
      LOG_MSG_ERROR("\nAdding SNAT entry on A5 Failed!!",0,0,0);
      return false;
    }
  }
  else
  {
    nat_entry->port_fwding_protocol = PS_IPPROTO_TCP;
    if( !AddSNATEntryOnA5(nat_entry, qmi_err_num) )
    {
      LOG_MSG_ERROR("\nAdding SNAT entry on A5 Failed!!",0,0,0);
      return false;
    }
    nat_entry->port_fwding_protocol = PS_IPPROTO_UDP;
    if( !AddSNATEntryOnA5(nat_entry, qmi_err_num) )
    {
      LOG_MSG_ERROR("\nAdding SNAT entry on A5 Failed!!",0,0,0);
      return false;
    }
  }

  return true;
}

/*===========================================================================
  FUNCTION DeleteStaticNatEntry
==========================================================================*/
/*!
@brief
  Deletes the snat entry on A5 and in mobileap configuration file.

@parameters
  qcmap_cm_port_fwding_entry_conf_t* nat_entry

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::DeleteStaticNatEntry(qcmap_cm_port_fwding_entry_conf_t* nat_entry, qmi_error_type_v01 *qmi_err_num)
{
  int qcmap_cm_errno;
  int ret_val, i, j;

  for (i = 0; i < this->cfg.nat_config.num_port_fwding_entries; i++)
  {
    if(this->cfg.nat_config.port_fwding_entries[i].port_fwding_private_ip ==
       nat_entry->port_fwding_private_ip &&
       this->cfg.nat_config.port_fwding_entries[i].port_fwding_private_port ==
       nat_entry->port_fwding_private_port &&
       this->cfg.nat_config.port_fwding_entries[i].port_fwding_global_port ==
       nat_entry->port_fwding_global_port &&
       this->cfg.nat_config.port_fwding_entries[i].port_fwding_protocol ==
       nat_entry->port_fwding_protocol)
    {
      break;
    }
  }

  if ( i == this->cfg.nat_config.num_port_fwding_entries )
  {
      /* we do not have this one  */
      LOG_MSG_ERROR("\nSNAT Entry not found!!",0,0,0);
      *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
      return false;
  }

  /* save into the config */
  for ( j = i; j < (this->cfg.nat_config.num_port_fwding_entries - 1); j++ )
  {
    memcpy(&this->cfg.nat_config.port_fwding_entries[j], &this->cfg.nat_config.port_fwding_entries[j+1],
           sizeof(qcmap_cm_port_fwding_entry_conf_t));
  }

  this->cfg.nat_config.num_port_fwding_entries--;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  if ( nat_entry->port_fwding_protocol != PS_IPPROTO_TCP_UDP )
  {
    if ( !DeleteSNATEntryOnA5(nat_entry, qmi_err_num) )
      return false;
  }
  else
  {
    nat_entry->port_fwding_protocol = PS_IPPROTO_TCP;
    if ( !DeleteSNATEntryOnA5(nat_entry, qmi_err_num) )
      return false;
    nat_entry->port_fwding_protocol = PS_IPPROTO_UDP;
    if ( !DeleteSNATEntryOnA5(nat_entry, qmi_err_num) )
      return false;
  }

  return true;
}

/*===========================================================================
  FUNCTION GetStaticNatEntries
==========================================================================*/
/*!
@brief
  Gets all the snat entries configured.

@parameters
  qcmap_msgr_snat_entry_config_v01 *snat_config
  unsigned int                     *num_entries

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetStaticNatEntries(qcmap_msgr_snat_entry_config_v01 *snat_config, unsigned int* num_entries, qmi_error_type_v01 *qmi_err_num)
{
  int i = 0;

  if( this->cfg.nat_config.num_port_fwding_entries > QCMAP_MSGR_MAX_SNAT_ENTRIES_V01 )
  {
    LOG_MSG_ERROR("\nSNAT entries has crossed, allowed max no of configurable entries",0,0,0);
    *qmi_err_num = QMI_ERR_INSUFFICIENT_RESOURCES_V01;
    return false;
  }
  else
  {
    *num_entries = this->cfg.nat_config.num_port_fwding_entries;
  }

  if ( *num_entries <= 0 )
  {
    LOG_MSG_ERROR("\nNo SNAT entries configured",0,0,0);
    *qmi_err_num = QMI_ERR_DISABLED_V01;
    return false;
  }

  for ( i=0; i < *num_entries; i++ )
  {
    snat_config[i].private_ip_addr = this->cfg.nat_config.port_fwding_entries[i].port_fwding_private_ip;
    snat_config[i].private_port = this->cfg.nat_config.port_fwding_entries[i].port_fwding_private_port;
    snat_config[i].global_port = this->cfg.nat_config.port_fwding_entries[i].port_fwding_global_port;
    snat_config[i].protocol = this->cfg.nat_config.port_fwding_entries[i].port_fwding_protocol;
  }
  return true;
}

/*===========================================================================
  FUNCTION GetIpsecVpnPassthroughFlag
==========================================================================*/
/*!
@brief
  Gets ipsec vpn pass through flag.

@parameters
  uint8 *flag

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetIpsecVpnPassthroughFlag(uint8 *flag, qmi_error_type_v01 *qmi_err_num)
{
  if ( flag == NULL )
  {
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
    return false;
  }

  *flag = this->cfg.nat_config.enable_ipsec_vpn_pass_through;
  return true;
}

/*===========================================================================
  FUNCTION SetIPSECVpnPassThroughOnA5
==========================================================================*/
/*!
@brief
  Sets ipsec vpn pass through.

@parameters
  boolean enable

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetIPSECVpnPassThroughOnA5
(
  boolean enable,
  qmi_error_type_v01 *qmi_err_num
)
{
  char command[MAX_COMMAND_STR_LEN], devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  int qcmap_cm_error=0;

  memset(devname, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);
  memset(command, 0, MAX_COMMAND_STR_LEN);

  if( !this->sta_connected && !this->cradle_backhaul_connected )
  {
    if ( qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_MSGR_IP_FAMILY_V4_V01, devname, &qcmap_cm_error) != QCMAP_CM_SUCCESS )
    {
      LOG_MSG_ERROR("Couldn't get rmnet name. error %d\n", qcmap_cm_error,0,0);
      *qmi_err_num = QMI_ERR_INTERFACE_NOT_FOUND_V01;
      return false;
    }
  }
  else if ( this->cradle_backhaul_connected )
  {
    strlcpy(devname, ECM_IFACE, QCMAP_MSGR_INTF_LEN);
  }
  else if (this->sta_connected)
  {
    strlcpy(devname, this->cfg.lan_config.sta_interface, QCMAP_MSGR_INTF_LEN);
  }

   /* Delete the existing rule, only if it was added .*/
  if ( !this->cfg.nat_config.enable_ipsec_vpn_pass_through )
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -D FORWARD -p esp -i %s -j %s", devname, "DROP" );

    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("%s\n", command,0,0);
  }

  /* Add entry only in case of disable rule, by default it's accept*/
  if( !enable )
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -A FORWARD -p esp -i %s -j %s", devname, "DROP" );

    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("%s\n", command,0,0);
  }

  return true;
}


/*===========================================================================
  FUNCTION SetIPSECVpnPassThrough
==========================================================================*/
/*!
@brief
  Configures ipsec vpn pass through.

@parameters
  boolean enable

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetIPSECVpnPassThrough(boolean enable, qmi_error_type_v01 *qmi_err_num)
{
  if (!SetIPSECVpnPassThroughOnA5(enable, qmi_err_num))
  {
    this->cfg.nat_config.enable_ipsec_vpn_pass_through = enable;

    /* if it is OK, call write to XML */
    this->WriteConfigToXML();
    return false;
  }

  this->cfg.nat_config.enable_ipsec_vpn_pass_through = enable;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}


/*===========================================================================
  FUNCTION GetPptpVpnPassthroughFlag
==========================================================================*/
/*!
@brief
  Get the status of PPTP VPN pass through flag.

@parameters
  boolean enable

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetPptpVpnPassthroughFlag(uint8 *flag, qmi_error_type_v01 *qmi_err_num)
{
  if ( flag == NULL )
  {
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
    return false;
  }

  *flag = this->cfg.nat_config.enable_pptp_vpn_pass_through;
  return true;
}


/*===========================================================================
  FUNCTION SetPPTPVpnPassThroughOnA5
==========================================================================*/
/*!
@brief
  Configure the PPTP VPN pass through flag.

@parameters
  boolean enable

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetPPTPVpnPassThroughOnA5
(
  boolean enable,
  qmi_error_type_v01 *qmi_err_num
)
{
  char command[MAX_COMMAND_STR_LEN], devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  int qcmap_cm_error=0;

  memset(devname, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);
  memset(command, 0, MAX_COMMAND_STR_LEN);

  if( !this->sta_connected && !this->cradle_backhaul_connected )
  {
    if ( qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_MSGR_IP_FAMILY_V4_V01, devname, &qcmap_cm_error) != QCMAP_CM_SUCCESS )
    {
      LOG_MSG_ERROR("Couldn't get rmnet name. error %d\n", qcmap_cm_error,0,0);
      *qmi_err_num = QMI_ERR_INTERFACE_NOT_FOUND_V01;
      return false;
    }
  }
  else if ( this->cradle_backhaul_connected )
  {
    strlcpy(devname, ECM_IFACE, QCMAP_MSGR_INTF_LEN);
  }
  else if (this->sta_connected)
  {
    strlcpy(devname, this->cfg.lan_config.sta_interface, QCMAP_MSGR_INTF_LEN);
  }

   /* Delete the existing rule, only if it was added .*/
  if( !this->cfg.nat_config.enable_pptp_vpn_pass_through )
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
        "iptables -D FORWARD -p gre -i %s -j %s", devname,"DROP" );
    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("%s\n", command,0,0);
  }

  /* Add entry only in case of disabel rule, by default it's accept*/
  if( !enable )
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -A FORWARD -p gre -i %s -j %s", devname,"DROP" );
    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("%s\n", command,0,0);
  }

  return true;
}


/*===========================================================================
  FUNCTION SetPPTPVpnPassThrough
==========================================================================*/
/*!
@brief
  Configures PPTP VPN pass through flag and updates the
  same in mobileap configuration file.

@parameters
  boolean enable

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetPPTPVpnPassThrough( boolean enable, qmi_error_type_v01 *qmi_err_num )
{
  if ( !SetPPTPVpnPassThroughOnA5(enable, qmi_err_num) )
  {
    this->cfg.nat_config.enable_pptp_vpn_pass_through = enable;

    /* if it is OK, call write to XML */
    this->WriteConfigToXML();
    return false;
  }

  this->cfg.nat_config.enable_pptp_vpn_pass_through = enable;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

/*===========================================================================
  FUNCTION GetL2tpVpnPassthroughFlag
==========================================================================*/
/*!
@brief
  Gets the status of L2TP VPN pass through flag.

@parameters
  uint8 *flag

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetL2tpVpnPassthroughFlag(uint8 *flag, qmi_error_type_v01 *qmi_err_num)
{
  if ( flag == NULL )
  {
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
    return false;
  }

  *flag = this->cfg.nat_config.enable_l2tp_vpn_pass_through;
  return true;
}

/*===========================================================================
  FUNCTION SetL2TPVpnPassThroughOnA5
==========================================================================*/
/*!
@brief
  Configures the L2TP VPN pass through flag.

@parameters
  boolean enable

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetL2TPVpnPassThroughOnA5
(
  boolean enable,
  qmi_error_type_v01 *qmi_err_num
)
{
  char command[MAX_COMMAND_STR_LEN], devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  int qcmap_cm_error=0;

  memset(devname, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);
  memset(command, 0, MAX_COMMAND_STR_LEN);

  if( !this->sta_connected && !this->cradle_backhaul_connected )
  {
    if ( qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_MSGR_IP_FAMILY_V4_V01, devname, &qcmap_cm_error) != QCMAP_CM_SUCCESS )
    {
      LOG_MSG_ERROR("Couldn't get rmnet name. error %d\n", qcmap_cm_error,0,0);
      *qmi_err_num = QMI_ERR_INTERFACE_NOT_FOUND_V01;
      return false;
    }
  }
  else if ( this->cradle_backhaul_connected )
  {
    strlcpy(devname, ECM_IFACE, QCMAP_MSGR_INTF_LEN);
  }
  else if (this->sta_connected)
  {
    strlcpy(devname, this->cfg.lan_config.sta_interface, QCMAP_MSGR_INTF_LEN);
  }

   /* Delete the existing rule, only if it was added .*/
  if( !this->cfg.nat_config.enable_l2tp_vpn_pass_through )
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -D FORWARD -p esp -i %s -j %s", devname,"DROP" );

    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("%s\n", command,0,0);
  }

  /* Add entry only in case of disable rule, by default it's accept*/
  if( !enable )
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -A FORWARD -p esp -i %s -j %s", devname,"DROP" );

    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("%s\n", command,0,0);
  }

  return true;
}

/*===========================================================================
  FUNCTION SetL2TPVpnPassThrough
==========================================================================*/
/*!
@brief
  Configures L2TP VPN pass through flag and updates in the mobileap
  configuration file.

@parameters
  boolean enable

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetL2TPVpnPassThrough(boolean enable, qmi_error_type_v01 *qmi_err_num)
{

  if ( !SetL2TPVpnPassThroughOnA5(enable, qmi_err_num) )
  {
    this->cfg.nat_config.enable_l2tp_vpn_pass_through = enable;

    /* if it is OK, call write to XML */
    this->WriteConfigToXML();
    return false;
  }
  this->cfg.nat_config.enable_l2tp_vpn_pass_through = enable;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

/*===========================================================================
  FUNCTION GetWebserverWWANAccessFlag
==========================================================================*/
/*!
@brief
  Gets the status of whether Webserver can be accessed from WWAN.

@parameters
  uint8 *flag

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetWebserverWWANAccessFlag
(
  uint8 *flag,
  qmi_error_type_v01 *qmi_err_num
)
{
  if ( flag == NULL )
  {
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
    return false;
  }

  *flag = this->cfg.nat_config.enable_webserver_wwan_access;
  return true;
}

/*===========================================================================
  FUNCTION SetWebserverWWANAccessOnA5
==========================================================================*/
/*!
@brief
  Configures the Webserver WWAN Access.

@parameters
  boolean enable

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetWebserverWWANAccessOnA5
(
  boolean enable,
  qmi_error_type_v01 *qmi_err_num
)
{
  char command[MAX_COMMAND_STR_LEN], devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  int qcmap_cm_error=0;

  memset(devname, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);
  memset(command, 0, MAX_COMMAND_STR_LEN);

  if( !this->sta_connected && !this->cradle_backhaul_connected )
  {
    if ( qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_MSGR_IP_FAMILY_V4_V01,
                               devname, &qcmap_cm_error) != QCMAP_CM_SUCCESS )
    {
      LOG_MSG_ERROR("Couldn't get rmnet name. error %d\n", qcmap_cm_error,0,0);
      *qmi_err_num = QMI_ERR_INTERFACE_NOT_FOUND_V01;
      return false;
    }
  }
  else if ( this->cradle_backhaul_connected )
  {
    strlcpy(devname, ECM_IFACE, QCMAP_MSGR_INTF_LEN);
  }
  else if (this->sta_connected)
  {
    strlcpy(devname, this->cfg.lan_config.sta_interface, QCMAP_MSGR_INTF_LEN);
  }

   /* Delete the existing rule, only if it was added .*/
  if( !this->cfg.nat_config.enable_webserver_wwan_access)
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -t filter -D INPUT -i %s -p tcp --dport 80 -j %s", devname,"DROP" );
    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("%s\n", command,0,0);
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -t filter -D INPUT -i %s -p tcp --dport 443 -j %s", devname,"DROP" );
    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("%s\n", command,0,0);
  }

  /* Add entry only in case of disable rule, by default it's accept*/
  if( !enable )
  {
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -t filter -I INPUT -i %s -p tcp --dport 80 -j %s", devname,"DROP" );
    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("%s\n", command,0,0);
    snprintf( command, MAX_COMMAND_STR_LEN,
              "iptables -t filter -I INPUT -i %s -p tcp --dport 443 -j %s", devname,"DROP" );
    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("%s\n", command,0,0);
  }

  return true;
}

/*===========================================================================
  FUNCTION SetWebserverWWANAccess
==========================================================================*/
/*!
@brief
  Configures Webserver WWAN Access flag and updates in the mobileap
  configuration file.

@parameters
  boolean enable

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetWebserverWWANAccess
(
  boolean enable,
  qmi_error_type_v01 *qmi_err_num
)
{

  if ( !SetWebserverWWANAccessOnA5(enable, qmi_err_num) )
  {
    this->cfg.nat_config.enable_webserver_wwan_access = enable;

    /* if it is OK, call write to XML */
    this->WriteConfigToXML();
    return false;
  }
  this->cfg.nat_config.enable_webserver_wwan_access = enable;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

/*===========================================================================
  FUNCTION GetNatType
==========================================================================*/
/*!
@brief
  Gets configured value of L2TP VPN passthrough flag.

@parameters
  boolean enable

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetNatType
(
  qcmap_msgr_nat_enum_v01 *cur_nat_type,
  qmi_error_type_v01 *qmi_err_num
)
{
  if ( cur_nat_type == NULL )
  {
    *qmi_err_num = QMI_ERR_DISABLED_V01;
    return false;
  }

  *cur_nat_type = this->cfg.nat_config.nat_type;
  return true;
}

/*===========================================================================
  FUNCTION SetNatType
==========================================================================*/
/*!
@brief
  Configures the nat type to be used and enables the same.

@parameters
  boolean enable

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetNatType
(
  qcmap_msgr_nat_enum_v01 nat_type,
  qmi_error_type_v01 *qmi_err_num
)
{

  if ( nat_type < QCMAP_MSGR_NAT_SYMMETRIC_NAT_V01 ||
       nat_type > QCMAP_MSGR_NAT_ADDRESS_RESTRICTED_NAT_V01 )
  {
    LOG_MSG_ERROR("Error: NAT type not supported.\n",0,0,0);
    *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
    return false;
  }

  this->cfg.nat_config.nat_type = nat_type;
  this->DisableNATonA5();
  this->FlushIPV4Firewall();
  this->EnableNATonA5();

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

/*===========================================================================
  FUNCTION Dump_firewall_conf
==========================================================================*/
/*!
@brief
  Displays the values of filrewall entry.

@parameters
  qcmap_msgr_firewall_entry_conf_t *firewall_entry

@return
  void

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::Dump_firewall_conf( qcmap_msgr_firewall_entry_conf_t *firewall_entry)
{
  uint8_t num_entry;
  int i;
  char str[INET6_ADDRSTRLEN];

  if( firewall_entry !=NULL )
  {
    LOG_MSG_INFO1("\n Start Displaying firewall configuration of handle =%d ",i,0,0);

      if ( firewall_entry->filter_spec.ip_vsn == IP_V4 )
      {
        LOG_MSG_INFO1("\nIp version : IPv4",0,0,0);

        if ( firewall_entry->filter_spec.ip_hdr.v4.field_mask & IPFLTR_MASK_IP4_SRC_ADDR )
        {
          readable_addr(AF_INET,(const uint32_t *)&firewall_entry->filter_spec.ip_hdr.v4.src.addr.ps_s_addr,(char *)&str);
          LOG_MSG_INFO1("\nSRC Addr : %s",str,0,0);
          readable_addr(AF_INET,(const uint32_t *)&firewall_entry->filter_spec.ip_hdr.v4.src.subnet_mask.ps_s_addr,(char *)&str);
          LOG_MSG_INFO1("\nSRC Addr Mask : %s",str,0,0);
        }

        if ( firewall_entry->filter_spec.ip_hdr.v4.field_mask & IPFLTR_MASK_IP4_DST_ADDR )
        {
          readable_addr(AF_INET,(const uint32_t *)&firewall_entry->filter_spec.ip_hdr.v4.dst.addr.ps_s_addr,(char *)&str);
          LOG_MSG_INFO1("\nDest Addr : %s",str,0,0);
          readable_addr(AF_INET,(const uint32_t *)&firewall_entry->filter_spec.ip_hdr.v4.dst.subnet_mask.ps_s_addr,(char *)&str);
          LOG_MSG_INFO1("\nDest Addr Mask : %s",str,0,0);
        }

        if (  firewall_entry->filter_spec.ip_hdr.v4.field_mask & IPFLTR_MASK_IP4_TOS )
        {
          LOG_MSG_INFO1("\nTos value : %x ",firewall_entry->filter_spec.ip_hdr.v4.tos.val,0,0);
          LOG_MSG_INFO1("\nTos Mask : %x ",firewall_entry->filter_spec.ip_hdr.v4.tos.mask,0,0);
        }

      }else
      {
        LOG_MSG_INFO1("\nIp version : Ipv6",0,0,0);

        if ( firewall_entry->filter_spec.ip_hdr.v6.field_mask & IPFLTR_MASK_IP6_SRC_ADDR )
        {
          readable_addr(AF_INET6,(uint32 *)&firewall_entry->filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr32,(char *)&str);
          LOG_MSG_INFO1("\nSrc Addr : %s ",str,0,0);
          LOG_MSG_INFO1("\nSrc Prefixlen : %d ",firewall_entry->filter_spec.ip_hdr.v6.src.prefix_len,0,0);
        }

        if ( firewall_entry->filter_spec.ip_hdr.v6.field_mask & IPFLTR_MASK_IP6_DST_ADDR )
        {
          readable_addr(AF_INET6,(uint32 *)&firewall_entry->filter_spec.ip_hdr.v6.dst.addr.in6_u.u6_addr32,(char *)&str);
          LOG_MSG_INFO1("\nDst Addr : %s ",str,0,0);
          LOG_MSG_INFO1("\nDst Prefixlen : %d ",firewall_entry->filter_spec.ip_hdr.v6.dst.prefix_len,0,0);
        }
      }

       switch( firewall_entry->filter_spec.ip_hdr.v4.next_hdr_prot )
      {
        case PS_IPPROTO_TCP:

          LOG_MSG_INFO1("\nProtocol : TCP",0,0,0);

          if ( firewall_entry->filter_spec.next_prot_hdr.tcp.field_mask & IPFLTR_MASK_TCP_SRC_PORT )
          {
            LOG_MSG_INFO1("\nSrc port : %d",firewall_entry->filter_spec.next_prot_hdr.tcp.src.port,0,0);
            LOG_MSG_INFO1("\nSrc portrange : %d",firewall_entry->filter_spec.next_prot_hdr.tcp.src.range,0,0);
          }

          if ( firewall_entry->filter_spec.next_prot_hdr.tcp.field_mask & IPFLTR_MASK_TCP_DST_PORT )
          {
            LOG_MSG_INFO1("\nDst port : %d",firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port,0,0);
            LOG_MSG_INFO1("\nDst portrange : %d",firewall_entry->filter_spec.next_prot_hdr.tcp.dst.range,0,0);
          }
          break;

        case PS_IPPROTO_UDP:

          LOG_MSG_INFO1("\nProtocol: UDP",0,0,0);

          if( firewall_entry->filter_spec.next_prot_hdr.udp.field_mask & IPFLTR_MASK_UDP_SRC_PORT )
          {
            LOG_MSG_INFO1("\nSrc port : %d",firewall_entry->filter_spec.next_prot_hdr.udp.src.port,0,0);
            LOG_MSG_INFO1("\nDst portrange : %d",firewall_entry->filter_spec.next_prot_hdr.udp.src.range,0,0);
          }

          if( firewall_entry->filter_spec.next_prot_hdr.udp.field_mask & IPFLTR_MASK_UDP_DST_PORT )
          {
            LOG_MSG_INFO1("\nDst port : %d",firewall_entry->filter_spec.next_prot_hdr.udp.dst.port,0,0);
            LOG_MSG_INFO1("\nDst portrange : %d",firewall_entry->filter_spec.next_prot_hdr.udp.dst.range,0,0);
          }
          break;

        case PS_IPPROTO_TCP_UDP:

          LOG_MSG_INFO1("\nProtocol: TCP_UDP",0,0,0);

          if( firewall_entry->filter_spec.next_prot_hdr.udp.field_mask & IPFLTR_MASK_UDP_SRC_PORT )
          {
            LOG_MSG_INFO1("\nSrc port : %d",firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.src.port,0,0);
            LOG_MSG_INFO1("\nDst portrange : %d",firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.src.range,0,0);
          }

          if ( firewall_entry->filter_spec.next_prot_hdr.udp.field_mask & IPFLTR_MASK_UDP_DST_PORT )
          {
            LOG_MSG_INFO1("\nDst port : %d",firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.dst.port,0,0);
            LOG_MSG_INFO1("\nDst portrange : %d",firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.dst.range,0,0);
          }
          break;
        case PS_IPPROTO_ICMP:

          LOG_MSG_INFO1("\nProtocol : ICMP",0,0,0);

          if ( firewall_entry->filter_spec.next_prot_hdr.icmp.field_mask & IPFLTR_MASK_ICMP_MSG_CODE )
          {
            LOG_MSG_INFO1("\nIcmp Type: %d ",firewall_entry->filter_spec.next_prot_hdr.icmp.type,0,0);
          }
          break;

        case PS_IPPROTO_ICMP6:

          LOG_MSG_INFO1("\nProtocol : ICMP6",0,0,0);

          if ( firewall_entry->filter_spec.next_prot_hdr.icmp.field_mask & IPFLTR_MASK_ICMP_MSG_CODE )
          {
            LOG_MSG_INFO1("\nICMPv6 type: %d ",firewall_entry->filter_spec.next_prot_hdr.icmp.type,0,0);
          }
          break;

        case PS_IPPROTO_ESP:

          LOG_MSG_INFO1("\nProtocol : ESP",0,0,0);

          if ( firewall_entry->filter_spec.next_prot_hdr.esp.field_mask & IPFLTR_MASK_ESP_SPI )
          {
            LOG_MSG_INFO1("\nESP spi : %d",firewall_entry->filter_spec.next_prot_hdr.esp.spi,0,0);
          }
          break;

        default:

          LOG_MSG_INFO1("\nUnsupported protocol \n",0,0,0);
          break;
      }
    }
  LOG_MSG_INFO1("\n End of Firewall configuration of handle =%d ",i,0,0);
}


/*===========================================================================
  FUNCTION readable_addr
==========================================================================*/
/*!
@brief
  Converts the address iin intgeger to readable string.

@parameters
  int            domain
  const uint32_t *addr
  char           *str

@return
  -1 - on failure
   0 - on success
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
int QCMAP_ConnectionManager::readable_addr(int domain,const uint32_t *addr, char *str)
{
  if ( inet_ntop(domain, (void *)addr, str, INET6_ADDRSTRLEN) == NULL )
  {
    LOG_MSG_ERROR("\n Not in presentation format \n",0,0,0);
    return -1;
  }
  return 0;
}


/*===========================================================================
  FUNCTION SetFirewallV4
==========================================================================*/
/*!
@brief
  Adds firewall entry by preparing, the command to add firewall entry based on the
  firewall configuration value and executes the same.
  It also generates delete ommand for the same.

@parameters
  qcmap_msgr_firewall_conf_t *firewall
  uint8_t *del_command

@return
  -1 - on failure
   0 - on success

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
int QCMAP_ConnectionManager::SetFirewallV4
(
  qcmap_msgr_firewall_conf_t *firewall,
  boolean add_rule,
  qmi_error_type_v01 *qmi_err_num)
{
  qcmap_msgr_firewall_entry_conf_t *firewall_entry=&(firewall->extd_firewall_entry);
  char command[MAX_COMMAND_STR_LEN]={0}, devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2]={0};
  char command_sec[MAX_COMMAND_STR_LEN];
  char tmp[MAX_COMMAND_STR_LEN]={0};
  char str[INET6_ADDRSTRLEN];
  char *ptr=NULL;
  int ret=0;
  uint32_t addr;
  uint8 next_hdr_prot;

  if( firewall_entry == NULL )
  {
    LOG_MSG_ERROR("NULL firewall_entry\n",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01 ;
    return QCMAP_CM_ERROR;
  }

  if (firewall_entry->filter_spec.ip_vsn != IP_V4) {
      LOG_MSG_ERROR("Incorrect firewall_entry version. IP Version = %x\n",firewall_entry->filter_spec.ip_vsn,0,0);
     *qmi_err_num = QMI_ERR_INVALID_ARG_V01 ;
      return QCMAP_CM_ERROR;
  }

  if( !this->sta_connected && !this->cradle_backhaul_connected )
  {
    if (qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_MSGR_IP_FAMILY_V4_V01, devname, &ret) != QCMAP_CM_SUCCESS )
    {
       LOG_MSG_ERROR("Couldn't get rmnet name. error %d\n", ret,0,0);
       *qmi_err_num = QMI_ERR_INTERFACE_NOT_FOUND_V01;
       return QCMAP_CM_ERROR;
    }
  }
  else if ( this->cradle_backhaul_connected )
  {
    strlcpy(devname, ECM_IFACE, QCMAP_MSGR_INTF_LEN);
  }
  else if ( this->sta_connected )
  {
    strlcpy(devname,this->cfg.lan_config.sta_interface,QCMAP_MSGR_INTF_LEN);
  }

   /*In comming traffic from wwan is handled in PREROUTING chain in mangle table */
   if ( add_rule )
   {
     snprintf(command,MAX_COMMAND_STR_LEN,"iptables -t mangle -I PREROUTING -i %s ",devname);
   }
   else
   {
     snprintf(command,MAX_COMMAND_STR_LEN,"iptables -t mangle -D PREROUTING -i %s ",devname);
   }

   if ( firewall_entry->filter_spec.ip_hdr.v4.field_mask & IPFLTR_MASK_IP4_SRC_ADDR )
   {
     readable_addr(AF_INET,&(firewall_entry->filter_spec.ip_hdr.v4.src.addr.ps_s_addr),(char *)&str);
     strlcat(command," -s ",MAX_COMMAND_STR_LEN);
     strlcat(command,str,MAX_COMMAND_STR_LEN);
     readable_addr(AF_INET,&(firewall_entry->filter_spec.ip_hdr.v4.src.subnet_mask.ps_s_addr),(char *)&str);
     strlcat(command,"/",MAX_COMMAND_STR_LEN);
     strlcat(command,str,MAX_COMMAND_STR_LEN);
   }

   if (  firewall_entry->filter_spec.ip_hdr.v4.field_mask & IPFLTR_MASK_IP4_TOS )
   {
     snprintf( tmp,MAX_COMMAND_STR_LEN," -m tos --tos %x/%x ", firewall_entry->filter_spec.ip_hdr.v4.tos.val,
               firewall_entry->filter_spec.ip_hdr.v4.tos.mask );
     strlcat(command,tmp,MAX_COMMAND_STR_LEN);
     memset(tmp,0,MAX_COMMAND_STR_LEN);
   }
   next_hdr_prot = firewall_entry->filter_spec.ip_hdr.v4.next_hdr_prot;

   switch( next_hdr_prot )
  {
   case PS_IPPROTO_TCP:

     strlcat(command," -p tcp ",MAX_COMMAND_STR_LEN);

     if ( firewall_entry->filter_spec.next_prot_hdr.tcp.field_mask & IPFLTR_MASK_TCP_SRC_PORT )
     {
       if( firewall_entry->filter_spec.next_prot_hdr.tcp.src.range !=0 )
       {
         snprintf(tmp,MAX_COMMAND_STR_LEN," --sport %d:%d ",firewall_entry->filter_spec.next_prot_hdr.tcp.src.port,
             ( firewall_entry->filter_spec.next_prot_hdr.tcp.src.port + firewall_entry->filter_spec.next_prot_hdr.tcp.src.range));
       }else
       {
         snprintf(tmp,MAX_COMMAND_STR_LEN," --sport %d ",firewall_entry->filter_spec.next_prot_hdr.tcp.src.port);
       }
       strlcat(command,tmp,MAX_COMMAND_STR_LEN);
       memset(tmp,0,MAX_COMMAND_STR_LEN);
     }

     if ( firewall_entry->filter_spec.next_prot_hdr.tcp.field_mask & IPFLTR_MASK_TCP_DST_PORT )
     {
       if ( firewall_entry->filter_spec.next_prot_hdr.tcp.dst.range !=0 )
       {
         snprintf(tmp,MAX_COMMAND_STR_LEN," --dport %d:%d ",firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port,
             (firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port + firewall_entry->filter_spec.next_prot_hdr.tcp.dst.range));
       }else
       {
         snprintf(tmp,MAX_COMMAND_STR_LEN," --dport %d ",firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port);
       }
       strlcat(command,tmp,MAX_COMMAND_STR_LEN);
       memset(tmp,0,MAX_COMMAND_STR_LEN);
     }

     break;
   case PS_IPPROTO_UDP:

     strlcat(command," -p udp ",MAX_COMMAND_STR_LEN);

     if ( firewall_entry->filter_spec.next_prot_hdr.udp.field_mask & IPFLTR_MASK_UDP_SRC_PORT )
     {
       if ( firewall_entry->filter_spec.next_prot_hdr.udp.src.range !=0 )
       {
         snprintf(tmp,MAX_COMMAND_STR_LEN," --sport %d:%d ",firewall_entry->filter_spec.next_prot_hdr.udp.src.port,
                 (firewall_entry->filter_spec.next_prot_hdr.udp.src.range+firewall_entry->filter_spec.next_prot_hdr.udp.src.port));
       }else
       {
         snprintf(tmp,MAX_COMMAND_STR_LEN," --sport %d ",firewall_entry->filter_spec.next_prot_hdr.udp.src.port);
       }
       strlcat(command,tmp,MAX_COMMAND_STR_LEN);
       memset(tmp,0,MAX_COMMAND_STR_LEN);
     }

     if ( firewall_entry->filter_spec.next_prot_hdr.udp.field_mask & IPFLTR_MASK_UDP_DST_PORT )
     {
       if ( firewall_entry->filter_spec.next_prot_hdr.udp.dst.range !=0 )
       {
         snprintf(tmp,MAX_COMMAND_STR_LEN," --dport %d:%d ",firewall_entry->filter_spec.next_prot_hdr.udp.dst.port,
                  (firewall_entry->filter_spec.next_prot_hdr.udp.dst.range + firewall_entry->filter_spec.next_prot_hdr.udp.dst.port));
       }else
       {
         snprintf(tmp,MAX_COMMAND_STR_LEN," --dport %d ",firewall_entry->filter_spec.next_prot_hdr.udp.dst.port);
       }
       strlcat(command,tmp,MAX_COMMAND_STR_LEN);
       memset(tmp,0,MAX_COMMAND_STR_LEN);
     }

     break;
   case PS_IPPROTO_TCP_UDP:

     strlcpy(command_sec, command, MAX_COMMAND_STR_LEN);
     strlcat(command, " -p tcp ", MAX_COMMAND_STR_LEN);
     strlcat(command_sec, " -p udp ", MAX_COMMAND_STR_LEN);

     if( firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask &
         IPFLTR_MASK_TCP_UDP_SRC_PORT )
     {
       snprintf(tmp, MAX_COMMAND_STR_LEN, " --sport %d:%d ",
           firewall_entry->filter_spec.next_prot_hdr.tcp.src.port,
           firewall_entry->filter_spec.next_prot_hdr.tcp.src.range + firewall_entry->filter_spec.next_prot_hdr.tcp.src.port);
       strlcat(command, tmp, MAX_COMMAND_STR_LEN);
       strlcat(command_sec, tmp, MAX_COMMAND_STR_LEN);
       memset(tmp,0,MAX_COMMAND_STR_LEN);
     }

     if ( firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask &
          IPFLTR_MASK_TCP_UDP_DST_PORT )
     {
       snprintf(tmp, MAX_COMMAND_STR_LEN, " --dport %d:%d ",
           firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port,
           firewall_entry->filter_spec.next_prot_hdr.tcp.dst.range + firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port);
       strlcat(command, tmp, MAX_COMMAND_STR_LEN);
       strlcat(command_sec, tmp, MAX_COMMAND_STR_LEN);
       memset(tmp,0,MAX_COMMAND_STR_LEN);
     }
     break;

   case PS_IPPROTO_ICMP:

     strlcat(command," -p icmp ",MAX_COMMAND_STR_LEN);

     if ( firewall_entry->filter_spec.next_prot_hdr.icmp.field_mask & IPFLTR_MASK_ICMP_MSG_TYPE )
     {
       snprintf(tmp,MAX_COMMAND_STR_LEN," --icmp-type %d",firewall_entry->filter_spec.next_prot_hdr.icmp.type);
       strlcat(command,tmp,MAX_COMMAND_STR_LEN);
       memset(tmp,0,MAX_COMMAND_STR_LEN);
     }

     if ( firewall_entry->filter_spec.next_prot_hdr.icmp.field_mask & IPFLTR_MASK_ICMP_MSG_CODE )
     {
       snprintf(tmp,MAX_COMMAND_STR_LEN, "/%d ",firewall_entry->filter_spec.next_prot_hdr.icmp.code);
       strlcat(command,tmp,MAX_COMMAND_STR_LEN);
       memset(tmp,0,MAX_COMMAND_STR_LEN);
     }
     break;

   case PS_IPPROTO_ESP:

     strlcat(command," -p esp ",MAX_COMMAND_STR_LEN);
     if ( firewall_entry->filter_spec.next_prot_hdr.esp.field_mask & IPFLTR_MASK_ESP_SPI )
     {
       snprintf(tmp,MAX_COMMAND_STR_LEN," --espspi %d ",firewall_entry->filter_spec.next_prot_hdr.esp.spi);
       strlcat(command,tmp,MAX_COMMAND_STR_LEN);
     }
     break;
   default:
     LOG_MSG_ERROR("Unsupported protocol \n",next_hdr_prot,0,0);
     break;
  }

   LOG_MSG_INFO1("SET FIREWALL pkts allowed = %d",this->cfg.nat_config.firewall_pkts_allowed,0,0);
   if( this->cfg.nat_config.firewall_pkts_allowed )
      snprintf(tmp,MAX_COMMAND_STR_LEN," -j %s","ACCEPT");
   else
      snprintf(tmp,MAX_COMMAND_STR_LEN," -j %s","DROP");

   strlcat(command,tmp,MAX_COMMAND_STR_LEN);
   LOG_MSG_INFO1("\n %s \n",command,0,0);
   ds_system_call(command, strlen(command));

   if(next_hdr_prot == PS_IPPROTO_TCP_UDP)
   {
     strlcat(command_sec, tmp, MAX_COMMAND_STR_LEN);
     ds_system_call(command_sec, strlen(command_sec));
     LOG_MSG_INFO1("\n %s \n",command,0,0);
   }
   return QCMAP_CM_SUCCESS;
}


/*===========================================================================
  FUNCTION SetFirewallV6
==========================================================================*/
/*!
@brief
  Adds firewall entry by preparing, the command to add firewall entry based on the
  firewall configuration value and executes the same.
  It also generates delete ommand for the same.

@parameters
  qcmap_msgr_firewall_conf_t *firewall
  uint8_t *del_command

@return
  -1 - on failure
   0 - on success

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
int QCMAP_ConnectionManager::SetFirewallV6
(
  qcmap_msgr_firewall_conf_t *firewall,
  boolean add_rule,
  qmi_error_type_v01 *qmi_err_num)
{
  qcmap_msgr_firewall_entry_conf_t *firewall_entry=&(firewall->extd_firewall_entry);
  char command[MAX_COMMAND_STR_LEN]={0}, devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2]={0};
  char command_sec[MAX_COMMAND_STR_LEN];
  char tmp[MAX_COMMAND_STR_LEN]={0};
  char str[INET6_ADDRSTRLEN];
  char *ptr=NULL;
  int ret=0;
  uint32_t addr;
  uint8 next_hdr_prot;

  if( firewall_entry == NULL )
  {
    LOG_MSG_ERROR("NULL firewall_entry\n",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01 ;
    return QCMAP_CM_ERROR;
  }

  if (firewall_entry->filter_spec.ip_vsn != IP_V6) {
      LOG_MSG_ERROR("Incorrect firewall_entry version. IP Version = %x\n",firewall_entry->filter_spec.ip_vsn,0,0);
     *qmi_err_num = QMI_ERR_INVALID_ARG_V01 ;
      return QCMAP_CM_ERROR;
  }

  if( !this->sta_connected && !this->cradle_backhaul_connected )
  {
    if (qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_MSGR_IP_FAMILY_V6_V01, devname, &ret) != QCMAP_CM_SUCCESS )
    {
       LOG_MSG_ERROR("Couldn't get rmnet name. error %d\n", ret,0,0);
       *qmi_err_num = QMI_ERR_INTERFACE_NOT_FOUND_V01;
       return QCMAP_CM_ERROR;
    }
  }
  else if ( this->cradle_backhaul_connected )
  {
    strlcpy(devname, ECM_IFACE, QCMAP_MSGR_INTF_LEN);
  }
  else if ( this->sta_connected )
  {
    strlcpy(devname,this->cfg.lan_config.sta_interface,QCMAP_MSGR_INTF_LEN);
  }

   /*In comming traffic from wwan is handled in PREROUTING chain in mangle table */
   if ( add_rule )
   {
      snprintf(command,MAX_COMMAND_STR_LEN,"ip6tables -t mangle -I PREROUTING -i %s ",devname);
   }
   else
   {
      snprintf(command,MAX_COMMAND_STR_LEN,"ip6tables -t mangle -D PREROUTING -i %s ",devname);
   }


   if ( firewall_entry->filter_spec.ip_hdr.v6.field_mask & IPFLTR_MASK_IP6_SRC_ADDR)
   {
     readable_addr(AF_INET6,(uint32 *)&firewall_entry->filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr32,(char *)&str);
     strlcat(command," -s ",MAX_COMMAND_STR_LEN);
     strlcat(command,str,MAX_COMMAND_STR_LEN);
     snprintf(tmp,MAX_COMMAND_STR_LEN,"/%d ", firewall_entry->filter_spec.ip_hdr.v6.src.prefix_len);
     strlcat(command,tmp,MAX_COMMAND_STR_LEN);
     memset(tmp,0,MAX_COMMAND_STR_LEN);
   }
   if(firewall_entry->filter_spec.ip_hdr.v6.field_mask & IPFLTR_MASK_IP6_TRAFFIC_CLASS)
   {
     snprintf(tmp, MAX_COMMAND_STR_LEN," -m tos --tos %d/%d ",firewall_entry->filter_spec.ip_hdr.v6.trf_cls.val,
         firewall_entry->filter_spec.ip_hdr.v6.trf_cls.mask);
     strlcat(command, tmp, MAX_COMMAND_STR_LEN);
       memset(tmp,0,MAX_COMMAND_STR_LEN);
   }
   next_hdr_prot = firewall_entry->filter_spec.ip_hdr.v6.next_hdr_prot;

    switch( next_hdr_prot )
    {
      case PS_IPPROTO_TCP:

        strlcat(command," -p tcp ",MAX_COMMAND_STR_LEN);

        if ( firewall_entry->filter_spec.next_prot_hdr.tcp.field_mask & IPFLTR_MASK_TCP_SRC_PORT )
        {
          if( firewall_entry->filter_spec.next_prot_hdr.tcp.src.range !=0 )
          {
            snprintf(tmp,MAX_COMMAND_STR_LEN," --sport %d:%d ",firewall_entry->filter_spec.next_prot_hdr.tcp.src.port,
                ( firewall_entry->filter_spec.next_prot_hdr.tcp.src.port + firewall_entry->filter_spec.next_prot_hdr.tcp.src.range));
          }else
          {
            snprintf(tmp,MAX_COMMAND_STR_LEN," --sport %d ",firewall_entry->filter_spec.next_prot_hdr.tcp.src.port);
          }
          strlcat(command,tmp,MAX_COMMAND_STR_LEN);
          memset(tmp,0,MAX_COMMAND_STR_LEN);
        }

        if ( firewall_entry->filter_spec.next_prot_hdr.tcp.field_mask & IPFLTR_MASK_TCP_DST_PORT )
        {
          if ( firewall_entry->filter_spec.next_prot_hdr.tcp.dst.range !=0 )
          {
            snprintf(tmp,MAX_COMMAND_STR_LEN," --dport %d:%d ",firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port,
                (firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port + firewall_entry->filter_spec.next_prot_hdr.tcp.dst.range));
          }else
          {
            snprintf(tmp,MAX_COMMAND_STR_LEN," --dport %d ",firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port);
          }
          strlcat(command,tmp,MAX_COMMAND_STR_LEN);
          memset(tmp,0,MAX_COMMAND_STR_LEN);
        }

        break;
      case PS_IPPROTO_UDP:

        strlcat(command," -p udp ",MAX_COMMAND_STR_LEN);

        if ( firewall_entry->filter_spec.next_prot_hdr.udp.field_mask & IPFLTR_MASK_UDP_SRC_PORT )
        {
          if ( firewall_entry->filter_spec.next_prot_hdr.udp.src.range !=0 )
          {
            snprintf(tmp,MAX_COMMAND_STR_LEN," --sport %d:%d ",firewall_entry->filter_spec.next_prot_hdr.udp.src.port,
                    (firewall_entry->filter_spec.next_prot_hdr.udp.src.range+firewall_entry->filter_spec.next_prot_hdr.udp.src.port));
          }else
          {
            snprintf(tmp,MAX_COMMAND_STR_LEN," --sport %d ",firewall_entry->filter_spec.next_prot_hdr.udp.src.port);
          }
          strlcat(command,tmp,MAX_COMMAND_STR_LEN);
          memset(tmp,0,MAX_COMMAND_STR_LEN);
        }

        if ( firewall_entry->filter_spec.next_prot_hdr.udp.field_mask & IPFLTR_MASK_UDP_DST_PORT )
        {
          if ( firewall_entry->filter_spec.next_prot_hdr.udp.dst.range !=0 )
          {
            snprintf(tmp,MAX_COMMAND_STR_LEN," --dport %d:%d ",firewall_entry->filter_spec.next_prot_hdr.udp.dst.port,
                     (firewall_entry->filter_spec.next_prot_hdr.udp.dst.range + firewall_entry->filter_spec.next_prot_hdr.udp.dst.port));
          }else
          {
            snprintf(tmp,MAX_COMMAND_STR_LEN," --dport %d ",firewall_entry->filter_spec.next_prot_hdr.udp.dst.port);
          }
          strlcat(command,tmp,MAX_COMMAND_STR_LEN);
          memset(tmp,0,MAX_COMMAND_STR_LEN);
        }

        break;
      case PS_IPPROTO_TCP_UDP:

        strlcpy(command_sec, command, MAX_COMMAND_STR_LEN);
        strlcat(command, " -p tcp ", MAX_COMMAND_STR_LEN);
        strlcat(command_sec, " -p udp ", MAX_COMMAND_STR_LEN);

        if( firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask &
            IPFLTR_MASK_TCP_UDP_SRC_PORT )
        {
          snprintf(tmp, MAX_COMMAND_STR_LEN, " --sport %d:%d ",
              firewall_entry->filter_spec.next_prot_hdr.tcp.src.port,
              firewall_entry->filter_spec.next_prot_hdr.tcp.src.range + firewall_entry->filter_spec.next_prot_hdr.tcp.src.port);
          strlcat(command, tmp, MAX_COMMAND_STR_LEN);
          strlcat(command_sec, tmp, MAX_COMMAND_STR_LEN);
          memset(tmp,0,MAX_COMMAND_STR_LEN);
        }

        if ( firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask &
             IPFLTR_MASK_TCP_UDP_DST_PORT )
        {
          snprintf(tmp, MAX_COMMAND_STR_LEN, " --dport %d:%d ",
              firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port,
              firewall_entry->filter_spec.next_prot_hdr.tcp.dst.range + firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port);
          strlcat(command, tmp, MAX_COMMAND_STR_LEN);
          strlcat(command_sec, tmp, MAX_COMMAND_STR_LEN);
          memset(tmp,0,MAX_COMMAND_STR_LEN);
        }
        break;

      case PS_IPPROTO_ICMP6:

        strlcat(command," -p icmpv6 ",MAX_COMMAND_STR_LEN);

        if ( firewall_entry->filter_spec.next_prot_hdr.icmp.field_mask & IPFLTR_MASK_ICMP_MSG_TYPE)
        {
          snprintf(tmp,MAX_COMMAND_STR_LEN," --icmpv6-type %d",firewall_entry->filter_spec.next_prot_hdr.icmp.type);
          strlcat(command,tmp,MAX_COMMAND_STR_LEN);
          memset(tmp,0,MAX_COMMAND_STR_LEN);
        }

        if( firewall_entry->filter_spec.next_prot_hdr.icmp.field_mask & IPFLTR_MASK_ICMP_MSG_CODE )
        {
          snprintf(tmp,MAX_COMMAND_STR_LEN, "/%d ",firewall_entry->filter_spec.next_prot_hdr.icmp.code);
          strlcat(command, tmp, MAX_COMMAND_STR_LEN);
          memset(tmp,0,MAX_COMMAND_STR_LEN);
        }
        break;
      case PS_IPPROTO_ESP:

        strlcat(command," -p esp ",MAX_COMMAND_STR_LEN);
        if ( firewall_entry->filter_spec.next_prot_hdr.esp.field_mask & IPFLTR_MASK_ESP_SPI )
        {
          snprintf(tmp,MAX_COMMAND_STR_LEN," --espspi %d ",firewall_entry->filter_spec.next_prot_hdr.esp.spi);
          strlcat(command,tmp,MAX_COMMAND_STR_LEN);
        }
        break;
      default:
        LOG_MSG_ERROR("Unsupported protocol %d \n",next_hdr_prot,0,0);
        break;
    }

    if( this->cfg.nat_config.firewall_pkts_allowed )
    snprintf(tmp,MAX_COMMAND_STR_LEN," -j %s","ACCEPT");
    else
    snprintf(tmp,MAX_COMMAND_STR_LEN," -j %s","DROP");

    strlcat(command,tmp,MAX_COMMAND_STR_LEN);
    LOG_MSG_INFO1("\n %s \n",command,0,0);
    ds_system_call(command, strlen(command));

    if(next_hdr_prot == PS_IPPROTO_TCP_UDP)
    {
      strlcat(command_sec, tmp, MAX_COMMAND_STR_LEN);
      ds_system_call(command_sec, strlen(command_sec));
      LOG_MSG_INFO1("\n %s \n",command,0,0);
    }
    return QCMAP_CM_SUCCESS;
}

/*===========================================================================
  FUNCTION SetFirewall
==========================================================================*/
/*!
@brief
  Adds firewall entry to the appropriate IP tables based on IPV4 and IPV6

@parameters
  qcmap_msgr_firewall_conf_t *firewall
  boolean add_rule

@return
  -1 - on failure
   0 - on success

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
int QCMAP_ConnectionManager::SetFirewall
(
  qcmap_msgr_firewall_conf_t *firewall,
  boolean add_rule,
  qmi_error_type_v01 *qmi_err_num)
{
  qcmap_msgr_firewall_entry_conf_t *firewall_entry;
  char command[MAX_COMMAND_STR_LEN]={0}, devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2]={0};
  char command_sec[MAX_COMMAND_STR_LEN];
  char tmp[MAX_COMMAND_STR_LEN]={0};
  char str[INET6_ADDRSTRLEN];
  char *ptr=NULL;
  int ret=0;
  uint32_t addr;
  uint8 next_hdr_prot;

  if( firewall == NULL )
  {
    LOG_MSG_ERROR("NULL firewall_entry\n",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01 ;
    return QCMAP_CM_ERROR;
  }

  firewall_entry = &(firewall->extd_firewall_entry);

  if (firewall_entry->filter_spec.ip_vsn == IP_V4 )
  {
    if ( SetFirewallV4( firewall, add_rule, qmi_err_num ) != QCMAP_CM_SUCCESS )
    {
      return QCMAP_CM_ERROR;
    }
  }
  else if ( firewall_entry->filter_spec.ip_vsn == IP_V6 )
  {
    if( SetFirewallV6( firewall, add_rule, qmi_err_num ) != QCMAP_CM_SUCCESS )
    {
      return QCMAP_CM_ERROR;
    }
  }
  else
  {
    LOG_MSG_ERROR("Invalid IP Version %x",firewall_entry->filter_spec.ip_vsn,0,0);
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01 ;
    return QCMAP_CM_ERROR;
  }

  return QCMAP_CM_SUCCESS;
}


/*===========================================================================
  FUNCTION GetFireWallHandleList
==========================================================================*/
/*!
@brief
  Get all the handles of firewall entries.

@parameters
  qcmap_msgr_firewall_conf_t *get_handle_list

@return
  true  - on success
  false - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetFireWallHandleList
(
  qcmap_msgr_firewall_conf_t *get_handle_list,
  qmi_error_type_v01 *qmi_err_num
)
{
  int i,len = 0;
  int family;
  family=get_handle_list->extd_firewall_handle.ip_family;

  for( i=0; i< this->cfg.nat_config.num_firewall_entries ; i++)
  {
   if( family == this->cfg.nat_config.extd_firewall_entries[i]->extd_firewall_entry.\
                                                                    filter_spec.ip_vsn)
   {
     get_handle_list->extd_firewall_handle_list.handle_list[len++] =
     this->cfg.nat_config.extd_firewall_entries[i]->extd_firewall_entry.firewall_handle;
   }
  }
  get_handle_list->extd_firewall_handle_list.num_of_entries = len;
  return true;
}

/*===========================================================================
  FUNCTION AddFireWallEntry
==========================================================================*/
/*!
@brief
  Checks for the duplicate entries of firewall entryi, if unique
  adds the firewall entry and update the same in the mobileap configuration file.

@parameters
  qcmap_msgr_firewall_conf_t* extd_firewall_conf

@return
  0  - on success
  -1 - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
int QCMAP_ConnectionManager::AddFireWallEntry
(
  qcmap_msgr_firewall_conf_t* extd_firewall_conf,
  qmi_error_type_v01 *qmi_err_num
)
{
  int qcmap_cm_errno;
  int ret_val, i,j;
  int next_hdr_prot;
  int index;

  for ( i = 0; i < this->cfg.nat_config.num_firewall_entries; i++ )
  {
    if ( (memcmp(&(extd_firewall_conf->extd_firewall_entry.filter_spec),
          &(this->cfg.nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec),
          sizeof(ip_filter_type)) == 0) )
    {
      LOG_MSG_ERROR("Firewall entry is already present\n",0,0,0);
      /* we already added this one */
      *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
      return QCMAP_MSGR_ENTRY_PRESENT;
    }
  }

  if ( this->cfg.nat_config.num_firewall_entries >= QCMAP_MSGR_MAX_FIREWALL_ENTRIES_V01 )
  {
    /* we reach MAX entries */
    LOG_MSG_ERROR("Exceeds maximum number of firewall entries\n",0,0,0);
    *qmi_err_num = QMI_ERR_INSUFFICIENT_RESOURCES_V01;
    return QCMAP_MSGR_ENTRY_FULL;
  }

  // Add firewall entry only if firewall is enabled
  if ( this->cfg.nat_config.firewall_enabled)
  {
    if( SetFirewall( extd_firewall_conf, TRUE, qmi_err_num ) != QCMAP_CM_SUCCESS )
    {
       if( *qmi_err_num != QMI_ERR_INTERFACE_NOT_FOUND_V01 )
       {
         LOG_MSG_ERROR("SetFirewall Entry failed\n",0,0,0);
         return  QCMAP_CM_ERROR;
       }
    }
  }

  index = this->cfg.nat_config.num_firewall_entries;

  /* save into the config */
  this->cfg.nat_config.extd_firewall_entries[index] =
   (qcmap_msgr_firewall_conf_t *)calloc(1,sizeof( qcmap_msgr_firewall_conf_t));

  this->cfg.nat_config.extd_firewall_entries[index]->extd_firewall_entry.firewall_handle = index;

  extd_firewall_conf->extd_firewall_entry.firewall_handle = index;
  this->cfg.nat_config.extd_firewall_entries[index]->extd_firewall_entry.filter_spec.ip_vsn =
   extd_firewall_conf->extd_firewall_entry.filter_spec.ip_vsn;

  if ( extd_firewall_conf->extd_firewall_entry.filter_spec.ip_vsn == IP_V4 )
  {
    next_hdr_prot = extd_firewall_conf->extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot;
    memcpy(&(this->cfg.nat_config.extd_firewall_entries[index]->extd_firewall_entry.filter_spec.ip_hdr.v4),
           &(extd_firewall_conf->extd_firewall_entry.filter_spec.ip_hdr.v4),
           sizeof(this->cfg.nat_config.extd_firewall_entries[index]->extd_firewall_entry.filter_spec.ip_hdr.v4));
  }
  else if ( extd_firewall_conf->extd_firewall_entry.filter_spec.ip_vsn == IP_V6 )
  {
    next_hdr_prot = extd_firewall_conf->extd_firewall_entry.filter_spec.ip_hdr.v6.next_hdr_prot;
    memcpy(&(this->cfg.nat_config.extd_firewall_entries[index]->extd_firewall_entry.filter_spec.ip_hdr.v6),
           &(extd_firewall_conf->extd_firewall_entry.filter_spec.ip_hdr.v6),
           sizeof(this->cfg.nat_config.extd_firewall_entries[index]->extd_firewall_entry.filter_spec.ip_hdr.v6));
  }

  switch(next_hdr_prot)
  {
    case PS_IPPROTO_TCP:
      memcpy(&(this->cfg.nat_config.extd_firewall_entries[index]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp),
             &(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.tcp),
             sizeof(this->cfg.nat_config.extd_firewall_entries[index]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp));
      break;

    case PS_IPPROTO_UDP:
      memcpy(&(this->cfg.nat_config.extd_firewall_entries[index]->extd_firewall_entry.filter_spec.next_prot_hdr.udp),
             &(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.udp),
             sizeof(this->cfg.nat_config.extd_firewall_entries[index]->extd_firewall_entry.filter_spec.next_prot_hdr.udp));
      break;

    case PS_IPPROTO_ICMP:
    case PS_IPPROTO_ICMP6:

      memcpy(&(this->cfg.nat_config.extd_firewall_entries[index]->extd_firewall_entry.filter_spec.next_prot_hdr.icmp),
             &(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.icmp),
             sizeof(this->cfg.nat_config.extd_firewall_entries[index]->extd_firewall_entry.filter_spec.next_prot_hdr.icmp));
      break;

    case PS_IPPROTO_ESP:

      memcpy(&(this->cfg.nat_config.extd_firewall_entries[index]->extd_firewall_entry.filter_spec.next_prot_hdr.esp),
             &(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.esp),
             sizeof(this->cfg.nat_config.extd_firewall_entries[index]->extd_firewall_entry.filter_spec.next_prot_hdr.esp));
      break;

    case PS_IPPROTO_TCP_UDP:

      memcpy(&(this->cfg.nat_config.extd_firewall_entries[index]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range),
             &(extd_firewall_conf->extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range),
             sizeof(this->cfg.nat_config.extd_firewall_entries[index]->extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range));
      break;

    default:
      LOG_MSG_INFO1("\n Un Supported protocol",0,0,0);
      break;
  }

  Dump_firewall_conf(&this->cfg.nat_config.extd_firewall_entries[index]->extd_firewall_entry);

  this->cfg.nat_config.num_firewall_entries++;
  /* if it is OK, call write to XML */
  if( false == this->WriteConfigToFirewallXML() )
  {
    LOG_MSG_INFO1("\n Failed to write firewall entry \n",0,0,0);
    *qmi_err_num =  QMI_ERR_INTERNAL_V01;
    return QCMAP_CM_ERROR;
  }

  /*we will indicate the user that interface was down but entry is stored */
  if( *qmi_err_num == QMI_ERR_INTERFACE_NOT_FOUND_V01 )
  {
    LOG_MSG_ERROR("Entry is added but not reflected in kernel iptables yet\n",0,0,0);
    return QCMAP_CM_ERROR;
  }

  return QCMAP_CM_SUCCESS;
}

/*===========================================================================
  FUNCTION DeleteFireWallEntry
==========================================================================*/
/*!
@brief
  Deletes the firewall entry on A5.

@parameters
  qcmap_msgr_firewall_conf_t* extd_firewall_conf

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::DeleteFireWallEntry
(
   qcmap_msgr_firewall_conf_t* extd_firewall_conf,
   qmi_error_type_v01  *qmi_err_num
)
{
  uint8_t qcmap_cm_errno;
  uint8_t ret_val,i,j;
  uint8_t *ptr=NULL;
  for (i = 0; i < this->cfg.nat_config.num_firewall_entries; i++)
  {
    /* Do not compare the handle value, the last uint32 bytes of the struct. */
    if (extd_firewall_conf->extd_firewall_entry.firewall_handle ==
        this->cfg.nat_config.extd_firewall_entries[i]->extd_firewall_entry.firewall_handle)
    {
      LOG_MSG_INFO1("Found matching handle %d and %d index is %d",
          extd_firewall_conf->extd_firewall_entry.firewall_handle,
          this->cfg.nat_config.extd_firewall_entries[i]->extd_firewall_entry.\
          firewall_handle, i );
      break;
    }
   }
   if (i >= this->cfg.nat_config.num_firewall_entries)
   {   /* we do not have this one  */
    LOG_MSG_INFO1("Did not find matching handle",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_INDEX_V01;
    return false;
   }

   if( SetFirewall(this->cfg.nat_config.extd_firewall_entries[i],FALSE,qmi_err_num) != QCMAP_CM_SUCCESS)
   {
       if(*qmi_err_num != QMI_ERR_INTERFACE_NOT_FOUND_V01)
       {
         LOG_MSG_ERROR("SetFirewall Entry failed\n",0,0,0);
         return  false;
       }
   }

    /* save into the config */
    LOG_MSG_INFO1("\n Deleting Entry =%d \n",i,0,0);
    free(this->cfg.nat_config.extd_firewall_entries[i]);

    this->cfg.nat_config.extd_firewall_entries[i]=NULL;

    for ( j = i; j < (this->cfg.nat_config.num_firewall_entries - 1); j++ )
    {
      this->cfg.nat_config.extd_firewall_entries[j]=this->cfg.nat_config.extd_firewall_entries[j+1];
      this->cfg.nat_config.extd_firewall_entries[j]->extd_firewall_entry.firewall_handle = j;
      this->cfg.nat_config.extd_firewall_entries[j+1]=NULL;
    }

    this->cfg.nat_config.num_firewall_entries--;
    /* if it is OK, call write to XML */
    if(this->WriteConfigToFirewallXML()==false)
    {
      LOG_MSG_INFO1("\n Failed to write firewall entry \n",0,0,0);
      *qmi_err_num =  QMI_ERR_INTERNAL_V01;
      return false;
    }

    /* For QMI_ERR_INTERFACE_NOT_FOUND_V01 we will delete but indicate user with the error message*/
    if( *qmi_err_num == QMI_ERR_INTERFACE_NOT_FOUND_V01 )
    {
      return false;
    }
    return true;
}


/*===========================================================================
  FUNCTION GetFireWallEntry
==========================================================================*/
/*!
@brief
  Gets the firewall entry pointed by the handle.

@parameters
  qcmap_msgr_get_firewall_entry_resp_msg_v01 *resp_msg
  uint32_t handle

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetFireWallEntry
(
  qcmap_msgr_get_firewall_entry_resp_msg_v01 *resp_msg,
  uint32_t handle,
  qmi_error_type_v01 *qmi_err_num
)
{
  uint8_t i;
  qcmap_msgr_firewall_entry_conf_t *firewall_entry;
  uint8_t ip_vsn,next_hdr_prot=0;

  for ( i = 0; i < this->cfg.nat_config.num_firewall_entries; i++ )
  {
    if( handle == this->cfg.nat_config.extd_firewall_entries[i]->extd_firewall_entry.firewall_handle )
    {
      break;
    }
  }

  firewall_entry = &this->cfg.nat_config.extd_firewall_entries[i]->extd_firewall_entry;

  Dump_firewall_conf(firewall_entry);

  if ( i == QCMAP_MSGR_MAX_FIREWALL_ENTRIES_V01 )
  {
    *qmi_err_num = QMI_ERR_INVALID_INDEX_V01;
    return false;
  }

  ip_vsn = this->cfg.nat_config.extd_firewall_entries[i]->extd_firewall_entry.filter_spec.ip_vsn;

  resp_msg->ip_version_valid = TRUE;

  switch(ip_vsn)
  {
    LOG_MSG_INFO1("IP_VSN %d", ip_vsn , 0, 0);

    case IP_V4:

      resp_msg->ip_version = QCMAP_MSGR_IP_FAMILY_V4_V01;
      if ( firewall_entry->filter_spec.ip_hdr.v4.field_mask &
           IPFLTR_MASK_IP4_SRC_ADDR )
      {
        resp_msg->ip4_src_addr_valid = TRUE;
        resp_msg->ip4_src_addr.addr =
          (firewall_entry->filter_spec.ip_hdr.v4.src.addr.ps_s_addr);
        resp_msg->ip4_src_addr.subnet_mask =
          (firewall_entry->filter_spec.ip_hdr.v4.src.subnet_mask.ps_s_addr);
        LOG_MSG_INFO1("IP4 source address is:",0, 0, 0);
        IPV4_ADDR_MSG(firewall_entry->filter_spec.ip_hdr.v4.src.addr.ps_s_addr);
        LOG_MSG_INFO1("IP4 subnet mask is:", 0, 0, 0);
        IPV4_ADDR_MSG(firewall_entry->filter_spec.ip_hdr.v4.src.subnet_mask.ps_s_addr);
      }

      if ( firewall_entry->filter_spec.ip_hdr.v4.field_mask &
           IPFLTR_MASK_IP4_DST_ADDR )
      {
        resp_msg->ip4_dst_addr_valid = TRUE;
        resp_msg->ip4_dst_addr.addr =
          (firewall_entry->filter_spec.ip_hdr.v4.dst.addr.ps_s_addr);
        resp_msg->ip4_dst_addr.subnet_mask =
          (firewall_entry->filter_spec.ip_hdr.v4.dst.subnet_mask.ps_s_addr);
        LOG_MSG_INFO1("IP4 dst address is:",0, 0, 0);
        IPV4_ADDR_MSG(firewall_entry->filter_spec.ip_hdr.v4.dst.addr.ps_s_addr);
        LOG_MSG_INFO1("IP4 dst subnet mask is:", 0, 0, 0);
        IPV4_ADDR_MSG(firewall_entry->filter_spec.ip_hdr.v4.dst.subnet_mask.ps_s_addr);
      }

      if ( firewall_entry->filter_spec.ip_hdr.v4.field_mask &
           IPFLTR_MASK_IP4_TOS )
      {
        resp_msg->ip4_tos_valid = TRUE;
        resp_msg->ip4_tos.value =
          firewall_entry->filter_spec.ip_hdr.v4.tos.val;
        resp_msg->ip4_tos.mask =
          firewall_entry->filter_spec.ip_hdr.v4.tos.mask;
        LOG_MSG_INFO1("IP4 TOS val %d mask %d",
            resp_msg->ip4_tos.value ,
            resp_msg->ip4_tos.mask, 0);
      }

      if ( firewall_entry->filter_spec.ip_hdr.v4.field_mask &
           IPFLTR_MASK_IP4_NEXT_HDR_PROT )
      {
        LOG_MSG_INFO1("IP4 next header protocol %d ", resp_msg->next_hdr_prot, 0, 0);
        resp_msg->next_hdr_prot_valid = TRUE;
        resp_msg->next_hdr_prot = firewall_entry->filter_spec.ip_hdr.v4.next_hdr_prot;
        next_hdr_prot = firewall_entry->filter_spec.ip_hdr.v4.next_hdr_prot;
      }
      break;

    case IP_V6:

      resp_msg->ip_version = QCMAP_MSGR_IP_FAMILY_V6_V01;
      if ( firewall_entry->filter_spec.ip_hdr.v6.field_mask &
           IPFLTR_MASK_IP6_SRC_ADDR )
      {
        memcpy(resp_msg->ip6_src_addr.addr,
               firewall_entry->filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr8,
               QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8));

        resp_msg->ip6_src_addr.prefix_len =
          firewall_entry->filter_spec.ip_hdr.v6.src.prefix_len;
        resp_msg->ip6_src_addr_valid = TRUE;

        LOG_MSG_INFO1("IPV6 src_addr is:", 0, 0, 0);
        IPV6_ADDR_MSG(firewall_entry->filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr64);
        LOG_MSG_INFO1("IP6 prefix length %d  ",
                      resp_msg->ip6_src_addr.prefix_len, 0, 0);
      }

      if ( firewall_entry->filter_spec.ip_hdr.v6.field_mask &
           IPFLTR_MASK_IP6_DST_ADDR )
      {

        memcpy(resp_msg->ip6_dst_addr.addr,
               firewall_entry->filter_spec.ip_hdr.v6.dst.addr.in6_u.u6_addr8,
               QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8));
        resp_msg->ip6_dst_addr.prefix_len =
          firewall_entry->filter_spec.ip_hdr.v6.dst.prefix_len;
        resp_msg->ip6_dst_addr_valid = TRUE;
        LOG_MSG_INFO1("IPV6 dst_addr is:", 0, 0, 0);
        IPV6_ADDR_MSG(firewall_entry->filter_spec.ip_hdr.v6.dst.addr.in6_u.u6_addr64);
        LOG_MSG_INFO1("IP6 prefix length %d  ",
                      resp_msg->ip6_dst_addr.prefix_len, 0, 0);
      }

      if ( firewall_entry->filter_spec.ip_hdr.v6.field_mask &
           IPFLTR_MASK_IP6_TRAFFIC_CLASS )
      {
        resp_msg->ip6_trf_cls.value =
          firewall_entry->filter_spec.ip_hdr.v6.trf_cls.val;
        resp_msg->ip6_trf_cls.mask =
          firewall_entry->filter_spec.ip_hdr.v6.trf_cls.mask;
        resp_msg->ip6_trf_cls_valid = TRUE;
        LOG_MSG_INFO1("trf class val %d  mask %d",
                      resp_msg->ip6_trf_cls.value,
                      resp_msg->ip6_trf_cls.mask, 0);
      }

      if ( firewall_entry->filter_spec.ip_hdr.v6.field_mask &
           IPFLTR_MASK_IP6_NEXT_HDR_PROT )
      {
        resp_msg->next_hdr_prot_valid = TRUE;
        next_hdr_prot = firewall_entry->filter_spec.ip_hdr.v6.next_hdr_prot;
        resp_msg->next_hdr_prot = next_hdr_prot;
        LOG_MSG_INFO1("IP6 next header protocol  %d  ", next_hdr_prot,0, 0);
      }
      break;
    default:
      LOG_MSG_ERROR("Unsupported IP protocol %d", ip_vsn, 0, 0);
  }

  switch(next_hdr_prot)
  {
    case PS_IPPROTO_TCP:

      if ( firewall_entry->filter_spec.next_prot_hdr.tcp.field_mask
           & IPFLTR_MASK_TCP_SRC_PORT )
      {
        resp_msg->tcp_udp_src.port =
          (firewall_entry->filter_spec.next_prot_hdr.tcp.src.port);
        resp_msg->tcp_udp_src.range =
          firewall_entry->filter_spec.next_prot_hdr.tcp.src.range;
        resp_msg->tcp_udp_src_valid = TRUE;
        LOG_MSG_INFO1("TCP src port  %d  range %d",
                      resp_msg->tcp_udp_src.port,
                      resp_msg->tcp_udp_src.range, 0);
      }

      if ( firewall_entry->filter_spec.next_prot_hdr.tcp.field_mask
           & IPFLTR_MASK_TCP_DST_PORT )
      {
        resp_msg->tcp_udp_dst.port =
          (firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port);
        resp_msg->tcp_udp_dst.range =
          firewall_entry->filter_spec.next_prot_hdr.tcp.dst.range;
        resp_msg->tcp_udp_dst_valid = TRUE;
        LOG_MSG_INFO1("TCP dst port  %d  range %d",
                      resp_msg->tcp_udp_dst.port,
                      resp_msg->tcp_udp_dst.range, 0);
      }
      break;

    case PS_IPPROTO_UDP:

      if ( firewall_entry->filter_spec.next_prot_hdr.udp.field_mask
           & IPFLTR_MASK_UDP_SRC_PORT )
      {
        resp_msg->tcp_udp_src.port =
          (firewall_entry->filter_spec.next_prot_hdr.udp.src.port);
        resp_msg->tcp_udp_src.range =
          firewall_entry->filter_spec.next_prot_hdr.udp.src.range;
        resp_msg->tcp_udp_src_valid = TRUE;
        LOG_MSG_INFO1("UDP src port  %d  range %d",
                      resp_msg->tcp_udp_src.port,
                      resp_msg->tcp_udp_src.range, 0);
      }

      if ( firewall_entry->filter_spec.next_prot_hdr.udp.field_mask
           & IPFLTR_MASK_UDP_DST_PORT )
      {
        resp_msg->tcp_udp_dst.port =
          (firewall_entry->filter_spec.next_prot_hdr.udp.dst.port);
        resp_msg->tcp_udp_dst.range =
          firewall_entry->filter_spec.next_prot_hdr.udp.dst.range;
        resp_msg->tcp_udp_dst_valid = TRUE;
        LOG_MSG_INFO1("UDP dst port  %d  range %d",
                      resp_msg->tcp_udp_dst.port ,
                      resp_msg->tcp_udp_dst.range, 0);
      }
      break;

    case PS_IPPROTO_ICMP:
    case PS_IPPROTO_ICMP6:

      if ( firewall_entry->filter_spec.next_prot_hdr.icmp.field_mask
           & IPFLTR_MASK_ICMP_MSG_TYPE )
      {
        resp_msg->icmp_type =
         firewall_entry->filter_spec.next_prot_hdr.icmp.type;
        resp_msg->icmp_type_valid = TRUE;
        LOG_MSG_INFO1("ICMP type  %d  ", resp_msg->icmp_type , 0, 0);
      }

      if ( firewall_entry->filter_spec.next_prot_hdr.udp.field_mask
           & IPFLTR_MASK_ICMP_MSG_CODE )
      {
        resp_msg->icmp_code =
         firewall_entry->filter_spec.next_prot_hdr.icmp.code;
        resp_msg->icmp_code_valid = TRUE;
        LOG_MSG_INFO1("ICMP code  %d  ", resp_msg->icmp_code , 0, 0);
      }
      break;

    case PS_IPPROTO_ESP:

      if ( firewall_entry->filter_spec.next_prot_hdr.esp.field_mask
           & IPFLTR_MASK_ESP_SPI )
      {
        resp_msg->esp_spi =
         (firewall_entry->filter_spec.next_prot_hdr.esp.spi);
        resp_msg->esp_spi_valid = TRUE;
        LOG_MSG_INFO1("ESP spi %d", resp_msg->esp_spi , 0, 0);
      }
      break;

    case PS_IPPROTO_TCP_UDP:

      if ( firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask
           & IPFLTR_MASK_TCP_UDP_SRC_PORT )
      {
        resp_msg->tcp_udp_src.port =
          (firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.src.port);
        resp_msg->tcp_udp_src.range =
          firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.src.range;
        resp_msg->tcp_udp_src_valid = TRUE;
        LOG_MSG_INFO1("TCP_UDP port %d range %d",
                      resp_msg->tcp_udp_src.port ,
                      resp_msg->tcp_udp_src.range, 0);
      }

      if ( firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask
           & IPFLTR_MASK_TCP_UDP_DST_PORT )
      {
        resp_msg->tcp_udp_dst.port =
          (firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.dst.port);
        resp_msg->tcp_udp_dst.range =
          firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.dst.range;
        resp_msg->tcp_udp_dst_valid = TRUE;
        LOG_MSG_INFO1("TCP_UDP port %d range %d",
                      resp_msg->tcp_udp_dst.port ,
                      resp_msg->tcp_udp_dst.range, 0);
      }
     break;

    default:
      LOG_MSG_ERROR("Unsupported next header protocol %d",next_hdr_prot, 0, 0);
  }

  return true;
}

/*===========================================================================
  FUNCTION GetWWANStatus
==========================================================================*/
/*!
@brief
  Gets the WWAN status.

@parameters
  uint8_t *status

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::GetWWANStatus(qcmap_msgr_wwan_call_type_v01 call_type, uint8_t *status, qmi_error_type_v01 *qmi_err_num)
{
  if ( status == NULL )
  {
      LOG_MSG_ERROR("NULL args", 0, 0, 0);
      *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
      return false;
    }
    if ( call_type == QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01 )
      *status=qcmap_cm_get_state(qmi_err_num);
    else if ( call_type == QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01 )
      *status=qcmap_cm_get_ipv6_state(qmi_err_num);
    else
    {
       LOG_MSG_ERROR("Invalid Call Type:%d", call_type, 0, 0);
       *qmi_err_num = QMI_ERR_INVALID_IP_FAMILY_PREF_V01;
       return false;
    }
   return true;
}


/*===========================================================================
  FUNCTION GetMobileAPhandle
==========================================================================*/
/*!
@brief
  Gets the status of mobileap handle.

@parameters
  qcmap_msgr_get_firewall_entry_resp_msg_v01 *resp_msg
  uint32_t handle

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
int  QCMAP_ConnectionManager::GetMobileAPhandle(qmi_error_type_v01 *qmi_err_num)
{
  return this->qcmap_cm_handle;
}

/*===========================================================================
  FUNCTION GetNatTimeoutOnA5
==========================================================================*/
/*!
@brief
  Get the NAT timeout value for the requested nat type.

@parameters
  qcmap_msgr_nat_timeout_enum_v01 timeout_type
  uint32                          *timeout_value

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetNatTimeoutOnA5
(
  qcmap_msgr_nat_timeout_enum_v01 timeout_type,
  uint32 *timeout_value,
  qmi_error_type_v01 *qmi_err_num
)
{
  FILE *fp = NULL;

  if ( timeout_value == NULL )
  {
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
    return false;
  }

  switch ( timeout_type )
  {
     case QCMAP_MSGR_NAT_TIMEOUT_GENERIC_V01:
       fp = fopen("/proc/sys/net/ipv4/netfilter/ip_conntrack_generic_timeout", "r");
       break;
     case QCMAP_MSGR_NAT_TIMEOUT_ICMP_V01:
       fp = fopen("/proc/sys/net/ipv4/netfilter/ip_conntrack_icmp_timeout", "r");
       break;
     case QCMAP_MSGR_NAT_TIMEOUT_TCP_ESTABLISHED_V01:
       fp = fopen("/proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_established", "r");
       break;
     case QCMAP_MSGR_NAT_TIMEOUT_UDP_V01:
       fp = fopen("/proc/sys/net/ipv4/netfilter/ip_conntrack_udp_timeout", "r");
       break;
     default:
       LOG_MSG_INFO1("Timeout Type:%d not supported.\n", timeout_type,0,0);
       *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
       return false;
  }

  if ( fp == NULL )
  {
     LOG_MSG_ERROR("Error opening timeout file: %d.\n", timeout_type,0,0);
     *qmi_err_num = QMI_ERR_INTERNAL_V01;
     return false;
  }

  if (fscanf(fp, "%d", timeout_value) != 1) {
    LOG_MSG_ERROR("Error reading timeout file: %d.\n", timeout_type,0,0);
    *qmi_err_num = QMI_ERR_INTERNAL_V01;
    return false;
  }

  LOG_MSG_INFO1("Timeout Type: %d Timeout Value: %d.\n", timeout_type, *timeout_value,0);

  fclose(fp);

  return true;
}


/*===========================================================================
  FUNCTION SetNatTimeoutOnA5
==========================================================================*/
/*!
@brief
  Will set the NAT timeout value for the identified nat type.

@parameters
  qcmap_msgr_nat_timeout_enum_v01 timeout_type
  uint32                          timeout_value

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetNatTimeoutOnA5
(
  qcmap_msgr_nat_timeout_enum_v01 timeout_type,
  uint32 timeout_value,
  qmi_error_type_v01 *qmi_err_num
)
{
  char command[MAX_COMMAND_STR_LEN];

  switch ( timeout_type )
  {
     case QCMAP_MSGR_NAT_TIMEOUT_GENERIC_V01:
       snprintf( command, MAX_COMMAND_STR_LEN,
            "echo %d > /proc/sys/net/ipv4/netfilter/ip_conntrack_generic_timeout", timeout_value);
       this->cfg.nat_config.nat_entry_generic_timeout = timeout_value;
       break;
     case QCMAP_MSGR_NAT_TIMEOUT_ICMP_V01:
       snprintf( command, MAX_COMMAND_STR_LEN,
            "echo %d > /proc/sys/net/ipv4/netfilter/ip_conntrack_icmp_timeout", timeout_value);
       this->cfg.nat_config.nat_entry_icmp_timeout = timeout_value;
       break;
     case QCMAP_MSGR_NAT_TIMEOUT_TCP_ESTABLISHED_V01:
       snprintf( command, MAX_COMMAND_STR_LEN,
            "echo %d > /proc/sys/net/ipv4/netfilter/ip_conntrack_tcp_timeout_established", timeout_value);
       this->cfg.nat_config.nat_entry_tcp_established_timeout = timeout_value;
       break;
     case QCMAP_MSGR_NAT_TIMEOUT_UDP_V01:
       snprintf( command, MAX_COMMAND_STR_LEN,
            "echo %d > /proc/sys/net/ipv4/netfilter/ip_conntrack_udp_timeout", timeout_value);
       ds_system_call(command, strlen(command));
       snprintf( command, MAX_COMMAND_STR_LEN,
            "echo %d > /proc/sys/net/ipv4/netfilter/ip_conntrack_udp_timeout_stream", timeout_value);
       this->cfg.nat_config.nat_entry_udp_timeout = timeout_value;
       break;
     default:
       LOG_MSG_INFO1("Timeout Type:%d not supported.\n", timeout_type,0,0);
       *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
       return false;
  }
  ds_system_call(command, strlen(command));
  LOG_MSG_INFO1("%s\n", command,0,0);
  return true;
}


/*===========================================================================
  FUNCTION GetNatTimeout
==========================================================================*/
/*!
@brief
  Gets the NAT timeout value configured on A5.

@parameters
  qcmap_msgr_nat_timeout_enum_v01 timeout_type
  uint32                          timeout_value

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetNatTimeout
(
  qcmap_msgr_nat_timeout_enum_v01 timeout_type,
  uint32 *timeout_value,
  qmi_error_type_v01 *qmi_err_num
)
{
  if( !this->GetNatTimeoutOnA5(timeout_type, timeout_value, qmi_err_num) )
  {
    return false;
  }
  return true;
}


/*===========================================================================
  FUNCTION SetNatTimeout
==========================================================================*/
/*!
@brief
  Sets the NAT timeout on A5.

@parameters
  qcmap_msgr_nat_timeout_enum_v01 timeout_type
  uint32                          timeout_value

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetNatTimeout
(
  qcmap_msgr_nat_timeout_enum_v01 timeout_type,
  uint32 timeout_value,
  qmi_error_type_v01 *qmi_err_num
)
{

  if ( timeout_value < QCMAP_NAT_ENTRY_MIN_TIMEOUT )
  {
    LOG_MSG_ERROR("Timeout value should be greater than: %d Got: %d.\n",
                  QCMAP_NAT_ENTRY_MIN_TIMEOUT, timeout_value, 0);
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01 ;
    return false;
  }

  if( !this->SetNatTimeoutOnA5(timeout_type, timeout_value, qmi_err_num) )
  {
    return false;
  }
    /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;
}

/*===========================================================================
  FUNCTION AddEbtablesUSBRulesForBridgeMode
==========================================================================*/
/*!
@brief
  Add Etables rules for LAN clients in AP-STA Bridge Mode

@parameters

@return
  true  - on success
  flase - on failure

@note
  - Dependencies
  - None

- Side Effects
- None
*/
/*=========================================================================*/
bool QCMAP_ConnectionManager::AddEbtablesUSBRulesForBridgeMode(void)
{
  char netmask [QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char usb_intf_name [DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  char command[MAX_COMMAND_STR_LEN];
  ds_dll_el_t * node = NULL;
  qcmap_cm_client_data_info_t* list_data = NULL;
  qcmap_addr_info_list_t* addrList = &(this->addrList);
  in_addr addr;
  qcmap_msgr_device_type_enum_v01 device_type = QCMAP_MSGR_DEVICE_TYPE_ENUM_MIN_ENUM_VAL_V01;

  LOG_MSG_INFO1("QCMAP_ConnectionManager::AddEbtablesUSBRulesForBridgeMode",0,0,0);
  snprintf(command,MAX_COMMAND_STR_LEN,"AddEbtablesUSBRulesForBridgeMode() > /dev/kmsg");
  ds_system_call(command,strlen(command));

  if ( this->cfg.lan_config.usb_conf.link_type == QCMAP_QTI_USB_LINK_RNDIS )
  {
    strlcpy(usb_intf_name, "rndis0",16);
  }
  else if ( this->cfg.lan_config.usb_conf.link_type == QCMAP_QTI_USB_LINK_ECM )
  {
    strlcpy(usb_intf_name, "ecm0",16);
  }
  else
  {
    LOG_MSG_ERROR("Incorrect USB LINK Detected Link Type = %x", this->cfg.lan_config.usb_conf.link_type,0,0);
    return false;
  }

  strlcpy(netmask, inet_ntoa(this->ap_sta_bridge.bridge_netmask), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
  snprintf(command, MAX_COMMAND_STR_LEN,
                     "ebtables -t nat -A PREROUTING -i %s -p arp --arp-ip-src=%s/%s --arp-opcode Request -j arpreply --arpreply-mac %02x:%02x:%02x:%02x:%02x:%02x --arpreply-target ACCEPT",
                     usb_intf_name, inet_ntoa(this->ap_sta_bridge.bridge_def_gw), netmask, this->ap_sta_bridge.bridge_mac[0], this->ap_sta_bridge.bridge_mac[1],
                     this->ap_sta_bridge.bridge_mac[2], this->ap_sta_bridge.bridge_mac[3], this->ap_sta_bridge.bridge_mac[4], this->ap_sta_bridge.bridge_mac[5]);
  ds_system_call(command, strlen(command));

  /* Iterate through the list and get check for any WLAN-AP based entries */
  if ( addrList->addrListHead == NULL)
  {
    LOG_MSG_ERROR("\nMatchMacAddrList() - Linked list head is NULL \n",0,0,0);
    return false;
  }

  if ( addrList->addrListHead == NULL)
  {
    LOG_MSG_ERROR("Empty List: No rules to be entered, per WLAN Iface\n",0,0,0);
    return true;
  }

  /* Add Rules for WLAN Clients connected to Secondary AP */
  device_type = QCMAP_MSGR_DEVICE_TYPE_GUEST_AP_V01;
  node = ds_dll_search (addrList->addrListHead , (void*)&device_type, qcmap_match_device_type);
  while(node)
  {
    list_data = (qcmap_cm_client_data_info_t*)node->data;
    if (list_data != NULL)
    {
      addr.s_addr = list_data->ip_addr;
      ds_log_med(" AddEbtablesUSBRulesForBridgeMode() for IP address %s \n",inet_ntoa(addr));
      snprintf(command, MAX_COMMAND_STR_LEN,
                  "ebtables -t nat -I PREROUTING -i %s -p arp --arp-opcode Request --arp-ip-dst %s -j ACCEPT",
                   usb_intf_name, inet_ntoa(addr));
      ds_system_call(command, strlen(command));
    }
    else
    {
      LOG_MSG_ERROR(" The device information node data is NULL\n",0,0,0);
    }
    node = ds_dll_search (node , (void*)&device_type, qcmap_match_device_type);
  }

  /* Add Rules for WLAN Clients connected to Primary AP */
  device_type = QCMAP_MSGR_DEVICE_TYPE_PRIMARY_AP_V01;
  node = ds_dll_search (addrList->addrListHead , (void*)&device_type, qcmap_match_device_type);
  while(node)
  {
    list_data = (qcmap_cm_client_data_info_t*)node->data;
    if (list_data != NULL)
    {
      addr.s_addr = list_data->ip_addr;
      ds_log_med(" AddEbtablesUSBRulesForBridgeMode() for IP address %s \n",inet_ntoa(addr));
      snprintf(command, MAX_COMMAND_STR_LEN,
                  "ebtables -t nat -I PREROUTING -i %s -p arp --arp-opcode Request --arp-ip-dst %s -j ACCEPT",
                   usb_intf_name, inet_ntoa(addr));
      ds_system_call(command, strlen(command));
    }
    else
    {
      LOG_MSG_ERROR(" The device information node is NULL\n",0,0,0);
    }
    node = ds_dll_search (node , (void*)&device_type, qcmap_match_device_type);
  }
  return true;
}

/*===========================================================================
 FUNCTION DelEbtablesUSBRulesForBridgeMode
==========================================================================*/
/*!
@brief
 Del Etables rules for LAN clients in AP-STA Bridge Mode

@parameters

@return
 true  - on success
 flase - on failure

@note
 - Dependencies
 - None

 - Side Effects
 - None
*/
/*=========================================================================*/
bool QCMAP_ConnectionManager::DelEbtablesUSBRulesForBridgeMode(void)
{
  char netmask [QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char usb_intf_name [DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  char command[MAX_COMMAND_STR_LEN];
  ds_dll_el_t * node = NULL;
  qcmap_cm_client_data_info_t* list_data = NULL;
  qcmap_addr_info_list_t* addrList = &(this->addrList);
  in_addr addr;
  qcmap_msgr_device_type_enum_v01 device_type = QCMAP_MSGR_DEVICE_TYPE_ENUM_MIN_ENUM_VAL_V01;

  LOG_MSG_INFO1("QCMAP_ConnectionManager::DelEbtablesUSBRulesForBridgeMode",0,0,0);

  if ( this->cfg.lan_config.usb_conf.link_type == QCMAP_QTI_USB_LINK_RNDIS )
  {
    strlcpy(usb_intf_name, "rndis0",16);
  }
  else if ( this->cfg.lan_config.usb_conf.link_type == QCMAP_QTI_USB_LINK_ECM )
  {
    strlcpy(usb_intf_name, "ecm0",16);
  }
  else
  {
    LOG_MSG_ERROR("Incorrect USB LINK Detected Link Type = %x", this->cfg.lan_config.usb_conf.link_type,0,0);
    return false;
  }

  /* Add the default Rule for LAN Side Reply */
  strlcpy(netmask, inet_ntoa(this->ap_sta_bridge.bridge_netmask), QCMAP_LAN_MAX_IPV4_ADDR_SIZE);
  snprintf(command, MAX_COMMAND_STR_LEN,
                     "ebtables -t nat -D PREROUTING -i %s -p arp --arp-ip-src=%s/%s --arp-opcode Request -j arpreply --arpreply-mac %02x:%02x:%02x:%02x:%02x:%02x --arpreply-target ACCEPT",
                     usb_intf_name, inet_ntoa(this->ap_sta_bridge.bridge_def_gw), netmask, this->ap_sta_bridge.bridge_mac[0], this->ap_sta_bridge.bridge_mac[1],
                     this->ap_sta_bridge.bridge_mac[2], this->ap_sta_bridge.bridge_mac[3], this->ap_sta_bridge.bridge_mac[4], this->ap_sta_bridge.bridge_mac[5]);
  ds_system_call(command, strlen(command));

  if ( addrList->addrListHead == NULL)
  {
    LOG_MSG_ERROR("Empty List: No rules to be deleted, per WLAN Iface\n",0,0,0);
    return true;
  }

  /* Del Rules for WLAN Clients connected to Secondary AP */
  device_type = QCMAP_MSGR_DEVICE_TYPE_GUEST_AP_V01;
  node = ds_dll_search (addrList->addrListHead , (void*)&device_type, qcmap_match_device_type);
  while(node)
  {
    list_data = (qcmap_cm_client_data_info_t*)node->data;
    if (list_data != NULL)
    {
      addr.s_addr = list_data->ip_addr;
      ds_log_med(" DelEbtablesUSBRulesForBridgeMode() for IP address %s \n",inet_ntoa(addr));
      snprintf(command, MAX_COMMAND_STR_LEN,
                  "ebtables -t nat -D PREROUTING -i %s -p arp --arp-opcode Request --arp-ip-dst %s -j ACCEPT",
                   usb_intf_name, inet_ntoa(addr));
      ds_system_call(command, strlen(command));
    }
    else
    {
      LOG_MSG_ERROR(" The device information data  node is NULL\n",0,0,0);
    }
    node = ds_dll_search (node, (void*)&device_type, qcmap_match_device_type);
  }

  /* Del Rules for WLAN Clients connected to Primary AP */
  device_type = QCMAP_MSGR_DEVICE_TYPE_PRIMARY_AP_V01;
  node = ds_dll_search (addrList->addrListHead , (void*)&device_type, qcmap_match_device_type);
  while(node)
  {
    list_data = (qcmap_cm_client_data_info_t*)node->data;
    if (list_data != NULL)
    {
      addr.s_addr = list_data->ip_addr;
      ds_log_med(" DelEbtablesUSBRulesForBridgeMode() for IP address %s \n",inet_ntoa(addr));
      snprintf(command, MAX_COMMAND_STR_LEN,
                  "ebtables -t nat -D PREROUTING -i %s -p arp --arp-opcode Request --arp-ip-dst %s -j ACCEPT",
                   usb_intf_name, inet_ntoa(addr));
      ds_system_call(command, strlen(command));
    }
    else
    {
      LOG_MSG_ERROR(" The device information node data is NULL\n",0,0,0);
    }
    node = ds_dll_search (node , (void*)&device_type, qcmap_match_device_type);
  }
  return true;
}

/*===========================================================================
  FUNCTION SetupUSBLink
==========================================================================*/
/*!
@brief
   Sets up the USB tethered link as RNDIS or ECM.

@parameters
  qcmap_qti_usb_link_type    usb_link
  int                        *err_num

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetupUSBLink
(
   qcmap_qti_usb_link_type    usb_link,
   int                        *err_num
)
{
  char                        command[MAX_COMMAND_STR_LEN];
  char                        temp_buf[MAX_COMMAND_STR_LEN];
  char                        link_type[16];
  int                         retval;
  in_addr                     start, end, subnet_mask, gateway,gateway_subnet, ppp_addr;
  qmi_error_type_v01          qmi_err_num;
  int j = 0;
  uint32_t time_out = (PPP_DEV_INIT_DELAYS_MAX/PPP_DEV_INIT_DELAY);
/*------------------------------------------------------------------------*/

  LOG_MSG_INFO1("USB link up impl",0,0,0);

  /* PPP should not be enabled in AP-STA Bridge Mode */
  if ( (usb_link == QCMAP_QTI_USB_LINK_PPP) && (this->IsAPSTABridgeActivated() == false))
  {
    /* Check if we have reserved IP. if not return error.*/
    if ( this->cfg.lan_config.ppp_reserved_ip == 0)
    {
        LOG_MSG_ERROR("Reserved IP not available",0,0,0);
      *err_num = QMI_ERR_INSUFFICIENT_RESOURCES_V01;
      return false;
    }

    this->cfg.lan_config.usb_conf.ppp_enabled = true;
    memset(command, 0, MAX_COMMAND_STR_LEN);

    ppp_addr.s_addr = htonl(this->cfg.lan_config.ppp_reserved_ip);

    /* Start the PPP Daemon. */
    /* Refer pppd man page for more info on arguements. */
    /*
     * crtscts --> Specifies that pppd should set the serial port to use
     * hardware flow control using the RTS and CTS signals in the RS-232
     * interface.
     * lock --> Locks the serial device /dev/ttyGS1.
     * %s:%s --> Our IP:Host IP.
     * /dev/ttyGS1 --> Serial device.
     * 115200 --> Baud rate.
     * silent --> PPP negotiation will be started by host and not by us.
     * debug --> Enable debugging.
     * +ipv6 --> Enable IPV6.
     * disconnect <script> --> execute the script when the dun call is
     * disconnected.
     */
    snprintf(command, MAX_COMMAND_STR_LEN, "pppd crtscts lock \
           %s:%s /dev/ttyGS1 115200 \
           silent debug +ipv6 disconnect \"/usr/sbin/chat -V -f /etc/disconnect\"",
           PPP_IFACE_LL_ADDR, inet_ntoa(ppp_addr));
    ds_system_call( command, strlen(command));

    /* Wait till PPP interface is created.. */
    snprintf(command, MAX_COMMAND_STR_LEN, "/proc/sys/net/ipv4/conf/ppp0");
    while ( j++ < time_out )
    {
      std::ifstream ifile( command );
      if ( ifile )
      {
        break;
      }
      usleep( PPP_DEV_INIT_DELAY );
    }

    /* Restart RADISH. */
    StopRadish();
    StartRadish();

    return true;
  }
  else if ((usb_link == QCMAP_QTI_USB_LINK_PPP) && (this->IsAPSTABridgeActivated() == true))
  {
    LOG_MSG_ERROR("Can't enable DUN when in AP-STA Brigde Mode",0,0,0);
    *err_num = QMI_ERR_NOT_SUPPORTED_V01;
    return false;
  }

  SetUSBEnable(true);

/*-----------------------------------------------------------------------
  Assign the interface names
------------------------------------------------------------------------*/
  if ( usb_link == QCMAP_QTI_USB_LINK_RNDIS )
  {
    this->cfg.lan_config.usb_conf.link_type = QCMAP_QTI_USB_LINK_RNDIS;
    strlcpy(link_type, "rndis",16);
  }
  else if ( usb_link == QCMAP_QTI_USB_LINK_ECM )
  {
    this->cfg.lan_config.usb_conf.link_type = QCMAP_QTI_USB_LINK_ECM;
    strlcpy(link_type, "ecm",16);

    /* Enable v6 for ecm0, we disable this in bringDownUSB for cradle */
    snprintf(command, MAX_COMMAND_STR_LEN, "echo 0 > /proc/sys/net/ipv6/conf/%s/disable_ipv6",
             ECM_IFACE);
    ds_system_call(command, strlen(command));
  }

  if (usb_link == QCMAP_QTI_USB_LINK_RNDIS ||
      (this->cfg.cradle_config.cradle_mode == QCMAP_MSGR_CRADLE_DISABLED_V01
       && usb_link == QCMAP_QTI_USB_LINK_ECM))
  {
    /*------------------------------------------------------------------------
      Assign a pre-defined IP address to the RNDIS/ECM gateway interface on A5
      -------------------------------------------------------------------------*/
    strlcpy(command, "ifconfig ", MAX_COMMAND_STR_LEN);

    snprintf(temp_buf, MAX_COMMAND_STR_LEN, "%s0 ", link_type);
    strlcat(command, temp_buf, MAX_COMMAND_STR_LEN);

    gateway.s_addr = htonl(this->cfg.lan_config.usb_conf.gateway_addr);
    strlcat(command, inet_ntoa(gateway), MAX_COMMAND_STR_LEN);

    strlcat(command, " netmask ", MAX_COMMAND_STR_LEN);

    subnet_mask.s_addr = htonl(this->cfg.lan_config.ll_subnet_mask);
    strlcat(command, inet_ntoa(subnet_mask), MAX_COMMAND_STR_LEN);

    ds_system_call( command, strlen(command));
    /*-----------------------------------------------------------------------
      Delete the subnet based route to USB interface
    ------------------------------------------------------------------------*/
    strlcpy(command, "route del -net ", MAX_COMMAND_STR_LEN);

    gateway_subnet.s_addr = htonl(this->cfg.lan_config.usb_conf.gateway_addr &
                                  this->cfg.lan_config.ll_subnet_mask);
    strlcat(command, inet_ntoa(gateway_subnet), MAX_COMMAND_STR_LEN);

    strlcat(command, " netmask ", MAX_COMMAND_STR_LEN);

    subnet_mask.s_addr = htonl(this->cfg.lan_config.ll_subnet_mask);
    strlcat(command, inet_ntoa(subnet_mask), MAX_COMMAND_STR_LEN);

    snprintf(temp_buf, MAX_COMMAND_STR_LEN, " dev %s0 ", link_type);
    strlcat(command, temp_buf, MAX_COMMAND_STR_LEN);

    ds_system_call(command, strlen(command));
    printf("command %s \n",command);

    /* First delete the link-local route. */
    snprintf(command, MAX_COMMAND_STR_LEN, "ip -6 route del fe80::/64 dev %s0",
             link_type);
    ds_system_call(command, strlen(command));

    snprintf(command, MAX_COMMAND_STR_LEN, "brctl addif %s %s0",
             BRIDGE_IFACE,link_type);
    ds_system_call(command, strlen(command));

    if(usb_link == QCMAP_QTI_USB_LINK_RNDIS)
    {
      /*After starting the dhcp server, we need to indicate to the kernel to
        start accepting dhcp requestes from host pc. This is required to avoid
        dhcp request going for a backoff timer at the host PC*/
      ds_system_call("echo 1 > /sys/class/android_usb/f_rndis/rx_trigger",\
                     strlen("echo 1 > /sys/class/android_usb/f_rndis/rx_trigger"));
    }

    /*------------------------------------------------------------------------
       Enable IP forwarding
    ------------------------------------------------------------------------*/
    ds_system_call("echo 1 > /proc/sys/net/ipv4/ip_forward",
                   strlen("echo 1 > /proc/sys/net/ipv4/ip_forward"));

    /* Enter Default Rules for AP-STA Bridge Mode */
    if (this->IsAPSTABridgeActivated() == true)
    {
      this->AddEbtablesUSBRulesForBridgeMode();
    }

    /* Dont enable Multicast services for AP-STA Bridge mode */
    if(this->cfg.srvc_config.dlna_config && (this->IsAPSTABridgeActivated() == false))
    {
      if(!EnableDLNA(&qmi_err_num))
      {
        LOG_MSG_ERROR("Cannot enable DLNA, error: %d.", qmi_err_num, 0, 0);
      }
    }

    if(this->cfg.srvc_config.upnp_config && (this->IsAPSTABridgeActivated() == false))
    {
      if(!EnableUPNP(&qmi_err_num))
      {
        LOG_MSG_ERROR("Cannot enable UPnP, error: %d.", qmi_err_num, 0, 0);
      }
    }
  }
  else if (usb_link == QCMAP_QTI_USB_LINK_ECM &&
           this->cfg.cradle_config.cradle_mode != QCMAP_MSGR_CRADLE_DISABLED_V01)
  {
    if (this->cfg.cradle_config.cradle_mode == QCMAP_MSGR_CRADLE_WAN_ROUTER_V01)
    {
      //unbridge ecm0 and bridge0
      snprintf( command, MAX_COMMAND_STR_LEN, "brctl delif %s %s",
                BRIDGE_IFACE, ECM_IFACE);
      ds_system_call(command, strlen(command));

      /* Obtain and assign IP address via dhcpcd daemon */
      LOG_MSG_INFO1("Running DHCP client on ecm0\n",0,0,0);
      snprintf( command, MAX_COMMAND_STR_LEN, "dhcpcd %s -t 0 -o domain_name_servers --noipv4ll -b",
                ECM_IFACE);
      ds_system_call(command, strlen(command));

      if(this->cfg.lan_config.enable_ipv6)
      {
        snprintf(command, MAX_COMMAND_STR_LEN,
                 "echo 2 > /proc/sys/net/ipv6/conf/%s/accept_ra", ECM_IFACE);
        ds_system_call(command, strlen(command));
      }
    }
    else
    {
      LOG_MSG_ERROR("Unsupported Cradle Mode %d, in usb mode %d", this->cfg.cradle_config.cradle_mode, usb_link, 0);
    }
  }

  this->usb_link = usb_link;
  return true;
}

/*===========================================================================
  FUNCTION BringDownUSBLink
==========================================================================*/
/*!
@brief
  Brings down the USB tethered RNDIS or ECM link.

@parameters
  qcmap_qti_usb_link_type    usb_link
  int                        *err_num

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::BringDownUSBLink
(
   qcmap_qti_usb_link_type  usb_link,
   void                     *softApHandle,
   int                      *err_num
)
{
  qmi_qcmap_msgr_softap_handle_type *handle = (qmi_qcmap_msgr_softap_handle_type*) softApHandle;
  char                        command[MAX_COMMAND_STR_LEN];
  char                        temp_buf[MAX_COMMAND_STR_LEN];
  char                        link_type[16];
  int                         retval;
  uint8_t                     *mac_addr;
  in_addr                     start, end, subnet_mask;
  qmi_error_type_v01 qmi_err_num;

/*------------------------------------------------------------------------*/
  if ( usb_link == QCMAP_QTI_USB_LINK_PPP)
  {
    /* Kill the PPP Daemon. */
    ds_system_call("killall -TERM pppd", strlen("killall -TERM pppd"));
    if(!IsPPPkilled())
    {
      LOG_MSG_ERROR("PPP still running check. Kill forcefully", 0, 0, 0);
      /* Kill forcefully. */
      ds_system_call("killall -KILL pppd", strlen("killall -KILL pppd"));
    }

    this->cfg.lan_config.usb_conf.ppp_enabled = false;

    /* Restart RADISH. */
    StopRadish();
    StartRadish();

    /* Stop services before bringing down bridge so byebye messages are sent */
    if( this->wifi_mode == QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01 )
    {
      if(this->cfg.srvc_config.upnp_config)
      {
        InterimDisableUPNP(&qmi_err_num);
      }

      if(this->cfg.srvc_config.dlna_config)
      {
        InterimDisableDLNA(&qmi_err_num);
      }
    }

    /* Delete delegated prefix. */
    if (prefix_delegation_activated &&
        !qcmap_cm_delete_ipv6_delegated_prefix(this->qcmap_cm_handle, true, this->ppp_ipv6_addr, &qmi_err_num))
    {
      LOG_MSG_ERROR("Error: Unable flush prefix's %d", qmi_err_num, 0, 0);
    }

    /* Delete Device Info. */
    mac_addr = GetUSBMac();
    DeleteDeviceEntryInfo(GetUSBMac(), QCMAP_MSGR_DEVICE_TYPE_USB_V01);

    /* Clear USB MAC */
    memset(mac_addr, 0, QCMAP_MSGR_MAC_ADDR_LEN_V01);

    /* Clear PPP IPV6 address. */
    memset(this->ppp_ipv6_addr, 0, QCMAP_MSGR_IPV6_ADDR_LEN_V01);
    memset(this->ppp_ipv6_iid, 0, QCMAP_MSGR_IPV6_ADDR_LEN_V01);

    return true;
  }

  if (this->IsAPSTABridgeActivated() == true)
  {
    this->DelEbtablesUSBRulesForBridgeMode();
  }

  SetUSBEnable(false);

  /* Stop services before bringing down bridge so byebye messages are sent */
  if( (this->wifi_mode == QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01) && (this->IsAPSTABridgeActivated() == false))
  {
    if(this->cfg.srvc_config.upnp_config)
    {
      InterimDisableUPNP(&qmi_err_num);
    }

    if(this->cfg.srvc_config.dlna_config)
    {
      InterimDisableDLNA(&qmi_err_num);
    }
  }

/*-----------------------------------------------------------------------
  Assign the interface names
------------------------------------------------------------------------*/
  if ( usb_link == QCMAP_QTI_USB_LINK_RNDIS )
  {
    strlcpy(link_type, "rndis",16);
  }
  else if ( usb_link == QCMAP_QTI_USB_LINK_ECM )
  {
    strlcpy(link_type, "ecm",16);
  }


  snprintf(command, MAX_COMMAND_STR_LEN, "brctl delif %s %s0",
           BRIDGE_IFACE, link_type);
  ds_system_call(command, strlen(command));

/*------------------------------------------------------------------------
 Clear the pre-defined IP address. If in Cradle Mode process
 cradle discAssoc
-------------------------------------------------------------------------*/
  /* Remove all v4 IP information from ecm0 */
  snprintf(command, MAX_COMMAND_STR_LEN, "ifconfig %s 0",
           ECM_IFACE);
  ds_system_call(command, strlen(command));

  /* Remove all v6 IP information from ecm0 */
  snprintf(command, MAX_COMMAND_STR_LEN, "echo 1 > /proc/sys/net/ipv6/conf/%s/disable_ipv6",
           ECM_IFACE);
  ds_system_call(command, strlen(command));

  if (usb_link == QCMAP_QTI_USB_LINK_ECM &&
      this->cfg.cradle_config.cradle_mode == QCMAP_MSGR_CRADLE_WAN_ROUTER_V01 &&
      !this->cradle_connect_in_progress)
  {
    LOG_MSG_INFO1("Disconnect Cradle",0,0,0);
    this->ProcessCradleDisAssoc(handle);
  }
  else if (this->cradle_connect_in_progress)
  {
    this->cradle_connect_in_progress = false;
  }
/*----------------------------------------------------------------------
  Delete IPv4 and IPv6 forwarding if WLAN is disabled
-----------------------------------------------------------------------*/
  if( this->wifi_mode == QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01 )
  {
    ds_system_call("echo 0 > /proc/sys/net/ipv4/ip_forward",
                 strlen("echo 0 > /proc/sys/net/ipv4/ip_forward"));
  }

  /*------------------------------------------------------------------------
    Delete Stale Conntrack Entries and Set the Neigh IP to 0 due to Link DN
  ------------------------------------------------------------------------*/
  DelUsbConntrack();
  SetUSBNeighIP(0);

  mac_addr = GetUSBMac();

  /* Delete delegated prefix */
  if (prefix_delegation_activated && !this->sta_connected && !this->cradle_backhaul_connected)
  {
    int num_entries, i;
    qcmap_msgr_connected_device_info_v01 connected_devices[QCMAP_MSGR_MAX_CONNECTED_DEVICES_V01];

    if(GetConnectedDevicesInfo(connected_devices,
                                          &num_entries, &qmi_err_num))
    {
      for (i =0; i < num_entries; i++)
      {
        if (memcmp(connected_devices[i].client_mac_addr, mac_addr, sizeof(mac_addr)) == 0)
          break;
      }
      if (!qcmap_cm_delete_ipv6_delegated_prefix(this->qcmap_cm_handle, true, connected_devices[i].ipv6_addr, &qmi_err_num))
      {
        LOG_MSG_ERROR("Error: Unable flush prefix's %d", qmi_err_num, 0, 0);
      }
    }
    else
      LOG_MSG_ERROR("Error: Unable get connectedDevices %d", qmi_err_num, 0, 0);
  }

  DeleteDeviceEntryInfo((void*) mac_addr,
                        QCMAP_MSGR_DEVICE_TYPE_USB_V01);

  this->usb_link = QCMAP_QTI_USB_LINK_NONE;
  return true;
}

boolean QCMAP_ConnectionManager::SetFirewallConfig(boolean enable_firewall, boolean pkts_allowed,qmi_error_type_v01  *qmi_err_num )
{
  uint i;

  if( enable_firewall )
  {
    if(!this->cfg.nat_config.firewall_enabled)
    {
       LOG_MSG_INFO1("\n Firewall Was disabled Enabling the same \n",0,0,0);
       this->cfg.nat_config.firewall_enabled = enable_firewall;
       this->cfg.nat_config.firewall_pkts_allowed = pkts_allowed;
       SetDefaultFirewall();
       for (i = 0; i < this->cfg.nat_config.num_firewall_entries; i++)
      {
        SetFirewall(this->cfg.nat_config.extd_firewall_entries[i], true, qmi_err_num);
      }
    }

    if( this->cfg.nat_config.firewall_enabled &&
        this->cfg.nat_config.firewall_pkts_allowed != pkts_allowed)
    {
      LOG_MSG_INFO1("\n Firewall Was enabled changing the action\n",0,0,0);
      for (i = 0; i < this->cfg.nat_config.num_firewall_entries; i++)
      {
        SetFirewall(this->cfg.nat_config.extd_firewall_entries[i], false, qmi_err_num);
      }
      this->cfg.nat_config.firewall_pkts_allowed = pkts_allowed;
      SetDefaultFirewall();
      for (i = 0; i < this->cfg.nat_config.num_firewall_entries; i++)
      {
        SetFirewall(this->cfg.nat_config.extd_firewall_entries[i], true, qmi_err_num);
      }
    }
  }
  else
  {
     LOG_MSG_INFO1("\n In Disable firewall \n",0,0,0);
    if(this->cfg.nat_config.firewall_enabled)
    {
      LOG_MSG_INFO1("\n Firewall was enabled disabling the same \n",0,0,0);
      for (i = 0; i < this->cfg.nat_config.num_firewall_entries; i++)
      {
        SetFirewall(this->cfg.nat_config.extd_firewall_entries[i], false, qmi_err_num);
      }
    }
    this->cfg.nat_config.firewall_enabled = enable_firewall;
    SetDefaultFirewall();
  }

  this->WriteConfigToFirewallXML();
  return true;
}

boolean QCMAP_ConnectionManager::GetFirewallConfig( boolean *enable_firewall,
                                                    boolean *pkts_allowed,
                                                    qmi_error_type_v01  *qmi_err_numi )
{
  *enable_firewall = this->cfg.nat_config.firewall_enabled;

  if( this->cfg.nat_config.firewall_enabled )
    *pkts_allowed = this->cfg.nat_config.firewall_pkts_allowed;

  LOG_MSG_INFO1( "\nenable_firewall = %d firewall_pkts_allowed =%d \n",
                 this->cfg.nat_config.firewall_enabled,*pkts_allowed,
                 this->cfg.nat_config.firewall_pkts_allowed );
  return true;
}
/*===========================================================================
  FUNCTION GetIPv4State
==========================================================================*/
/*!
@brief
  Gets the IPv4 state.

@parameters
  uint8_t *status,
  qmi_error_type_v01 *qmi_err_num

@return
  true  - on Success
  false - on Failure

@note

Dependencies
  None

Side Effects
  None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetIPv4State(uint8_t *status, qmi_error_type_v01 *qmi_err_num)
{

  *status=this->cfg.lan_config.enable_ipv4 ;

  return true;

}

/*===========================================================================
  FUNCTION GetIPv6State
==========================================================================*/
/*!
@brief
  Gets the IPv6 state.

@parameters
  uint8_t *status,
  qmi_error_type_v01 *qmi_err_num

@return
  true  - on Success
  false - on Failure

@note

Dependencies
  None

Side Effects
  None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetIPv6State(uint8_t *status, qmi_error_type_v01 *qmi_err_num)
{

  *status=this->cfg.lan_config.enable_ipv6 ;

  return true;

}

/*===========================================================================
  FUNCTION GetWWANPolicy
==========================================================================*/
/*!
@brief
  Gets the WWAN profile configured value.

@parameters
  qcmap_msgr_net_policy_info_v01 *wwan_policy,
  qmi_error_type_v01 *qmi_err_num

@return
  true  - on Success
  false - on Failure

@note

Dependencies
  None

Side Effects
  None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetWWANPolicy(qcmap_msgr_net_policy_info_v01 *wwan_policy,
                                 qmi_error_type_v01 *qmi_err_num)
{
  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Get WWAN config */
  wwan_policy->tech_pref = this->cfg.wan_config.tech;
  wwan_policy->ip_family = QCMAP_MSGR_IP_FAMILY_V4V6_V01;
  wwan_policy->v4_profile_id_3gpp = this->cfg.wan_config.profile_id.v4.umts_profile_index;
  wwan_policy->v4_profile_id_3gpp2 = this->cfg.wan_config.profile_id.v4.cdma_profile_index;
  wwan_policy->v6_profile_id_3gpp = this->cfg.wan_config.profile_id.v6.umts_profile_index;
  wwan_policy->v6_profile_id_3gpp2 = this->cfg.wan_config.profile_id.v6.cdma_profile_index;

  return true;

}
/*===========================================================================
  FUNCTION SetWWANPolicy
==========================================================================*/
/*!
@brief
  Sets the WWAN profile.

@parameters
  qcmap_msgr_net_policy_info_v01 *wwan_policy,
  qmi_error_type_v01 *qmi_err_num

@return
  true  - on Success
  false - on Failure

@note

Dependencies
  None

Side Effects
  None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetWWANPolicy(qcmap_msgr_net_policy_info_v01 wwan_policy,
                                               qmi_error_type_v01 *qmi_err_num)
{
  int ret_val = QCMAP_CM_SUCCESS;

  /* Set WWAN config */
  ret_val = qcmap_cm_set_wwan_policy(this->qcmap_cm_handle,
                                     wwan_policy, qmi_err_num);

  if (ret_val != QCMAP_CM_SUCCESS)
    return false;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();

  return true;

}

/*===========================================================================
  FUNCTION  EnableUPNP
==========================================================================*/
/*!
@brief
  Enables UPnP daemon.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::EnableUPNP(qmi_error_type_v01 *qmi_err_num)
{
  int ret = 0;
  uint8_t status = 0;
  qmi_error_type_v01 wan_err_num;
  QCMAP_CM_LOG_FUNC_ENTRY();
  char tmp[MAX_CMD_SIZE];
  char bef[] = "sed -i \"56 apresentation_url=/";
  char aft[] = ":8201\" /etc/miniupnpd/miniupnpd.conf";
  in_addr addr;

  if(this->upnp_state == QCMAP_MSGR_UPNP_MODE_UP_V01)
  {
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    return false;
  }

  /* Stop UPnP in case it was started manually by the user */
  ret = system("pgrep miniupnpd -f /etc/miniupnpd/miniupnpd.conf");
  if(ret == 0)
  {
    ds_system_call("/etc/init.d/miniupnpd stop", strlen("/etc/init.d/miniupnpd stop"));
    sleep(1);
  }

  /* Check that the bridge is up */
  if(this->bridge_inited){
    /* update the presentation page url to the current IP address */
    ds_system_call("sed -i \"/presentation_url=/d\" /etc/miniupnpd/miniupnpd.conf",
                 strlen("sed -i \"/presentation_url=/d\" /etc/miniupnpd/miniupnpd.conf"));
    addr.s_addr = htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].a5_ip_addr);
    snprintf(tmp, sizeof(tmp), "%s%s%s", bef, inet_ntoa(addr), aft);
    ds_system_call(tmp, strlen(tmp));

    ret = system("/etc/init.d/miniupnpd start");
  }
  else{
    /* Incorrect Mode. Restore it to default mode and continue. */
    LOG_MSG_ERROR("Bridge0 is not up: %d, UPnP not starting.\n",
                this->cfg.lan_config.wlan_mode, 0, 0);

    *qmi_err_num = QMI_ERR_INTERFACE_NOT_FOUND_V01;
    return false;
  }

  if(ret != 0){
    LOG_MSG_ERROR("UPNP: Ret value = %d.\n", ret, 0, 0);
    *qmi_err_num = QMI_ERR_INTERFACE_NOT_FOUND_V01;

    return false;
  }

  this->cfg.srvc_config.upnp_config = true;
  this->upnp_state = QCMAP_MSGR_UPNP_MODE_UP_V01;
  this->WriteConfigToXML();
  return true;
}


/*===========================================================================
  FUNCTION  DisableUPNP
==========================================================================*/
/*!
@brief
  Disables UPnP daemon.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::DisableUPNP(qmi_error_type_v01 *qmi_err_num)
{
  int ret = 0;
  QCMAP_CM_LOG_FUNC_ENTRY();

  ret = system("/etc/init.d/miniupnpd stop");

  if(ret != 0)
  {
    LOG_MSG_ERROR("UPNP Disable: Unable to stop UPNP, Ret value = %d.\n",
                  ret, 0, 0);
  }

  this->cfg.srvc_config.upnp_config = false;
  this->upnp_state = QCMAP_MSGR_UPNP_MODE_DOWN_V01;
  this->WriteConfigToXML();
  return true;
}


/*===========================================================================
  FUNCTION  InterimDisableUPNP
==========================================================================*/
/*!
@brief
  Disables UPnP daemon.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::InterimDisableUPNP(qmi_error_type_v01 *qmi_err_num)
{
  int ret = 0;
  QCMAP_CM_LOG_FUNC_ENTRY();

  ret = system("/etc/init.d/miniupnpd stop");

  if(ret != 0)
  {
    LOG_MSG_ERROR("UPNP Interim Disable: Unable to stop UPNP, Ret value = %d.\n",
                  ret, 0, 0);
  }

  this->upnp_state = QCMAP_MSGR_UPNP_MODE_DOWN_V01;
  return true;
}


/*===========================================================================
  FUNCTION GetUPNPStatus
==========================================================================*/
/*!
@brief
  Gets the UPNP status.

@parameters
  uint8_t *status

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::GetUPNPStatus(qcmap_msgr_upnp_mode_enum_v01 *status, qmi_error_type_v01 *qmi_err_num)
{
  *status = this->upnp_state;

  return true;
}


/*===========================================================================
  FUNCTION  EnableDLNA
==========================================================================*/
/*!
@brief
  Enables DLNA.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::EnableDLNA(qmi_error_type_v01 *qmi_err_num)
{
  int ret = 0;
  uint8_t status = 0;
  qmi_error_type_v01 wan_err_num;
  QCMAP_CM_LOG_FUNC_ENTRY();

  if(this->dlna_state == QCMAP_MSGR_DLNA_MODE_UP_V01)
  {
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    return false;
  }

  /*  Check if the DLNA is already running or not.(could be started by a user manually)*/
  /* IsDLNAKilled() returns FALSE if DLNA is running.*/
  if(!IsDLNAKilled())
  {
    ds_system_call("/etc/init.d/minidlna stop", strlen("/etc/init.d/minidlna stop"));
    sleep(1);

    /* Once stopped check if it has actually stopped or not. */
    if (!IsDLNAKilled())
    {
      LOG_MSG_ERROR("Unable to stop DLNA in 1 second.\n", 0, 0, 0);
      *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
      return false;
    }
  }

  /* Check that bridge0 is up */
  if(this->bridge_inited){
    ret = system("/etc/init.d/minidlna start");
  }
  else{
    /* Incorrect Mode. */
    LOG_MSG_ERROR("Bridge0 is not up: %d, DLNA not starting.\n",
                this->cfg.lan_config.wlan_mode, 0, 0);

    *qmi_err_num = QMI_ERR_INTERFACE_NOT_FOUND_V01;
    return false;
  }

  if(ret != 0){
    LOG_MSG_ERROR("DLNA: Ret value = %d.\n", ret, 0, 0);
    *qmi_err_num = QMI_ERR_INTERFACE_NOT_FOUND_V01;

    return false;
  }

  this->cfg.srvc_config.dlna_config = true;
  this->dlna_state = QCMAP_MSGR_DLNA_MODE_UP_V01;
  this->WriteConfigToXML();
  return true;
}


/*===========================================================================
  FUNCTION  DisableDLNA
==========================================================================*/
/*!
@brief
  Disable DLNA based.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::DisableDLNA(qmi_error_type_v01 *qmi_err_num)
{
  int ret = 0;
  QCMAP_CM_LOG_FUNC_ENTRY();

  ret = system("/etc/init.d/minidlna stop");
  if(ret != 0)
  {
    LOG_MSG_ERROR("DLNA Disable: Unable to stop DLNA, Ret value = %d.\n",
                  ret, 0, 0);
  }

  this->cfg.srvc_config.dlna_config = false;
  this->dlna_state = QCMAP_MSGR_DLNA_MODE_DOWN_V01;
  this->WriteConfigToXML();

  if (!IsDLNAKilled())
  {
    LOG_MSG_ERROR("Unable to stop DLNA in 1 second.\n", 0, 0, 0);
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    return false;
  }
  return true;
}


/*===========================================================================
  FUNCTION  InterimDisableDLNA
==========================================================================*/
/*!
@brief
  Disable DLNA based.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::InterimDisableDLNA(qmi_error_type_v01 *qmi_err_num)
{
  int ret = 0;

  ret = system("/etc/init.d/minidlna stop");
  if(ret != 0)
  {
    LOG_MSG_ERROR("DLNA Interim Disable: Unable to stop DLNA, Ret value = %d.\n",
                  ret, 0, 0);
  }

  this->dlna_state = QCMAP_MSGR_DLNA_MODE_DOWN_V01;

  if (!IsDLNAKilled())
  {
    LOG_MSG_ERROR("Unable to stop DLNA in 1 second.\n", 0, 0, 0);
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    return false;
  }

  return true;
}


/*===========================================================================
  FUNCTION  RestartDLNA
==========================================================================*/
/*!
@brief
  Restart DLNA daemon if already running

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::RestartDLNA(qmi_error_type_v01 *qmi_err_num)
{
  /* Start DLNA if auto start is enabled. Restart if already running */
  if(this->cfg.srvc_config.dlna_config)
  {
    if (this->dlna_state == QCMAP_MSGR_DLNA_MODE_UP_V01)
    {
      if (!InterimDisableDLNA(qmi_err_num))
      {
        LOG_MSG_INFO1("Disable DLNA failed: %d.", *qmi_err_num,0,0);
      }
    }

    if(!EnableDLNA(qmi_err_num))
    {
      LOG_MSG_ERROR("Cannot enable DLNA, error: %d.", *qmi_err_num, 0, 0);
      return false;
    }
  }

  return true;
}


/*===========================================================================
  FUNCTION GetDLNAStatus
==========================================================================*/
/*!
@brief
  Gets the DLNA status.

@parameters
  uint8_t *status

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::GetDLNAStatus(qcmap_msgr_dlna_mode_enum_v01 *status, qmi_error_type_v01 *qmi_err_num)
{
  *status = this->dlna_state;

  return true;
}


/*===========================================================================
  FUNCTION SetDLNAMediaDir
==========================================================================*/
/*!
@brief
  Sets the DLNA media directory.

@parameters
  char *media_dir

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::SetDLNAMediaDir(char media_dir[], qmi_error_type_v01 *qmi_err_num)
{
  int ret;
  int size = sizeof(media_dir_blacklist)/sizeof(media_dir_blacklist[0]);
  char tmp[MAX_CMD_SIZE];
  char bef[] = "sed -i \"13 amedia_dir=";
  char aft[] = "\" /etc/minidlna.conf";
  char *token, *ptr;

  //check against black list
  for (int i=0; i < size ; i++)
  {
    if (strstr(media_dir, media_dir_blacklist[i].c_str()))
    {
      LOG_MSG_ERROR("DLNA Media dir matches blacklist, not adding: %d", i, 0, 0);
      *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
      return false;
    }
  }

  ret = system("sed -i \"/media_dir=\\/.*/d\" /etc/minidlna.conf");

  token = strtok_r(media_dir, ",", &ptr);
  while (token != NULL)
  {
    snprintf(tmp, sizeof(tmp), "%s%s%s", bef, token, aft);
    ret = system(tmp);
    token = strtok_r(NULL, ",", &ptr);
  }

  if(!RestartDLNA(qmi_err_num))
  {
    LOG_MSG_ERROR("Restart DLNA failed. \n",0,0,0);
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    return false;
  }

  return true;
}


/*===========================================================================
  FUNCTION GetDLNAMediaDir
==========================================================================*/
/*!
@brief
  Gets the DLNA media directory.

@parameters
  char *media_dir

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::GetDLNAMediaDir(char media_dir[], qmi_error_type_v01 *qmi_err_num)
{
  FILE *file = NULL;
  char temp[MAX_SCAN_SIZE] = "";
  int i;

  file = fopen("/etc/minidlna.conf", "rt");

  if (file == NULL)
  {
    LOG_MSG_ERROR("GetDLNAMediaDir unable to mind /etc/minidlna.conf!", 0, 0, 0);
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    return false;
  }

  while (fgets(temp, MAX_SCAN_SIZE, file) != NULL)
  {
    if (strstr(temp, "media_dir=") != NULL && temp[0] != '#')
    {
      strlcat(media_dir, &temp[10], QCMAP_MSGR_MAX_DLNA_DIR_LEN_V01);
    }
  }

  fclose(file);
  return true;
}


/*===========================================================================
  FUNCTION  EnableMDNS
==========================================================================*/
/*!
@brief
  Enable M-DNS if it not already running

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::EnableMDNS(qmi_error_type_v01 *qmi_err_num)
{

  QCMAP_CM_LOG_FUNC_ENTRY();
  int ret=0;

  if(this->mdns_state == QCMAP_MSGR_MDNS_MODE_UP_V01) {
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    LOG_MSG_INFO1("M-DNS Already Enabled \n", 0, 0, 0);
    return false;
  } else {
    // Enable MDNS
    ret = ds_system_call("/etc/init.d/avahi-daemon start",strlen("/etc/init.d/avahi-daemon start"));
	if (ret == 0) {
	  LOG_MSG_INFO1("M-DNS ENABLED!! ", 0, 0, 0);
	  this->mdns_state = QCMAP_MSGR_MDNS_MODE_UP_V01;
	  this->cfg.srvc_config.mdns_config = true;
	  this->WriteConfigToXML();
	  return true;
	} else {
      LOG_MSG_INFO1("M-DNS ENABLE Failed, ds_system_call failed", 0, 0, 0);
      /* need to return appropriate QMI error*/
      return false;
	}
  }
}


/*===========================================================================
  FUNCTION  DisableMDNS
==========================================================================*/
/*!
@brief
  Disable M-DNS if it is already running

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::DisableMDNS(qmi_error_type_v01 *qmi_err_num)
{

  QCMAP_CM_LOG_FUNC_ENTRY();
  int ret=0;

  if(this->mdns_state == QCMAP_MSGR_MDNS_MODE_DOWN_V01) {
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    LOG_MSG_INFO1("M-DNS Not Enabled \n", 0, 0, 0);
    return false;
  } else {
    // Disable MDNS
    ret = ds_system_call("/etc/init.d/avahi-daemon stop",strlen("/etc/init.d/avahi-daemon stop"));
	if (ret == 0) {
	  LOG_MSG_INFO1("M-DNS DISABLED", 0, 0, 0);
	  this->mdns_state = QCMAP_MSGR_MDNS_MODE_DOWN_V01;
	  this->cfg.srvc_config.mdns_config = false;
	  this->WriteConfigToXML();
	  return true;
	} else {
      LOG_MSG_INFO1("M-DNS DISABLED Failed, ds_system_call failed", 0, 0, 0);
      /* need to return appropriate QMI error*/
      return false;
	}
  }
}

/*===========================================================================
  FUNCTION GetMDNSStatus
==========================================================================*/
/*!
@brief
  Gets the MDNS status.

@parameters
  uint8_t *status

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::GetMDNSStatus(qcmap_msgr_mdns_mode_enum_v01 *status, qmi_error_type_v01 *qmi_err_num)
{
  *status = this->mdns_state;

  return true;
}

/*===========================================================================
  FUNCTION get_backhaul_name_for_firewall
==========================================================================*/
/*!
@brief
  Gets the backhaul name to apply firewall configuration

@parameters
   qcmap_msgr_firewall_conf_t *firewall

@return
  char * devname

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::get_backhaul_name_for_firewall(qcmap_backhaul_interface_type interface, char* devname)
{
    int ret=0;
    LOG_MSG_INFO1("get_backhaul_name_for_firewall",0,0,0);
   // Sanity checks
    if ((!interface) || (devname == NULL) ) {
       LOG_MSG_ERROR("Invalid Arguments interface = %x, devname = %x",interface,devname,0);
       return false;
    }

    // If Cradle/STA Mode then return STA Interface
    if ( this->cradle_backhaul_connected )
    {
      strlcpy(devname, ECM_IFACE, QCMAP_MSGR_INTF_LEN);
      return true;
    }
    else if (this->sta_connected) {
       strlcpy(devname,this->cfg.lan_config.sta_interface,QCMAP_MSGR_INTF_LEN);
       return true;
    }

    switch (interface)
    {
     case TYPE_IPV4:
       {
          if (qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_MSGR_IP_FAMILY_V4_V01, devname, &ret) != QCMAP_CM_SUCCESS ) {
              LOG_MSG_ERROR("Couldn't get rmnet name. error %d\n", ret,0,0);
              return false;
          }
        }
        break;
       case TYPE_IPV6:
       {
          if (qcmap_cm_get_dev_name(this->qcmap_cm_handle, QCMAP_MSGR_IP_FAMILY_V6_V01, devname, &ret) != QCMAP_CM_SUCCESS ) {
              LOG_MSG_ERROR("Couldn't get rmnet name. error %d\n", ret,0,0);
              return false;
          }
        }
        break;
        default:
       {
          LOG_MSG_ERROR("Incorrect Interface passed interface = %d",interface,0,0);
       }
       break;
    }

    return true;
}

/*===========================================================================
  FUNCTION SetDefaultFirewallRule
==========================================================================*/
/*!
@brief
  Set the default firewall rule based on pkts_allowed and firewall enable bits for various interfaces

@parameters
   none

@return
   returns succesful

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetDefaultFirewallRule(qcmap_backhaul_interface_type interface)
{
   char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2]={0};
   char command[MAX_COMMAND_STR_LEN]={0};

   if  (!get_backhaul_name_for_firewall(interface,devname)) {
        LOG_MSG_ERROR(" Could not get backhaul interface for interface %d ",interface,0,0);
        return false;
    }

   LOG_MSG_INFO1("SetDefaultFirewallRule:: Enter for Interface %x and devname = %s",interface,devname,0);

   /* Default Firewall Configuration based on the mode*/
   switch (interface)
   {
    case TYPE_IPV4:
      {
         if ((this->cfg.nat_config.firewall_enabled == false) || ((this->cfg.nat_config.firewall_enabled == true) && (cfg.nat_config.firewall_pkts_allowed == false)) ) {
            snprintf(command,MAX_COMMAND_STR_LEN,"iptables -t mangle -D PREROUTING -i %s  -j DROP",devname);
            ds_system_call(command,strlen(command));
         } else if ((this->cfg.nat_config.firewall_enabled == true) && (cfg.nat_config.firewall_pkts_allowed == true)) {
            snprintf(command,MAX_COMMAND_STR_LEN,"iptables -t mangle -A PREROUTING -i %s  -j DROP",devname);
            ds_system_call(command,strlen(command));
         }
      }
      break;
      case TYPE_IPV6:
      {
         if ((this->cfg.nat_config.firewall_enabled == false) || ((this->cfg.nat_config.firewall_enabled == true) && (cfg.nat_config.firewall_pkts_allowed == false)) ) {
            snprintf(command,MAX_COMMAND_STR_LEN,"ip6tables -t mangle -D PREROUTING -i %s  -j DROP",devname);
            ds_system_call(command,strlen(command));
         } else if ((this->cfg.nat_config.firewall_enabled == true) && (cfg.nat_config.firewall_pkts_allowed == true)) {
            snprintf(command,MAX_COMMAND_STR_LEN,"ip6tables -t mangle -A PREROUTING -i %s  -j DROP",devname);
            ds_system_call(command,strlen(command));
         }
       }
       break;
       default:
       {
          LOG_MSG_ERROR("Incorrect Interface passed interface = %d",interface,0,0);
       }
       break;
   }

   return true;
}


/*===========================================================================
  FUNCTION FlushIPV4Firewall
==========================================================================*/
/*!
@brief
  Flush IPV4 firewall entries

@parameters
   none

@return
   none

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::FlushIPV4Firewall(void)
{
     LOG_MSG_INFO1("QCMAP_ConnectionManager::FlushIPV4Firewall() Enter",0,0,0);
     ds_system_call("iptables -t mangle -F",strlen("iptables -t mangle -F"));
}

/*===========================================================================
  FUNCTION FlushIPv6Firewall
==========================================================================*/
/*!
@brief
  Flush IPV6 firewall entries

@parameters
   none

@return
   none

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::FlushIPV6Firewall(void)
{
     LOG_MSG_INFO1("QCMAP_ConnectionManager::FlushIPV6Firewall() Enter",0,0,0);
     ds_system_call("ip6tables -t mangle -F",strlen("ip6tables -t mangle -F"));
}

/*===========================================================================
  FUNCTION EnableIPv6Firewall
==========================================================================*/
/*!
@brief
  EnableIPv6Firewall based on the WAN Connect

@parameters
   none

@return
   returns succesful

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::EnableIPV6DefaultFirewall(void)
{
     LOG_MSG_INFO1("QCMAP_ConnectionManager::EnableIPV6Firewall() Enter",0,0,0);
     qcmap_backhaul_interface_type interface;

     interface = TYPE_IPV6;
     if (!SetDefaultFirewallRule(interface)) {
        LOG_MSG_ERROR("Default firewall rules not added for IPV6",0,0,0);
        return false;
     }
     return true;
}

/*===========================================================================
  FUNCTION EnableIPv6Firewall
==========================================================================*/
/*!
@brief
  EnableIPv6Firewall based on the WAN Connect

@parameters
   none

@return
   returns succesful

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::EnableIPV6Firewall(void)
{
    LOG_MSG_INFO1("QCMAP_ConnectionManager::EnableIPV6Firewall() Enter is_sta_enabled  = %x",InStaMode(),0,0);
    qcmap_backhaul_interface_type  interface = TYPE_IPV6;
    qcmap_msgr_firewall_entry_conf_t *firewall_entry = NULL;
    int i =0;
    qmi_error_type_v01 qmi_err_num;

    if (this->IsAPSTABridgeActivated())
    {
      LOG_MSG_INFO1("QCMAP_ConnectionManager::EnableIPV6Firewall() Not needed in Bridge Mode",0,0,0);
      return true;
    }

    // Flush firewall rules
    FlushIPV6Firewall();

   // Enter default firewall rule only for IPV6
    interface = TYPE_IPV6;
    if (!SetDefaultFirewallRule(interface)) {
        LOG_MSG_ERROR("Default firewall rules not added for IPV6",0,0,0);
    }

    // Enter IPV6 Firewall rules
    if (this->cfg.nat_config.firewall_enabled) {
    for (i = 0; i < this->cfg.nat_config.num_firewall_entries; i++) {
       firewall_entry = &cfg.nat_config.extd_firewall_entries[i]->extd_firewall_entry;
       if (firewall_entry && firewall_entry->filter_spec.ip_vsn == IP_V6 ) {
           SetFirewallV6(this->cfg.nat_config.extd_firewall_entries[i],true, &qmi_err_num);
       }
    }
  }
    return true;
}

/*===========================================================================
  FUNCTION SetDefaultFirewall(void)
==========================================================================*/
/*!
@brief
  EnableIPv6Firewall based on the WAN Connect

@parameters
   none

@return
   returns succesful

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::SetDefaultFirewall(void)
{
    LOG_MSG_INFO1("QCMAP_ConnectionManager::SetDefaultFirewall() Enter",0,0,0);
    qcmap_backhaul_interface_type interface;
   // Adding default firewall rules for sta and non-sta mode
   interface = TYPE_IPV4;
   if (!SetDefaultFirewallRule(interface)) {
      LOG_MSG_ERROR("Default firewall rules not added for IPV4",0,0,0);
   }
   interface = TYPE_IPV6;
   if (!SetDefaultFirewallRule(interface)) {
      LOG_MSG_ERROR("Default firewall rules not added for IPV6",0,0,0);
   }
}

/*===========================================================================
  FUNCTION IsLanCfgUpdated
==========================================================================*/
/*!
@brief
  Check is current applied LAN config and the prev LAN config are the same

@parameters
   none

@return
   returns succesful if prev and current lan config are different

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::IsLanCfgUpdated(void)
{

  if ((this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].a5_ip_addr != prev_lan_config.gw_ip) ||
       (this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].sub_net_mask != prev_lan_config.netmask) ||
       (this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].enable_dhcpd != prev_lan_config.enable_dhcp)) {
     return true;
  }

  if (this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].enable_dhcpd) {
    if ((this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_start_address != prev_lan_config.dhcp_config.dhcp_start_ip) ||
        (this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_end_address != prev_lan_config.dhcp_config.dhcp_end_ip ) ||
        ((uint32)atoi(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_lease_time) != prev_lan_config.dhcp_config.lease_time)){
       return true;
    }
    /* Check if the reservations are updated. */
    if (this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_reservations_updated == TRUE)
    {
      return true;
    }
  }
  return false;
}

/*===========================================================================
  FUNCTION IsStaCfgUpdated
==========================================================================*/
/*!
@brief
  Check is current applied Sta config and the previous are the same.

@parameters
   none

@return
   returns succesful if prev and current sta config are different

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::IsStaCfgUpdated(void)
{
   if ( (memcmp(&this->prev_station_mode_config.static_ip_config,&this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].static_ip_config,sizeof(qcmap_msgr_sta_static_ip_config_v01))) ||
         (this->prev_station_mode_config.conn_type != this->cfg.lan_config.interface[QCMAP_MSGR_INTF_STATION_INDEX].conn_type) ||
         (this->prev_ap_sta_bridge_mode != this->cfg.lan_config.ap_sta_bridge_mode)  )
   {
      return true;
   }

   return false;
}


/*===========================================================================
  FUNCTION UpdateAccessProfileRules
==========================================================================*/
/*!
@brief
  Updates Ebtables rules for changes in access profile.

@parameters
   none

@return
   returns succesful if prev and current sta config are different

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::UpdateAccessProfileRules(void)
{
  char command[MAX_COMMAND_STR_LEN];

  if (this->prev_guest_profile == QCMAP_MSGR_PROFILE_INTERNET_ONLY_V01)
  {
      /* Enable InterBSS in WLAN Driver.. */
      snprintf( command, MAX_COMMAND_STR_LEN,
                "wmiconfig -i wlan0 --setinterbss 1");
      ds_system_call( command, strlen(command));

      this->DeleteGuestAPAccessRules();
   }
   else if (this->prev_guest_profile == QCMAP_MSGR_PROFILE_FULL_ACCESS_V01)
   {
      /* Disable InterBSS in WLAN Driver.. */
      snprintf( command, MAX_COMMAND_STR_LEN,
                "wmiconfig -i wlan0 --setinterbss 0");
      ds_system_call( command, strlen(command));

      this->InstallGuestAPAccessRules();
  }
}

/*===========================================================================
  FUNCTION InstallGuestAPAccessRules
==========================================================================*/
/*!
@brief
  Install Ebtables/iptables rules for Guest AP access.

@parameters
   none

@return
   none

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::InstallGuestAPAccessRules(void)
{
  char command[MAX_COMMAND_STR_LEN];
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  char a5_ip[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  char netmask[QCMAP_LAN_MAX_IPV4_ADDR_SIZE];
  struct in_addr addr;
  char prefix_v6[MAX_IPV6_PREFIX + 1];
  struct in6_addr *prefix_ptr = NULL;
  qcmap_cm_nl_prefix_info_t* ipv6_prefix_info_ptr = &this->ipv6_prefix_info;
  FILE *access_fd = NULL;

  memset(prefix_v6, 0, MAX_IPV6_PREFIX + 1);
  prefix_ptr = &((struct sockaddr_in6 *)&(ipv6_prefix_info_ptr->prefix_addr))->sin6_addr;
  inet_ntop(AF_INET6, (struct in6_addr *)prefix_ptr->s6_addr, prefix_v6, MAX_IPV6_PREFIX );

  addr.s_addr = htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].a5_ip_addr);
  strlcpy(a5_ip, inet_ntoa(addr), 16);
  addr.s_addr = htonl(this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].sub_net_mask);
  strlcpy(netmask, inet_ntoa(addr), 16);
  access_fd = fopen(QCMAP_MULTIPLE_SYSTEM_CALL_FILE,"w");
  /* File could not be opened for writing/append*/
  if (access_fd == NULL)
  {
     LOG_MSG_ERROR("Cannot open %s file for writing",QCMAP_MULTIPLE_SYSTEM_CALL_FILE,0,0);
     return;
  }
  fprintf( access_fd,"#!/bin/sh \n");

  /* ----------------------------------------------------- */
  /* ------------------ IPv4 Rules ----------------------- */
  /* ----------------------------------------------------- */
  /* Rules to allow DNS/DHCP and block the rest of the traffic from Guest AP Client to A5/local subnet*/
  fprintf( access_fd,"ebtables -I INPUT -i wlan%d -p IPv4 --ip-proto udp --ip-dport 53 -j ACCEPT \n",
           this->ap_dev_num+1);
  fprintf( access_fd, "ebtables -I INPUT -i wlan%d -p IPv4 --ip-proto tcp --ip-dport 53 -j ACCEPT \n",
           this->ap_dev_num+1);
  fprintf( access_fd, "ebtables -I INPUT -i wlan%d -p IPv4 --ip-proto udp --ip-dport 67 -j ACCEPT \n",
           this->ap_dev_num+1);
  fprintf( access_fd, "ebtables -A INPUT -i wlan%d -p IPv4 --ip-destination %s/%s -j DROP \n",
           this->ap_dev_num+1,a5_ip,netmask);

  /* Rules to allow DNS/DHCP and block the rest of the traffic from A5/local subnet to Guest AP Client*/
  fprintf( access_fd, "ebtables -I OUTPUT -o wlan%d -p IPv4 --ip-proto udp --ip-sport 53 -j ACCEPT \n",
           this->ap_dev_num+1);
  fprintf( access_fd, "ebtables -I OUTPUT -o wlan%d -p IPv4 --ip-proto tcp --ip-sport 53 -j ACCEPT \n",
           this->ap_dev_num+1);
  fprintf( access_fd, "ebtables -I OUTPUT -o wlan%d -p IPv4 --ip-proto udp --ip-sport 67 -j ACCEPT \n",
           this->ap_dev_num+1);
  fprintf( access_fd, "ebtables -A OUTPUT -o wlan%d -p IPv4 --ip-source %s/%s -j DROP \n",
           this->ap_dev_num+1,a5_ip,netmask);


  /* ----------------------------------------------------- */
  /* ------------------ IPv6 Rules ----------------------- */
  /* ----------------------------------------------------- */
  /* Rule to allow ICMP/DNS messages and block rest of the traffic to A5 or local LAN*/
  fprintf( access_fd, "ebtables -I INPUT -i wlan%d -p IPv6 --ip6-proto ipv6-icmp --ip6-icmp-type 133:136/0 -j ACCEPT \n",
           this->ap_dev_num+1);
  fprintf( access_fd, "ebtables -I INPUT -i wlan%d -p IPv6 --ip6-proto udp --ip6-dport 53 -j ACCEPT \n",
           this->ap_dev_num+1);
  fprintf( access_fd, "ebtables -I INPUT -i wlan%d -p IPv6 --ip6-proto tcp --ip6-dport 53 -j ACCEPT \n",
           this->ap_dev_num+1);
  fprintf( access_fd, "ebtables -A INPUT -i wlan%d -p IPv6 --ip6-destination fe80::/64 -j DROP \n",
           this->ap_dev_num+1);
  if (this->ipv6_prefix_info.prefix_info_valid)    // If a global IPv6 prefix is available, also add prefix based rules.
  {
    fprintf( access_fd, "ebtables -A INPUT -i wlan%d -p IPv6 --ip6-destination %s/%d -j DROP \n",
             this->ap_dev_num+1,prefix_v6,ipv6_prefix_info_ptr->prefix_len);
    // Set the flag to indicate ipv6 prefix based rules are added
    this->ipv6_prefix_based_rules_added = true;
  }

  /* Rule to Drop traffic routed from local subnet to guest AP. Allow DNS/ICMP responses*/
  fprintf( access_fd, "ebtables -I OUTPUT -o wlan%d -p IPv6 --ip6-proto ipv6-icmp --ip6-icmp-type 133:136/0 -j ACCEPT \n",
           this->ap_dev_num+1);
  fprintf( access_fd, "ebtables -I OUTPUT -o wlan%d -p IPv6 --ip6-proto udp --ip6-sport 53 -j ACCEPT \n",
           this->ap_dev_num+1);
  fprintf( access_fd, "ebtables -I OUTPUT -o wlan%d -p IPv6 --ip6-proto tcp --ip6-sport 53 -j ACCEPT \n",
           this->ap_dev_num+1);
  fprintf( access_fd, "ebtables -A OUTPUT -o wlan%d -p IPv6 --ip6-source fe80::/64 -j DROP \n",
           this->ap_dev_num+1);
  if (ipv6_prefix_info.prefix_info_valid)    // If a global IPv6 prefix is available, also add prefix based rules.
  {
    fprintf( access_fd, "ebtables -A OUTPUT -o wlan%d -p IPv6 --ip6-source %s/%d -j DROP \n",
             this->ap_dev_num+1,prefix_v6,ipv6_prefix_info_ptr->prefix_len);
  }

  /* Rules to drop briged traffic to/from LAN from/to guest AP*/
  fprintf( access_fd, "ebtables -A FORWARD -o wlan%d -j DROP \n",
           this->ap_dev_num+1);
  fprintf( access_fd, "ebtables -A FORWARD -i wlan%d -j DROP \n",
           this->ap_dev_num+1);

  fclose(access_fd);

  /* Changing permissions */
  snprintf( command, MAX_COMMAND_STR_LEN,
            "chmod 777 %s",QCMAP_MULTIPLE_SYSTEM_CALL_FILE);
  ds_system_call( command, strlen(command));
  snprintf( command, MAX_COMMAND_STR_LEN,QCMAP_MULTIPLE_SYSTEM_CALL_FILE);
  ds_system_call( command, strlen(command));
  snprintf( command, MAX_COMMAND_STR_LEN,
            "rm -f %s",QCMAP_MULTIPLE_SYSTEM_CALL_FILE);
  ds_system_call( command, strlen(command));

}

/*===========================================================================
  FUNCTION DeleteGuestAPAccessRules
==========================================================================*/
/*!
@brief
  Delete Ebtables/iptables rules for Guest AP access.

@parameters
   none

@return
   none

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::DeleteGuestAPAccessRules(void)
{
  char command[MAX_COMMAND_STR_LEN];
  /* Flush complete Ebtables  */
  snprintf( command, MAX_COMMAND_STR_LEN,
             "ebtables -t filter --flush");
  ds_system_call( command, strlen(command));
  this->ipv6_prefix_based_rules_added = false;
}

/*===========================================================================
  FUNCTION SetQCMAPBootupConfig
==========================================================================*/
/*!
@brief
  Set QCMAP Bootup Configuration

@parameters
   none

@return
   returns succesful

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetQCMAPBootupConfig(qcmap_msgr_bootup_flag_v01 mobileap_enable, qcmap_msgr_bootup_flag_v01 wlan_enable, qmi_error_type_v01 *qmi_err_num)
{
   LOG_MSG_INFO1("QCMAP_ConnectionManager::SetQCMAPBootupConfig() Enter  mobileap_enable =%d, wlan_enable =%d",mobileap_enable, wlan_enable,0);

  // Write to MobileAP Bootup Flag
  if (mobileap_enable != QCMAP_MSGR_BOOTUP_FLAG_MIN_ENUM_VAL_V01) {
      this->cfg.bootup_config.enable_mobileap_at_bootup = (mobileap_enable == QCMAP_MSGR_ENABLE_ON_BOOT_V01)? true : false;
  }

  // Write to WLAN Bootup Flag
  if (wlan_enable != QCMAP_MSGR_BOOTUP_FLAG_MIN_ENUM_VAL_V01) {
      this->cfg.bootup_config.enable_wlan_at_bootup = (wlan_enable == QCMAP_MSGR_ENABLE_ON_BOOT_V01)? true : false;
  }
  this->WriteConfigToXML();
  return true;
}


/*===========================================================================
  FUNCTION GetQCMAPBootupConfig
==========================================================================*/
/*!
@brief
  Get QCMAP Bootup Configuration

@parameters
   none

@return
   returns succesful

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetQCMAPBootupConfig(qcmap_msgr_bootup_flag_v01 *mobileap_enable, qcmap_msgr_bootup_flag_v01 *wlan_enable, qmi_error_type_v01 *qmi_err_num)
{
   LOG_MSG_INFO1("QCMAP_ConnectionManager::SetQCMAPBootupConfig() Enter  CFG mobileap_enable =%d, wlan_enable =%d",this->cfg.bootup_config.enable_mobileap_at_bootup, this->cfg.bootup_config.enable_wlan_at_bootup,0);

   *mobileap_enable = this->cfg.bootup_config.enable_mobileap_at_bootup;
   *wlan_enable= this->cfg.bootup_config.enable_wlan_at_bootup;
   return true;

}

/*===========================================================================
  FUNCTION GetDataBitrate(void)
==========================================================================*/
/*!
@brief
  Gets the current channel rate from dsi_netctrl

@parameters
   none

@return
   returns succesful

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetDataBitrate(
                                   qcmap_msgr_data_bitrate_v01 *data_rate,
                                   qmi_error_type_v01 *qmi_err_num)
{
  int ret_val;

  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Get Data Bitrate */
  ret_val = qcmap_cm_get_data_bitrate(this->qcmap_cm_handle,
                                      data_rate,
                                      qmi_err_num);
  if ( ret_val != QCMAP_CM_SUCCESS )
    return false;

  return true;
}

/*===========================================================================
  FUNCTION SetUPNPNotifyInterval
==========================================================================*/
/*!
@brief
  Sets the UPnP notify interval.

@parameters
  char *media_dir

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::SetUPNPNotifyInterval(int notify_int, qmi_error_type_v01 *qmi_err_num)
{
  int ret;
  char tmp[MAX_CMD_SIZE];
  char bef[] = "sed -i \"62 anotify_interval=";
  char aft[] = "\" /etc/miniupnpd/miniupnpd.conf";
  char *token, *ptr;

  if (notify_int < MIN_NOTIFY_INTERVAL || notify_int > MAX_NOTIFY_INTERVAL)
  {
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
    return false;
  }

  ds_system_call("sed -i \"/notify_interval=/d\" /etc/miniupnpd/miniupnpd.conf",
                 strlen("sed -i \"/notify_interval=/d\" /etc/miniupnpd/miniupnpd.conf"));

  snprintf(tmp, sizeof(tmp), "%s%d%s", bef, notify_int, aft);
  ds_system_call(tmp, strlen(tmp));

  return true;
}


/*===========================================================================
  FUNCTION GetUPNPNotifyInterval
==========================================================================*/
/*!
@brief
  Gets the UPnP notify interval.

@parameters
  char *media_dir

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::GetUPNPNotifyInterval(int *notify_int, qmi_error_type_v01 *qmi_err_num)
{
  FILE *file = NULL;
  char temp[MAX_SCAN_SIZE] = "";
  int i;

  file = fopen("/etc/miniupnpd/miniupnpd.conf", "rt");

  if (file == NULL)
  {
    LOG_MSG_ERROR("GetUPNPNotifyInterval unable to find /etc/miniupnpd/miniupnpd.conf!", 0, 0, 0);
    *qmi_err_num = QMI_ERR_INSUFFICIENT_RESOURCES_V01;
    return false;
  }

  while (fgets(temp, MAX_SCAN_SIZE, file) != NULL)
  {
    if (strstr(temp, "notify_interval=") != NULL && temp[0] != '#')
    {
      *notify_int = atoi(&temp[16]);
      fclose(file);
      return true;
    }
  }

  fclose(file);
  *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
  return false;
}

/*===========================================================================
  FUNCTION SetDLNANotifyInterval
==========================================================================*/
/*!
@brief
  Sets the DLNA notify interval.

@parameters
  char *media_dir

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::SetDLNANotifyInterval(int notify_int, qmi_error_type_v01 *qmi_err_num)
{
  int ret;
  char tmp[MAX_CMD_SIZE];
  char bef[] = "sed -i \"50 anotify_interval=";
  char aft[] = "\" /etc/minidlna.conf";
  char *token, *ptr;

  if (notify_int < MIN_NOTIFY_INTERVAL || notify_int > MAX_NOTIFY_INTERVAL)
  {
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
    return false;
  }

  ds_system_call("sed -i \"/notify_interval=/d\" /etc/minidlna.conf",
                 strlen("sed -i \"/notify_interval=/d\" /etc/minidlna.conf"));

  snprintf(tmp, sizeof(tmp), "%s%d%s", bef, notify_int, aft);
  ds_system_call(tmp, strlen(tmp));

  return true;
}


/*===========================================================================
  FUNCTION GetDLNANotifyInterval
==========================================================================*/
/*!
@brief
  Gets the DLNA notify interval.

@parameters
  char *media_dir

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::GetDLNANotifyInterval(int *notify_int, qmi_error_type_v01 *qmi_err_num)
{
  FILE *file = NULL;
  char temp[MAX_SCAN_SIZE] = "";
  int i;

  file = fopen("/etc/minidlna.conf", "rt");

  if (file == NULL)
  {
    LOG_MSG_ERROR("GetUPNPNotifyInterval unable to find /etc/minidlna.conf!", 0, 0, 0);
    *qmi_err_num = QMI_ERR_INSUFFICIENT_RESOURCES_V01;
    return false;
  }

  while (fgets(temp, MAX_SCAN_SIZE, file) != NULL)
  {
    if (strstr(temp, "notify_interval=") != NULL && temp[0] != '#')
    {
      *notify_int = atoi(&temp[16]);
      fclose(file);
      return true;
    }
  }

  fclose(file);
  *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
  return false;
}


/*===========================================================================
  FUNCTION IsDLNAKilled
==========================================================================*/
/*!
@brief
  Waits 1 second for the process to die.

@parameters
  char *process

@return
  true  - process killed
  flase - process running after waiting 1 second

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::IsDLNAKilled()
{
  int i = 0;
  FILE *cmd=NULL;
  char pid_s[MAX_CMD_SIZE];
  int pid = 0;
  const char process[] = "pidof minidlnad";

  while (i++ < 5)
  {
    usleep(200000);
    cmd = popen(process, "r");
    pid = 0;
    memset(pid_s, 0, MAX_CMD_SIZE);
    if(cmd)
    {
      fgets(pid_s, MAX_CMD_SIZE, cmd);
      pclose(cmd);
    }
    pid = atoi(pid_s);
    if(pid == 0)
    {
      return true;
    }
  }

  LOG_MSG_ERROR("PID still running after waiting 1 second\n", 0, 0, 0);
  return false;
}

/*===========================================================================
  FUNCTION AddDHCPReservRecord
==========================================================================*/
/*!
@brief
  Checks existing DHCP Reservation Records for the redundancy and if unique
  adds a new DHCP Reservation Record.

@parameters
  qcmap_msgr_dhcp_reservation_v01* dhcp_reserv_record

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::AddDHCPReservRecord
( qcmap_msgr_dhcp_reservation_v01* dhcp_reserv_record,
  qmi_error_type_v01 *qmi_err_num
)
{
  int i;
  in_addr addr;
  boolean mac_addr_non_empty = true, client_ip_match = false;
  int  client_mac_match = 0, client_name_match = 0;
  qcmap_msgr_dhcp_reservation_v01 *local_rec = NULL;
  uint32 num_records = 0;

  QCMAP_CM_LOG_FUNC_ENTRY();

  mac_addr_non_empty = check_non_empty_mac_addr(dhcp_reserv_record->client_mac_addr);
  num_records = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].num_dhcp_reservation_records;
  /*check for duplicate entries*/
  for ( i = 0; i < num_records; i++ )
  {
    local_rec = &this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_reservation_records[i];
    client_ip_match = ( dhcp_reserv_record->client_reserved_ip == local_rec->client_reserved_ip );
    client_mac_match = memcmp(dhcp_reserv_record->client_mac_addr,local_rec->client_mac_addr,\
                              sizeof(dhcp_reserv_record->client_mac_addr));
    client_name_match = strncmp(dhcp_reserv_record->client_device_name,\
                                local_rec->client_device_name,\
                                strlen(dhcp_reserv_record->client_device_name));

    if ( ( ( 0 == client_mac_match  )&& mac_addr_non_empty ) ||
         ( ( 0 == client_name_match )&& dhcp_reserv_record->client_device_name[0] != '\0' )||
         ( client_ip_match ) )
    {
      /* we already added this one */
      LOG_MSG_ERROR("\n IP Address already reserved for another Client!!",0,0,0);
      *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
      return false;
    }
  }
   /* Check if we have already reached a max limit on the records*/
  if ( num_records  == QCMAP_MSGR_MAX_DHCP_RESERVATION_ENTRIES_V01 )
  {
    /* we reach MAX entries */
    LOG_MSG_ERROR("\nReached Max limit on DHCP Reservation Record Entries!!",0,0,0);
    *qmi_err_num = QMI_ERR_INSUFFICIENT_RESOURCES_V01;
    return false;
  }
  /* if  <ipaddr>  is gievn outside the AP dhcp-range , throw an error */
  if ( ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_start_address >
                   dhcp_reserv_record->client_reserved_ip) ||
        ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_end_address <
                  dhcp_reserv_record->client_reserved_ip) )
  {
      LOG_MSG_ERROR("\nIP address is outside the dhcp range!!",0,0,0);
      *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
      return false;
  }

  /* if DHCP enable flag is FALSE for a record during addition, then the corresponding
     record would be not added in the dhcp-hosts file.
     however, we would be still adding the record in the XML */
  if ( dhcp_reserv_record->enable_reservation )
  {
   /* If we have reached this far, then check if /etc/dhcp_hosts file exists.
     if it does not, then create a file and add the following per line
     for AP clients:
     [<hwaddr>] [,<hostname>] [,<ipaddr>]
     for USB client:
     [,<hostname>] [,<ipaddr>]
   */
   dhcp_hosts_fp = fopen(DHCP_HOSTS_FILE, "ab");

   /* File could not be opened for writing/append*/
   if (NULL == dhcp_hosts_fp)
   {
      LOG_MSG_ERROR("\nUnable to open /etc/dhcp_hosts file!!",0,0,0);
      *qmi_err_num = QMI_ERR_INTERNAL_V01;
      return false;
   }
   else
   {
     if ( dhcp_reserv_record->client_device_name[0] != '\0' )
     {
         fprintf(dhcp_hosts_fp,"%s,",dhcp_reserv_record->client_device_name);
     }
     /* mac address is not provided for USB client*/
     if ( mac_addr_non_empty )
     {
       fprintf(dhcp_hosts_fp,"%s,",mac_addr_string);
     }
     addr.s_addr = htonl(dhcp_reserv_record->client_reserved_ip);
     fprintf(dhcp_hosts_fp,"%s\n",inet_ntoa(addr));
     fclose(dhcp_hosts_fp);
     this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_reservations_updated = TRUE;
   }
  }
  /* save into the config */
  memcpy(&this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
         dhcp_reservation_records[num_records],dhcp_reserv_record,\
         sizeof(qcmap_msgr_dhcp_reservation_v01 ));
  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].num_dhcp_reservation_records++;

  //LOG_MSG_INFO1("\n Writing the DHCP record to XML config!!",0,0,0);
  /* if it is OK, call write to XML */
  this->WriteConfigToXML();
  return true;
}


/*===========================================================================
  FUNCTION GetDHCPReservRecords
==========================================================================*/
/*!
@brief
  Displays all the DHCP Reservation Records.

@parameters
  qcmap_msgr_dhcp_reservation_v01* dhcp_reserv_records

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetDHCPReservRecords
( qcmap_msgr_dhcp_reservation_v01* dhcp_reservation_record,
  unsigned int* num_entries,
  qmi_error_type_v01 *qmi_err_num
)
{
  int i;

  QCMAP_CM_LOG_FUNC_ENTRY();

  *num_entries = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].num_dhcp_reservation_records;
  for ( i=0; i < *num_entries; i++ )
  {
    memcpy(&dhcp_reservation_record[i], &this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
           dhcp_reservation_records[i],sizeof(qcmap_msgr_dhcp_reservation_v01));
  }
  return true;
}


/*===========================================================================
  FUNCTION EditDHCPReservRecord
==========================================================================*/
/*!
@brief
  Edit a DHCP record

@parameters
  qcmap_msgr_dhcp_reservation_v01* dhcp_reserv_record

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::EditDHCPReservRecord
( uint32_t *client_ip,
  qcmap_msgr_dhcp_reservation_v01 *record,
  qmi_error_type_v01 *qmi_err_num
)
{
  int i,j;
  in_addr addr;
  boolean mac_addr_non_empty = true;
  boolean match_result = false, client_ip_match = false;
  uint32 num_records = 0;
  int  client_mac_match = 0, client_name_match = 0;
  qcmap_msgr_dhcp_reservation_v01 *local_rec = NULL;

  QCMAP_CM_LOG_FUNC_ENTRY();

  /*check if record is existing*/
  num_records = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].num_dhcp_reservation_records;
  for ( i = 0; i < num_records; i++ )
  {
    if (  *client_ip == this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
          dhcp_reservation_records[i].client_reserved_ip )
      break;
  }
  if ( i == num_records )
  {
      /* Record not found */
      LOG_MSG_ERROR("\nDHCP record not found!!",0,0,0);
      *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
      return false;
  }

  /* if  updated <ipaddr>  is gievn outside the AP dhcp-range , throw an error */
  if ( record->client_reserved_ip != 0 )
  {
    if ( ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_start_address >
                   record->client_reserved_ip) ||
        ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_end_address <
                  record->client_reserved_ip) )
    {
      LOG_MSG_ERROR("\nIP address is outside the dhcp range!!",0,0,0);
      *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
      return false;
    }
  }
  /*Check if the mac address is not entered by the user
  If its empty then we have don't have edit the mac address
  for AP client*/
  mac_addr_non_empty = check_non_empty_mac_addr(record->client_mac_addr);

  /*mac address should not be edited for USB clients.Return error*/
  if ( ( check_non_empty_mac_addr( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                                   dhcp_reservation_records[i].client_mac_addr ) == false ) && \
                                 ( mac_addr_non_empty == true ) )
  {
      LOG_MSG_ERROR("\nMAC address cannot be entered for USB client!!",0,0,0);
      *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
      return false;
  }

  /*check for duplicate entries*/
  for ( j = 0; j < num_records; j++ )
  {
    /*skip the record which has been matched already*/
    if ( j == i )
    {
      continue;
    }
    local_rec = &this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].\
                   dhcp_reservation_records[j];
    client_ip_match = ( local_rec->client_reserved_ip == record->client_reserved_ip );
    client_mac_match = memcmp(record->client_mac_addr,local_rec->client_mac_addr,\
                              sizeof(record->client_mac_addr));
    client_name_match = strncmp(record->client_device_name,local_rec->client_device_name,\
                                strlen(record->client_device_name));

    if ( ( ( 0 == client_mac_match )&& mac_addr_non_empty ) ||
         ( ( 0 == client_name_match )&& record->client_device_name[0] != '\0') ||
         client_ip_match )
    {
      /* we already added this one */
      LOG_MSG_ERROR("\n IP Address already reserved for another Client!!",0,0,0);
      *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
      return false;
    }
  }
  local_rec = &this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_reservation_records[i];
   /*replace the existing record with the records that were entered*/
  if ( mac_addr_non_empty )
  {
     memcpy(local_rec->client_mac_addr,record->client_mac_addr,sizeof(local_rec->client_mac_addr));
  }
  if ( record->client_reserved_ip != 0)
  {
     local_rec->client_reserved_ip = record->client_reserved_ip;
  }
  if ( record->client_device_name[0] != '\0')
  {
     memcpy(local_rec->client_device_name,record->client_device_name,sizeof(local_rec->client_device_name));
  }
  if (local_rec->enable_reservation != record->enable_reservation)
  {
     local_rec->enable_reservation = record->enable_reservation;
  }

  /* check if dhcp-host files exists. it could be possible that dhcp-host files
      has been removed from the user space. if thats the case, create the file */
  dhcp_hosts_fp = fopen(DHCP_HOSTS_FILE, "rb+");
  /* File could not be opened for writing/append*/
  if (NULL == dhcp_hosts_fp)
  {
       /*dhcp-hosts files might have been removed from user space.
      creating a new file below*/
      LOG_MSG_INFO1("\nUnable to open /etc/dhcp_hosts file for read!!",0,0,0);
  }
  else
  {
     /* if dhcp-hosts files exits, search for the record based on ip-address.
     if found, delete the same. If not found , it could be possible that
     the entry might have been removed from the user space.
     Go ahead and add the record if dhcp-reservation flag is true*/
     if ( false == ( match_result = find_record_delete_ip(*client_ip)) )
     {
      *qmi_err_num = QMI_ERR_INTERNAL_V01;
      fclose(dhcp_hosts_fp);
      return false;
     }
  }

  if ( match_result == true ||record->enable_reservation == true)
  {
    dhcp_hosts_fp = fopen(DHCP_HOSTS_FILE, "ab");
    /* File could not be opened for writing/append*/
    if (NULL == dhcp_hosts_fp)
    {
        LOG_MSG_ERROR("\nUnable to open /etc/dhcp_hosts file!!",0,0,0);
        *qmi_err_num = QMI_ERR_INTERNAL_V01;
        return false;
    }
    else
    {
       if ( record->enable_reservation == true )
       {
           /* If we have reached this far, then edit as below
             for AP clients:
             [<hwaddr>] [,<hostname>] [,<ipaddr>]
             for USB client:
             [,<hostname>] [,<ipaddr>]
            */
         if ( local_rec->client_device_name[0] != '\0')
         {
           fprintf(dhcp_hosts_fp,"%s,",local_rec->client_device_name);
         }
         /* mac address is not provided for USB client*/
         if ( check_non_empty_mac_addr(local_rec->client_mac_addr))
         {
             fprintf(dhcp_hosts_fp,"%s,",mac_addr_string);
         }
         addr.s_addr = htonl(local_rec->client_reserved_ip);
         fprintf(dhcp_hosts_fp,"%s\n",inet_ntoa(addr));
       }
       fclose(dhcp_hosts_fp);
       this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_reservations_updated = TRUE;
    }
  }

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();
  return true;
}

/*===========================================================================
  FUNCTION DeleteDHCPReservRecord
==========================================================================*/
/*!
@brief
  Delete a DHCP record

@parameters
  qcmap_msgr_dhcp_reservation_v01* dhcp_reserv_record

@return
  true  - on Success
  false - on Failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::DeleteDHCPReservRecord
( uint32_t *client_reserved_ip,
  qmi_error_type_v01 *qmi_err_num
)
{
  int i, j;
  uint32 num_records = 0;

  QCMAP_CM_LOG_FUNC_ENTRY();

  /*check if record exists*/
  num_records = this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].num_dhcp_reservation_records;
  for ( i = 0; i < num_records; i++ )
  {
    if ( this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_reservation_records[i].\
         client_reserved_ip == *client_reserved_ip )
      break;
  }

  if ( i == num_records )
  {
      /* we do not have this one  */
      LOG_MSG_ERROR("\nDHCP record not found!!",0,0,0);
      *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
      return false;
  }

 /* check if dhcp-host files exists. if
      it does not exists its fine. this might have
      been removed from the user space. Return TRUE*/
    dhcp_hosts_fp = fopen(DHCP_HOSTS_FILE, "rb+");
  /* File could not be opened for writing/append*/
  if (NULL == dhcp_hosts_fp)
  {
      LOG_MSG_INFO1("\nFile /etc/dhcp_hosts does not exists!!",0,0,0);
  }
  else
  {
     /* if dhcp-hosts files exits, seach for the record based on ip-address.
     if found, delete the same. If not found , it could be possible that
     the entry might have been already removed from the user space.*/
     if ( false == find_record_delete_ip(*client_reserved_ip) )
     {
      *qmi_err_num = QMI_ERR_INTERNAL_V01;
      fclose(dhcp_hosts_fp);
      return false;
     }
     this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_reservations_updated = TRUE;
  }
  for ( j = i; j < (num_records - 1); j++ )
  {
     memcpy(&this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_reservation_records[j],
            &this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].dhcp_reservation_records[j+1],
            sizeof(qcmap_msgr_dhcp_reservation_v01));
  }

  this->cfg.lan_config.interface[QCMAP_MSGR_INTF_AP_INDEX].num_dhcp_reservation_records--;

  /* if it is OK, call write to XML */
  this->WriteConfigToXML();
  return true;
}


/*===========================================================================
  FUNCTION IsHostapdkilled
==========================================================================*/
/*!
@brief
  Waits 1 second for the process to die.

@parameters
  char *process

@return
  true  - process killed
  flase - process running after waiting 1 second

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::IsHostapdkilled(int omit_pid)
{
  char process[MAX_COMMAND_STR_LEN];
  int i = 0;
  FILE *cmd;
  char pid_s[MAX_CMD_SIZE];
  int pid;

  memset(process, 0, MAX_COMMAND_STR_LEN);

  if ( omit_pid == 0 )
  {
    snprintf(process, MAX_COMMAND_STR_LEN, "pidof hostapd");
  }
  else
  {
    snprintf(process, MAX_COMMAND_STR_LEN, "pidof -o %d hostapd", omit_pid);
  }

  while (i++ < QCMAP_PROCESS_KILL_RETRY)
  {
    usleep(QCMAP_PROCESS_KILL_WAIT_MS);/*50 mini second*/
    cmd = popen(process, "r");
    pid = 0;
    memset(pid_s, 0, MAX_CMD_SIZE);
    if(cmd)
    {
      fgets(pid_s, MAX_CMD_SIZE, cmd);
      pclose(cmd);
    }
    pid = atoi(pid_s);
    /* If pid is zero we break from while*/
    if(pid == 0)
    {
      return true;
    }
  }

  LOG_MSG_ERROR("PID still running after waiting 2 second\n", 0, 0, 0);
  return false;
}

/*===========================================================================
  FUNCTION IsWpaSupplicantkilled
==========================================================================*/
/*!
@brief
  Waits 1 second for the process to die.

@parameters
  char *process

@return
  true  - process killed
  flase - process running after waiting 1 second

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::IsWpaSupplicantkilled()
{
  int i = 0;
  FILE *cmd;
  char pid_s[MAX_CMD_SIZE];
  int pid;
  const char process[] = "pidof wpa_supplicant";

  while (i++ < QCMAP_PROCESS_KILL_RETRY)
  {
    usleep(QCMAP_PROCESS_KILL_WAIT_MS);/*10 mili second*/
    cmd = popen(process, "r");
    pid = 0;
    memset(pid_s, 0, MAX_CMD_SIZE);
    if(cmd)
    {
      fgets(pid_s, MAX_CMD_SIZE, cmd);
      pclose(cmd);
    }
    pid = atoi(pid_s);
    LOG_MSG_INFO1("\n wpa_supplicant pid =%d \n", pid, 0, 0);
    /* If pid is zero we break from while*/
    if(pid == 0)
    {
      return true;
    }
  }

  LOG_MSG_ERROR("PID still running after waiting 2 second\n", 0, 0, 0);
  return false;
}


/*===========================================================================
  FUNCTION IsWpaClikilled
==========================================================================*/
/*!
@brief
  Waits 1 second for the process to die.

@parameters
  char *process

@return
  true  - process killed
  flase - process running after waiting 1 second

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::IsWpaClikilled()
{
  int i = 0;
  FILE *cmd;
  char pid_s[MAX_CMD_SIZE];
  int pid;
  const char process[] = "pidof wpa_cli";

  while (i++ < QCMAP_PROCESS_KILL_RETRY)
  {
    usleep(QCMAP_PROCESS_KILL_WAIT_MS);/*50 mini second*/
    cmd = popen(process, "r");
    pid = 0;
    memset(pid_s, 0, MAX_CMD_SIZE);
    fgets(pid_s, MAX_CMD_SIZE, cmd);
    pid = atoi(pid_s);
    LOG_MSG_INFO1("\n wpa_cli pid =%d \n", pid, 0, 0);
    pclose(cmd);
    /* If pid is zero we break from while*/
    if(pid == 0)
    {
      return true;
    }
  }

  LOG_MSG_ERROR("PID still running after waiting 2 second\n", 0, 0, 0);
  return false;
}

/*===========================================================================
  FUNCTION  AddWWANIPv4SIPServerInfo
==========================================================================*/
/*!
@brief
- retrieves the network assigned PCSCF address list and PCSCF FQDN list
  from modem
- restarts dnsmasq with the network assigned PCSCF address and PCSCF
  FQDN list

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::AddWWANIPv4SIPServerInfo()
{
  dsi_pcscf_addr_info_t pcscf_addr_list;
  dsi_pcscf_fqdn_list_t pcscf_fqdn_list;
  qmi_error_type_v01    qmi_err_num;
  int                   ret_val;
  int                   addr_count;
  int                   fqdn_count;
  struct sockaddr_in    *sin   = NULL;

/*-------------------------------------------------------------------------*/

  QCMAP_CM_LOG_FUNC_ENTRY();

  memset(&pcscf_addr_list, 0, sizeof(dsi_pcscf_addr_info_t));
  memset(&pcscf_fqdn_list, 0, sizeof(dsi_pcscf_fqdn_list_t));

  ret_val = qcmap_cm_get_ipv4_sip_server_info(this->qcmap_cm_handle,
                                              &pcscf_addr_list,
                                              &pcscf_fqdn_list,
                                              &qmi_err_num);
    if (ret_val == QCMAP_CM_SUCCESS)
    {
      LOG_MSG_INFO1("Successfully obtained PCSCF info", 0, 0, 0);

      LOG_MSG_INFO1("Got %d PCSCF addresses", pcscf_addr_list.addr_count, 0, 0);

      for (addr_count = 0; addr_count < pcscf_addr_list.addr_count; addr_count ++)
      {
        if (pcscf_addr_list.pcscf_address[addr_count].valid_addr)
          {
            sin = (struct sockaddr_in *)&pcscf_addr_list.pcscf_address[addr_count].addr;
            LOG_MSG_INFO1("pcscf_ip:0x%x", sin->sin_addr.s_addr, 0, 0);

            this->cfg.wan_config.sip_server_info_list[this->cfg.wan_config.sip_server_list_count].sip_serv_type =
              QCMAP_CM_SIP_SERVER_ADDR;
            this->cfg.wan_config.sip_server_info_list[this->cfg.wan_config.sip_server_list_count].sip_serv_info.sip_serv_ip =
              (uint32)sin->sin_addr.s_addr;
            this->cfg.wan_config.sip_server_list_count++;
        }
      }

      LOG_MSG_INFO1("Successfully got %d PCSCF FQDN", pcscf_fqdn_list.fqdn_count, 0, 0);

      for (fqdn_count = 0; fqdn_count < pcscf_fqdn_list.fqdn_count; fqdn_count ++)
      {
        if (pcscf_fqdn_list.pcscf_domain_list[fqdn_count].fqdn_length > 0)
        {
          this->cfg.wan_config.sip_server_info_list[this->cfg.wan_config.sip_server_list_count].sip_serv_type =
                                                                                       QCMAP_CM_SIP_SERVER_FQDN;
          memcpy
            (this->cfg.wan_config.sip_server_info_list[this->cfg.wan_config.sip_server_list_count].sip_serv_info.sip_fqdn,
             pcscf_fqdn_list.pcscf_domain_list[fqdn_count].fqdn_string,
             pcscf_fqdn_list.pcscf_domain_list[fqdn_count].fqdn_length);
          this->cfg.wan_config.sip_server_list_count++;
        }
      }
      UpdateDHCPDNetworkInfo();
    }
    else
    {
      LOG_MSG_ERROR("Failure in getting PCSCF info %d", qmi_err_num, 0, 0);
      return false;
    }

  return true;
}


/*===========================================================================
  FUNCTION  DeleteWWANIPv4SIPServerInfo
==========================================================================*/
/*!
@brief
- resets the WWAN provided IPv4 SIP server information

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::DeleteWWANIPv4SIPServerInfo()
{

  QCMAP_CM_LOG_FUNC_ENTRY();

  this->cfg.wan_config.sip_server_list_count = 0;
  memset(this->cfg.wan_config.sip_server_info_list,
         0,
         sizeof(this->cfg.wan_config.sip_server_info_list));

  UpdateDHCPDNetworkInfo();

  return true;
}

/*===========================================================================
  FUNCTION EnableAlg
==========================================================================*/
/*!
@brief
  Enables Algs Functionality.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::EnableAlg
(
  qcmap_msgr_alg_type_mask_v01 alg_type,
  qmi_error_type_v01 *qmi_err_num
)
{
  int ret_rtsp, ret_sip;

  QCMAP_CM_LOG_FUNC_ENTRY();

  /* Cant continue if MobileAP is not enabled! */
  if (!this->qcmap_enable)
  {
    LOG_MSG_ERROR("Error: QCMAP not enabled.\n",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    return false;
  }

  if( !this->sta_connected )
  {
    if ( qcmap_cm_get_state(qmi_err_num) != QCMAP_CM_WAN_CONNECTED )
    {
      LOG_MSG_ERROR("IPv4 backhaul down:cannot enable ALG now, but will change cfg",
                     0, 0, 0);
      *qmi_err_num = QMI_ERR_INTERFACE_NOT_FOUND_V01;

      if (alg_type & QCMAP_MSGR_MASK_RTSP_ALG_V01)
      {
        this->cfg.nat_config.enable_alg_mask |= QCMAP_MSGR_MASK_RTSP_ALG_V01 ;
      }
      else if (alg_type & QCMAP_MSGR_MASK_SIP_ALG_V01)
      {
        this->cfg.nat_config.enable_alg_mask |= QCMAP_MSGR_MASK_SIP_ALG_V01;
      }

      this->WriteConfigToXML();
      return false;
    }
  }

  if( (alg_type & QCMAP_MSGR_MASK_RTSP_ALG_V01) ==
       QCMAP_MSGR_MASK_RTSP_ALG_V01 )
  {
    /*-----------------------------------------------------------------------
      Install RTSP ALG Kernel Module
    -----------------------------------------------------------------------*/
    ret_rtsp = EnableRTSPAlg(qmi_err_num);
    if ( ret_rtsp != true )
      {
        LOG_MSG_ERROR("Fail to enable RTSP ALG: = %d.\n",
                     ret_rtsp, 0, 0);
    }
    else
    {
      LOG_MSG_INFO1("Successfully enabled RTSP ALG", 0, 0, 0);
      // Set mask bit for RTSP ALG module enabled
      this->cfg.nat_config.enable_alg_mask |= QCMAP_MSGR_MASK_RTSP_ALG_V01 ;
    }
  }

  if(alg_type & QCMAP_MSGR_MASK_SIP_ALG_V01)
  {
    ret_sip = EnableSIPAlg(qmi_err_num);
    if ( ret_sip != true )
    {
      LOG_MSG_ERROR("Fail to enable SIP ALG, Ret value = %d",
                       ret_sip, 0, 0);
    }
    else
    {
      LOG_MSG_INFO1("Successfully enabled SIP ALG", 0, 0, 0);
      // Set mask for ALG module enabled
      this->cfg.nat_config.enable_alg_mask |= QCMAP_MSGR_MASK_SIP_ALG_V01;
    }
  }

  /* Save the Configuration. */
  this->WriteConfigToXML();

  if((ret_rtsp == false) && (ret_sip == false))
  {
    *qmi_err_num = QMI_ERR_INTERNAL_V01;
    return false;
  }
  else if((ret_rtsp == false)|| (ret_sip == false))
  {
    *qmi_err_num = QMI_ERR_OP_PARTIAL_FAILURE_V01;
    return false;
  }
  else
  {
    return true;
  }
}
/*===========================================================================
  FUNCTION DisableAlg
==========================================================================*/
/*!
@brief
  Disables Algs Functionality.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::DisableAlg
(
  qcmap_msgr_alg_type_mask_v01 alg_type,
  qmi_error_type_v01 *qmi_err_num
)
{
  int ret_rtsp, ret_sip;

  QCMAP_CM_LOG_FUNC_ENTRY();

  if (!this->qcmap_enable)
  {
    /* QCMAP is not enabled */
    LOG_MSG_ERROR("QCMAP not enabled\n",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    return false;
  }

  if( !this->sta_connected )
  {
    if ( qcmap_cm_get_state(qmi_err_num) != QCMAP_CM_WAN_CONNECTED )
    {
      LOG_MSG_ERROR(" IPv4 backhaul down: cannot disable ALG", 0,0,0);
      *qmi_err_num = QMI_ERR_INTERFACE_NOT_FOUND_V01;

      if (alg_type & QCMAP_MSGR_MASK_RTSP_ALG_V01)
      {
        this->cfg.nat_config.enable_alg_mask &= ~QCMAP_MSGR_MASK_RTSP_ALG_V01;
      }
      else if (alg_type & QCMAP_MSGR_MASK_SIP_ALG_V01)
      {
        this->cfg.nat_config.enable_alg_mask &= ~QCMAP_MSGR_MASK_SIP_ALG_V01;
      }
      return false;
    }
  }

  if(alg_type & QCMAP_MSGR_MASK_RTSP_ALG_V01)
  {
     /*-----------------------------------------------------------------------
      Unload the RTSP ALG module from Kernel
    -----------------------------------------------------------------------*/
    ret_rtsp = DisableRTSPAlg(qmi_err_num);
    if ( ret_rtsp != true )
    {
      LOG_MSG_ERROR("Fail to disable RTSP ALG. error = %d.\n",
                     ret_rtsp, 0, 0);
    }
    else
    {
      LOG_MSG_INFO1("Successfully disabled RTSP ALG", 0, 0, 0);
      // Reset mask bit for RTSP ALG module
      this->cfg.nat_config.enable_alg_mask &= ~QCMAP_MSGR_MASK_RTSP_ALG_V01 ;
    }
  }

  if(alg_type & QCMAP_MSGR_MASK_SIP_ALG_V01)
  {
    ret_sip = DisableSIPAlg(qmi_err_num);
    if ( ret_sip != true )
    {
      LOG_MSG_ERROR("Fail to disable SIP ALG, Ret value = %d.\n",
                       ret_sip, 0, 0);
    }
    else
    {
      LOG_MSG_INFO1("Successfully disabled SIP ALG", 0, 0, 0);
      // Reset mask bit for SIP ALG module
      this->cfg.nat_config.enable_alg_mask &= ~QCMAP_MSGR_MASK_SIP_ALG_V01 ;
    }
  }

  /* Save the Configuration. */
  this->WriteConfigToXML();

  if((ret_rtsp == false) && (ret_sip == false))
  {
    *qmi_err_num = QMI_ERR_INTERNAL_V01;
    return false;
  }
  else if((ret_rtsp == false)|| (ret_sip == false))
  {
    *qmi_err_num = QMI_ERR_OP_PARTIAL_FAILURE_V01;
    return false;
  }
  else
  {
    return true;
  }
}
/*===========================================================================
  FUNCTION EnableRTSPAlg
==========================================================================*/
/*!
@brief
  Enables RTSP Alg Functionality.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::EnableRTSPAlg
(
  qmi_error_type_v01 *qmi_err_num
)
{
  int ret = QCMAP_CM_SUCCESS;
  char Kernel_ver[KERNEL_VERSION_LENGTH];
  char command[MAX_COMMAND_STR_LEN];

  if ((this->alg_enabled_disabled_mask & QCMAP_MSGR_MASK_RTSP_ALG_V01) ==
      QCMAP_MSGR_MASK_RTSP_ALG_V01)
  {
    LOG_MSG_ERROR("RTSP ALG is already enabled.\n", 0, 0, 0);
    return false;
  }
  else
  {
    if ( false == this->GetKernelVer(Kernel_ver) )
    {
      LOG_MSG_ERROR("Unable to get the kernel version info", 0, 0, 0);
      return false;
    }
    /*-----------------------------------------------------------------------
      Install RTSP ALG Kernel Module
    -----------------------------------------------------------------------*/
    snprintf(command,MAX_COMMAND_STR_LEN, "insmod /usr/lib/modules/%s/kernel/drivers/net/nf_conntrack_rtsp.ko",Kernel_ver);
    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("%s\n", command,0,0);

    snprintf(command,MAX_COMMAND_STR_LEN, "insmod /usr/lib/modules/%s/kernel/drivers/net/nf_nat_rtsp.ko",Kernel_ver);
    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("%s\n", command,0,0);

    this->alg_enabled_disabled_mask = this->alg_enabled_disabled_mask |
                                      QCMAP_MSGR_MASK_RTSP_ALG_V01;
  }
  return true;
}

/*===========================================================================
  FUNCTION DisableRTSPAlg
==========================================================================*/
/*!
@brief
  Disables RTSP Alg Functionality.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::DisableRTSPAlg
(
  qmi_error_type_v01 *qmi_err_num
)
{
  int ret = QCMAP_CM_SUCCESS;
  char command[MAX_COMMAND_STR_LEN];

  if ((this->alg_enabled_disabled_mask & QCMAP_MSGR_MASK_RTSP_ALG_V01) ==
       QCMAP_MSGR_MASK_RTSP_ALG_V01)
  {
    snprintf(command,MAX_COMMAND_STR_LEN, "rmmod  nf_nat_rtsp");
    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("%s\n", command,0,0);

    snprintf(command,MAX_COMMAND_STR_LEN, "rmmod nf_conntrack_rtsp");
    ds_system_call(command, strlen(command));
    LOG_MSG_INFO1("%s\n", command,0,0);

    this->alg_enabled_disabled_mask = (this->alg_enabled_disabled_mask &
                                     ~(QCMAP_MSGR_MASK_RTSP_ALG_V01));
  }
  else
  {
    LOG_MSG_ERROR("RTSP ALG is already disabled.\n", 0, 0, 0);
    return false;
  }

  return true;
}

/*===========================================================================
  FUNCTION EnableSIPAlg
==========================================================================*/
/*!
@brief
  - Enables SIP Alg Functionality.
  - Writes to a proc/sys entry which indicates netfilter to enable SIP ALG
    processing (corresponding code added in kernel)

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::EnableSIPAlg
(
  qmi_error_type_v01 *qmi_err_num
)
{
  ds_system_call("echo 0 > /proc/sys/net/netfilter/nf_conntrack_disable_sip_alg",
                 strlen("echo 0 > /proc/sys/net/netfilter/nf_conntrack_disable_sip_alg"));

  /*Enable media streaming between non signalling endpoints */
  ds_system_call("echo 0 > /proc/sys/net/netfilter/nf_conntrack_sip_direct_media",
                 strlen("echo 0 > /proc/sys/net/netfilter/nf_conntrack_sip_direct_media"));

  LOG_MSG_INFO1("SIP ALG enabled", 0, 0, 0);

  this->alg_enabled_disabled_mask = this->alg_enabled_disabled_mask |
                                    QCMAP_MSGR_MASK_SIP_ALG_V01;
  return true;
}


/*===========================================================================
  FUNCTION DisableSIPAlg
==========================================================================*/
/*!
@brief
  - Disables SIP Alg Functionality.
  - Writes to a proc/sys entry which indicates netfilter to bypass SIP ALG
    processing (corresponding code added in kernel)

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::DisableSIPAlg
(
  qmi_error_type_v01 *qmi_err_num
)
{
  ds_system_call("echo 1 > /proc/sys/net/netfilter/nf_conntrack_disable_sip_alg",
                 strlen("echo 1 > /proc/sys/net/netfilter/nf_conntrack_disable_sip_alg"));

  /*Disable media streaming between non signalling endpoints */
  ds_system_call("echo 1 > /proc/sys/net/netfilter/nf_conntrack_sip_direct_media",
                 strlen("echo 1 > /proc/sys/net/netfilter/nf_conntrack_sip_direct_media"));

  LOG_MSG_INFO1("SIP ALG disabled", 0, 0, 0);

  this->alg_enabled_disabled_mask = (this->alg_enabled_disabled_mask &
                                     ~(QCMAP_MSGR_MASK_SIP_ALG_V01));
  return true;
}

/*===========================================================================
  FUNCTION SetSIPServerInfo
==========================================================================*/
/*!
@brief
  - Sets the default user configured SIP Server Information onto qcmap config.
  - Restarts DHCP server with the user provided SIP server information.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::SetSIPServerInfo
(
   qcmap_msgr_sip_server_info_v01 default_sip_server_info,
   qmi_error_type_v01 *qmi_err_num
)
{

  this->cfg.wan_config.default_sip_server_info.sip_serv_type =
                                    (uint32)default_sip_server_info.pcscf_info_type;

  if (this->cfg.wan_config.default_sip_server_info.sip_serv_type ==
                                                  QCMAP_CM_SIP_SERVER_ADDR)
  {
    this->cfg.wan_config.default_sip_server_info.sip_serv_info.sip_serv_ip =
                                      default_sip_server_info.pcscf_ip_addr;
  }
  else if (this->cfg.wan_config.default_sip_server_info.sip_serv_type ==
                                                  QCMAP_CM_SIP_SERVER_FQDN)
  {
    strlcpy(this->cfg.wan_config.default_sip_server_info.sip_serv_info.sip_fqdn,
            default_sip_server_info.pcscf_fqdn,
            strlen(default_sip_server_info.pcscf_fqdn));
  }

  LOG_MSG_INFO1("Default SIP server info type %d set",
                this->cfg.wan_config.default_sip_server_info.sip_serv_type,
                0, 0);
  this->WriteConfigToXML();

  UpdateDHCPDNetworkInfo();

  return true;
}

/*===========================================================================
  FUNCTION GetSIPServerInfo
==========================================================================*/
/*!
@brief
  - Retrives default user configured SIP server Information
  - Also retrieves network assigned SIP server information list.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::GetSIPServerInfo
(
   qcmap_msgr_sip_server_info_v01 *default_sip_info,
   qcmap_msgr_sip_server_info_v01 *network_sip_info,
   int *count_network_sip_info,
   qmi_error_type_v01 *qmi_err_num
)
{

  int cnt;

/*-------------------------------------------------------------------------*/

  if (this->cfg.wan_config.default_sip_server_info.sip_serv_type !=
      QCMAP_CM_SIP_SERVER_MIN)
  {
    default_sip_info->pcscf_info_type =
      (uint32)this->cfg.wan_config.default_sip_server_info.sip_serv_type;

    if (this->cfg.wan_config.default_sip_server_info.sip_serv_type ==
                                                    QCMAP_CM_SIP_SERVER_ADDR)
    {
      default_sip_info->pcscf_ip_addr =
        this->cfg.wan_config.default_sip_server_info.sip_serv_info.sip_serv_ip;
    }
    else if (this->cfg.wan_config.default_sip_server_info.sip_serv_type ==
                                                    QCMAP_CM_SIP_SERVER_FQDN)
    {
      strlcpy(default_sip_info->pcscf_fqdn,
              this->cfg.wan_config.default_sip_server_info.sip_serv_info.sip_fqdn,
              strlen(this->cfg.wan_config.default_sip_server_info.sip_serv_info.sip_fqdn));
    }

    LOG_MSG_INFO1("Default SIP server info type %d retrieved",
                  default_sip_info->pcscf_info_type, 0, 0);
  }
  else
  {
    LOG_MSG_INFO1("Default SIP server info not configured", 0, 0, 0);
  }

  if (this->cfg.wan_config.sip_server_list_count > 0)
  {
    LOG_MSG_INFO1("%d network assigned SIP server info present",
                   this->cfg.wan_config.sip_server_list_count, 0, 0);
    *count_network_sip_info = this->cfg.wan_config.sip_server_list_count;

    for (cnt=0; cnt<this->cfg.wan_config.sip_server_list_count; cnt++)
    {
      network_sip_info[cnt].pcscf_info_type =
        (uint32)this->cfg.wan_config.sip_server_info_list[cnt].sip_serv_type;
      if (network_sip_info[cnt].pcscf_info_type == QCMAP_CM_SIP_SERVER_ADDR)
      {
        network_sip_info[cnt].pcscf_ip_addr =
          this->cfg.wan_config.sip_server_info_list[cnt].sip_serv_info.sip_serv_ip;
      }
      else if (network_sip_info[cnt].pcscf_info_type == QCMAP_CM_SIP_SERVER_FQDN)
      {
        strlcpy(network_sip_info[cnt].pcscf_fqdn,
                this->cfg.wan_config.sip_server_info_list[cnt].sip_serv_info.sip_fqdn,
                strlen(this->cfg.wan_config.sip_server_info_list[cnt].sip_serv_info.sip_fqdn));
      }
    }
  }
  else
  {
     LOG_MSG_INFO1("Network SIP server info not available", 0, 0, 0);
  }

  return true;
}
/*===========================================================================
  FUNCTION GetKernelVer
==========================================================================*/
/*!
@brief
  Queries the kernel version.

@parameters
  char *version

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetKernelVer
(
  char *version
)
{
  FILE *fp = NULL;
  char command[MAX_COMMAND_STR_LEN];
  char kernel_ver[KERNEL_VERSION_LENGTH];

  if ( version == NULL )
  {
     LOG_MSG_ERROR("NULL Args.\n", 0,0,0);
     return false;
  }
  memset(kernel_ver, 0, KERNEL_VERSION_LENGTH);

  snprintf( command, MAX_COMMAND_STR_LEN,
            "uname -r | awk '{print $1}' > /etc/kernel_ver.txt");
  ds_system_call(command, strlen(command));
  LOG_MSG_INFO1("%s\n", command,0,0);

  fp = fopen("/etc/kernel_ver.txt", "r");

  if ( fp == NULL )
  {
     LOG_MSG_ERROR("Error opening Kernel version file: %d.\n", errno, 0, 0);
     return false;
  }

  if (fscanf(fp, "%s", kernel_ver) != 1)
  {
    LOG_MSG_ERROR("Error reading Kernel version file: %d.\n", errno, 0, 0);
    fclose(fp);
    return false;
  }
  memcpy(version,kernel_ver,strlen(kernel_ver)+1);
  fclose(fp);
  return true;
}

/*===========================================================================
  FUNCTION DelUsbConntrack
==========================================================================*/
/*!
@brief
  Deletes the conntrack entries associated with USB TE

@parameters
  none

@return
  none

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::DelUsbConntrack(void)
{
  char ipaddr[INET_ADDRSTRLEN];
  char command[MAX_COMMAND_STR_LEN];

  memset(ipaddr, 0, INET_ADDRSTRLEN);
  memset(command, 0, MAX_COMMAND_STR_LEN);

  uint32 usb_ip=GetUSBNeighIP();
  inet_ntop(AF_INET,&usb_ip, (char *)&ipaddr, INET_ADDRSTRLEN);

  LOG_MSG_INFO1("QCMAP_ConnectionManager::DelUsbConntrack(), usb_ip %d",ipaddr,0,0);

  if (usb_ip)
  {
    snprintf(command,MAX_COMMAND_STR_LEN,"echo Deleting USB Client Conntrack IP:%s > /dev/kmsg",(char *)&ipaddr);
    ds_system_call(command,strlen(command));
    snprintf(command,MAX_COMMAND_STR_LEN, "conntrack -D --orig-src %s",(char *)&ipaddr);
    ds_system_call(command, strlen(command));
    snprintf(command,MAX_COMMAND_STR_LEN, "conntrack -D --reply-src %s",(char *)&ipaddr);
    ds_system_call(command, strlen(command));
  }
}

/*===========================================================================
  FUNCTION GetV6SIPServerInfo
==========================================================================*/
/*!
@brief
  -retrieves network assigned SIP server information list.

@return
  true  - on success
  false - on failure

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::GetV6SIPServerInfo
(
   qcmap_msgr_ipv6_sip_server_info_v01 *network_v6_sip_info,
   int *count_network_v6_sip_info,
   qmi_error_type_v01 *qmi_err_num
)
{
  dsi_pcscf_addr_info_t pcscf_addr_list;
  dsi_pcscf_fqdn_list_t pcscf_fqdn_list;
  int                   addr_count, count = 0;
  int                   ret_val = QCMAP_CM_SUCCESS;

  if ( qmi_err_num == NULL )
  {
    LOG_MSG_ERROR("NULL QMI error pointer.\n",0,0,0);
    return false;
  }
  if ( network_v6_sip_info == NULL || count_network_v6_sip_info == NULL )
  {
    LOG_MSG_ERROR("NULL params.\n",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01 ;
    return false;
  }

  memset(&pcscf_addr_list, 0, sizeof(dsi_pcscf_addr_info_t));
  memset(&pcscf_fqdn_list, 0, sizeof(dsi_pcscf_fqdn_list_t));
  *count_network_v6_sip_info = 0;

/*-------------------------------------------------------------------------*/

  QCMAP_CM_LOG_FUNC_ENTRY();

  ret_val = qcmap_cm_get_ipv6_sip_server_info(this->qcmap_cm_handle,
                                              &pcscf_addr_list,
                                              &pcscf_fqdn_list,
                                              qmi_err_num);

  LOG_MSG_INFO1("qcmap_cm_get_ipv6_sip_server_info: pcscf_addr, rval:%d",
                 ret_val, 0, 0);
  if(ret_val != QCMAP_CM_SUCCESS)
  {
    LOG_MSG_ERROR("Couldn't get pcscf ipv6  address. rval %d",
                  ret_val, 0, 0);
    return false;
  }
  else
  {
    LOG_MSG_INFO1("Got %dpcscf %dFQDN IPv6 Address, rval:%d", pcscf_addr_list.addr_count,
                   pcscf_fqdn_list.fqdn_count, ret_val);
  }

  // store valid IPV6 pcscf address recieved
  for (addr_count = 0; addr_count < pcscf_addr_list.addr_count; addr_count ++)
  {
    if (pcscf_addr_list.pcscf_address[addr_count].valid_addr)
    {
      if (SASTORAGE_FAMILY(pcscf_addr_list.pcscf_address[addr_count].addr) == AF_INET6)
      {
        memcpy(network_v6_sip_info[count].pcscf_ipv6_addr ,
               SASTORAGE_DATA(pcscf_addr_list.pcscf_address[addr_count].addr),
               QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8));
        network_v6_sip_info[count].pcscf_info_type =
                   (qcmap_msgr_pcscf_info_type_v01)QCMAP_CM_SIP_SERVER_ADDR;
        count++;
      }
      else
      {
        LOG_MSG_INFO1("Network IPV6 SIP server info not available", 0, 0, 0);
      }
    }
    else
    {
      LOG_MSG_INFO1("Network IPV6 SIP server %d info not valid", addr_count, 0, 0);
    }
  }

  // store valid pcscf fqdn list rcieved
  for (addr_count = 0; addr_count < pcscf_fqdn_list.fqdn_count; addr_count ++)
  {
    if (pcscf_fqdn_list.pcscf_domain_list[addr_count].fqdn_length > 0)
    {
      memcpy(network_v6_sip_info[count].pcscf_fqdn,
             pcscf_fqdn_list.pcscf_domain_list[addr_count].fqdn_string,
             pcscf_fqdn_list.pcscf_domain_list[addr_count].fqdn_length);
      network_v6_sip_info[count].pcscf_info_type =
               (qcmap_msgr_pcscf_info_type_v01)QCMAP_CM_SIP_SERVER_FQDN;
      count++;
    }
    else
    {
      LOG_MSG_INFO1("Network %d IPV6 SIP server FQDN not available", addr_count, 0, 0);
    }
  }
  *count_network_v6_sip_info = count;
  LOG_MSG_INFO1("Network %d IPV6 SIP server  available", count, 0, 0);
  return true;
}


/*===========================================================================
  FUNCTION RestoreFactoryConfig
==========================================================================*/
/*!
@brief
  Restore to Factory Configuration

@parameters
   none

@return
   returns succesful
/*=========================================================================*/
boolean QCMAP_ConnectionManager::RestoreFactoryConfig(qmi_error_type_v01 *qmi_err_num)
{
  char command[MAX_COMMAND_STR_LEN];

  bzero(&command,MAX_COMMAND_STR_LEN);
  LOG_MSG_INFO1("QCMAP_ConnectionManager::RestoreFactoryConfig()",0,0,0);
  snprintf( command, MAX_COMMAND_STR_LEN,
            "cp %s %s ",FACTORY_CONF,this->xml_path);
  ds_system_call(command, strlen(command));

  /* sleep for 500 miliseconds */
  sleep(QCMAP_RESET_CONFIG_TIMEOUT);
  snprintf( command, MAX_COMMAND_STR_LEN,
            "reboot",strlen("reboot"));
  ds_system_call(command, strlen(command));

  return true;
}

/*===========================================================================
  FUNCTION GetODUMode
==========================================================================*/
/*!
@brief
  Gets the ODU mode.

@parameters
  uint8_t *status

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::GetODUMode(qcmap_msgr_odu_mode_v01 *status, qmi_error_type_v01 *qmi_err_num)
{
  int mode;

  if (qcmap_cm_read_odu_xml(IPA_XML_PATH, &mode) == QCMAP_CM_SUCCESS){

    if (mode == 0)
       *status = QCMAP_MSGR_ODU_BRIDGE_MODE_V01;
    else
       *status = QCMAP_MSGR_ODU_ROUTER_MODE_V01;

    LOG_MSG_INFO1("GetODUMode %d %d", mode, *status,0);
    return true;
  }

  *qmi_err_num = QMI_ERR_INTERNAL_V01;
  return false;
}

/*===========================================================================
  FUNCTION SetODUMode
==========================================================================*/
/*!
@brief
  Sets the ODU mode.

@parameters
  uint8_t status

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::SetODUMode(qcmap_msgr_odu_mode_v01 status, qmi_error_type_v01 *qmi_err_num)
{
  FILE *fp;

  if((fp = fopen(ODU_NET_CARRIER, "r")) == NULL){
      return false;
  }
  fclose(fp);

  if (status == QCMAP_MSGR_ODU_BRIDGE_MODE_V01)
  {
     //set bridge mode
     ds_system_call("sed -i \"s/<Mode>router/<Mode>bridge/g\" /etc/IPACM_cfg.xml",
                    strlen("sed -i \"s/<Mode>router/<Mode>bridge/g\" /etc/IPACM_cfg.xml"));
     LOG_MSG_INFO1("Changing to Bridge mode", 0,0,0);
  }
  else //default mode
  {
     //set router mode
     ds_system_call("sed -i \"s/<Mode>bridge/<Mode>router/g\" /etc/IPACM_cfg.xml",
                    strlen("sed -i \"s/<Mode>bridge/<Mode>router/g\" /etc/IPACM_cfg.xml"));
     LOG_MSG_INFO1("Changing to Router mode", 0,0,0);
  }

  return true;
}

/*===========================================================================
 FUNCTION AddEbtablesRuleForBridgeMode
==========================================================================*/
/*!
@brief
  - Match the MAC address to find the node
  - If a match is found, add the default Ebtables rules for various interfaces

@parameters
  Pointer to qcmap_cm_client_data_info_t
@return
  none
@note
  - Dependencies
  - None
  - Side Effects
  - None
*/
/*=========================================================================*/
bool QCMAP_ConnectionManager::AddEbtablesRuleForBridgeMode(qcmap_cm_client_data_info_t* data)
{
  in_addr addr;
  char usb_intf_name [DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  char command[MAX_COMMAND_STR_LEN];
  LOG_MSG_INFO1("QCMAP_ConnectionManager::AddEbtablesRuleForBridgeMode()",0,0,0);
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];

  if ( data == NULL)
  {
    LOG_MSG_ERROR("AddEbtablesRuleForBridgeMode - NULL pointer passed \n",0,0,0);
    return false;
  }

  addr.s_addr = data->ip_addr;
  ds_log_med(" AddEbtablesRuleForBridgeMode for IP address %s \n",inet_ntoa(addr));
  /* If the Update for IP address is for Primary AP or USB TE*
   * Update Ebtables WLAN-STA side for ARP-REPLY target for this entry
   * Update Ebtables WLAN-AP, USB side for ARP-ACCEPT target for local LAN*/
  if ((data->device_type == QCMAP_MSGR_DEVICE_TYPE_PRIMARY_AP_V01) ||
      (data->device_type == QCMAP_MSGR_DEVICE_TYPE_GUEST_AP_V01) ||
      (data->device_type == QCMAP_MSGR_DEVICE_TYPE_USB_V01))
  {
    /* WLAN-STA Side Entry */
    snprintf(command, MAX_COMMAND_STR_LEN,
              "ebtables -t nat -A PREROUTING -i %s -p arp --arp-opcode Request --arp-ip-dst %s -j arpreply --arpreply-mac %02x:%02x:%02x:%02x:%02x:%02x --arpreply-target ACCEPT",
               this->cfg.lan_config.sta_interface, inet_ntoa(addr), this->ap_sta_bridge.bridge_mac[0], this->ap_sta_bridge.bridge_mac[1],this->ap_sta_bridge.bridge_mac[2],
               this->ap_sta_bridge.bridge_mac[3], this->ap_sta_bridge.bridge_mac[4], this->ap_sta_bridge.bridge_mac[5]);
    ds_system_call(command, strlen(command));
    /* AP Side Entry */
    snprintf(command, MAX_COMMAND_STR_LEN,
              "ebtables -t nat -I PREROUTING -i wlan%d -p arp --arp-opcode Request --arp-ip-dst %s -j ACCEPT",
               this->ap_dev_num, inet_ntoa(addr));
    ds_system_call(command, strlen(command));

    /* USB Side Entry */
    if ( GetUSBEnable())
    {
      if ( this->cfg.lan_config.usb_conf.link_type == QCMAP_QTI_USB_LINK_RNDIS )
      {
        strlcpy(usb_intf_name, "rndis0",16);
      }
      else if ( this->cfg.lan_config.usb_conf.link_type == QCMAP_QTI_USB_LINK_ECM )
      {
        strlcpy(usb_intf_name, "ecm0",16);
      }
      else
      {
        LOG_MSG_ERROR("Incorrect USB LINK Detected Link Type = %x", this->cfg.lan_config.usb_conf.link_type,0,0);
        return false;
      }
      snprintf(command, MAX_COMMAND_STR_LEN,
                "ebtables -t nat -I PREROUTING -i %s -p arp --arp-opcode Request --arp-ip-dst %s -j ACCEPT",
                 usb_intf_name, inet_ntoa(addr));
      ds_system_call(command, strlen(command));
    }
  }
  return true;
}

/*===========================================================================
 FUNCTION DelEbtablesRuleForBridgeMode
==========================================================================*/
/*!
@brief
  - Match the MAC address to find the node
  - If a match is found, delete the Ebtables rules for various interfaces

@parameters
  Pointer to qcmap_cm_client_data_info_t
@return
  none
@note
 - Dependencies
 - None
 - Side Effects
 - None
*/
/*=========================================================================*/
bool QCMAP_ConnectionManager::DelEbtablesRuleForBridgeMode(qcmap_cm_client_data_info_t* data)
{
  in_addr addr;
  char usb_intf_name [DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  char command[MAX_COMMAND_STR_LEN];
  LOG_MSG_INFO1("QCMAP_ConnectionManager::DelEbtablesRulesForBridgeSingle()",0,0,0);
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];

  if ( data == NULL)
  {
    LOG_MSG_ERROR("DelEbtablesRuleForBridgeMode - NULL pointer passed \n",0,0,0);
    return false;
  }

  addr.s_addr = data->ip_addr;
  /* If the Update for IP address is for Primary AP or USB TE*
   * Update Ebtables WLAN-STA side for ARP-REPLY target for this entry
   * Update Ebtables WLAN-AP, USB side for ARP-ACCEPT target for local LAN
   * Also Delete the ARP-Entry Manually, instead of wating for it to time.
   * This ensures imediate NEWNEIGH Event on re-connection*/
 if ((data->device_type == QCMAP_MSGR_DEVICE_TYPE_PRIMARY_AP_V01) ||
     (data->device_type == QCMAP_MSGR_DEVICE_TYPE_GUEST_AP_V01) ||
     (data->device_type == QCMAP_MSGR_DEVICE_TYPE_USB_V01))
 {
   /* WLAN-STA Side Entry */
   snprintf(command, MAX_COMMAND_STR_LEN,
             "ebtables -t nat -D PREROUTING -i %s -p arp --arp-opcode Request --arp-ip-dst %s -j arpreply --arpreply-mac %02x:%02x:%02x:%02x:%02x:%02x --arpreply-target ACCEPT",
              this->cfg.lan_config.sta_interface, inet_ntoa(addr), this->ap_sta_bridge.bridge_mac[0], this->ap_sta_bridge.bridge_mac[1],this->ap_sta_bridge.bridge_mac[2],
              this->ap_sta_bridge.bridge_mac[3], this->ap_sta_bridge.bridge_mac[4], this->ap_sta_bridge.bridge_mac[5]);
   ds_system_call(command, strlen(command));
   /* AP Side Entry */
   snprintf(command, MAX_COMMAND_STR_LEN,
            "ebtables -t nat -D PREROUTING -i wlan%d -p arp --arp-opcode Request --arp-ip-dst %s -j ACCEPT",
             this->ap_dev_num, inet_ntoa(addr));
   ds_system_call(command, strlen(command));
   /* USB Side Entry */
   if ( GetUSBEnable())
   {
     if ( this->cfg.lan_config.usb_conf.link_type == QCMAP_QTI_USB_LINK_RNDIS )
     {
       strlcpy(usb_intf_name, "rndis0",16);
     }
     else if ( this->cfg.lan_config.usb_conf.link_type == QCMAP_QTI_USB_LINK_ECM )
     {
       strlcpy(usb_intf_name, "ecm0",16);
     }
     else
     {
       LOG_MSG_ERROR("Incorrect USB LINK Detected Link Type = %x", this->cfg.lan_config.usb_conf.link_type,0,0);
       return false;
     }
     snprintf(command, MAX_COMMAND_STR_LEN,
              "ebtables -t nat -D PREROUTING -i %s -p arp --arp-opcode Request --arp-ip-dst %s -j ACCEPT",
              usb_intf_name, inet_ntoa(addr));
     ds_system_call(command, strlen(command));
   }

   /* Delete ARP Entry */
   snprintf(command, MAX_COMMAND_STR_LEN,"arp -d %s",inet_ntoa(addr));
   ds_system_call(command, strlen(command));
 }
  return true;
}

/*===========================================================================
  FUNCTION MatchMacAddrInList
==========================================================================*/
/*!
@brief
  - Match the MAC address
  - If a match is found, store the IP address of the client in a Linked List
@parameters
  Pointer to qcmap_nl_addr_t
@return
  none
@note
- Dependencies
- None
- Side Effects
- None
*/
/*=========================================================================*/
bool QCMAP_ConnectionManager::MatchMacAddrInList
(
  qcmap_nl_addr_t* nl_addr
)
{
  ds_dll_el_t * node = NULL;
  qcmap_cm_client_data_info_t* list_data = NULL;
  qcmap_addr_info_list_t* addrList = &(this->addrList);
  qmi_error_type_v01 qmi_err_num;
  struct ps_in6_addr *prefix_ptr, *nl_prefix_ptr;
  uint8 origIPv6[QCMAP_MSGR_IPV6_ADDR_LEN_V01];
  uint8 ipv6PrefixLen = 0;
  int qcmap_cm_error;
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  qcmap_cm_nl_prefix_info_t   ipv6_del_prefix_info;
  in_addr addr;
  char ipv6addr[INET6_ADDRSTRLEN];
  uint8 mac_addr_char[QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01];
  struct ps_in6_addr *global_addr_ptr = NULL, *iid_addr_ptr = NULL;
  struct in6_addr null_ipv6_address;

  memset(devname, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);
  memset(origIPv6,0,QCMAP_MSGR_IPV6_ADDR_LEN_V01);
  memset(ipv6addr, 0, INET6_ADDRSTRLEN);
  memset(mac_addr_char, 0, QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01);
  memset(&null_ipv6_address, 0, sizeof(struct in6_addr));

  if ( addrList->addrListHead == NULL)
  {
    LOG_MSG_ERROR("\nMatchMacAddrInList - Linked list head is NULL \n",0,0,0);
    return false;
  }

  node = ds_dll_search (addrList->addrListHead , (void*)nl_addr->mac_addr,
                        qcmap_match_mac_address);
  if( node == NULL)
  {
    LOG_MSG_ERROR("\nMatchMacAddrInList - No match for MAC address "
                  "found in the list\n",0,0,0);
    return false;
  }
  else
  {
    //When a node is found, update the IP address from the netlink event
    list_data = (qcmap_cm_client_data_info_t*)(node->data);
    if (list_data == NULL)
    {
      LOG_MSG_ERROR("MatchMacAddrInList - Error in fetching node data ",0,0,0);
      return false;
    }
    ds_mac_addr_ntop(list_data->mac_addr, mac_addr_char);
    ds_log_med(" QCMAP_ConnectionManager::MatchMacAddrInList() Found node with MAC addr %s \n",
               mac_addr_char);

    if(nl_addr->isValidIPv4address == true && nl_addr->ip_addr != 0)
    {
      if (list_data->ip_addr == nl_addr->ip_addr)
      {
        addr.s_addr = nl_addr->ip_addr;
        ds_log_med(" Duplicate NEWNEIGH Event recieved for IP %s \n",inet_ntoa(addr));
      }
      else
      {
        addr.s_addr = nl_addr->ip_addr;
        ds_log_med("Adding a new entry for IP %s \n",inet_ntoa(addr));
        list_data->ip_addr = nl_addr->ip_addr;
        if (this->IsAPSTABridgeActivated())
        {
          if (!this->AddEbtablesRuleForBridgeMode(list_data))
            LOG_MSG_ERROR("Unable to Add Ebtables rules for NEWNEIGH AP-STA Bridge Mode",0,0,0);
        }
      }
    }

    memcpy(origIPv6, nl_addr->ip_v6_addr,QCMAP_MSGR_IPV6_ADDR_LEN_V01);
    nl_prefix_ptr = (struct ps_in6_addr*)(nl_addr->ip_v6_addr);

    if (QCMAP_IN6_IS_PREFIX_LINKLOCAL(nl_prefix_ptr->ps_s6_addr32)){
      LOG_MSG_INFO1("MatchMacAddrInList - LL address received ",0,0,0);
    }

    if((nl_addr->isValidIPv6address == true) &&
        !(QCMAP_IN6_IS_PREFIX_LINKLOCAL(nl_prefix_ptr->ps_s6_addr32))&&
       ((this->sta_connected) || (this->cradle_backhaul_connected) ||
       (qcmap_cm_get_ipv6_state(&qmi_err_num) == QCMAP_CM_V6_WAN_CONNECTED)))
      {
        if ((!this->sta_connected) && (!this->cradle_backhaul_connected))
        {
          if ((qcmap_cm_get_dev_name(this->qcmap_cm_handle,
              QCMAP_MSGR_IP_FAMILY_V6_V01, devname, &qcmap_cm_error)) !=
              QCMAP_CM_SUCCESS )
          {
            LOG_MSG_ERROR("Couldn't get ipv6 rmnet name. error %d\n",
                          qcmap_cm_error,0,0);
            return false;
          }
        }
        else if ( this->sta_connected )
        {
          if (this->IsAPSTABridgeActivated())
          {
            strlcpy(devname,BRIDGE_IFACE, QCMAP_MSGR_INTF_LEN);
          }
          else
          {
            strlcpy(devname,this->cfg.lan_config.sta_interface, QCMAP_MSGR_INTF_LEN);
          }
        }
        else if ( this->cradle_backhaul_connected )
        {
          strlcpy(devname, ECM_IFACE, QCMAP_MSGR_INTF_LEN);
        }

        memset(&this->ipv6_prefix_info, 0, sizeof(qcmap_cm_nl_prefix_info_t));
        this->ipv6_prefix_info.cache_info.ifa_prefered = IPV6_DEFAULT_PREFERED_LIFETIME;
        this->ipv6_prefix_info.cache_info.ifa_valid = IPV6_DEFAULT_VALID_LIFETIME;
        this->GetIPV6PrefixInfo(devname,&(this->ipv6_prefix_info));
        prefix_ptr = (struct ps_in6_addr *) &(((struct sockaddr_in6 *)&(this->ipv6_prefix_info.prefix_addr))->sin6_addr);

        //Store the original Ipv6 address
        memcpy(origIPv6, nl_addr->ip_v6_addr,QCMAP_MSGR_IPV6_ADDR_LEN_V01);
        nl_prefix_ptr = (struct ps_in6_addr*)(nl_addr->ip_v6_addr);
        ipv6PrefixLen = this->ipv6_prefix_info.prefix_len;

        /* Zero out the non prefix bits. */
        prefix_ptr->ps_s6_addr64[0] >>= (IPV6_MIN_PREFIX_LENGTH - ipv6PrefixLen);
        prefix_ptr->ps_s6_addr64[0] <<= (IPV6_MIN_PREFIX_LENGTH - ipv6PrefixLen);
        nl_prefix_ptr->ps_s6_addr64[0] >>= (IPV6_MIN_PREFIX_LENGTH - ipv6PrefixLen);
        nl_prefix_ptr->ps_s6_addr64[0] <<= (IPV6_MIN_PREFIX_LENGTH - ipv6PrefixLen);
        /* Zero out the IID part */
        prefix_ptr->ps_s6_addr64[1] = 0;
        nl_prefix_ptr->ps_s6_addr64[1] = 0;

        if (!memcmp(prefix_ptr->ps_s6_addr64,
                     nl_prefix_ptr->ps_s6_addr64,
                     sizeof(uint64)))
        {
          if (memcmp(&null_ipv6_address, (struct in6_addr *)list_data->link_local_v6_addr,
               sizeof(struct in6_addr) ))
          {
            /* Update only prefix part. IID part will be from LL address. */
            global_addr_ptr = (struct ps_in6_addr *)origIPv6;
            iid_addr_ptr = (struct ps_in6_addr *)list_data->link_local_v6_addr;
            global_addr_ptr->ps_s6_addr64[1] = iid_addr_ptr->ps_s6_addr64[1];
          }
          memcpy (list_data->ip_v6_addr,
                  origIPv6,QCMAP_MSGR_IPV6_ADDR_LEN_V01);
          inet_ntop(AF_INET6,
                    (void *)list_data->ip_v6_addr,
                    ipv6addr, INET6_ADDRSTRLEN);
          ds_log_med("MatchMacAddrInList- New V6 Address %s \n",ipv6addr);
        }
        else if (prefix_delegation_activated)
        {
          if (qcmap_cm_get_ipv6_state(&qmi_err_num) == QCMAP_CM_V6_WAN_CONNECTED)
          {
            if (memcmp(&null_ipv6_address, (struct in6_addr *)list_data->link_local_v6_addr,
                 sizeof(struct in6_addr) ))
            {
              /* Update only prefix part. IID part will be from LL address. */
              global_addr_ptr = (struct ps_in6_addr *)origIPv6;
              iid_addr_ptr = (struct ps_in6_addr *)list_data->link_local_v6_addr;
              global_addr_ptr->ps_s6_addr64[1] = iid_addr_ptr->ps_s6_addr64[1];
            }
            memcpy (list_data->ip_v6_addr,
                  origIPv6,QCMAP_MSGR_IPV6_ADDR_LEN_V01);
            inet_ntop(AF_INET6,
                      (void *)list_data->ip_v6_addr,
                      ipv6addr, INET6_ADDRSTRLEN);
            ds_log_med("MatchMacAddrInList- New V6 Address %s \n",ipv6addr);
          }
          else
          {
            memset(&ipv6_del_prefix_info, 0, sizeof(qcmap_cm_nl_prefix_info_t));
            ipv6_del_prefix_info.cache_info.ifa_prefered = IPV6_DEFAULT_PREFERED_LIFETIME;
            ipv6_del_prefix_info.cache_info.ifa_valid = IPV6_DEFAULT_VALID_LIFETIME;
            ipv6_del_prefix_info.prefix_len =  (unsigned char) IPV6_MIN_PREFIX_LENGTH;
            memcpy(((struct sockaddr_in6*)&(ipv6_del_prefix_info.prefix_addr))->sin6_addr.s6_addr,
                   list_data->ip_v6_addr, ipv6PrefixLen);

            if(this->UpdatePrefix(&ipv6_del_prefix_info, true, true, list_data->link_local_v6_addr) == -1)
            {
              LOG_MSG_ERROR("Error: Unable to deprecate prefix", 0, 0, 0);
              return false;
            }
          }
        }
    }
    else if ((nl_addr->isValidIPv6address == true) &&
              QCMAP_IN6_IS_PREFIX_LINKLOCAL(nl_prefix_ptr->ps_s6_addr32))
    {
      //Copy the link local Ipv6 address of the client
        memcpy (list_data->link_local_v6_addr,
                origIPv6,QCMAP_MSGR_IPV6_ADDR_LEN_V01);
        if (memcmp(&null_ipv6_address, (struct in6_addr *)list_data->ip_v6_addr,
                   sizeof(struct in6_addr) ))
        {
          /* Update the global address with this IID. */
          global_addr_ptr = (struct ps_in6_addr *)list_data->ip_v6_addr;
          iid_addr_ptr = (struct ps_in6_addr *)list_data->link_local_v6_addr;
          global_addr_ptr->ps_s6_addr64[1] = iid_addr_ptr->ps_s6_addr64[1];
        }
        inet_ntop(AF_INET6,
                  (void *)list_data->link_local_v6_addr,
                  ipv6addr, INET6_ADDRSTRLEN);
        ds_log_med("MatchMacAddrInList- New LL V6 Address %s \n",ipv6addr);
    }
  }

  return true;
}

/*===========================================================
  FUNCTION DeleteDeviceEntryInfo
============================================================
@brief
  Remove the client entry (node) from the List

@parameters
  - MAC address of client
  - Device type to indicate device is USB or WLAN device
@return
  none

@note

- Dependencies
- None

- Side Effects
- None
*/
/*===========================================================*/
bool QCMAP_ConnectionManager::DeleteDeviceEntryInfo
(
  void* mac_addr,
  qcmap_msgr_device_type_enum_v01 device_type
)
{
  ds_dll_el_t * node;
  uint8 mac_addr_hex[QCMAP_MSGR_MAC_ADDR_LEN_V01];
  qcmap_cm_client_data_info_t* connectedDevicesList = NULL;
  qcmap_addr_info_list_t* addrList = &(this->addrList);
  uint8 mac_addr_char[QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01];
  qmi_error_type_v01 qmi_err_num;

  LOG_MSG_INFO1("\n In DeleteDeviceEntryInfo \n",0,0,0);

  if(addrList->addrListHead == NULL)
  {
    LOG_MSG_ERROR("DeleteDeviceEntryInfo - Linked list head is NULL \n",0,0,0);
    return false;
  }

  if ((device_type == QCMAP_MSGR_DEVICE_TYPE_PRIMARY_AP_V01)||
      (device_type == QCMAP_MSGR_DEVICE_TYPE_GUEST_AP_V01))
  {
    ds_log_med("DeleteDeviceEntryInfo- Delete client mac %s, Device_type:%d \n",
               mac_addr, device_type);
    //Convert the MAC from char to hex
    if(!ds_mac_addr_pton((char*)mac_addr, mac_addr_hex))
    {
      LOG_MSG_ERROR("DeleteDeviceEntryInfo - Error in MAC address conversion",
                    0,0,0);
      return false;
    }
    mac_addr = (void*)mac_addr_hex;
  }
  else
  {
    memset(mac_addr_char, 0, QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01);
    ds_mac_addr_ntop(mac_addr, mac_addr_char);
    ds_log_med("DeleteDeviceEntryInfo- Delete client mac %s, Device_type:%d \n",
               mac_addr_char, device_type);
  }

  //Delete the node from the list
  if ((node = ds_dll_delete( addrList->addrListHead, &(addrList->addrListTail),
       (void*)mac_addr, qcmap_match_mac_address))== NULL)
  {
    LOG_MSG_ERROR("DeleteDeviceEntryInfo - Error in finding the node",0,0,0);
    return false;
  }

  //Free the memory for the device details structure
  connectedDevicesList = (qcmap_cm_client_data_info_t*)node->data;

  if (connectedDevicesList == NULL)
  {
    LOG_MSG_ERROR("DeleteDeviceEntryInfo - The device information"
                  "node is NULL\n",0,0,0);
    //Free the memory of the linked list node
    ds_dll_free(node);
    node = NULL;
    return false;
  }

  this->DeleteDelegatedPrefix(connectedDevicesList->ip_v6_addr, &qmi_err_num);

  if (this->IsAPSTABridgeActivated())
  {
    if (DelEbtablesRuleForBridgeMode(connectedDevicesList) == false)
    {
      ds_log_med("Could not Delete Ebtables rules for device %s\n",mac_addr);
    }
  }

  //Free the device information structure
  ds_free (connectedDevicesList);
  connectedDevicesList = NULL;

  //Free the memory of the linked list node
  ds_dll_free(node);
  node = NULL;

  // Add logic to check if there are no more nodes in the list and free the head
  if ( addrList->addrListHead->next == NULL)
  {
    LOG_MSG_INFO1("\n No device info exist\n",0,0,0);
    ds_dll_free(addrList->addrListHead);
    addrList->addrListHead = NULL;
  }
  this->conn_device_info.numOfNodes--;
  LOG_MSG_INFO1("\n Number of Connected Devices: %d\n", this->conn_device_info.numOfNodes, 0, 0);
  return true;
}

/*==========================================================
  FUNCTION AddNewDeviceEntry
===========================================================*/
/*!
@brief
  Stores the MAC address and interface name of client in a list.

@parameters
  - MAC address of client
  - Device type
  - Ipv4 address
  - IPv6 address
  - bool to check valid ipv4 address
  - bool to check valid ipv6 address
@return
  none

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================*/
bool QCMAP_ConnectionManager::AddNewDeviceEntry
(
  void* mac_addr,
  qcmap_msgr_device_type_enum_v01 device_type,
  uint32* ip_addr,
  uint8* ipv6_addr,
  bool isIpv4valid,
  bool isIpv6valid
)
{
  ds_dll_el_t * node = NULL;
  qmi_error_type_v01 qmi_err_num;
  int qcmap_cm_error;
  struct ps_in6_addr *prefix_ptr, *nl_prefix_ptr;
  uint8 ipv6PrefixLen = 0;
  uint8 origIPv6[QCMAP_MSGR_IPV6_ADDR_LEN_V01];
  uint8 mac_addr_hex[QCMAP_MSGR_MAC_ADDR_LEN_V01];
  uint8 mac_addr_char[QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01];
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  char ipv6addr[INET6_ADDRSTRLEN];
  struct ps_in6_addr *global_addr_ptr = NULL, *iid_addr_ptr = NULL;
  struct in6_addr null_ipv6_address;

  qcmap_addr_info_list_t* addrList = &(this->addrList);
  qcmap_cm_client_data_info_t* connectedDevicesList = NULL;

  if ( (device_type == QCMAP_MSGR_DEVICE_TYPE_PRIMARY_AP_V01 ||
       device_type == QCMAP_MSGR_DEVICE_TYPE_GUEST_AP_V01) &&
       ( this->wifi_mode < QCMAP_MSGR_WLAN_MODE_AP_V01 ||
       this->wifi_mode > QCMAP_MSGR_WLAN_MODE_AP_STA_V01 ))
  {
    LOG_MSG_ERROR("\n WLAN is not enabled. Do not add this entry\n",
                   0 ,0, 0);
    return false;
  }

  connectedDevicesList = (qcmap_cm_client_data_info_t*)ds_malloc(sizeof(qcmap_cm_client_data_info_t));

  if( connectedDevicesList == NULL )
  {
    LOG_MSG_ERROR("AddNewDeviceEntry - Error in allocating memory for"
                  "device information structure",0,0,0);
    return false;
  }

  memset (connectedDevicesList,0,sizeof(qcmap_cm_client_data_info_t));
  memset (origIPv6, 0 ,QCMAP_MSGR_IPV6_ADDR_LEN_V01);
  memset(mac_addr_char, 0, QCMAP_MSGR_MAC_ADDR_NUM_CHARS_V01);
  memset(ipv6addr, 0, INET6_ADDRSTRLEN);
  memset(&null_ipv6_address, 0, sizeof(struct in6_addr));

  /* NOTE - With AP-AP-STA mode, we need to set the device type accordingly*/
  ds_mac_addr_ntop((char*)mac_addr, mac_addr_char);
  ds_log_med("AddNewDeviceEntry- Add new address for mac %s \n",mac_addr_char);

  // Store Client MAC Address in QCMAP context
  memcpy( connectedDevicesList->mac_addr, (uint8*)mac_addr,
          QCMAP_MSGR_MAC_ADDR_LEN_V01);

  //Store the device type to the QCMAP context
  connectedDevicesList->device_type = device_type;

  /* For USB Clients AddNewDeviceEntry() is called from NEWNEIGH events.
   * Hence update IP address too.*/
  if(device_type == QCMAP_MSGR_DEVICE_TYPE_USB_V01)
  {
    if (isIpv4valid == true)
    {
      connectedDevicesList->ip_addr = *ip_addr;
      if (this->IsAPSTABridgeActivated())
      {
        if (!this->AddEbtablesRuleForBridgeMode(connectedDevicesList))
          LOG_MSG_ERROR("Unable to Add Ebtables rules for NEWNEIGH AP-STA Bridge Mode",0,0,0);
      }
    }

    //Store the original Ipv6 address
    if (isIpv6valid == true)
    {
      memcpy(origIPv6, ipv6_addr, QCMAP_MSGR_IPV6_ADDR_LEN_V01);
      nl_prefix_ptr = (struct ps_in6_addr*)(ipv6_addr);

      if ((!(QCMAP_IN6_IS_PREFIX_LINKLOCAL(nl_prefix_ptr->ps_s6_addr32)))&&
          ((this->sta_connected) || (this->cradle_backhaul_connected) ||
          (qcmap_cm_get_ipv6_state(&qmi_err_num) == QCMAP_CM_V6_WAN_CONNECTED)))
      {
        if ( (!this->sta_connected) && (!this->cradle_backhaul_connected))
        {
          if (qcmap_cm_get_dev_name(this->qcmap_cm_handle,
              QCMAP_MSGR_IP_FAMILY_V6_V01, devname, &qcmap_cm_error) !=
              QCMAP_CM_SUCCESS )
          {
            LOG_MSG_ERROR("Couldn't get ipv6 rmnet name. error %d\n",
                          qcmap_cm_error,0,0);
            ds_free(connectedDevicesList);
            connectedDevicesList = NULL;
            return false;
          }
        }
        else if ( this->sta_connected )
        {
          if (this->IsAPSTABridgeActivated())
          {
            strlcpy(devname,BRIDGE_IFACE, QCMAP_MSGR_INTF_LEN);
          }
          else
          {
            strlcpy(devname,this->cfg.lan_config.sta_interface, QCMAP_MSGR_INTF_LEN);
          }
        }
        else if ( this->cradle_backhaul_connected )
        {
          strlcpy(devname, ECM_IFACE, QCMAP_MSGR_INTF_LEN);
        }
        memset(&this->ipv6_prefix_info, 0, sizeof(qcmap_cm_nl_prefix_info_t));
        this->ipv6_prefix_info.cache_info.ifa_prefered = IPV6_DEFAULT_PREFERED_LIFETIME;
        this->ipv6_prefix_info.cache_info.ifa_valid = IPV6_DEFAULT_VALID_LIFETIME;
        this->GetIPV6PrefixInfo(devname,
                              &(this->ipv6_prefix_info));
        prefix_ptr = (struct ps_in6_addr *) &(((struct sockaddr_in6 *)&(this->ipv6_prefix_info.prefix_addr))->sin6_addr);

        ipv6PrefixLen = this->ipv6_prefix_info.prefix_len;

        /* Zero out the non prefix bits. */
        prefix_ptr->ps_s6_addr64[0] >>= (IPV6_MIN_PREFIX_LENGTH - ipv6PrefixLen);
        prefix_ptr->ps_s6_addr64[0] <<= (IPV6_MIN_PREFIX_LENGTH - ipv6PrefixLen);
        nl_prefix_ptr->ps_s6_addr64[0] >>= (IPV6_MIN_PREFIX_LENGTH - ipv6PrefixLen);
        nl_prefix_ptr->ps_s6_addr64[0] <<= (IPV6_MIN_PREFIX_LENGTH - ipv6PrefixLen);
        /* Zero out the IID part */
        prefix_ptr->ps_s6_addr64[1] = 0;
        nl_prefix_ptr->ps_s6_addr64[1] = 0;

        if ((!memcmp(prefix_ptr->ps_s6_addr64,
                     nl_prefix_ptr->ps_s6_addr64,
                     sizeof(uint64))))
        {
          if (memcmp(&null_ipv6_address, (struct in6_addr *)connectedDevicesList->link_local_v6_addr,
               sizeof(struct in6_addr) ))
          {
            /* Update only prefix part. IID part will be from LL address. */
            global_addr_ptr = (struct ps_in6_addr *)origIPv6;
            iid_addr_ptr = (struct ps_in6_addr *)connectedDevicesList->link_local_v6_addr;
            global_addr_ptr->ps_s6_addr64[1] = iid_addr_ptr->ps_s6_addr64[1];
          }
          memcpy(connectedDevicesList->ip_v6_addr,origIPv6,
                  QCMAP_MSGR_IPV6_ADDR_LEN_V01);
          inet_ntop(AF_INET6,
                    (void *)connectedDevicesList->ip_v6_addr,
                    ipv6addr, INET6_ADDRSTRLEN);
          ds_log_med("AddNewDeviceEntry- New V6 Address %s \n",ipv6addr);
        }
      }
      else if (QCMAP_IN6_IS_PREFIX_LINKLOCAL(nl_prefix_ptr->ps_s6_addr32))
      {
        //Copy the link local Ipv6 address of the client
        memcpy(connectedDevicesList->link_local_v6_addr,origIPv6,
               QCMAP_MSGR_IPV6_ADDR_LEN_V01);
        if (memcmp(&null_ipv6_address, (struct in6_addr *)connectedDevicesList->ip_v6_addr,
                   sizeof(struct in6_addr) ))
        {
          /* Update the global address with this IID. */
          global_addr_ptr = (struct ps_in6_addr *)connectedDevicesList->ip_v6_addr;
          iid_addr_ptr = (struct ps_in6_addr *)connectedDevicesList->link_local_v6_addr;
          global_addr_ptr->ps_s6_addr64[1] = iid_addr_ptr->ps_s6_addr64[1];
        }
        /* Update the global address with this IID. */
        inet_ntop(AF_INET6,
                  (void *)connectedDevicesList->link_local_v6_addr,
                  ipv6addr, INET6_ADDRSTRLEN);
        ds_log_med("AddNewDeviceEntry- New LL V6 Address %s \n",ipv6addr);
      }
    }
  }

  if (addrList->addrListHead == NULL )
  {
    /*The first node which is created is a dummy node which does not store any device
           information. This is done to make use of the doubly linked list framework which
           is already existing*/
    if (( node = ds_dll_init(NULL)) == NULL)
    {
      LOG_MSG_ERROR("AddNewDeviceEntry - Error in allocating memory for node",
                    0,0,0);
      ds_free(connectedDevicesList);
      connectedDevicesList = NULL;
      return false;
    }
    addrList->addrListHead = node;
  }

  //Store the mac address, interface name in the linked list
  if ((node = ds_dll_enq(addrList->addrListHead,
                         NULL, (void*)connectedDevicesList )) == NULL)
  {
    LOG_MSG_ERROR("AddNewDeviceEntry - Error in adding a node",0,0,0);
    ds_free(connectedDevicesList);
    connectedDevicesList = NULL;
    return false;
  }
  this->conn_device_info.numOfNodes++;
  LOG_MSG_INFO1("\n Number of Connected Devices: %d\n", this->conn_device_info.numOfNodes, 0, 0);
  //Update the tail pointer when a new node is added everytime
  addrList->addrListTail = node;
  return true;
}


/*=====================================================
  FUNCTION GetConnectedDevicesInfo
======================================================*/
/*!
@brief
 Fetches the list of devices connected to the Access point

@parameters
  - Pointer to qcmap_msgr_connected_device_info_v01
  - Number of device entries
  - Pointer to qmi_error_type_v01

@return
  boolean
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=====================================================*/


boolean QCMAP_ConnectionManager::GetConnectedDevicesInfo
(
 qcmap_msgr_connected_device_info_v01 *conn_devices,
 unsigned int* num_entries,
 qmi_error_type_v01 *qmi_err_num
)
{
  int i = 0;
  FILE *fd = NULL;
  ds_dll_el_t * node;
  qcmap_cm_client_data_info_t* connectedDevicesList = NULL;
  qcmap_addr_info_list_t* addrList = &(this->addrList);
  char command[MAX_COMMAND_STR_LEN];
  LOG_MSG_INFO1("QCMAP_ConnectionManager:GetConnectedDevicesInfo",0,0,0);

  if (conn_devices == NULL)
  {
    LOG_MSG_ERROR("Null arguements passed.\n",0,0,0);
    return false;
  }

  //Fetch the number of nodes in linked list and store in num_entries
  *num_entries = this->conn_device_info.numOfNodes;
  if ( *num_entries == 0 )
  {
    LOG_MSG_INFO1("GetConnectedDevicesInfo - No devices present",0,0,0);
    return true;
  }

  //Delete the hostname.txt file if present
  snprintf(command, MAX_COMMAND_STR_LEN, "rm %s",HOSTNAME_PATH);
  ds_system_call(command, strlen(command));

  //Prune the dnsmasq.leases file to have lease expiry time, MAC address and hostname
  snprintf(command, MAX_COMMAND_STR_LEN,
           "cut -f1,2,4 -d ' ' /var/lib/misc/dnsmasq.leases > %s",
           HOSTNAME_PATH);
  ds_system_call(command, strlen(command));

  //Open the hostname.txt file to fetch lease time and hostname
  fd = fopen(HOSTNAME_PATH,"r");
  if(fd == NULL)
  {
    LOG_MSG_ERROR("GetConnectedDevicesInfo - Error in opening %s",
                  HOSTNAME_PATH,0,0);
    return false;
  }

  if(addrList->addrListHead == NULL)
  {
    LOG_MSG_ERROR("GetConnectedDevicesInfo - Linked list head is NULL"
                  "Cant fetch connected devices info",0,0,0);
    fclose(fd);
    return false;
  }

  node = addrList->addrListHead->next;
  if ( node == NULL)
  {
    LOG_MSG_ERROR("GetConnectedDevicesInfo - Device info is NULL"
                  "Cant fetch connected devices info",0,0,0);
    fclose(fd);
    return false;
  }

  for ( i=0; i < *num_entries; i++ )
  {
    connectedDevicesList = ( qcmap_cm_client_data_info_t* )node->data;

    fetchHostNameAndLeasetime(fd ,connectedDevicesList->mac_addr,
                              conn_devices[i].host_name,
                              &conn_devices[i].lease_expiry_time);

    //Copy the MAC, device type, IPv4 and IPv6 address
    memcpy (conn_devices[i].client_mac_addr, connectedDevicesList->mac_addr,
            QCMAP_MSGR_MAC_ADDR_LEN_V01);
    conn_devices[i].device_type = connectedDevicesList->device_type;
    conn_devices[i].ipv4_addr = connectedDevicesList->ip_addr;
    memcpy (conn_devices[i].ll_ipv6_addr,
            connectedDevicesList->link_local_v6_addr, QCMAP_MSGR_IPV6_ADDR_LEN_V01);
    memcpy (conn_devices[i].ipv6_addr,
            connectedDevicesList->ip_v6_addr, QCMAP_MSGR_IPV6_ADDR_LEN_V01);

    //Point to the next node
    node = node->next;
  }

  //Close the hostname.txt file
  fclose(fd);
  return true;
}

/*=====================================================
  FUNCTION DeleteGlobalipv6AddressInfo
======================================================*/
/*!
@brief
 Deletes the Global Ipv6 address from the connected
 devices linked list

@parameters
  - void

@return
  bool
  -true on successful deletion
  -false on failure
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=====================================================*/
boolean QCMAP_ConnectionManager:: DeleteGlobalipv6AddressInfo ()
{
  int i = 0;
  ds_dll_el_t * node;
  qcmap_cm_client_data_info_t* connectedDevicesList = NULL;
  qcmap_addr_info_list_t* addrList = &(this->addrList);

  if(addrList->addrListHead == NULL)
  {
    LOG_MSG_ERROR("DeleteGlobalipv6AddressInfo - Linked list head is NULL \n",
                  0, 0, 0);
    return false;
  }

  node = addrList->addrListHead->next;
  if ( node == NULL)
  {
    LOG_MSG_ERROR("DeleteGlobalipv6AddressInfo - Device info is NULL"
                  "Cant fetch linked list node", 0, 0, 0);
    return false;
  }

  for (i = 0; i < GetNumofConnectedDevices(); i++)
  {
    connectedDevicesList = ( qcmap_cm_client_data_info_t* )node->data;
    if (connectedDevicesList == NULL)
    {
      LOG_MSG_ERROR("DeleteGlobalipv6AddressInfo - Connected Device context for"
                    " node %d+1 is NULL", i, 0, 0);
      return false;
    }
    memset (connectedDevicesList->ip_v6_addr, 0,
            QCMAP_MSGR_IPV6_ADDR_LEN_V01);
    node = node->next;
  }

  LOG_MSG_INFO1("DeleteGlobalipv6AddressInfo:Global Ipv6 address of clients "
                "successfully deleted", 0, 0, 0);
  return true;
}

/*===========================================================================
  FUNCTION SetSupplicantConfig
==========================================================================*/
/*!
@brief
  Enables/Disables the wpa_supplicant based on the status flag.

@parameters
  boolean status,
  qmi_error_type_v01 *qmi_err_num

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetWPASupplicantConfig
(
  boolean status,
  qmi_error_type_v01 *qmi_err_num
)
{
  char command[MAX_COMMAND_STR_LEN];
  char temp_command[MAX_COMMAND_STR_LEN];

  LOG_MSG_INFO1("QCMAP_ConnectionManager::SetWPASupplicantConfig",0,0,0);

  if (qmi_err_num == NULL)
  {
    LOG_MSG_ERROR("SetWPASupplicantConfig: NULL Arguments",0,0,0);
    return false;
  }

  if (this->wifi_mode != QCMAP_MSGR_WLAN_MODE_AP_STA_V01)
  {
    LOG_MSG_ERROR("SetWPASupplicantConfig: Invalid Wifi mode",0,0,0);
    *qmi_err_num = QMI_ERR_NOT_SUPPORTED_V01;
    return false;
  }

  if(status == FALSE)
  {
    /* Kill any running instance */
    snprintf( command, MAX_COMMAND_STR_LEN, "killall -15 wpa_supplicant" );
    ds_system_call(command, strlen(command));

    if (!IsWpaSupplicantkilled())
    {
      /* Kill wpa_supplicant forcefully. */
      snprintf( command, MAX_COMMAND_STR_LEN, "killall -9 wpa_supplicant" );
      ds_system_call(command, strlen(command));
    }

    /* Kill any wpa_cli running instance */
    snprintf( command, MAX_COMMAND_STR_LEN, "killall -15 wpa_cli" );
    ds_system_call(command, strlen(command));

    if (!IsWpaClikilled())
    {
      /* Kill wpa_cli forcefully. */
      snprintf( command, MAX_COMMAND_STR_LEN, "killall -9 wpa_cli" );
      ds_system_call(command, strlen(command));
    }

    LOG_MSG_INFO1("SetWPASupplicantConfig:Stopped all the services.", 0,0,0);
  }
  else
  {
    /* Restart Supplicant as per supplicant config */
    LOG_MSG_INFO1("Starting station iface %s with config as in: %s\n",
                  this->cfg.lan_config.sta_interface,
                  this->cfg.lan_config.interface[this->sta_iface_index].path_to_supplicant_conf,0);

    /* Run wpa_supplicant for sta interface with specified config */
    if (strncmp(this->cfg.lan_config.module,AR6003,strlen(AR6003)) == 0)
    {
      snprintf(command, MAX_COMMAND_STR_LEN,
               "wpa_supplicant -i %s -Dar6003 -B -c %s",
               this->cfg.lan_config.sta_interface,
               this->cfg.lan_config.interface[this->sta_iface_index].path_to_supplicant_conf );
    }
    else
    {
      snprintf(command, MAX_COMMAND_STR_LEN,
               "wpa_supplicant -i %s -Dnl80211 -B -c %s",
               this->cfg.lan_config.sta_interface,
               this->cfg.lan_config.interface[this->sta_iface_index].path_to_supplicant_conf);
    }

    /* If debug mode is enabled add concat dbg cmd */
    if (this->cfg.debug_config.supplicant_debug)
    {
      snprintf(temp_command, MAX_COMMAND_STR_LEN, "%s %s", command,
               this->cfg.debug_config.supplicant_dbg_cmd);
      memcpy(command, temp_command, MAX_COMMAND_STR_LEN);
    }

    ds_system_call(command, strlen(command));

    /* Run wpa_cli */
    snprintf(command, MAX_COMMAND_STR_LEN,
             "wpa_cli -i %s -p %s -B -a /usr/bin/QCMAP_StaInterface",
             this->cfg.lan_config.sta_interface,
             WPA_CTRL_PATH);
    ds_system_call(command, strlen(command));

    LOG_MSG_INFO1("SetWPASupplicantConfig::Started all the services.",0,0,0);

  }
  return true;
}
/*===========================================================================
  FUNCTION IsPPPdkilled
==========================================================================*/
/*!
@brief
  Waits 1 second for the process to die.

@parameters
  char *process

@return
  true  - process killed
  flase - process running after waiting 1 second

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::IsPPPkilled()
{
  char process[MAX_COMMAND_STR_LEN];
  int i = 0;
  FILE *cmd;
  char pid_s[MAX_COMMAND_STR_LEN];
  int pid;

  memset(process, 0, MAX_COMMAND_STR_LEN);

  snprintf(process, MAX_COMMAND_STR_LEN, "pidof pppd");

  while (i++ < QCMAP_PROCESS_KILL_RETRY)
  {
    usleep(QCMAP_PROCESS_KILL_WAIT_MS);
    cmd = popen(process, "r");
    pid = 0;
    memset(pid_s, 0, MAX_COMMAND_STR_LEN);
    fgets(pid_s, MAX_COMMAND_STR_LEN, cmd);
    pid = atoi(pid_s);
    LOG_MSG_INFO1("\n pid =%d \n", pid, 0, 0);
    pclose(cmd);
    /* If pid is zero we break from while*/
    if(pid == 0)
    {
      return true;
    }
  }

  LOG_MSG_ERROR("PID still running after waiting 2 second\n", 0, 0, 0);
  return false;
}

/*===========================================================================
  FUNCTION GetCradleMode
==========================================================================*/
/*!
@brief
  Retreives the usb cradle mode.

@parameters
  char *process

@return
  true  - cradle mode found
  flase - cradle mode not set

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetCradleMode
(
  qcmap_msgr_cradle_mode_v01 *mode,
  qmi_error_type_v01 *qmi_err_num
)
{
  if (mode == NULL || qmi_err_num == NULL)
  {
    LOG_MSG_ERROR("NULL Args.\n", 0,0,0);
    return false;
  }

  *mode = this->cfg.cradle_config.cradle_mode;

  return true;
}

/*===========================================================================
  FUNCTION SetCradleMode
==========================================================================*/
/*!
@brief
  Sets the usb cradle mode.

@parameters
  char *process

@return
  true  - cradle mode set.
  flase - cradle mode set failed.

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetCradleMode
(
  qcmap_msgr_cradle_mode_v01  mode,
  void                        *softApHandle,
  qmi_error_type_v01          *qmi_err_num
)
{
  qmi_qcmap_msgr_softap_handle_type *handle = (qmi_qcmap_msgr_softap_handle_type*) softApHandle;
  char command[MAX_COMMAND_STR_LEN];
  int ecm_cat, retval, err_num;

  *qmi_err_num = QMI_ERR_NO_EFFECT_V01;

  if (handle == NULL)
  {
    LOG_MSG_ERROR("NULL Args.\n", 0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
    return false;
  }

  if(mode == this->cfg.cradle_config.cradle_mode)
  {
    LOG_MSG_INFO1("Cradle mode %d is already in affect", mode,0,0);
    return false;
  }

  if(mode != QCMAP_MSGR_CRADLE_DISABLED_V01 &&
     mode != QCMAP_MSGR_CRADLE_WAN_ROUTER_V01)
  {
    LOG_MSG_ERROR("Cradle mode not supported %d.", mode, 0, 0);
    *qmi_err_num = QMI_ERR_INVALID_TECH_PREF_V01;
    return false;
  }

  LOG_MSG_INFO1("Set cradle mode %d",mode,0,0);
  this->cfg.cradle_config.cradle_mode = mode;
  this->WriteConfigToXML();

  if (this->usb_link != QCMAP_QTI_USB_LINK_ECM)
  {
    /* Since ECM is not connected, check IPA xml and write to config only */
    LOG_MSG_INFO1("USB link is not ECM, wait for ecm connection",0,0,0);

    //qcmap would  first check ecm0 category. If WAN, update toLAN
    if(GetECMCategory(&ecm_cat, qmi_err_num))
    {
      if (ecm_cat == LAN_CATEGORY && mode == QCMAP_MSGR_CRADLE_WAN_ROUTER_V01)
      {
        SetECMCategory(WAN_CATEGORY);
      }
      else if (ecm_cat == WAN_CATEGORY && mode == QCMAP_MSGR_CRADLE_DISABLED_V01)
      {
        SetECMCategory(LAN_CATEGORY);
      }
    }
    return true;
  }

  switch (mode)
  {
    case QCMAP_MSGR_CRADLE_DISABLED_V01:
  /* Disable Cradle Mode */
    if (!(retval = this->BringDownUSBLink(usb_link, handle, &err_num)))
    {
      LOG_MSG_ERROR("BringDownUSBLink failed %d %d", retval, err_num,0);
    }

    //qcmap would  first check ecm0 category. If WAN, update toLAN
    if(GetECMCategory(&ecm_cat, qmi_err_num))
    {
      if (ecm_cat == WAN_CATEGORY)
      {
        SetECMCategory(LAN_CATEGORY);
      }
    }

    if(!(retval = this->SetupUSBLink(QCMAP_QTI_USB_LINK_ECM, &err_num)))
    {
      LOG_MSG_ERROR("SetupUSBLink failed %d %d", retval, err_num,0);
    }

    //Unbridge ecm0 and bridge0 before sending deprecate RA
    snprintf(command, MAX_COMMAND_STR_LEN, "brctl delif %s %s",
             BRIDGE_IFACE, ECM_IFACE);
    ds_system_call(command, strlen(command));

    this->ProcessCradleDisAssoc(handle);

    //Bridge ecm0 and bridge0
    snprintf(command, MAX_COMMAND_STR_LEN, "brctl addif %s %s",
             BRIDGE_IFACE, ECM_IFACE);
    ds_system_call(command, strlen(command));
      break;
    case QCMAP_MSGR_CRADLE_LAN_BRIDGE_V01:
      /* Not currently supported */
    case QCMAP_MSGR_CRADLE_LAN_ROUTER_V01:
      /* Not currently supported */
    case QCMAP_MSGR_CRADLE_WAN_BRIDGE_V01:
      /* Not currently supported */
      return false;
      break;
    case QCMAP_MSGR_CRADLE_WAN_ROUTER_V01:
      this->cradle_connect_in_progress = true;

    //qcmap would  first check ecm0 category. If WAN, update toLAN
    if(GetECMCategory(&ecm_cat, qmi_err_num))
    {
      if (ecm_cat == LAN_CATEGORY)
      {
        if (!(retval = this->BringDownUSBLink(usb_link, handle, &err_num)))
        {
          LOG_MSG_ERROR("BringDownUSBLink failed %d %d", retval, err_num,0);
        }

        SetECMCategory(WAN_CATEGORY);

        if(!(retval = this->SetupUSBLink(QCMAP_QTI_USB_LINK_ECM, &err_num)))
        {
          LOG_MSG_ERROR("SetupUSBLink failed %d %d", retval, err_num,0);
        }
      }
    }
      break;
    default:
      /* Unknown mode */
      LOG_MSG_INFO1("Cradle mode unknown %d",mode,0,0);
      return false;
      break;
  }

  *qmi_err_num = QMI_ERR_NONE_V01;
  return true;
}


/*===========================================================================
  FUNCTION GetECMMode
==========================================================================*/
/*!
@brief
  Gets the ECM mode.

@parameters
  uint8_t *status

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::GetECMCategory(int *category, qmi_error_type_v01 *qmi_err_num)
{
  if (qcmap_cm_get_ecm_cat_ipa_xml(IPA_XML_PATH, category) == QCMAP_CM_SUCCESS){
    LOG_MSG_INFO1("GetECMCategory %d", *category, 0,0);
    return true;
  }

  LOG_MSG_ERROR("Retreive ECM failed",0,0,0);
  *qmi_err_num = QMI_ERR_INTERNAL_V01;
  return false;
}

/*===========================================================================
  FUNCTION SetECMMode
==========================================================================*/
/*!
@brief
  Sets the ECM mode.

@parameters
  uint8_t status

@return
  true  - on success
  flase - on failure

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

boolean QCMAP_ConnectionManager::SetECMCategory(int category)
{
  if (category == WAN_CATEGORY)
  {
     //set WAN category
                    //sed -i "/ecm0.*/ {N; s/ecm0.*WAN/ecm0<\/Name>\n\t\t\t   <Category>BAHAHA/g}" /etc/IPACM_cfg.xml
     ds_system_call("sed -i \"/ecm0.*/ {N; s/ecm0.*LAN/ecm0<\\/Name>\\n\\t\\t\\t   <Category>WAN/g}\" /etc/IPACM_cfg.xml",
                    strlen("sed -i \"/ecm0.*/ {N; s/ecm0.*LAN/ecm0<\\/Name>\\n\\t\\t\\t   <Category>WAN/g}\" /etc/IPACM_cfg.xml"));
     LOG_MSG_INFO1("Changing ECM to WAN category", 0,0,0);
  }
  else //default mode
  {
     //set LAN category
     ds_system_call("sed -i \"/ecm0.*/ {N; s/ecm0.*WAN/ecm0<\\/Name>\\n\\t\\t\\t   <Category>LAN/g}\" /etc/IPACM_cfg.xml",
                    strlen("sed -i \"/ecm0.*/ {N; s/ecm0.*WAN/ecm0<\\/Name>\\n\\t\\t\\t   <Category>LAN/g}\" /etc/IPACM_cfg.xml"));
     LOG_MSG_INFO1("Changing ECM to LAN category", 0,0,0);
  }

  return true;
}


/*===========================================================================
  FUNCTION DisableCradle
==========================================================================*/
/*!
@brief
  Bring down the Cradle iface

@parameters
  void

@return
  void
@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/

void QCMAP_ConnectionManager::DisableCradle(qmi_error_type_v01 *qmi_err_num)
{
  char command[MAX_COMMAND_STR_LEN];

  LOG_MSG_INFO1("Entering DisableCradle",0,0,0);

  /* Cant continue if MobileAP is not enabled! */
  if (!this->qcmap_enable)
  {
    LOG_MSG_ERROR("Error: QCMAP not enabled.\n",0,0,0);
    return;
  }

  if ( this->cfg.cradle_config.cradle_mode == QCMAP_MSGR_CRADLE_DISABLED_V01 )
  {
    LOG_MSG_ERROR("\n Cradle is already in disabled state.\n",0,0,0);
    return;
  }

  if ( !this->cradle_backhaul_connected )
  {
    LOG_MSG_ERROR("\n Cradle is not connected.\n",0,0,0);
    return;
  }

  /* Disable NAT on A5 and clean IPV4 firewall */
  this->DisableNATonA5();
  this->FlushIPV4Firewall();

  /* Disable IPV6 Forwarding and firewall */
  this->DisableIPV6Forwarding(true);
  this->FlushIPV6Firewall();

  /*Reset Cradle conencted flag*/
  this->cradle_backhaul_connected = false;

  /* stop dhcp client on ecm0 */
  LOG_MSG_INFO1("Killing dhcpcd process.\n",0,0,0);
  snprintf(command, MAX_COMMAND_STR_LEN, "kill -15 $(cat /var/run/dhcpcd-%s.pid)", ECM_IFACE);
  ds_system_call(command, strlen(command));
  snprintf(command, MAX_COMMAND_STR_LEN, "rm -rf /var/run/dhcpcd-%s.pid", ECM_IFACE);
  ds_system_call(command, strlen(command));

  /* Disable IPv4 MCAST */
  LOG_MSG_INFO1(" Stop PIMD to disable MCAST forwarding on Cradle interface.\n",0,0,0);
  StopMcastDaemon();

  return;
}
/*===========================================================================
  FUNCTION UpdateGlobalV6addr
==========================================================================*/
/*!
@brief
  Handles the RA packet

@parameters
  uint8_t ipv6_addr[]
  boolean  - Tells whether to update the prefix for all
             clients in case of multicast RA

@return
  void

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::UpdateGlobalV6addr
(
  qcmap_nl_addr_t* nl_addr,
  boolean forceUpdate
)
{
  struct ps_in6_addr *dst_addr_ptr = NULL, *iid_ptr = NULL;
  struct ps_in6_addr *global_addr_ptr = NULL, *prefix_addr_ptr = NULL;
  struct ps_in6_addr * curr_prefix_info = NULL, *prefix_info_ptr;
  qcmap_nl_addr_t qcmap_nl_buffer;
  qmi_error_type_v01 qmi_err_num;
  ds_dll_el_t * node = NULL;
  qcmap_cm_client_data_info_t *clientInfo = NULL;
  qcmap_addr_info_list_t* addrList = &(this->addrList);
  qcmap_cm_prefix_iid_info_t tempPrefixIidptr;
  uint8 ipv6PrefixLen = 0;
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  int qcmap_cm_error;
  qcmap_nl_sock_msg_t pktbuf;
  char ipv6addr[INET6_ADDRSTRLEN];

  if(this->cfg.lan_config.enable_ipv6)
  {
    if ( (!this->sta_connected) && (!this->cradle_backhaul_connected) &&
         qcmap_cm_get_ipv6_state(&qmi_err_num) != QCMAP_CM_V6_WAN_CONNECTED )
    {
      /* QCMAP IPV6 backhaul not up */
      LOG_MSG_ERROR("QCMAP IPV6 backhaul not UP\n", 0, 0, 0);
      return;
    }

    dst_addr_ptr = (struct ps_in6_addr *)nl_addr->ra_ipv6_dst_addr;
    prefix_addr_ptr = (struct ps_in6_addr *)nl_addr->ip_v6_addr;

    if (QCMAP_IN6_IS_PREFIX_LINKLOCAL(dst_addr_ptr->ps_s6_addr32))
    {
      LOG_MSG_INFO1("Destination based RA address received\n ", 0, 0, 0);
      memset(ipv6addr, 0, INET6_ADDRSTRLEN);
      inet_ntop(AF_INET6,
                  (void *)dst_addr_ptr->ps_s6_addr,
                  ipv6addr, INET6_ADDRSTRLEN);
      ds_log_med("UpdateGlobalV6addr- Destination V6 Address %s \n",ipv6addr);
      if ( addrList->addrListHead == NULL)
      {
        LOG_MSG_ERROR("UpdateGlobalV6addr() - Linked list head is NULL \n",
                      0, 0, 0);
        return ;
      }

      tempPrefixIidptr.iid_ptr = dst_addr_ptr;
      node = ds_dll_search (addrList->addrListHead , (void*)&tempPrefixIidptr,
                            qcmap_match_v6_iid);

      if ( node == NULL)
      {
        LOG_MSG_ERROR("UpdateGlobalV6addr() - No IID Match found. Check why??",0,0,0);
        return;
      }
      else
      {
        clientInfo = ( qcmap_cm_client_data_info_t* )ds_dll_data(node);
        if (clientInfo == NULL)
        {
          LOG_MSG_ERROR("UpdateGlobalV6addr - Error in fetching node data\n ",
                        0, 0, 0);
          return;
        }
        else
        {
          /* IID match is found Update Address the global v6 address*/
          iid_ptr = (struct ps_in6_addr *)clientInfo->link_local_v6_addr;
          global_addr_ptr = (struct ps_in6_addr *)clientInfo->ip_v6_addr;

          /* Update only if the prefix changes. */
          if (!memcmp(&global_addr_ptr->ps_s6_addr64[0],
                  &prefix_addr_ptr->ps_s6_addr64[0],
                  sizeof(uint64)))
          {
            LOG_MSG_INFO1("UpdateGlobalV6addr - Prefix unchanged no need to update\n ",
                          0, 0, 0);
          }
          else
          {
            global_addr_ptr->ps_s6_addr64[0] = prefix_addr_ptr->ps_s6_addr64[0];
            global_addr_ptr->ps_s6_addr64[1] = iid_ptr->ps_s6_addr64[1];

            if ( (clientInfo->device_type == QCMAP_MSGR_DEVICE_TYPE_USB_V01) &&
                 (this->cfg.lan_config.usb_conf.ppp_enabled) )
            {
              memcpy( this->ppp_ipv6_addr,
                      clientInfo->ip_v6_addr, QCMAP_MSGR_IPV6_ADDR_LEN_V01 );
              this->AddPPPIPv6Route();
            }
          }
        }
      }
    }
    else
    {
      if(!this->ipv6_prefix_info.prefix_info_valid)
      {
        /* Get the Current Prefix Info. */
        if ( this->cradle_backhaul_connected )
        {
          strlcpy(devname, ECM_IFACE, QCMAP_MSGR_INTF_LEN);
        }
        else if ( this->sta_connected )
        {
          if (this->IsAPSTABridgeActivated())
          {
            strlcpy(devname,BRIDGE_IFACE, QCMAP_MSGR_INTF_LEN);
          }
          else
          {
            strlcpy(devname,this->cfg.lan_config.sta_interface, QCMAP_MSGR_INTF_LEN);
          }
        }
        else if ((qcmap_cm_get_dev_name(this->qcmap_cm_handle,
                 QCMAP_MSGR_IP_FAMILY_V6_V01, devname, &qcmap_cm_error)) !=
                 QCMAP_CM_SUCCESS )
        {
          LOG_MSG_ERROR("Couldn't get ipv6 rmnet name. error %d\n",
                          qcmap_cm_error, 0, 0);
          return false;
        }
        else
        {
          LOG_MSG_ERROR("No backhaul availble. Ignore RA.\n",
                          0, 0, 0);
          return false;
        }

        memset(&this->ipv6_prefix_info, 0, sizeof(qcmap_cm_nl_prefix_info_t));
        this->ipv6_prefix_info.cache_info.ifa_prefered = IPV6_DEFAULT_PREFERED_LIFETIME;
        this->ipv6_prefix_info.cache_info.ifa_valid = IPV6_DEFAULT_VALID_LIFETIME;
        this->GetIPV6PrefixInfo(devname,&(this->ipv6_prefix_info));
      }
      curr_prefix_info = (struct ps_in6_addr *)\
                         &(((struct sockaddr_in6 *)\
                         &(this->ipv6_prefix_info.prefix_addr))->sin6_addr);

      /* Check if the RA received on current backhaul interface. If not ignore the RA. */
      if ( !this->ValidateGlobalV6addr(prefix_addr_ptr))
      {
        LOG_MSG_ERROR("Prefix not on the current backhaul. Ignore RA.\n",
                          0, 0, 0);
        return false;
      }

      if ((forceUpdate) ||
          (memcmp(&curr_prefix_info->ps_s6_addr64[0],
                  &prefix_addr_ptr->ps_s6_addr64[0],
                  sizeof(uint64))))
      {
        LOG_MSG_INFO1(" Multicast RA received \n",0,0,0);

        if (!this->AssignGlobalV6AddrAllClients(prefix_addr_ptr))
        {
          LOG_MSG_ERROR("Could not update Global Ipv6 address for clients\n",
                        0, 0, 0);
          return;
        }
      }
    }
  }
  else
  {
    LOG_MSG_ERROR("UpdateGlobalV6addr - Ipv6 is not enabled\n", 0, 0, 0);
  }
}

/*===========================================================================
  FUNCTION ValidateGlobalV6addr
==========================================================================*/
/*!
@brief
  Validates the prefix present in RA packet

@parameters
  struct ps_in6_addr *prefix_addr_ptr[]
  - Prefix to be validated.

@return
  TRUE - If it is a valid prefix.
  FALSE - If it not.

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::ValidateGlobalV6addr
(
  struct ps_in6_addr *prefix_addr_ptr
)
{
  struct ps_in6_addr *global_addr_ptr = NULL;
  struct ps_in6_addr *prefix_info_ptr;
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  int qcmap_cm_error;
  qcmap_cm_nl_prefix_info_t  wan_ipv6_prefix_info;

  memset(&wan_ipv6_prefix_info, 0, sizeof(qcmap_cm_nl_prefix_info_t));

  if ( this->cradle_backhaul_connected )
  {
    /* Get the STA/WWAN prefixes if available. */
    if ( this->wifi_mode == QCMAP_MSGR_WLAN_MODE_AP_STA_V01)
    {
      if (this->IsAPSTABridgeActivated())
      {
        strlcpy(devname,BRIDGE_IFACE, QCMAP_MSGR_INTF_LEN);
      }
      else
      {
        strlcpy(devname,this->cfg.lan_config.sta_interface, QCMAP_MSGR_INTF_LEN);
      }
      wan_ipv6_prefix_info.cache_info.ifa_prefered = IPV6_DEFAULT_PREFERED_LIFETIME;
      wan_ipv6_prefix_info.cache_info.ifa_valid = IPV6_DEFAULT_VALID_LIFETIME;
      this->GetIPV6PrefixInfo(devname,&(wan_ipv6_prefix_info));
      prefix_info_ptr = (struct ps_in6_addr *)\
                    &(((struct sockaddr_in6 *)\
                        &(wan_ipv6_prefix_info.prefix_addr))->sin6_addr);
      /* If the prefix matches with STA Prefix. Ignore RA. */
      if (!memcmp(&prefix_info_ptr->ps_s6_addr64[0],
                  &prefix_addr_ptr->ps_s6_addr64[0],
                  sizeof(uint64)))
      {
        LOG_MSG_ERROR("Prefix matches with STA prefix when in cradle mode. Ignore RA.\n",
                      0, 0, 0);
        return false;
      }
    }
    if ((qcmap_cm_get_dev_name(this->qcmap_cm_handle,
                               QCMAP_MSGR_IP_FAMILY_V6_V01,
                               devname, &qcmap_cm_error)) ==
                               QCMAP_CM_SUCCESS )
    {
      wan_ipv6_prefix_info.cache_info.ifa_prefered = IPV6_DEFAULT_PREFERED_LIFETIME;
      wan_ipv6_prefix_info.cache_info.ifa_valid = IPV6_DEFAULT_VALID_LIFETIME;
      this->GetIPV6PrefixInfo(devname,&(wan_ipv6_prefix_info));
      prefix_info_ptr = (struct ps_in6_addr *)\
                        &(((struct sockaddr_in6 *)\
                        &(wan_ipv6_prefix_info.prefix_addr))->sin6_addr);
      /* If the prefix matches with WWAN Prefix. Ignore RA. */
      if (!memcmp(&prefix_info_ptr->ps_s6_addr64[0],
                  &prefix_addr_ptr->ps_s6_addr64[0],
                  sizeof(uint64)))
      {
        LOG_MSG_ERROR("Prefix matches with WWAN prefix when in cradle mode. Ignore RA.\n",
                      0, 0, 0);
        return false;
      }
    }
  }
  else if (this->sta_connected)
  {
    /* Get the Cradle/WWAN prefixes if available. */
    if ( this->GetUSBEnable() && this->usb_link == QCMAP_QTI_USB_LINK_ECM &&
         this->cfg.cradle_config.cradle_mode == QCMAP_MSGR_CRADLE_WAN_ROUTER_V01 )
    {
      strlcpy(devname,ECM_IFACE, QCMAP_MSGR_INTF_LEN);
      wan_ipv6_prefix_info.cache_info.ifa_prefered = IPV6_DEFAULT_PREFERED_LIFETIME;
      wan_ipv6_prefix_info.cache_info.ifa_valid = IPV6_DEFAULT_VALID_LIFETIME;
      this->GetIPV6PrefixInfo(devname,&(wan_ipv6_prefix_info));
      prefix_info_ptr = (struct ps_in6_addr *)\
                        &(((struct sockaddr_in6 *)\
                        &(wan_ipv6_prefix_info.prefix_addr))->sin6_addr);
      /* If the prefix matches with Cradle Prefix. Ignore RA. */
      if (!memcmp(&prefix_info_ptr->ps_s6_addr64[0],
          &prefix_addr_ptr->ps_s6_addr64[0],
          sizeof(uint64)))
      {
        LOG_MSG_ERROR("Prefix matches with Cradle prefix when in STA mode. Ignore RA.\n",
                      0, 0, 0);
        return false;
      }
    }
    if ((qcmap_cm_get_dev_name(this->qcmap_cm_handle,
         QCMAP_MSGR_IP_FAMILY_V6_V01, devname, &qcmap_cm_error)) ==
         QCMAP_CM_SUCCESS )
    {
      wan_ipv6_prefix_info.cache_info.ifa_prefered = IPV6_DEFAULT_PREFERED_LIFETIME;
      wan_ipv6_prefix_info.cache_info.ifa_valid = IPV6_DEFAULT_VALID_LIFETIME;
      this->GetIPV6PrefixInfo(devname,&(wan_ipv6_prefix_info));
      prefix_info_ptr = (struct ps_in6_addr *)\
                        &(((struct sockaddr_in6 *)\
                        &(wan_ipv6_prefix_info.prefix_addr))->sin6_addr);
      /* If the prefix matches with WWAN Prefix. Ignore RA. */
      if (!memcmp(&prefix_info_ptr->ps_s6_addr64[0],
                  &prefix_addr_ptr->ps_s6_addr64[0],
                  sizeof(uint64)))
      {
        LOG_MSG_ERROR("Prefix matches with WWAN prefix when in station mode. Ignore RA.\n",
                      0, 0, 0);
        return false;
      }
    }
  }
  else if ((qcmap_cm_get_dev_name(this->qcmap_cm_handle,
            QCMAP_MSGR_IP_FAMILY_V6_V01, devname, &qcmap_cm_error)) ==
            QCMAP_CM_SUCCESS )
  {
    /* Get the STA/Cradle prefixes if available. */
    if ( this->GetUSBEnable() && this->usb_link == QCMAP_QTI_USB_LINK_ECM &&
         this->cfg.cradle_config.cradle_mode == QCMAP_MSGR_CRADLE_WAN_ROUTER_V01 )
    {
      strlcpy(devname,ECM_IFACE, QCMAP_MSGR_INTF_LEN);
      wan_ipv6_prefix_info.cache_info.ifa_prefered = IPV6_DEFAULT_PREFERED_LIFETIME;
      wan_ipv6_prefix_info.cache_info.ifa_valid = IPV6_DEFAULT_VALID_LIFETIME;
      this->GetIPV6PrefixInfo(devname,&(wan_ipv6_prefix_info));
      prefix_info_ptr = (struct ps_in6_addr *)\
                       &(((struct sockaddr_in6 *)\
                       &(wan_ipv6_prefix_info.prefix_addr))->sin6_addr);
      /* If the prefix matches with STA Prefix. Ignore RA. */
      if (!memcmp(&prefix_info_ptr->ps_s6_addr64[0],
                  &prefix_addr_ptr->ps_s6_addr64[0],
                  sizeof(uint64)))
      {
        LOG_MSG_ERROR("Prefix matches with Cradle prefix when in WWAN mode. Ignore RA.\n",
                       0, 0, 0);
        return false;
      }
    }
    if ( this->wifi_mode == QCMAP_MSGR_WLAN_MODE_AP_STA_V01)
    {
      if (this->IsAPSTABridgeActivated())
      {
        strlcpy(devname,BRIDGE_IFACE, QCMAP_MSGR_INTF_LEN);
      }
      else
      {
        strlcpy(devname,this->cfg.lan_config.sta_interface, QCMAP_MSGR_INTF_LEN);
      }
      wan_ipv6_prefix_info.cache_info.ifa_prefered = IPV6_DEFAULT_PREFERED_LIFETIME;
      wan_ipv6_prefix_info.cache_info.ifa_valid = IPV6_DEFAULT_VALID_LIFETIME;
      this->GetIPV6PrefixInfo(devname,&(wan_ipv6_prefix_info));
      prefix_info_ptr = (struct ps_in6_addr *)\
                        &(((struct sockaddr_in6 *)\
                        &(wan_ipv6_prefix_info.prefix_addr))->sin6_addr);
      /* If the prefix matches with STA Prefix. Ignore RA. */
      if (!memcmp(&prefix_info_ptr->ps_s6_addr64[0],
                  &prefix_addr_ptr->ps_s6_addr64[0],
                  sizeof(uint64)))
      {
        LOG_MSG_ERROR("Prefix matches with STA prefix when in WWAN mode. Ignore RA.\n",
                      0, 0, 0);
        return false;
      }
    }
  }
  return true;
}
/*===========================================================================
  FUNCTION AssignGlobalV6AddrAllClients
==========================================================================*/
/*!
@brief
  Prepend the Ipv6 prefix to the Link local address for all clients

@parameters
  struct ps_in6_addr *prefix_addr_ptr - To get the Ipv6 prefix

@return
  bool

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
bool QCMAP_ConnectionManager::AssignGlobalV6AddrAllClients
(
  struct ps_in6_addr *prefix_addr_ptr
)
{
  ds_dll_el_t * node = NULL;
  qcmap_cm_client_data_info_t* nodeInfo = NULL;
  qcmap_addr_info_list_t* addrList = &(this->addrList);
  struct ps_in6_addr *iid_ptr = NULL;
  struct ps_in6_addr *global_addr_ptr = NULL;
  const void   *dummy = NULL;

  if ( addrList->addrListHead == NULL)
  {
    LOG_MSG_ERROR("AssignGlobalV6AddrAllClients - Linked list head is NULL \n",
                  0, 0, 0);
    return false;
  }

  node = addrList->addrListHead->next;
  if ( node == NULL)
  {
    LOG_MSG_ERROR("AssignGlobalV6AddrAllClients - Linked list node is NULL",
                  0, 0, 0);
    return false;
  }

  /*  Update Global v6 address for all the client with the prefix*/
  while(node != NULL)
  {
    nodeInfo = ( qcmap_cm_client_data_info_t* )ds_dll_data(node);

    iid_ptr = (struct ps_in6_addr *)nodeInfo->link_local_v6_addr;
    global_addr_ptr = (struct ps_in6_addr *)nodeInfo->ip_v6_addr;

    global_addr_ptr->ps_s6_addr64[0] = prefix_addr_ptr->ps_s6_addr64[0];
    global_addr_ptr->ps_s6_addr64[1] = iid_ptr->ps_s6_addr64[1];

    if ((nodeInfo->device_type == QCMAP_MSGR_DEVICE_TYPE_USB_V01) &&
        (this->cfg.lan_config.usb_conf.ppp_enabled))
    {
      memcpy( this->ppp_ipv6_addr,
              nodeInfo->ip_v6_addr, QCMAP_MSGR_IPV6_ADDR_LEN_V01 );
      this->AddPPPIPv6Route();
    }
    node = ds_dll_next(node, &dummy);
  }
  return true;
}

/*===========================================================================
  FUNCTION AddPPPIPv6Route
==========================================================================*/
/*!
@brief
  Adds the route for PPP interface.

@parameters
  void

@return
  void

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
void QCMAP_ConnectionManager::AddPPPIPv6Route()
{
  unsigned char ip6_addr[MAX_IPV6_PREFIX+1];   // Getting IPv6 Address
  char command[MAX_COMMAND_STR_LEN];

  /* Flush the existing routes. */
  snprintf( command, MAX_COMMAND_STR_LEN,
            "ip -6 route flush dev %s proto boot", PPP_IFACE);
  ds_system_call( command, strlen(command));

  inet_ntop( AF_INET6,(struct in6_addr *)this->ppp_ipv6_addr, ip6_addr,
             MAX_IPV6_PREFIX);

  snprintf( command, MAX_COMMAND_STR_LEN,
            "ip -6 route add %s dev %s", ip6_addr, PPP_IFACE);
  ds_system_call( command, strlen(command));

  return;
}
/*=====================================================
  FUNCTION StopHostapdCli
======================================================*/
/*!
@brief
 Kills appropriate hostapd_cli process

@parameters
  - qcmap_msgr_activate_hostapd_ap_enum_v01 ap_type

@return
  void

@note

- Dependencies
- None

- Side Effects
- None

/*=====================================================*/
void QCMAP_ConnectionManager::StopHostapdCli
(
  qcmap_msgr_activate_hostapd_ap_enum_v01 ap_type
)
{
  char command[MAX_COMMAND_STR_LEN];

  LOG_MSG_INFO1("QCMAP_ConnectionManager::StopHostapdCli", 0, 0, 0);
  switch ( ap_type )
  {
    case QCMAP_MSGR_PRIMARY_AP_V01:
    {
      snprintf(command,
               MAX_COMMAND_STR_LEN,
               "kill -15 `ps -ef | grep hostapd_cli | grep wlan%d"
               "| awk '{print $1}'`",this->ap_dev_num);
      ds_system_call(command, strlen(command));
    }
    break;

    case QCMAP_MSGR_GUEST_AP_V01:
    {
      snprintf(command,
               MAX_COMMAND_STR_LEN,
               "kill -15 `ps -ef | grep hostapd_cli | grep wlan%d"
               "| awk '{print $1}'`",this->ap_dev_num+1);
      ds_system_call(command, strlen(command));
    }
    break;

    case QCMAP_MSGR_PRIMARY_AND_GUEST_AP_V01:
    {
      /*Kill the hostapd_cli */
      snprintf(command,
               MAX_COMMAND_STR_LEN,
               "killall hostapd_cli");
      ds_system_call(command, strlen(command));
    }
    break;
    default:
    {
       LOG_MSG_ERROR("QCMAP_ConnectionManager::StopHostapdCli-Invalid ap type %d ",
                     ap_type, 0, 0);
    }
    break;
  }
}

/*=====================================================
  FUNCTION StartHostapdCli
======================================================*/
/*!
@brief
 Start appropriate hostapd_cli process

@parameters
  - qcmap_msgr_activate_hostapd_ap_enum_v01 ap_type

@return
  void

@note

- Dependencies
- None

- Side Effects
- None
*/
/*=====================================================*/
void QCMAP_ConnectionManager::StartHostapdCli
(
  qcmap_msgr_activate_hostapd_ap_enum_v01 ap_type
)
{
  char command[MAX_COMMAND_STR_LEN];

  LOG_MSG_INFO1("QCMAP_ConnectionManager::StartHostapdCli", 0, 0, 0);

  switch ( ap_type )
  {
    case QCMAP_MSGR_PRIMARY_AP_V01:
    {
      snprintf(command,
               MAX_COMMAND_STR_LEN,
               "hostapd_cli -i wlan%d -p /var/run/hostapd -B -a %s",
               this->ap_dev_num, STA_INTERFACE_CTRL_PATH);
      ds_system_call(command, strlen(command));
    }
    break;

    case QCMAP_MSGR_GUEST_AP_V01:
    {
      snprintf(command,
               MAX_COMMAND_STR_LEN,
               "hostapd_cli -i wlan%d -p /var/run/hostapd -B -a %s",
               this->ap_dev_num+1, STA_INTERFACE_CTRL_PATH);
      ds_system_call(command, strlen(command));
    }
    break;

    case QCMAP_MSGR_PRIMARY_AND_GUEST_AP_V01:
    {
      snprintf(command,
               MAX_COMMAND_STR_LEN,
               "hostapd_cli -i wlan%d -p /var/run/hostapd -B -a %s",
               this->ap_dev_num, STA_INTERFACE_CTRL_PATH);
      ds_system_call(command, strlen(command));

      snprintf(command,
               MAX_COMMAND_STR_LEN,
               "hostapd_cli -i wlan%d -p /var/run/hostapd -B -a %s",
               this->ap_dev_num+1, STA_INTERFACE_CTRL_PATH);
      ds_system_call(command, strlen(command));
    }
    break;

    default:
    {
      LOG_MSG_ERROR("QCMAP_ConnectionManager::StartHostapdCli-Invalid ap type %d",
                    ap_type, 0, 0);
    }
    break;
  }
}

/*===========================================================================
  FUNCTION GetPrefixDelegationConfig
==========================================================================*/
/*!
@brief
  Retreives the Prefix Delegation config.

@parameters
  char *process

@return
  true  - Prefix Delegation config set
  flase - Prefix Delegation config not set

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetPrefixDelegationConfig
(
  boolean *pd_mode,
  qmi_error_type_v01 *qmi_err_num
)
{
  if ( qmi_err_num == NULL )
  {
    LOG_MSG_ERROR("NULL QMI error pointer.\n",0,0,0);
    return false;
  }

  if (pd_mode == NULL)
  {
    LOG_MSG_ERROR("NULL Args.\n", 0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
    return false;
  }

  *pd_mode = this->cfg.wan_config.prefix_delegation;
  return true;
}

/*===========================================================================
  FUNCTION SetPrefixDelegationConfig
==========================================================================*/
/*!
@brief
  Enable/disable the Prefix Delegation config.

@parameters
  char *process

@return
  true  - Prefix Delegation config set
  flase - Prefix Delegation config not set

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::SetPrefixDelegationConfig
(
  boolean pd_mode,
  qmi_error_type_v01 *qmi_err_num
)
{
  if (qmi_err_num == NULL)
  {
    LOG_MSG_ERROR("NULL Arg.\n", 0,0,0);
     *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
    return false;
  }

  if (pd_mode == this->cfg.wan_config.prefix_delegation)
  {
    LOG_MSG_ERROR("Prefix delegation config requested is already set.\n", 0,0,0);
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    return false;
  }

  this->cfg.wan_config.prefix_delegation = pd_mode;
  this->WriteConfigToXML();

  if ( qcmap_cm_get_ipv6_state(qmi_err_num) == QCMAP_CM_V6_WAN_CONNECTED )
  {
    LOG_MSG_ERROR("Prefix delegation config set, but v6 call is active. Will "
                  "take effect on next Call\n",0,0,0);
    *qmi_err_num = QMI_ERR_DEVICE_IN_USE_V01;
    return false;
  }

  return true;
}

/*===========================================================================
  FUNCTION GetPrefixDelegationStatus
==========================================================================*/
/*!
@brief
  Retreives the current Prefix Delegation mode.

@parameters
  char *process

@return
  true  - Prefix Delegation mode set
  flase - Prefix Delegation mode not set

@note
- Dependencies
- None

- Side Effects
- None
*/
/*=========================================================================*/
boolean QCMAP_ConnectionManager::GetPrefixDelegationStatus
(
  boolean *pd_mode,
  qmi_error_type_v01 *qmi_err_num
)
{
  if ( qmi_err_num == NULL )
  {
    LOG_MSG_ERROR("NULL QMI error pointer.\n",0,0,0);
    return false;
  }

  if (pd_mode == NULL)
  {
    LOG_MSG_ERROR("NULL Args.\n", 0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_ARG_V01;
    return false;
  }

  *pd_mode = false;

  /* If WAN v6 call is disconnected or STA/Cradle is active, we return true,
     but pd_mode is always false */
  if ( qcmap_cm_get_ipv6_state(qmi_err_num) != QCMAP_CM_V6_WAN_CONNECTED )
  {
    LOG_MSG_ERROR("Prefix delegation can only be enabled when a v6 call is active\n", 0,0,0);
    return true;
  }

  /* We only have control for Prefix Delegation over WAN */
  if (this->sta_connected || this->cradle_backhaul_connected)
  {
    LOG_MSG_ERROR("STA/Cradle connected, Prefix Delegation query invalid\n", 0,0,0);
    return true;
  }

  *pd_mode = prefix_delegation_activated;
  return true;
}
