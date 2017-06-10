#ifndef _QCMAP_CLIENT_H_
#define _QCMAP_CLIENT_H_

/*====================================================

FILE:  QCMAP_Client.h

SERVICES:
   QCMAP Client Class

=====================================================

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

=====================================================*/
/*===========================================================================
  EDIT HISTORY FOR MODULE

  Please notice that the changes are listed in reverse chronological order.

  when       who        what, where, why
  --------   ---        -------------------------------------------------------
  07/11/12   gk         Created module.
  10/26/12   cp         Added support for Dual AP and different types of NAT.
  02/27/13   cp         Added support to get IPV6 WAN status.
  04/17/13   mp         Added support to get IPv6 WWAN/STA mode configuration.
  06/12/13   sg         Added support for DHCP Reservation
  01/11/14   sr         Added support for connected devices information in SoftAP
  02/24/14   vm         Changes to Enable/Disable Station Mode to be in
                        accordance with IoE 9x15
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
#include "qualcomm_mobile_access_point_msgr_v01.h"
#include "qcmap_client_util.h"
#define QCMAP_MSGR_MAX_FILE_PATH_LEN 100
#define MAX_PORT_VALUE           65535
#define MAX_PROTO_VALUE          255
#define MAX_TOS_VALUE          255

#ifdef __cplusplus
extern "C"
{
#endif

#include "qmi_client.h"

#ifdef __cplusplus
}
#endif

/*
Log Message Macros
*/
#define LOG_MSG_INFO1_LEVEL           MSG_LEGACY_MED
#define LOG_MSG_INFO2_LEVEL           MSG_LEGACY_MED
#define LOG_MSG_INFO3_LEVEL           MSG_LEGACY_LOW
#define LOG_MSG_ERROR_LEVEL           MSG_LEGACY_ERROR
#define PRINT_MSG( level, fmtString, x, y, z)                         \
        MSG_SPRINTF_4( MSG_SSID_LINUX_DATA, level, "%s(): " fmtString,      \
                       __FUNCTION__, x, y, z);
#define LOG_MSG_INFO1( fmtString, x, y, z)                            \
{                                                                     \
  PRINT_MSG( LOG_MSG_INFO1_LEVEL, fmtString, x, y, z);                \
}
#define LOG_MSG_INFO2( fmtString, x, y, z)                            \
{                                                                     \
  PRINT_MSG( LOG_MSG_INFO2_LEVEL, fmtString, x, y, z);                \
}
#define LOG_MSG_INFO3( fmtString, x, y, z)                            \
{                                                                     \
  PRINT_MSG( LOG_MSG_INFO3_LEVEL, fmtString, x, y, z);                \
}
#define LOG_MSG_ERROR( fmtString, x, y, z)                            \
{                                                                     \
  PRINT_MSG( LOG_MSG_ERROR_LEVEL, fmtString, x, y, z);                \
}

typedef void (*client_status_ind_t)
(
  qmi_client_type user_handle,                    /* QMI user handle       */
  unsigned int    msg_id,                         /* Indicator message ID  */
  void           *ind_buf,                        /* Raw indication data   */
  unsigned int    ind_buf_len,                    /* Raw data length       */
  void           *ind_cb_data                     /* User call back handle */
);

typedef struct
{
  in_addr public_ip;
  in_addr primary_dns;
  in_addr secondary_dns;
}v4_conf_t;

typedef struct
{
  struct in6_addr public_ip_v6;
  struct in6_addr primary_dns_v6;
  struct in6_addr secondary_dns_v6;
}v6_conf_t;

typedef union
{
  v4_conf_t v4_conf;
  v6_conf_t v6_conf;
}qcmap_nw_params_t;

//===================================================================
//              Class Definitions
//===================================================================

class QCMAP_Client
{
public:

   QCMAP_Client(client_status_ind_t client_cb_ind);
   virtual ~QCMAP_Client(void);
   /* ---------------------------MobileAP Execution---------------------------*/

   /* Enable MobileAP */
   boolean EnableMobileAP(qmi_error_type_v01 *qmi_err_num);

   /* Disable MobileAP */
   boolean DisableMobileAP(qmi_error_type_v01 *qmi_err_num);

   /* Enable IPV4 */
   boolean EnableIPV4(qmi_error_type_v01 *qmi_err_num);

   /* Disable IPV4 */
   boolean DisableIPV4(qmi_error_type_v01 *qmi_err_num);


   /* Enable IPV6 */
   boolean EnableIPV6(qmi_error_type_v01 *qmi_err_num);

   /* Disable IPV6 */
   boolean DisableIPV6(qmi_error_type_v01 *qmi_err_num);

   /* ConnectBackHaul */
   boolean ConnectBackHaul(qcmap_msgr_wwan_call_type_v01 call_type, qmi_error_type_v01 *qmi_err_num);

   /* DisconnectBackHual */
   boolean DisconnectBackHaul(qcmap_msgr_wwan_call_type_v01 call_type, qmi_error_type_v01 *qmi_err_num);

  /* Bring up Linux LAN. */
   boolean EnableWLAN(qmi_error_type_v01 *qmi_err_num, boolean privileged_client = false);

   /* Bring down Linux LAN. */
   boolean DisableWLAN(qmi_error_type_v01 *qmi_err_num, boolean privileged_client = false);

  /* Set WLAN Config. */
  boolean SetWLANConfig(qcmap_msgr_wlan_mode_enum_v01 wlan_mode,
                                     qcmap_msgr_access_profile_v01 guest_ap_acess_profile,
                                     qcmap_msgr_station_mode_config_v01 station_config,
                                     qmi_error_type_v01 *qmi_err_num);

  /* Get  WLAN Configuation*/
  boolean GetWLANConfig(qcmap_msgr_wlan_mode_enum_v01 *wlan_mode,
                                     qcmap_msgr_access_profile_v01 *guest_ap_acess_profile,
                                     qcmap_msgr_station_mode_config_v01 *station_config,
                                     qmi_error_type_v01 *qmi_err_num);

  /* Actiavte WLAN Request. */
  boolean ActivateWLAN(qmi_error_type_v01 *qmi_err_num);

  /* Set LAN Config. */
  boolean SetLANConfig(qcmap_msgr_lan_config_v01 lan_config,
                                    qmi_error_type_v01 *qmi_err_num);

  /* Get  LAN Configuation*/
  boolean GetLANConfig(qcmap_msgr_lan_config_v01 *lan_config,
                                     qmi_error_type_v01 *qmi_err_num);

  /* Actiavte LAN Request. */
  boolean ActivateLAN(qmi_error_type_v01 *qmi_err_num);

  /* Get the Current WLAN Status. */
  boolean GetWLANStatus(qcmap_msgr_wlan_mode_enum_v01 *wlan_mode, qmi_error_type_v01 *qmi_err_num);

  /* Actiavte Hostapd Config. */
  boolean ActivateHostapdConfig(qcmap_msgr_activate_hostapd_ap_enum_v01 ap_type,
                                qcmap_msgr_activate_hostapd_action_enum_v01 action_type,
                                qmi_error_type_v01 *qmi_err_num);

  /* Actiavte Supplicant Config Request. */
  boolean ActivateSupplicantConfig(qmi_error_type_v01 *qmi_err_num);

   /* Add SNAT entry. */
   boolean AddStaticNatEntry(qcmap_msgr_snat_entry_config_v01 *snat_entry, qmi_error_type_v01 *qmi_err_num);
   /* Delete SNAT entry. */
   boolean DeleteStaticNatEntry(qcmap_msgr_snat_entry_config_v01 *snat_entry, qmi_error_type_v01 *qmi_err_num);

   /* Get SNAT config from Connection Manager. */
   boolean GetStaticNatConfig(qcmap_msgr_snat_entry_config_v01 *snat_config, int *num_entries, qmi_error_type_v01 *qmi_err_num);

   /* Add DMZ. */
   boolean AddDMZ(uint32 dmz_ip, qmi_error_type_v01 *qmi_err_num);
   /* Delete DMZ. */
   boolean DeleteDMZ(qmi_error_type_v01 *qmi_err_num);

   /* Get DMZ from ConnectionManager. */
   boolean GetDMZ(uint32_t *dmz_ip, qmi_error_type_v01 *qmi_err_num);

   /* Get WWAN Statistics. */
   boolean GetWWANStatistics(qcmap_msgr_ip_family_enum_v01 ip_family,
                             qcmap_msgr_wwan_statistics_type_v01 *wwan_stats,
                             qmi_error_type_v01 *qmi_err_num);

   /* Reset WWAN Statistics. */
   boolean ResetWWANStatistics(qcmap_msgr_ip_family_enum_v01 ip_family, qmi_error_type_v01 *qmi_err_num);

   /* Enable/Disable IPSEC VPN Passthrough */
   boolean SetIPSECVpnPassthrough(boolean enable, qmi_error_type_v01 *qmi_err_num);
   boolean GetIPSECVpnPassthrough(boolean *enable, qmi_error_type_v01 *qmi_err_num);

   /* Enable/Disable PPTP VPN Passthrough */
   boolean SetPPTPVpnPassthrough(boolean enable, qmi_error_type_v01 *qmi_err_num);
   boolean GetPPTPVpnPassthrough(boolean *enable, qmi_error_type_v01 *qmi_err_num);

   /* Enable/Disable L2TP VPN Passthrough */
   boolean SetL2TPVpnPassthrough(boolean enable, qmi_error_type_v01 *qmi_err_num);
   boolean GetL2TPVpnPassthrough(boolean *enable, qmi_error_type_v01 *qmi_err_num);

   /* Enable/Disable Webserver WWAN Access */
   boolean SetWebserverWWANAccess(boolean enable, qmi_error_type_v01 *qmi_err_num);
   boolean GetWebserverWWANAccess(boolean *enable, qmi_error_type_v01 *qmi_err_num);

   /* Set/Get NAT Type Symmetric/Port Restricted NAT */
   boolean SetNatType(qcmap_msgr_nat_enum_v01 nat_type, qmi_error_type_v01 *qmi_err_num);
   boolean GetNatType(qcmap_msgr_nat_enum_v01 *nat_type, qmi_error_type_v01 *qmi_err_num);

   /* Enable/Disable Autoconnect mode */
   boolean SetAutoconnect(boolean enable, qmi_error_type_v01 *qmi_err_num);
   boolean GetAutoconnect(boolean *enable, qmi_error_type_v01 *qmi_err_num);

   /* Enable/Disable Roaming mode */
   boolean SetRoaming(boolean enable, qmi_error_type_v01 *qmi_err_num);
   boolean GetRoaming(boolean *enable, qmi_error_type_v01 *qmi_err_num);

   boolean GetNetworkConfiguration
                                   (
                                     qcmap_msgr_ip_family_enum_v01 ip_family,
                                     qcmap_nw_params_t *qcmap_nw_params,
                                     qmi_error_type_v01 *qmi_err_num
                                   );

   boolean GetIPv4NetworkConfiguration
                                          (
                                            in_addr_t *public_ip,
                                            uint32 *primary_dns,
                                            in_addr_t *secondary_dns,
                                            qmi_error_type_v01 *qmi_err_num
                                          );
   boolean GetIPv6NetworkConfiguration
                                          (
                                            struct in6_addr *public_ip,
                                            struct in6_addr *primary_dns,
                                            struct in6_addr *secondary_dns,
                                            qmi_error_type_v01 *qmi_err_num
                                          );

/* Set/Get NAT Timeout */
   boolean SetNatTimeout(qcmap_msgr_nat_timeout_enum_v01 timeout_type, uint32 timeout_value, qmi_error_type_v01 *qmi_err_num);
   boolean GetNatTimeout(qcmap_msgr_nat_timeout_enum_v01 timeout_type, uint32 *timeout_value, qmi_error_type_v01 *qmi_err_num);

   /* QMI QCMAP_CM service info */
   qmi_client_type           qmi_qcmap_msgr_handle;
   qmi_client_type           qmi_qcmap_msgr_notifier;
   qmi_cci_os_signal_type    qmi_qcmap_msgr_os_params;

   boolean GetFireWallHandlesList(qcmap_msgr_get_firewall_handle_list_conf_t *handlist, qmi_error_type_v01 *qmi_err_num);


   /*Add an Extended Firewall rule to the configuration*/
   boolean AddFireWallEntry(qcmap_msgr_firewall_conf_t *extd_firewall_entry, qmi_error_type_v01 *qmi_err_num);

    /*Get an Extended Firewall rule from the configuration*/
   boolean GetFireWallEntry(qcmap_msgr_firewall_entry_conf_t *firewall_entry, qmi_error_type_v01 *qmi_err_num);
   /*Delete extended firewall rule from the configuration*/
   int DeleteFireWallEntry(int handle, qmi_error_type_v01  *qmi_err_num);
   boolean GetMobileAPStatus(qcmap_msgr_mobile_ap_status_enum_v01 *status, qmi_error_type_v01 *qmi_err_num);
   boolean GetWWANStatus(qcmap_msgr_wwan_status_enum_v01 *v4_status,
                         qcmap_msgr_wwan_status_enum_v01 *v6_status, qmi_error_type_v01 *qmi_err_num);
   boolean GetStationModeStatus(qcmap_msgr_station_mode_status_enum_v01 *status, qmi_error_type_v01 *qmi_err_num);
   boolean SetFirewall(boolean enable_firewall, boolean pkts_allowed, qmi_error_type_v01 *qmi_err_num);
   boolean GetFirewall(boolean *enable_firewall, boolean *pkts_allowed, qmi_error_type_v01 *qmi_err_num);
  /* get IPv4 state:Enabled, Disbaled. */
   boolean GetIPv4State (boolean *ipv4_state , qmi_error_type_v01 *qmi_err_num);
  /* get IPv6 state:Enabled, Disbaled. */
   boolean GetIPv6State (boolean *ipv6_state , qmi_error_type_v01 *qmi_err_num);
   /* get WWAN Policy. */
   boolean GetWWANPolicy(qcmap_msgr_net_policy_info_v01 *WWAN_config, qmi_error_type_v01 *qmi_err_num);
   /* Set WWAN Policy. */
   boolean SetWWANPolicy(qcmap_msgr_net_policy_info_v01 WWAN_config , qmi_error_type_v01 *qmi_err_num);

   /* Enable/Disable/Get UPNP. */
   boolean EnableUPNP(qmi_error_type_v01 *qmi_err_num);
   boolean DisableUPNP(qmi_error_type_v01 *qmi_err_num);
   boolean GetUPNPStatus(qcmap_msgr_upnp_mode_enum_v01 *upnp_status, qmi_error_type_v01 *qmi_err_num);

   /* Enable/Disable/Get DLNA. */
   boolean EnableDLNA(qmi_error_type_v01 *qmi_err_num);
   boolean DisableDLNA(qmi_error_type_v01 *qmi_err_num);
   boolean GetDLNAStatus(qcmap_msgr_dlna_mode_enum_v01 *upnp_status, qmi_error_type_v01 *qmi_err_num);
   boolean GetDLNAMediaDir(char media_dir[], qmi_error_type_v01 *qmi_err_num);
   boolean SetDLNAMediaDir(char media_dir[], qmi_error_type_v01 *qmi_err_num);

   /* Enable/Disable/Get MDNS. */
   boolean EnableMDNS(qmi_error_type_v01 *qmi_err_num);
   boolean DisableMDNS(qmi_error_type_v01 *qmi_err_num);
   boolean GetMDNSStatus(qcmap_msgr_mdns_mode_enum_v01 *upnp_status, qmi_error_type_v01 *qmi_err_num);

   /* Set/Get QCMAP Bootup Configuration */
   boolean SetQCMAPBootupCfg(qcmap_msgr_bootup_flag_v01 mobileap_enable, qcmap_msgr_bootup_flag_v01 wlan_enable, qmi_error_type_v01 *qmi_err_num);
   boolean GetQCMAPBootupCfg(qcmap_msgr_bootup_flag_v01 *mobileap_enable, qcmap_msgr_bootup_flag_v01 *wlan_enable, qmi_error_type_v01 *qmi_err_num);

   boolean GetDataRate(qcmap_msgr_data_bitrate_v01 *data_rate, qmi_error_type_v01 *qmi_err_num);

   /* Set/Get the UPnP SSDP notify interval rate */
   boolean SetUPNPNotifyInterval(int notify_int, qmi_error_type_v01 *qmi_err_num);
   boolean GetUPNPNotifyInterval(int *notify_int, qmi_error_type_v01 *qmi_err_num);

   /* Set/Get the DLNA SSDP notify interval rate */
   boolean SetDLNANotifyInterval(int notify_int, qmi_error_type_v01 *qmi_err_num);
   boolean GetDLNANotifyInterval(int *notify_int, qmi_error_type_v01 *qmi_err_num);
  /*Add/Get/Edit/Display DHCP Reservation*/
   boolean  AddDHCPReservRecord(qcmap_msgr_dhcp_reservation_v01 *dhcp_reserv_record, qmi_error_type_v01 *qmi_err_num);
   boolean  GetDHCPReservRecords(qcmap_msgr_dhcp_reservation_v01 *dhcp_reserv_record,uint32_t *num_entries,\
      qmi_error_type_v01 *qmi_err_num);
   boolean  EditDHCPReservRecord(uint32_t *addr,qcmap_msgr_dhcp_reservation_v01 *dhcp_reserv_record, qmi_error_type_v01 *qmi_err_num);
   boolean  DeleteDHCPReservRecord(uint32_t *addr, qmi_error_type_v01 *qmi_err_num);

   uint32_t                  mobile_ap_handle;

   /* Enable  ALGs. */
   boolean EnableAlg(qcmap_msgr_alg_type_mask_v01 alg_types, qmi_error_type_v01 *qmi_err_num);

   /* Disable  ALGs. */
   boolean DisableAlg(qcmap_msgr_alg_type_mask_v01 alg_types, qmi_error_type_v01 *qmi_err_num);

   /*Set SIP server information*/
   boolean SetSIPServerInfo(qcmap_msgr_sip_server_info_v01 *sip_server_info,
                            qmi_error_type_v01 *qmi_err_num);

   /*Get SIP server information*/
   boolean GetSIPServerInfo(qcmap_msgr_sip_server_info_v01 *default_sip_info,
                            qcmap_msgr_sip_server_info_v01 *network_sip_info,
                            int *count_network_sip_info,
                            qmi_error_type_v01 *qmi_err_num);

   /*Get IPV6 SIP server information*/
   boolean GetV6SIPServerInfo(qcmap_msgr_ipv6_sip_server_info_v01 *network_v6_sip_info,
                              int *count_network_sip_info,
                              qmi_error_type_v01 *qmi_err_num);

   /* Restore device to factory configuration */
   boolean RestoreFactoryConfig(qmi_error_type_v01 *qmi_err_num);

   /*Get Connected Devices information*/
   boolean GetConnectedDevicesInfo (qcmap_msgr_connected_device_info_v01 *conn_dev_info,
                                    int *num_entries, qmi_error_type_v01 *qmi_err_num);

   /* Enable/Disable WPASupplicant Config. */
   boolean SetSupplicantConfig(boolean status,
                               qmi_error_type_v01 *qmi_err_num);

   /* Get/Set Cradle Mode */
   boolean GetCradleMode(qcmap_msgr_cradle_mode_v01 *mode, qmi_error_type_v01 *qmi_err_num);
   boolean SetCradleMode(qcmap_msgr_cradle_mode_v01 mode, qmi_error_type_v01 *qmi_err_num);

   /* Get/Set Prefix Delegation Mode */
   boolean GetPrefixDelegationConfig(boolean *pd_mode, qmi_error_type_v01 *qmi_err_num);
   boolean SetPrefixDelegationConfig(boolean pd_mode, qmi_error_type_v01 *qmi_err_num);
   boolean GetPrefixDelegationStatus(boolean *pd_mode, qmi_error_type_v01 *qmi_err_num);

private:

   /* QCMAP CM Config */
   boolean                     qcmap_msgr_enable;


};

#endif
