#ifndef _QCMAP_CM_API_H_
#define _QCMAP_CM_API_H_

/******************************************************************************

                           QCMAP_CM_API.H

******************************************************************************/

/******************************************************************************

  @file    qcmap_cm_api.h
  @brief   Mobile AP Connection Manager Lib API

  DESCRIPTION
  Header file for Mobile AP Connection Manager Lib.

  ---------------------------------------------------------------------------
  Copyright (c) 2011-2013 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        -------------------------------------------------------
02/20/14   at         Fix for IPv6 address accumulation on rmnet interface.
11/08/13   pm         Removed depencency on mcm.
04/11/13   mp         Added support for DUN call over MobileAP.
10/03/12   mp         Fix to prevent QCMAP starting call before QTI Link UP.
10/03/12   mp         Fix to make QTI use QCMAP WWAN config.
09/11/12   mp         Added support for dualAPN in MobileAP.
08/03/12   mp         Added support for Port Restricted Cone NAT.
07/07/12   sc         Added support for tethering call bringup.
07/02/12   vb         Added MobileAP events for embedded call.
05/18/12   vb         Added support for embedded call bringup.
05/04/12   cp         Added static IP configuration for STA mode.
04/16/12   sb         IPV6 state machine fix
03/09/12   sb         Adding extended firewall support
03/03/12   ss         Adding support for Concurrent STA+AP mode.
03/01/12   SDT        Adding support for Dual AP mode.
02/20/12   SDT        Adding support for IPv6.

******************************************************************************/

/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/

#include "comdef.h"
#include "dssocket.h"
#include "ps_ipfltr_defs.h"
#include "ps_iface.h"
#include "qualcomm_mobile_access_point_v01.h"
#include "dsi_netctrl.h"
#include <linux/if_addr.h>
#include <sys/un.h>
#include <sys/socket.h>

#define DSI_IP_FAMILY_4   "IP"
#define DSI_IP_FAMILY_6   "IPV6"
#define DSI_IP_FAMILY_4_6 "IPV4V6"
#define QCMAP_DSI_UDS_FILE "/etc/qcmap_dsi_uds_file"

#define QCMAP_CM_QMI_TIMEOUT_VALUE     15000
#define QCMAP_COEX_TIMEOUT_VALUE       1000
#define QCMAP_COEX_MAX_RETRY           5
#define QCMAP_QTI_MSG_TIMEOUT_S        2

typedef enum
{
  QCMAP_CM_ADD_SNAT                    = 0x01,
  QCMAP_CM_GET_SNAT                    = 0x02,
  QCMAP_CM_DELETE_SNAT                 = 0x03
} qcmap_cm_snat_req_enum_type;

typedef enum
{
  QCMAP_CM_ADD_FIREWALL_ENTRY_RULE     = 0x01,
  QCMAP_CM_GET_FIREWALL_ENTRY_RULE     = 0x02,
  QCMAP_CM_DELETE_FIREWALL_ENTRY_RULE  = 0x03,
  QCMAP_CM_ADD_EXTD_FIREWALL_ENTRY_RULE = 0x04,
  QCMAP_CM_GET_EXTD_FIREWALL_ENTRY_RULE = 0x05,
  QCMAP_CM_GET_FIREWALL_HANDLE_LIST = 0x06,
  QCMAP_CM_DELETE_EXTD_FIREWALL_ENTRY_RULE = 0x07
} qcmap_cm_firewall_entry_req_enum_type;

typedef enum
{
  QCMAP_CM_ENABLE_FIREWALL             = 0x01,
  QCMAP_CM_GET_FIREWALL                = 0x02,
  QCMAP_CM_DISABLE_FIREWALL            = 0x03
} qcmap_cm_firewall_req_enum_type;

typedef enum
{
  QCMAP_CM_ADD_DMZ                     = 0x01,
  QCMAP_CM_GET_DMZ                     = 0x02,
  QCMAP_CM_DELETE_DMZ                  = 0x03
} qcmap_cm_dmz_req_enum;

typedef enum
{
  QCMAP_CM_SET_IPSEC                   = 0x01,
  QCMAP_CM_GET_IPSEC                   = 0x02,
  QCMAP_CM_SET_L2TP                    = 0x03,
  QCMAP_CM_GET_L2TP                    = 0x04,
  QCMAP_CM_SET_PPTP                    = 0x05,
  QCMAP_CM_GET_PPTP                    = 0x06
}qcmap_cm_vpn_req_enum;

typedef enum
{
  QCMAP_CM_SET_NAT_ENTRY_TIMEOUT       = 0x01,
  QCMAP_CM_GET_NAT_ENTRY_TIMEOUT       = 0x02
} qcmap_cm_timeout_req_enum;

typedef enum
{
  QCMAP_CM_INTF_PRIMARY_INDEX          = 0x00,
  QCMAP_CM_INTF_SSID2_INDEX            = 0x01,
  QCMAP_CM_INTF_SSID3_INDEX            = 0x02
} qcmap_cm_intf_index_type;

typedef enum
{
  QCMAP_CM_PROFILE_FULL_ACCESS         = 0x01,
  QCMAP_CM_PROFILE_INTERNET_ONLY       = 0x02
} qcmap_cm_access_profile_type;

typedef enum
{
  QCMAP_CM_DEVMODE_AP                  = 0x01,
  QCMAP_CM_DEVMODE_STA                 = 0x02
} qcmap_cm_devmode_type;

typedef enum
{
  QCMAP_CM_WIFI_MODE_NOT_SET,
  QCMAP_CM_WIFI_AP_MODE,
  QCMAP_CM_WIFI_AP_AP_MODE,
  QCMAP_CM_WIFI_AP_STA_MODE,
  QCMAP_CM_WIFI_AP_AP_STA_MODE
} qcmap_cm_wifi_mode_type;

typedef enum
{
  QCMAP_CM_SYMMETRIC_NAT               = 0x00,
  QCMAP_CM_PORT_RESTRICTED_CONE_NAT    = 0x01
} qcmap_cm_nat_type;

/*===========================================================================

                          PUBLIC DATA DECLARATIONS

===========================================================================*/

typedef struct {
  dsi_hndl_t handle;
  const char* tech;
  const char* family;
  int profile;
} dsi_call_info_t;

typedef struct qcmap_dsi_buffer_s {
  dsi_hndl_t            dsi_nethandle;
  void                  *user_data;
  dsi_net_evt_t         evt;
  dsi_evt_payload_t     *payload_ptr;
} qcmap_dsi_buffer_t;

/*-------------------------------------------------------------------------
                            MobileAP Event Buffer
--------------------------------------------------------------------------*/

typedef struct mcm_mobileap_event_buffer_s {
  int            qcmap_handle;
  uint32         evt;
  void *user_data;
} mcm_mobileap_event_buffer_t;





/*---------------------------------------------------------------------------
                     Port Forwarding Entry Request Type.
---------------------------------------------------------------------------*/
typedef qcmap_cm_snat_req_enum_type qcmap_cm_snat_req_e;


/*---------------------------------------------------------------------------
           Port Forwarding Entry Configuration.
---------------------------------------------------------------------------*/
typedef struct
{
  uint32   port_fwding_private_ip;
  uint16   port_fwding_private_port;
  uint16   port_fwding_global_port;
  uint8    port_fwding_protocol;
} qcmap_cm_port_fwding_entry_conf_t;

/*---------------------------------------------------------------------------
           FireWall Entry Request Type.
---------------------------------------------------------------------------*/
typedef qcmap_cm_firewall_entry_req_enum_type qcmap_cm_firewall_entry_req_e;

/*---------------------------------------------------------------------------
           FireWall Entry Configuration.
---------------------------------------------------------------------------*/
typedef struct
{
  uint16   firewall_start_dest_port;
  uint16   firewall_end_dest_port;
  uint8    firewall_protocol;
  uint32   firewall_handle;
} qcmap_cm_firewall_entry_conf_t;

/*---------------------------------------------------------------------------
           Extended FireWall Entry Configuration.
---------------------------------------------------------------------------*/
typedef struct
{
  ip_filter_type filter_spec;
  uint32         firewall_handle;
} qcmap_cm_extd_firewall_entry_conf_t;

/*---------------------------------------------------------------------------
           Extended FireWall handle list configuration.
---------------------------------------------------------------------------*/
typedef struct
{
  uint32 handle_list[QCMAP_MAX_FIREWALL_ENTRIES_V01];
  ip_version_enum_type ip_family;
  int num_of_entries;
} qcmap_cm_get_extd_firewall_handle_list_conf_t;

/*---------------------------------------------------------------------------
           Extended FireWall handle configuration.
---------------------------------------------------------------------------*/
typedef struct
{
  uint32 handle;
  ip_version_enum_type ip_family;
} qcmap_cm_extd_firewall_handle_conf_t;

/*---------------------------------------------------------------------------
           Extended FireWall configuration.
---------------------------------------------------------------------------*/
typedef union
{
  qcmap_cm_extd_firewall_entry_conf_t extd_firewall_entry;
  qcmap_cm_get_extd_firewall_handle_list_conf_t extd_firewall_handle_list;
  qcmap_cm_extd_firewall_handle_conf_t extd_firewall_handle;
} qcmap_cm_extd_firewall_conf_t;

/*---------------------------------------------------------------------------
           FireWall Request Type.
---------------------------------------------------------------------------*/
typedef qcmap_cm_firewall_req_enum_type qcmap_cm_firewall_req_e;

/*---------------------------------------------------------------------------
           DMZ Request Type.
---------------------------------------------------------------------------*/
typedef qcmap_cm_dmz_req_enum qcmap_cm_dmz_req_e;

/*---------------------------------------------------------------------------
           VPN Request Type.
---------------------------------------------------------------------------*/
typedef qcmap_cm_vpn_req_enum qcmap_cm_vpn_req_e;

/*---------------------------------------------------------------------------
           NAT Entry Timeout Request Type.
---------------------------------------------------------------------------*/
typedef qcmap_cm_timeout_req_enum qcmap_cm_nat_timeout_req_e;

/*---------------------------------------------------------------------------
   Connection type in STA mode.
---------------------------------------------------------------------------*/
typedef enum
{
  QCMAP_STA_CONNECTION_DYNAMIC = 0,
  QCMAP_STA_CONNECTION_STATIC
}qcmap_sta_connection_e;

/*---------------------------------------------------------------------------
   Static IP Configuration in STA mode.
---------------------------------------------------------------------------*/
typedef struct
{
  uint32 ip_addr;
  uint32 gw_ip;
  uint32 netmask;
  uint32 dns_addr;
}qcmap_sta_static_ip_config;

/*---------------------------------------------------------------------------
   User provided connection information in STA mode.
---------------------------------------------------------------------------*/
typedef struct
{
  qcmap_sta_connection_e conn_type;
  qcmap_sta_static_ip_config static_ip_config;
}qcmap_sta_connection_config;

/*---------------------------------------------------------------------------
           NAT Configuration.
---------------------------------------------------------------------------*/

#define QCMAP_CM_MAX_FILE_LEN          64

typedef struct
{
  uint16   nat_entry_timeout;
  uint32   dmz_ip; /* 0 mean disable DMZ */
  uint8    enable_ipsec_vpn_pass_through;
  uint8    enable_pptp_vpn_pass_through;
  uint8    enable_l2tp_vpn_pass_through;

  uint8    num_port_fwding_entries;
  qcmap_cm_port_fwding_entry_conf_t
           port_fwding_entries[QCMAP_MAX_SNAT_ENTRIES_V01];

  char     firewall_config_file[QCMAP_CM_MAX_FILE_LEN];

  uint8    num_extd_firewall_entries;
  qcmap_cm_extd_firewall_conf_t
           extd_firewall_entries[QCMAP_MAX_FIREWALL_ENTRIES_V01];

  uint8    firewall_enabled;
  uint8    ioe_mode;
  uint8    tethering_only;
  uint8    firewall_pkts_allowed;
  qcmap_cm_nat_type
           nat_type;
  qcmap_cm_nat_type
           prev_nat_type;
} qcmap_cm_nat_conf_t;

/*---------------------------------------------------------------------------
           LAN Configuration.
---------------------------------------------------------------------------*/
#define QCMAP_LAN_INVALID_QCMAP_HANDLE (-1)
#define QCMAP_LAN_INVALID_IFACE_INDEX  (-1)
#define QCMAP_LAN_MAX_IPV4_ADDR_SIZE   16    /* 3 dots + 4 * 3 #s + 1 null */
#define QCMAP_IP_V4V6_V01              10
/* 3 Interfaces - Possible Modes: AP, AP+AP, STA+AP, STA+AP+AP */
#define QCMAP_MAX_NUM_INTF             3

#define QCMAP_V4_DEFAULT_DEVICE_NAME "rmnet0"
#define QCMAP_V6_DEFAULT_DEVICE_NAME "rmnet1"

typedef struct
{
  /*------------ Interface parameters common to STA,AP modes ------------- */
  /* Enable and configure main interface. */
  boolean  enable;
  qcmap_cm_devmode_type devmode;

  /*------------ Interface parameters specific to AP Mode ---------------- */
  /* Path to WLAN AP config which contain SSID/Mode/Encryption info */
  char     path_to_hostapd_conf[QCMAP_CM_MAX_FILE_LEN];

  /* Main interface configuration. All Addresses are in host order */
  uint32   a5_ip_addr;
  uint32   sub_net_mask;

  /* Type of access main interface has to networks. */
  qcmap_cm_access_profile_type access_profile;

  /* DHCP server config */
  boolean  enable_dhcpd;
  uint32   dhcp_start_address;
  uint32   dhcp_end_address;
  char     dhcp_lease_time[QCMAP_CM_MAX_FILE_LEN];

  /*------------ Interface parameters specific to STA Mode --------------- */
  /* WPA Supplicant Config for WLAN SoftAP */
  boolean  enable_supplicant;
  /* Path to WLAN AP config which contain SSID/Mode/Encryption info */
  char     path_to_supplicant_conf[QCMAP_CM_MAX_FILE_LEN];
  /* SSID of external hotspot to which to connect to in case
     enable_supplicant is false.
     This external AP must allow open(non-secure) access. */
  char     external_ap_ssid[QCMAP_CM_MAX_FILE_LEN];
  /* Connection type for STA mode. */
  qcmap_sta_connection_e conn_type;
  /* IP configuration for STATIC STA mode. */
  qcmap_sta_static_ip_config static_ip_config;
} qcmap_cm_intf_conf_t;

typedef struct
{
  /* Interface information. */
  qcmap_cm_intf_conf_t interface[QCMAP_MAX_NUM_INTF];

  char     module[QCMAP_CM_MAX_FILE_LEN];
  uint32   wlan_gpio_num;
  uint32   a5_rmnet_ip_addr;
  uint32   q6_ip_addr_facing_a5;
  uint32   usb_rmnet_ip_addr;
  uint32   q6_ip_addr_facing_usb_rmnet;
  uint32   nat_ip_addr;
  boolean  qti_link_up;
  uint32   dun_client_ip_addr;
} qcmap_cm_lan_conf_t;

/*---------------------------------------------------------------------------
           WAN Configuration.
---------------------------------------------------------------------------*/
#define QCMAP_WAN_INVALID_QCMAP_HANDLE 0xFFFFFFFF
#define QCMAP_WAN_MAX_ERI_DATA_SIZE    256
#define QCMAP_WAN_TECH_ANY             0
#define QCMAP_WAN_TECH_3GPP            1
#define QCMAP_WAN_TECH_3GPP2           2
#define QCMAP_QTI_WWAN_PARAMS          6

typedef struct
{
  struct
  {
    int  umts_profile_index;
    int  cdma_profile_index;
  }v4;
  struct
  {
    int  umts_profile_index;
    int  cdma_profile_index;
  }v6;
}qcmap_cm_profile_index;

typedef struct
{
  boolean prefix_info_valid;
  unsigned char prefix_len;
  struct sockaddr_storage prefix_addr;
  struct ifa_cacheinfo          cache_info;
} qcmap_cm_nl_prefix_info_t;

typedef struct
{
  boolean  auto_connect;
  boolean  roaming;
  char     eri_config_file[QCMAP_CM_MAX_FILE_LEN];

  int      tech;
  int      ip_family;
  qcmap_cm_profile_index
           profile_id;
}qcmap_cm_wan_conf_t;

/*---------------------------------------------------------------------------
           Master Mobile AP Config.
---------------------------------------------------------------------------*/
typedef struct
{
  qcmap_cm_nat_conf_t nat_config;
  qcmap_cm_wan_conf_t wan_config;
  qcmap_cm_lan_conf_t lan_config;
} qcmap_cm_conf_t;

/*---------------------------------------------------------------------------
           Mobile AP WWAN Config to be sent to QTI.
---------------------------------------------------------------------------*/
typedef struct
{
  long mtype;
  int  mtext[QCMAP_QTI_WWAN_PARAMS];
} qcmap_qti_msg_t;

/*---------------------------------------------------------------------------
           Embedded Profile
---------------------------------------------------------------------------*/
typedef struct
{
  int      tech;
  int      umts_profile_index;
  int      cdma_profile_index;
  int      ip_family;

} qcmap_cm_embd_conf;


/* Request message to send to the kernel to retrieve prefix info. */
typedef struct
{
  struct nlmsghdr   nlh;
  struct ifaddrmsg  ifa;
} qcmap_nl_getaddr_req_t;


/*---------------------------------------------------------------------------
                    Return values indicating error status
---------------------------------------------------------------------------*/
#define QCMAP_CM_SUCCESS               0         /* Successful operation   */
#define QCMAP_CM_ERROR                -1         /* Unsuccessful operation */

/*---------------------------------------------------------------------------
           Error Condition Values
---------------------------------------------------------------------------*/
#define QCMAP_CM_ENOERROR              0        /* No error                */
#define QCMAP_CM_EWOULDBLOCK           1        /* Operation would block   */
#define QCMAP_CM_EINVAL                2        /* Invalid operation       */
#define QCMAP_CM_EOPNOTSUPP            3        /* Operation not supported */
#define QCMAP_CM_EBADAPP               4        /* Invalid application ID  */
#define QCMAP_CM_ENOWWAN               5        /* WWAN not connected      */
#define QCMAP_CM_ESUCCESS              6        /* Success Operation. */

/*---------------------------------------------------------------------------
           Mobile AP Events
---------------------------------------------------------------------------*/
typedef enum
{
  QCMAP_CM_EVENT_ENABLED = 0,
  QCMAP_CM_EVENT_LAN_CONNECTING,
  QCMAP_CM_EVENT_LAN_CONNECTING_FAIL,
  QCMAP_CM_EVENT_LAN_IPv6_CONNECTING_FAIL,
  QCMAP_CM_EVENT_LAN_CONNECTED,
  QCMAP_CM_EVENT_STA_CONNECTED,
  QCMAP_CM_EVENT_LAN_IPv6_CONNECTED,
  QCMAP_CM_EVENT_WAN_CONNECTING,
  QCMAP_CM_EVENT_WAN_CONNECTING_FAIL,
  QCMAP_CM_EVENT_WAN_IPv6_CONNECTING_FAIL,
  QCMAP_CM_EVENT_WAN_CONNECTED,
  QCMAP_CM_EVENT_WAN_IPv6_CONNECTED,
  QCMAP_CM_EVENT_WAN_DISCONNECTED,
  QCMAP_CM_EVENT_WAN_IPv6_DISCONNECTED,
  QCMAP_CM_EVENT_LAN_DISCONNECTED,
  QCMAP_CM_EVENT_LAN_IPv6_DISCONNECTED,
  QCMAP_CM_EVENT_DISABLED,
  QCMAP_CM_EVENT_EMBD_CONNECTED,
  QCMAP_CM_EVENT_EMBD_IPv6_CONNECTED,
  QCMAP_CM_EVENT_EMBD_DISCONNECTED,
  QCMAP_CM_EVENT_EMBD_IPv6_DISCONNECTED
} qcmap_cm_event_e;

/*---------------------------------------------------------------------------
   Type representing enumeration of QCMAP CM states
---------------------------------------------------------------------------*/
typedef enum
{
  QCMAP_CM_DISABLE = 0,
  QCMAP_CM_ENABLE,
  QCMAP_CM_LAN_CONNECTING,
  QCMAP_CM_LAN_DISCONNECTING,
  QCMAP_CM_LAN_CONNECTED,
  QCMAP_CM_WAN_CONNECTING,
  QCMAP_CM_WAN_DISCONNECTING,
  QCMAP_CM_WAN_CONNECTED
} qcmap_cm_state_e;

typedef enum
{
  QCMAP_CM_V6_DISABLE = 0,
  QCMAP_CM_V6_ENABLE,
  QCMAP_CM_V6_LAN_CONNECTING,
  QCMAP_CM_V6_LAN_DISCONNECTING,
  QCMAP_CM_V6_LAN_CONNECTED,
  QCMAP_CM_V6_WAN_CONNECTING,
  QCMAP_CM_V6_WAN_DISCONNECTING,
  QCMAP_CM_V6_WAN_CONNECTED
} qcmap_cm_v6_state_e;

#define QCMAP_CM_IND_V4_WAN_CONNECTED            1002
#define QCMAP_CM_IND_V4_WAN_DISCONNECTED         1003
#define QCMAP_CM_IND_V6_WAN_CONNECTED            1004
#define QCMAP_CM_IND_V6_WAN_DISCONNECTED         1005

/*---------------------------------------------------------------------------
           Mobile AP CM call back declarations.
---------------------------------------------------------------------------*/
typedef void (*qcmap_cm_cb_fcn)
(
  int                 handle,                  /* Mobile AP Application id */
  qcmap_cm_event_e    event,                   /* Type of Mobile AP Event  */
  void               *qcmap_cm_cb_user_data    /* Call back User data      */
);

/*--------------------------------------------------------------------------- 
  Mobile AP Statistics
---------------------------------------------------------------------------*/
typedef struct
{
  uint64     bytes_rx;
  uint64     bytes_tx;
  uint32     pkts_rx;
  uint32     pkts_tx;
  uint32     pkts_dropped_rx;
  uint32     pkts_dropped_tx;
}qcmap_cm_statistics_t;

#ifdef __cplusplus
extern "C" {
#endif
/*===========================================================================

                      PUBLIC FUNCTION DECLARATIONS

===========================================================================*/

/*===========================================================================
  FUNCTION  qcmap_cm_dss_net_cb_fcn
===========================================================================*/
/*!
@brief
    QCMAP CM DSS net callback

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/

extern void qcmap_cm_dss_net_cb_fcn
(
sint15            dss_nethandle,                 /* Application ID         */
dss_iface_id_type iface_id,                      /* Interface ID structure */
sint15            dss_errno,                     /* Type of network err    */
void             *net_cb_user_data               /* Call back user data    */
);




/*===========================================================================

FUNCTION QCMAP_CM_ENABLE()

DESCRIPTION

  Enable Mobile AP CM based on the config.
  It will register Mobile AP event callback.
  It will configure Modem in Mobile AP Mode and bring up RmNet between Q6 and A5.
  It will also bring up LAN if it is config.

DEPENDENCIES
  None.

RETURN VALUE
  Returns Mobile AP CM application ID on success.

  On error, return 0 and place the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
extern int qcmap_cm_enable
(
  qcmap_cm_conf_t   *qcmap_cm_cfg,              /* Config for Mobile AP CM */
  qcmap_cm_cb_fcn    qcmap_cm_cb,               /* Callback function       */
  void              *qcmap_cm_cb_user_data,     /* Callback user data      */
  int               *qcmap_cm_errno             /* Error condition value   */
);

/*===========================================================================

FUNCTION QCMAP_CM_DISABLE()

DESCRIPTION

  Disable Mobile AP CM.
  It will teardown LAN.
  It will configure Modem in non-Mobile AP mode.

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
extern int qcmap_cm_disable
(
  int    qcmap_cm_handle,                      /* Handle for Mobile AP CM  */
  int   *qcmap_cm_errno                        /* Error condition value    */
);

/*============================================================


FUNCTION QCMAP_CM_EMBDCALL()

DESCRIPTION

  Bringup Embedded Call .

DEPENDENCIES
  None.

RETURN VALUE
  QCMAP Handle

SIDE EFFECTS


=============================================================*/
extern int qcmap_cm_embdcall_enable
(
  int IP_Family,
  int UMTS_PROFILE_INDX,
  int CDMA_PROFILE_INDX,
  int Tech,
  int *qcmap_embd_errno
);

/*============================================================

FUNCTION QCMAP_CM_EMBD_DISABLE()

DESCRIPTION

  Disable MobileAP EMBD CALL

DEPENDENCIES
  None.

RETURN VALUE


SIDE EFFECTS

============================================================*/
extern int qcmap_cm_embdcall_disable
(
  int *qcmap_cm_errno                        /* Error condition value    */
);

/*===========================================================================

FUNCTION QCMAP_CM_STATIC_NAT_OP()

DESCRIPTION
  This Function add/delete/get static NAT entry

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
extern int qcmap_cm_static_nat_op
(
  int    qcmap_cm_handle,                       /* Handle for Mobile AP CM */
  qcmap_cm_snat_req_e snat_op_type,             /* Add/Delete/Get SNAT     */
  qcmap_cm_port_fwding_entry_conf_t *snat_entry,/* SNAT Entry              */
  int    *qcmap_cm_errno                        /* Error condition value   */
);

/*===========================================================================

FUNCTION QCMAP_CM_FIREWALL_ENTRY_OP()

DESCRIPTION
  This Function add/delete/get firewall entry

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
extern int qcmap_cm_firewall_entry_op
(
  int    qcmap_cm_handle,                        /* Handle for Mobile AP CM*/
  qcmap_cm_firewall_entry_req_e firewall_op_type,/* Add/Delete/Get Firewall*/
  qcmap_cm_firewall_entry_conf_t *firewall_entry,/* Firewall Entry         */
  int    *qcmap_cm_errno                         /* error condition value  */
);

/*===========================================================================

FUNCTION QCMAP_CM_EXTD_FIREWALL_ENTRY_OP()

DESCRIPTION
  This Function performs add/get/delete extended firewall entry and
  get firewall handle list

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

=============================================================================*/
extern int qcmap_cm_extd_firewall_op
(
  int    qcmap_cm_handle,                           /*Mobile AP CM Handle    */
  qcmap_cm_firewall_entry_req_e firewall_op_type,   /*Extd firewall operation*/
  qcmap_cm_extd_firewall_conf_t *extd_firewall_conf,/*Extended Firewall conf */
  int    *qcmap_cm_errno                            /* error condition value */
);
/*===========================================================================

FUNCTION QCMAP_CM_FIREWALL_OP()

DESCRIPTION
  This Function enables/disables/get firewall state.

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
extern int qcmap_cm_firewall_op
(
  int    qcmap_cm_handle,                      /* Handle for Mobile AP CM  */
  qcmap_cm_firewall_req_e firewall_op_type,    /* Enable/Set/Get Firewall  */
  uint8  *pkts_allowed,                        /* Pkts allowed value       */
  int    *qcmap_cm_errno                       /* error condition value    */
);

/*===========================================================================

FUNCTION QCMAP_CM_DMZ_OP()

DESCRIPTION
  This Function set/get DMZ

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
extern int qcmap_cm_dmz_op
(
  int    qcmap_cm_handle,                      /* Handle for Mobile AP CM  */
  qcmap_cm_dmz_req_e dmz_op_type,              /* Set/Get DMZ              */
  uint32 *dmz_addr,                            /* DMZ IP address           */
  int    *qcmap_cm_errno                       /* error condition value    */
);

/*===========================================================================

FUNCTION QCMAP_CM_VPN_OP()

DESCRIPTION
  This Function set/get VPN Passthrough

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
extern int qcmap_cm_vpn_op
(
  int    qcmap_cm_handle,                      /* Handle for Mobile AP CM  */
  qcmap_cm_vpn_req_e vpn_op_type,              /* Set/Get VPN Passthrough  */
  uint8  *vpn_value,                           /* VPN Passthrough value    */
  int    *qcmap_cm_errno                       /* error condition value    */
);

/*===========================================================================

FUNCTION QCMAP_CM_NAT_ENTRY_TIMEOUT_OP()

DESCRIPTION
  This Function set/get NAT Entry Timeout

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
extern int qcmap_cm_nat_timeout_op
(
  int    qcmap_cm_handle,                      /* Handle for Mobile AP CM  */
  qcmap_cm_nat_timeout_req_e nat_timeout_op_type,/* Set/Get NAT Timeout    */
  uint16 *timeout_value,                       /* Timeout value            */
  int    *qcmap_cm_errno                       /* Error condition value    */
);

/*===========================================================================

FUNCTION QCMAP_CM_CONNECT_BACKHAUL()

DESCRIPTION

  It will bringup WWAN.

DEPENDENCIES
  None.

RETURN VALUE
  If WAN is already connected, returns QCMAP_CM_SUCCESS.
  return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EBADAPP           invalid application ID specified
  QCMAP_CM_EWOULDBLOCK       the operation would block
  QCMAP_CM_EOPNOTSUPP        backhaul bringup/teardown in progress


SIDE EFFECTS

===========================================================================*/
extern int qcmap_cm_connect_backhaul
(
  int    qcmap_cm_handle,                      /* Handle for Mobile AP CM  */
  int    *qcmap_cm_errno                       /* Error condition value    */
);

/*===========================================================================

FUNCTION QCMAP_CM_DISCONNECT_BACKHAUL()

DESCRIPTION

  It will teardown WWAN.

DEPENDENCIES
  None.

RETURN VALUE
  If WAN is already disconnected, returns QCMAP_CM_SUCCESS.
  return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EBADAPP           invalid application ID specified
  QCMAP_CM_EWOULDBLOCK       the operation would block
  QCMAP_CM_EOPNOTSUPP        backhaul bringup/teardown in progress


SIDE EFFECTS

===========================================================================*/
extern int qcmap_cm_disconnect_backhaul
(
  int    qcmap_cm_handle,                      /* Handle for Mobile AP CM  */
  int    *qcmap_cm_errno                       /* error condition value    */
);

/*===========================================================================

FUNCTION QCMAP_CM_GET_STATE()

DESCRIPTION

  It will get QCMAP CM current state.

DEPENDENCIES
  None.

SIDE EFFECTS

===========================================================================*/

extern qcmap_cm_state_e qcmap_cm_get_state
(
  void
);

/*===========================================================================
  FUNCTION:  qcmap_cm_write_xml

  DESCRIPTION

  This function write QCMAP CM XML based on QCMAP CM Cfg

DEPENDENCIES
  None.

RETURN VALUE

  QCMAP_CM_ERROR
  QCMAP_CM_SUCCESS

SIDE EFFECTS

===========================================================================*/
int qcmap_cm_write_xml
(
  char *xml_file,                                 /* Filename and path     */
  qcmap_cm_conf_t *config                         /* Mobile AP config data */
);

/*===========================================================================
FUNCTION:  qcmap_cm_write_firewall_xml 

DESCRIPTION

  This function write QCMAP CM Firewall XML based on QCMAP CM Cfg 
   
DEPENDENCIES
  None.

RETURN VALUE

  QCMAP_CM_ERROR
  QCMAP_CM_SUCCESS

SIDE EFFECTS

===========================================================================*/
int qcmap_cm_write_firewall_xml
(
  char *xml_file,                                 /* Filename and path     */
  qcmap_cm_conf_t *config                         /* Mobile AP config data */
);

/*===========================================================================
FUNCTION:  qcmap_cm_read_xml

DESCRIPTION

  This function read QCMAP CM XML and populate the QCMAP CM Cfg

DEPENDENCIES
  None.

RETURN VALUE

  QCMAP_CM_ERROR
  QCMAP_CM_SUCCESS

SIDE EFFECTS

===========================================================================*/
int qcmap_cm_read_xml
(
  char *xml_file,                                 /* Filename and path     */
  qcmap_cm_conf_t *config                         /* Mobile AP config data */
);


/*===========================================================================

FUNCTION:  qcmap_cm_read_firewall_xml 

DESCRIPTION

  This function reads QCMAP CM Firewall XML and populate the QCMAP CM Cfg 
   
DEPENDENCIES
  None.

RETURN VALUE

  QCMAP_CM_ERROR
  QCMAP_CM_SUCCESS

SIDE EFFECTS

===========================================================================*/
int qcmap_cm_read_firewall_xml
(
  char *xml_file,                                 /* Filename and path     */
  qcmap_cm_conf_t *config                         /* Mobile AP config data */
);

 
/*===========================================================================

FUNCTION QCMAP_CM_SET_AUTO_CONNECT()

DESCRIPTION

  It will set autoconnect.

DEPENDENCIES
  None.

RETURN VALUE

  qcmap_cm_errno Values
  ----------------
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
extern int qcmap_cm_set_auto_connect
(
  int    qcmap_cm_handle,                     /* Handle for Mobile AP CM   */
  int    *qcmap_cm_errno,                     /* Error condition value     */
  boolean auto_connect                        /* Autoconnect Enable or Not */
);

/*===========================================================================

FUNCTION QCMAP_CM_SET_ROAMING()

DESCRIPTION

  It will set roaming.

DEPENDENCIES
  None.

RETURN VALUE

  qcmap_cm_errno Values
  ----------------
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
extern int qcmap_cm_set_roaming
(
  int    qcmap_cm_handle,                      /* Handle for Mobile AP CM  */
  int    *qcmap_cm_errno,                      /* Error condition value    */
  boolean roaming                              /* Roaming Enable or Not    */
);


/*===========================================================================

FUNCTION QCMAP_CM_GET_IPV4_NET_CONF()

DESCRIPTION
  This Function get the ipv4 WWAN network configuration.

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
extern int qcmap_cm_get_ipv4_net_conf
(
  int     qcmap_cm_handle,                     /* Handle for Mobile AP CM  */
  uint32 *public_ip,                           /* IP addr assigned to WWAN */
  uint32 *pri_dns_addr,                        /* Primary DNS IP address   */
  uint32 *sec_dns_addr,                        /* Secondary DNS IP address */
  int    *qcmap_cm_errno                       /* Error condition value    */
);

/*===========================================================================

FUNCTION QCMAP_CM_GET_DEV_NAME()

DESCRIPTION
  This function gets the rmnet device name for a dss handle.

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
extern int qcmap_cm_get_dev_name
(
  int     qcmap_cm_handle,                     /* Handle for Mobile AP CM  */
  qcmap_ip_family_enum_v01
          qcmap_dev_type,                      /* Dev type                 */
  char   *qcmap_dev_name,                      /* Device name              */
  int    *qcmap_cm_errno                       /* Error condition value    */
);

/*===========================================================================

FUNCTION QCMAP_CM_ENABLE_STA_MODE()

DESCRIPTION
  Enable QCMobileAP CM based on the config.
  It will register QCMAP event callback.
  It will configure Modem in QCMAP Mode and bring up RmNet between Q6 and A5.
  It will also bring up LAN if it is config.

DEPENDENCIES
  None.

RETURN VALUE
  Returns QCMobileAP CM application ID on success.
  On error, return 0 and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS
  None.

===========================================================================*/
int qcmap_cm_enable_sta_mode
(
  int    qcmap_cm_handle,                         /* Handler for QCMAP CM  */
  int   *qcmap_cm_errno                           /* error condition value */
);


/*===========================================================================

FUNCTION QCMAP_CM_DISABLE_STA_MODE()

DESCRIPTION
  Disable QCMAP CM.
  Send disable_sta_mode msg to modem.

DEPENDENCIES
  None.

RETURN VALUE
  Returns QCMAP CM application ID on success.
  On error, return 0 and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS
  None.

===========================================================================*/
int qcmap_cm_disable_sta_mode
(
  int                qcmap_cm_handle,      /* Handle for QCMAP CM   */
  int                *qcmap_cm_errno       /* error condition value */
);

/*===========================================================================


FUNCTION QCMAP_CM_ERI_READ_CONFIG()

DESCRIPTION
  This function reads the passed file name to store the ERI config for
  processing.

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

=============================================================================*/
int
qcmap_cm_eri_read_config
(
  int    qcmap_cm_handle,                           /* Mobile AP CM Handle   */
  char  *file_name,                                 /* ERI config file       */
  int   *qcmap_cm_errno                             /* error condition value */
);

/*===========================================================================

FUNCTION QCMAP_CM_GET_WWAN_STATISTICS()

DESCRIPTION
  This Function gets WWAN statistics
   
DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
extern int qcmap_cm_get_wwan_statistics
(
  int                             qcmap_cm_handle,/*Handle for Mobile AP CM*/
  ip_version_enum_type            ip_family,     /* V4 or V6 family        */
  qcmap_cm_statistics_t          *wwan_stats,    /* WWAN Statistics values */
  int                            *qcmap_cm_errno /* Error condition value  */
);

/*===========================================================================

FUNCTION QCMAP_CM_RESET_WWAN_STATISTICS()

DESCRIPTION
  This Function resets WWAN statistics
   
DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
extern int qcmap_cm_reset_wwan_statistics
(
  int                             qcmap_cm_handle,/*Handle for Mobile AP CM*/
  ip_version_enum_type            ip_family,     /* V4 or V6 family        */
  int                            *qcmap_cm_errno /* Error condition value  */
);

/*===========================================================================

FUNCTION COEX_GET_WWAN_STATUS()

DESCRIPTION
  This Function retrieves the LTE frequency used by WiFi-LTE coex feature.
   
DEPENDENCIES
  None.

RETURN VALUE
  On success, returns TRUE.
  On error, return FALSE.

SIDE EFFECTS

===========================================================================*/
boolean coex_get_wwan_status
(
int                              *lte_frequency
);

/*===========================================================================

FUNCTION QCMAP_CM_NAT_TYPE_SET()

DESCRIPTION
  This Function changes/sets the NAT type.

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.


SIDE EFFECTS

===========================================================================*/
int qcmap_cm_nat_type_set
(
  int                qcmap_cm_handle, /*Handle for Mobile AP CM*/
  qcmap_cm_nat_type  nat_type,        /*NAT Type option        */
  int                *qcmap_cm_errno  /*Error condition value  */
);

/*===========================================================================

FUNCTION QCMAP_CM_NAT_TYPE_GET()

DESCRIPTION
  This Function gets the NAT type.

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.


SIDE EFFECTS

===========================================================================*/
int qcmap_cm_nat_type_get
(
  int                qcmap_cm_handle,   /*Handle for Mobile AP CM*/
  qcmap_cm_nat_type  *cur_nat_type,     /*NAT Type option        */
  int                *qcmap_cm_errno    /*Error condition value  */
);

int
qcmap_cm_vpn_op_get_pptp
(
  uint8  *vpn_value,                         /* PPTP VPN Passthrough value */
  int    *qcmap_cm_errno                     /* Error condition value      */
);

int
qcmap_cm_vpn_op_get_l2tp
(
  uint8  *vpn_value,                         /* PPTP VPN Passthrough value */
  int    *qcmap_cm_errno                     /* Error condition value      */
);


int
qcmap_cm_vpn_op_get_ipsec
(
  uint8 *vpn_value,                         /* IPSEC VPN Passthrough value */
  int   *qcmap_cm_errno                     /* Error condition value       */
);



#ifdef __cplusplus
}
#endif

#endif
