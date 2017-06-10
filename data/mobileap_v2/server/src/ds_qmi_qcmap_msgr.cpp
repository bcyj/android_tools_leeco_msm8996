/*===========================================================================

                         D S _ Q M I _ Q C M A P _ M S G R. CPP

DESCRIPTION

  The Data Services QMI Qualcomm Mobile Access Point Messenger service source file.

EXTERNALIZED FUNCTIONS

  qmi_qcmap_msgr_init()
    Initialize the QMI QCMAP CM service

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/
/*===========================================================================

when        who    what, where, why
--------    ---    ----------------------------------------------------------
07/12/12    gk     Created module
10/26/12    cp     Added support for Dual AP and different types of NAT.
12/19/12    sb     Added support for RNDIS/ECM USb tethering
02/27/13    cp     Added support to get IPV6 WAN status.
04/17/13    mp     Added support to get IPv6 WWAN/STA mode configuration.
05/09/13    tw     Fix mobileAP disable error when UPnP is up.
06/12/13    sg     Added DHCP Reservation feature
09/17/13    at     Added support to Enable/Disable ALGs
01/03/14    vm     Changes to support IoE on 9x25
01/16/14    cp     Added support for modem loopback call.
01/20/14    sr     Added support for connected devices in SoftAP
25/02/14    pm     Added handling of RTM_NEWADDR event for STA assoc
02/24/14    vm     Changes to Enable/Disable Station Mode to be in accordance
                   with IoE 9x15
03/20/14    vm     Set appropriate error number while disabling MobileAP to
                   send response/indications to IoE clients.
03/27/14    cp     Added support to DUN+SoftAP.
05/02/14    pm     Removed dependency on dss_new
===========================================================================*/

/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

#include "comdef.h"

#include "qmi_idl_lib.h"
#include "qmi_csi.h"
#include "common_v01.h"
#include "qualcomm_mobile_access_point_msgr_v01.h"
#include "qcmap_cm_api.h"
#include "ds_qmi_qcmap_msgr.h"

#include "QCMAP_ConnectionManager.h"
#include "limits.h"
#include "qcmap_cmdq.h"
#include "dsi_netctrl.h"
#include "qcmap_netlink.h"

/*===========================================================================

                            CONSTANT DEFINITIONS

===========================================================================*/
#define QCMAP_CM_LOG(...)                         \
  fprintf( stderr, "%s %d:", __FILE__, __LINE__); \
  fprintf( stderr, __VA_ARGS__ )

#define QCMAP_MSGR_SOFTAP_HANDLE 0x65432
/*===========================================================================

   The original delay for dss_init was 6 seconds. Allowing this value to be
   set at runtime to determine if delay still necessary. */
#define MAX_BUF_LEN 256
#define MAX(a,b) (a > b ? a : b)

static QCMAP_ConnectionManager *QcMapMgr = NULL;

/* Modem Loopback mode */
unsigned int modem_loopback_mode;

/* Server sockets */
unsigned int qcmap_dsi_sockfd;
unsigned int qcmap_cmdq_sockfd;
unsigned int qcmap_nas_sockfd;
unsigned int qcmap_timer_sockfd;
unsigned int qcmap_sta_sockfd;
unsigned int qcmap_nl_sockfd;

/* Client sockets */
unsigned int dsi_qcmap_sockfd;
unsigned int cmdq_qcmap_sockfd;
unsigned int nas_qcmap_sockfd;
unsigned int timer_qcmap_sockfd;
unsigned int nl_qcmap_sockfd;

/*==========================================================================
                                DATA TYPES

===========================================================================*/

/*---------------------------------------------------------------------------
  QMI QCMAP CM Client state info
---------------------------------------------------------------------------*/
typedef struct {
  qmi_client_handle                 clnt;
  boolean                           wwan_status_ind_regd;
  boolean                           qcmap_status_ind_regd;
  boolean                           station_mode_status_ind_regd;
  unsigned int                      wwan_op_ind_msg_id;
  boolean                           wwan_op_ind_pending;
  boolean                           client_enabled;
}qmi_qcmap_msgr_client_info_type;

static qmi_qcmap_msgr_state_info_type    qmi_qcmap_msgr_state;
static qmi_csi_os_params               os_params;
static boolean                         qmi_qcmap_msgr_inited = FALSE;
static qmi_qcmap_msgr_softap_handle_type qcmap_handle;

/*===========================================================================

                               INTERNAL DATA

===========================================================================*/

/*---------------------------------------------------------------------------
  QMI service command handlers
  forward declarations & cmd handler dispatch table definition
---------------------------------------------------------------------------*/

static qmi_csi_cb_error  qmi_qcmap_msgr_mobile_ap_enable(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_mobile_ap_disable(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_enable_ipv4(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_disable_ipv4(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_enable_ipv6(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_disable_ipv6(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_bring_up_wwan(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_tear_down_wwan(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_wwan_status_ind_reg(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_enable_wlan(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_disable_wlan(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_mobile_ap_status_ind_reg(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_add_static_nat_entry(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_delete_static_nat_entry(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_static_nat_entries(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_set_dmz(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_delete_dmz(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_dmz(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_wwan_stats(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_reset_wwan_stats(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_ipsec_vpn_pt(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_set_ipsec_vpn_pt(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_pptp_vpn_pt(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_set_pptp_vpn_pt(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_l2tp_vpn_pt(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_set_l2tp_vpn_pt(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_set_nat_type(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_nat_type(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_auto_connect(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_set_auto_connect(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);
static qmi_csi_cb_error  qmi_qcmap_msgr_add_firewall_entry(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);
static qmi_csi_cb_error  qmi_qcmap_msgr_del_firewall_entry(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_wwan_config(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_usb_link_up(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_usb_link_down(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_station_mode_status_ind_reg(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_add_dhcp_reservation_record(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                    *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_dhcp_reservation_records(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                    *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_edit_dhcp_reservation_record(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                    *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_delete_dhcp_reservation_record(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);


/*===========================================================================

                       FORWARD FUNCTION DECLARATIONS

===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_svc_connect_cb
(
  qmi_client_handle         client_handle,
  void                      *service_handle,
  void                      **connection_handle
);

static void qmi_qcmap_msgr_svc_disconnect_cb
(
  void                      *connection_handle,
  void                      *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_handle_client_req_cb
(
  void                     *connection_handle,
  qmi_req_handle           req_handle,
  unsigned int             msg_id,
  void                     *req_c_struct,
  unsigned int             req_c_struct_len,
  void                     *service_handle
);
static qmi_csi_cb_error qmi_qcmap_msgr_get_firewall_handles_list
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_firewall_entry
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
);
static qmi_csi_cb_error qmi_qcmap_msgr_get_mobileap_status
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
);
static qmi_csi_cb_error qmi_qcmap_msgr_get_wwan_status
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_get_station_mode_status
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_nat_timeout(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_set_nat_timeout(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_wlan_status(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);


static qmi_csi_cb_error  qmi_qcmap_msgr_set_wlan_config(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_wlan_config(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_activate_wlan(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_set_lan_config(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_lan_config(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_activate_lan(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_roaming(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_set_roaming(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

/* Define service handle table for QCMAP messages */
static qmi_csi_cb_error qmi_qcmap_msgr_set_firewall_config
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_get_firewall_config
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_set_wwan_policy
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
 );

static qmi_csi_cb_error qmi_qcmap_msgr_get_wwan_policy
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
 );

static qmi_csi_cb_error qmi_qcmap_msgr_get_ipv4_state
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
 );

static qmi_csi_cb_error qmi_qcmap_msgr_get_ipv6_state
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
 );

static qmi_csi_cb_error qmi_qcmap_msgr_enable_upnp
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_disable_upnp
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_get_upnp_status
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_enable_dlna
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_disable_dlna
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_get_dlna_status
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_set_dlna_media_dir
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_get_dlna_media_dir
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_enable_multicast_dns_responder
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_disable_multicast_dns_responder
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_get_multicast_dns_responder_status
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_set_qcmap_bootup_cfg
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_get_qcmap_bootup_cfg
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_get_data_bitrate
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_set_upnp_notify_interval
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_get_upnp_notify_interval
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_set_dlna_notify_interval
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_get_dlna_notify_interval
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_activate_hostapd_config(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_activate_supplicant_config(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_enable_alg(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_disable_alg(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_set_webserver_wwan_access_flag(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_webserver_wwan_access_flag(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_set_sip_server_info(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_sip_server_info(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_ipv6_sip_server_info(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_restore_factory_config(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_set_odu_mode(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_odu_mode(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error  qmi_qcmap_msgr_get_connected_devices_info(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_set_supplicant_config
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_get_cradle_mode
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_set_cradle_mode
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_get_prefix_delegation_config
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_set_prefix_delegation_config
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error qmi_qcmap_msgr_get_prefix_delegation_status
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
);

static qmi_csi_cb_error (* const req_handle_table[])
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
) =
{
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,     /* Request handler for message ID 0x00 - 0x07*/
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,     /* Request handler for message ID 0x08 - 0x0F*/
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,     /* Request handler for message ID 0x10 - 0x17*/
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,     /* Request handler for message ID 0x18 - 0x1F*/
  qmi_qcmap_msgr_mobile_ap_enable,             /* Request handler for message ID 0x20 */
  qmi_qcmap_msgr_mobile_ap_disable,            /* Request handler for message ID 0x21 */
  qmi_qcmap_msgr_bring_up_wwan,                /* Request handler for message ID 0x22 */
  qmi_qcmap_msgr_tear_down_wwan,               /* Request handler for message ID 0x23 */
  qmi_qcmap_msgr_wwan_status_ind_reg,          /* Request handler for message ID 0x24 */
  NULL,                                        /* Request handler for message ID 0x25 */
  qmi_qcmap_msgr_enable_wlan,                  /* Request handler for message ID 0x26 */
  qmi_qcmap_msgr_disable_wlan,                 /* Request handler for message ID 0x27 */
  qmi_qcmap_msgr_mobile_ap_status_ind_reg,     /* Request handler for message ID 0x28 */
  NULL,
  qmi_qcmap_msgr_add_static_nat_entry,         /* Request handler for message ID 0x2A */
  qmi_qcmap_msgr_delete_static_nat_entry,      /* Request handler for message ID 0x2B */
  qmi_qcmap_msgr_get_static_nat_entries,       /* Request handler for message ID 0x2C */
  qmi_qcmap_msgr_set_dmz,                      /* Request handler for message ID 0x2D */
  qmi_qcmap_msgr_delete_dmz,                   /* Request handler for message ID 0x2E */
  qmi_qcmap_msgr_get_dmz,                      /* Request handler for message ID 0x2F */
  qmi_qcmap_msgr_get_wwan_stats,               /* Request handler for message ID 0x30 */
  qmi_qcmap_msgr_reset_wwan_stats,             /* Request handler for message ID 0x31 */
  qmi_qcmap_msgr_get_ipsec_vpn_pt,             /* Request handler for message ID 0x32 */
  qmi_qcmap_msgr_set_ipsec_vpn_pt,             /* Request handler for message ID 0x33 */
  qmi_qcmap_msgr_get_pptp_vpn_pt,              /* Request handler for message ID 0x34 */
  qmi_qcmap_msgr_set_pptp_vpn_pt,              /* Request handler for message ID 0x35 */
  qmi_qcmap_msgr_get_l2tp_vpn_pt,              /* Request handler for message ID 0x36 */
  qmi_qcmap_msgr_set_l2tp_vpn_pt,              /* Request handler for message ID 0x37 */
  qmi_qcmap_msgr_set_nat_type,                 /* Request handler for message ID 0x38 */
  qmi_qcmap_msgr_get_nat_type,                 /* Request handler for message ID 0x39 */
  qmi_qcmap_msgr_set_auto_connect,             /* Request handler for message ID 0x3A */
  qmi_qcmap_msgr_get_auto_connect,             /* Request handler for message ID 0x3B */
  qmi_qcmap_msgr_add_firewall_entry,           /* Request handler for message ID 0x3C */
  qmi_qcmap_msgr_get_firewall_handles_list,    /* Request handler for message ID 0x3D */
  qmi_qcmap_msgr_get_firewall_entry,           /* Request handler for message ID 0x3E */
  qmi_qcmap_msgr_del_firewall_entry,           /* Request handler for message ID 0x3F */
  qmi_qcmap_msgr_get_wwan_config,              /* Request handler for message ID 0x40 */
  qmi_qcmap_msgr_get_mobileap_status,          /* Request handler for message ID 0x41 */
  qmi_qcmap_msgr_get_wwan_status,              /* Request handler for message ID 0x42 */
  qmi_qcmap_msgr_set_nat_timeout,              /* Request handler for message ID 0x43 */
  qmi_qcmap_msgr_get_nat_timeout,              /* Request handler for message ID 0x44 */
  qmi_qcmap_msgr_set_lan_config,               /* Request handler for message ID 0x45 */
  qmi_qcmap_msgr_get_wlan_status,              /* Request handler for message ID 0x46 */
  qmi_qcmap_msgr_activate_wlan,                /* Request handler for message ID 0x47 */
  qmi_qcmap_msgr_get_lan_config,               /* Request handler for message ID 0x48 */
  qmi_qcmap_msgr_usb_link_up,                  /* Request handler for message ID 0x49 */
  qmi_qcmap_msgr_usb_link_down,                /* Request handler for message ID 0x4A */
  qmi_qcmap_msgr_enable_ipv6,                  /* Request handler for message ID 0x4B */
  qmi_qcmap_msgr_disable_ipv6,                 /* Request handler for message ID 0x4C */
  qmi_qcmap_msgr_set_roaming,                  /*Request handler for message ID 0x4D */
  qmi_qcmap_msgr_get_roaming,                  /*Request handler for message ID 0x4E */
  qmi_qcmap_msgr_set_wwan_policy,              /*Request handler for message ID 0x4F */
  qmi_qcmap_msgr_get_wwan_policy,              /*Request handler for message ID 0x50 */
  qmi_qcmap_msgr_get_ipv6_state,               /*Request handler for message ID 0x51 */
  qmi_qcmap_msgr_enable_upnp,                  /*Request handler for message ID 0x52 */
  qmi_qcmap_msgr_disable_upnp,                  /*Request handler for message ID 0x53 */
  qmi_qcmap_msgr_enable_dlna,                   /* Request handler for message ID 0x54 */
  qmi_qcmap_msgr_disable_dlna,                   /* Request handler for message ID 0x55 */
  qmi_qcmap_msgr_set_firewall_config,          /*Request handler for message ID 0x56 */
  qmi_qcmap_msgr_get_firewall_config,          /*Request handler for message ID 0x57 */
  qmi_qcmap_msgr_enable_multicast_dns_responder,     /*Request handler for message ID 0x58 */
  qmi_qcmap_msgr_disable_multicast_dns_responder,    /*Request handler for message ID 0x59 */
  qmi_qcmap_msgr_get_upnp_status,              /*Request handler for message ID 0x5A */
  qmi_qcmap_msgr_get_dlna_status,              /*Request handler for message ID 0x5B */
  qmi_qcmap_msgr_get_multicast_dns_responder_status,  /*Request handler for message ID 0x5C */
  qmi_qcmap_msgr_station_mode_status_ind_reg,         /* Request handler for message ID 0x5D */
  NULL,                                               /* Request handler for message ID 0x5E */
  qmi_qcmap_msgr_get_station_mode_status,             /* Request handler for message ID 0x5F */
  qmi_qcmap_msgr_set_qcmap_bootup_cfg,                /* Request handler for message ID 0x60 */
  qmi_qcmap_msgr_get_qcmap_bootup_cfg,               /* Request handler for message ID 0x61 */
  qmi_qcmap_msgr_set_dlna_media_dir,                  /* Request handler for message ID 0x62 */
  qmi_qcmap_msgr_get_dlna_media_dir,                   /* Request handler for message ID 0x63 */
  qmi_qcmap_msgr_set_wlan_config,                       /*Request handler for message ID 0x64 */
  qmi_qcmap_msgr_activate_lan,                            /*Request handler for message ID 0x65 */
  qmi_qcmap_msgr_get_wlan_config,                       /*Request handler for message ID 0x66 */
  qmi_qcmap_msgr_enable_ipv4,                  /* Request handler for message ID 0x67 */
  qmi_qcmap_msgr_disable_ipv4,                 /* Request handler for message ID 0x68 */
  qmi_qcmap_msgr_get_ipv4_state,               /* Request handler for message ID 0x69 */
  qmi_qcmap_msgr_get_data_bitrate,             /*Request handler for message ID 0x6A */
  qmi_qcmap_msgr_get_upnp_notify_interval,     /*Request handler for message ID 0x6B */
  qmi_qcmap_msgr_set_upnp_notify_interval,     /*Request handler for message ID 0x6C */
  qmi_qcmap_msgr_get_dlna_notify_interval,     /*Request handler for message ID 0x6D */
  qmi_qcmap_msgr_set_dlna_notify_interval,     /*Request handler for message ID 0x6E */
  qmi_qcmap_msgr_add_dhcp_reservation_record,         /* Request handler for message ID 0x6F*/
  qmi_qcmap_msgr_get_dhcp_reservation_records,        /* Request handler for message ID 0x70*/
  qmi_qcmap_msgr_edit_dhcp_reservation_record,        /* Request handler for message ID 0x71*/
  qmi_qcmap_msgr_delete_dhcp_reservation_record,       /* Request handler for message ID 0x72*/
  qmi_qcmap_msgr_activate_hostapd_config,                /* Request handler for message ID 0x73 */
  qmi_qcmap_msgr_activate_supplicant_config,                /* Request handler for message ID 0x74 */
  qmi_qcmap_msgr_enable_alg,                   /* Request handler for message ID 0x75 */
  qmi_qcmap_msgr_disable_alg,                   /* Request handler for message ID 0x76 */
  qmi_qcmap_msgr_get_webserver_wwan_access_flag,           /* Request handler for message ID 0x77 */
  qmi_qcmap_msgr_set_webserver_wwan_access_flag,           /* Request handler for message ID 0x78 */
  qmi_qcmap_msgr_set_sip_server_info,                      /* Request handler for message ID 0x79 */
  qmi_qcmap_msgr_get_sip_server_info,                      /* Request handler for message ID 0x7A */
  qmi_qcmap_msgr_restore_factory_config,                   /* Request handler for message ID 0x7B */
  qmi_qcmap_msgr_set_odu_mode,                             /* Request handler for message ID 0x7C */
  qmi_qcmap_msgr_get_odu_mode,                             /* Request handler for message ID 0x7D */
  NULL,
  qmi_qcmap_msgr_get_connected_devices_info,               /* Request handler for message ID 0x7F */
  NULL,                                                    /* Request handler for message ID 0x80 */
  NULL,                                                    /* Request handler for message ID 0x81 */
  qmi_qcmap_msgr_get_ipv6_sip_server_info,                 /* Request handler for message ID 0x82 */
  qmi_qcmap_msgr_set_supplicant_config,                    /* Request handler for message ID 0x83 */
  qmi_qcmap_msgr_get_cradle_mode,                          /* Request handler for message ID 0x84 */
  qmi_qcmap_msgr_set_cradle_mode,                          /* Request handler for message ID 0x85 */
  NULL,                                                    /* Request handler for message ID 0x86 */
  qmi_qcmap_msgr_get_prefix_delegation_config,               /* Request handler for message ID 0x87 */
  qmi_qcmap_msgr_set_prefix_delegation_config,               /* Request handler for message ID 0x88 */
  qmi_qcmap_msgr_get_prefix_delegation_status                /* Request handler for message ID 0x89 */
};

/*===========================================================================

                       EXTERNAL FUNCTION DEFINTIONS

===========================================================================*/

void sigHandler(int signal)
{
  int err_num=0;
  qmi_error_type_v01 qmi_err_num;

  switch (signal)
  {
    case SIGTERM:
    case SIGINT:
      if (QcMapMgr)
      {
        QcMapMgr->Disable(&err_num, &qmi_err_num);
        if (err_num == QCMAP_CM_EALDDISCONN)
        {
          LOG_MSG_INFO1("Mobile AP already disabled!!",0,0,0);
        }
      }
      /* Client sockets */
      close(dsi_qcmap_sockfd);
      close(cmdq_qcmap_sockfd);
      close(nas_qcmap_sockfd);
      close(timer_qcmap_sockfd);
      close(nl_qcmap_sockfd);

      /* Server sockets */
      close(qcmap_dsi_sockfd);
      close(qcmap_cmdq_sockfd);
      close(qcmap_nas_sockfd);
      close(qcmap_timer_sockfd);
      close(qcmap_sta_sockfd);
      close(qcmap_nl_sockfd);

      qmi_csi_unregister(qmi_qcmap_msgr_state.service_handle);
      exit(0);
      break;

    default:
      LOG_MSG_INFO1("Received unexpected signal %s\n", signal,0,0);
      break;
  }
}


int readable_addr(int domain, uint32 *addr, char *str)
{
  if (inet_ntop(domain, (void *)addr, str, INET6_ADDRSTRLEN) == NULL)
  {
  LOG_MSG_ERROR("\n Not in presentation format \n",0,0,0);
  return -1;
  }
}


int create_socket(unsigned int *sockfd)
{

  if ((*sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR("Error creating socket, errno: %d", errno, 0, 0);
    return QCMAP_CM_ERROR;
  }

  if(fcntl(*sockfd, F_SETFD, FD_CLOEXEC) < 0)
  {
    LOG_MSG_ERROR("Couldn't set Close on Exec, errno: %d", errno, 0, 0);
  }

  return QCMAP_CM_ENOERROR;
}

int create_qcmap_dsi_socket()
{
  int val, rval;
  struct sockaddr_un qcmap_dsi;
  int len;
  struct timeval rcv_timeo;

  rval = create_socket(&qcmap_dsi_sockfd);
  if( rval == QCMAP_CM_ERROR || qcmap_dsi_sockfd < 0 )
  {
    LOG_MSG_ERROR("failed to create qcmap_dsi_socket ", 0, 0, 0);
    return QCMAP_CM_ERROR;
  }
  rcv_timeo.tv_sec = 0;
  rcv_timeo.tv_usec = 100000;
  setsockopt(qcmap_dsi_sockfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&rcv_timeo, sizeof(rcv_timeo));
  val = fcntl(qcmap_dsi_sockfd, F_GETFL, 0);
  fcntl(qcmap_dsi_sockfd, F_SETFL, val | O_NONBLOCK);
  /* set bit in os_params */
  FD_SET(qcmap_dsi_sockfd, &os_params.fds);
  os_params.max_fd = MAX(os_params.max_fd, qcmap_dsi_sockfd);

  qcmap_dsi.sun_family = AF_UNIX;
  strcpy(qcmap_dsi.sun_path, QCMAP_DSI_UDS_FILE);
  unlink(qcmap_dsi.sun_path);
  len = strlen(qcmap_dsi.sun_path) + sizeof(qcmap_dsi.sun_family);
  if (bind(qcmap_dsi_sockfd, (struct sockaddr *)&qcmap_dsi, len) == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR("Error binding the socket, errno: %d", errno, 0, 0);
    return QCMAP_CM_ERROR;
  }
  return QCMAP_CM_ENOERROR;
}

int create_qcmap_cmdq_socket()
{
  int val, rval;
  struct sockaddr_un qcmap_cmdq;
  int len;
  struct timeval rcv_timeo;

  rval = create_socket(&qcmap_cmdq_sockfd);
  if( rval == QCMAP_CM_ERROR || qcmap_cmdq_sockfd < 0 )
  {
    LOG_MSG_ERROR("failed to create qcmap_cmdq_sockfd ", 0, 0, 0);
    return QCMAP_CM_ERROR;
  }

  rcv_timeo.tv_sec = 0;
  rcv_timeo.tv_usec = 100000;
  setsockopt(qcmap_cmdq_sockfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&rcv_timeo, sizeof(rcv_timeo));
  val = fcntl(qcmap_cmdq_sockfd, F_GETFL, 0);
  fcntl(qcmap_cmdq_sockfd, F_SETFL, val | O_NONBLOCK);
  /* set bit in os_params */
  FD_SET(qcmap_cmdq_sockfd, &os_params.fds);
  os_params.max_fd = MAX(os_params.max_fd, qcmap_cmdq_sockfd);

  qcmap_cmdq.sun_family = AF_UNIX;
  strcpy(qcmap_cmdq.sun_path, QCMAP_CMDQ_UDS_FILE);
  unlink(qcmap_cmdq.sun_path);
  len = strlen(qcmap_cmdq.sun_path) + sizeof(qcmap_cmdq.sun_family);
  if (bind(qcmap_cmdq_sockfd, (struct sockaddr *)&qcmap_cmdq, len) == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR("Error binding the socket, errno: %d", errno, 0, 0);
    return QCMAP_CM_ERROR;
  }
  return QCMAP_CM_ENOERROR;
}

int create_qcmap_nas_socket()
{
  int val, rval;
  struct sockaddr_un qcmap_nas;
  int len;
  struct timeval rcv_timeo;

  rval = create_socket(&qcmap_nas_sockfd);
  if( rval == QCMAP_CM_ERROR || qcmap_nas_sockfd < 0 )
  {
    LOG_MSG_ERROR("failed to create qcmap_nas_sockfd ", 0, 0, 0);
    return QCMAP_CM_ERROR;
  }

  rcv_timeo.tv_sec = 0;
  rcv_timeo.tv_usec = 100000;
  setsockopt(qcmap_nas_sockfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&rcv_timeo, sizeof(rcv_timeo));
  val = fcntl(qcmap_nas_sockfd, F_GETFL, 0);
  fcntl(qcmap_nas_sockfd, F_SETFL, val | O_NONBLOCK);
  /* set bit in os_params */
  FD_SET(qcmap_nas_sockfd, &os_params.fds);
  os_params.max_fd = MAX(os_params.max_fd, qcmap_nas_sockfd);

  qcmap_nas.sun_family = AF_UNIX;
  strcpy(qcmap_nas.sun_path, QCMAP_NAS_UDS_FILE);
  unlink(qcmap_nas.sun_path);
  len = strlen(qcmap_nas.sun_path) + sizeof(qcmap_nas.sun_family);
  if (bind(qcmap_nas_sockfd, (struct sockaddr *)&qcmap_nas, len) == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR("Error binding the socket, errno: %d", errno, 0, 0);
    return QCMAP_CM_ERROR;
  }
  return QCMAP_CM_ENOERROR;
}

int create_qcmap_nl_socket()
{
  int val, rval;
  struct sockaddr_un qcmap_nl;
  int len;
  struct timeval rcv_timeo;

  rval = create_socket(&qcmap_nl_sockfd);
  if( rval == QCMAP_CM_ERROR || qcmap_nl_sockfd < 0 )
  {
    LOG_MSG_ERROR("failed to create qcmap_nas_sockfd ", 0, 0, 0);
    return QCMAP_CM_ERROR;
  }

  rcv_timeo.tv_sec = 0;
  rcv_timeo.tv_usec = 100000;
  setsockopt(qcmap_nl_sockfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&rcv_timeo, sizeof(rcv_timeo));
  val = fcntl(qcmap_nl_sockfd, F_GETFL, 0);
  fcntl(qcmap_nl_sockfd, F_SETFL, val | O_NONBLOCK);
  /* set bit in os_params */
  FD_SET(qcmap_nl_sockfd, &os_params.fds);
  os_params.max_fd = MAX(os_params.max_fd, qcmap_nl_sockfd);

  qcmap_nl.sun_family = AF_UNIX;
  strcpy(qcmap_nl.sun_path, QCMAP_NL_UDS_FILE);
  unlink(qcmap_nl.sun_path);
  len = strlen(qcmap_nl.sun_path) + sizeof(qcmap_nl.sun_family);
  if (bind(qcmap_nl_sockfd, (struct sockaddr *)&qcmap_nl, len) == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR("Error binding the NL socket, errno: %d", errno, 0, 0);
    return QCMAP_CM_ERROR;
  }
  return QCMAP_CM_ENOERROR;
}

int create_qcmap_timer_socket()
{
  int val, rval;
  struct sockaddr_un qcmap_timer;
  int len;
  struct timeval rcv_timeo;

  rval = create_socket(&qcmap_timer_sockfd);
  if( rval == QCMAP_CM_ERROR || qcmap_timer_sockfd < 0 )
  {
    LOG_MSG_ERROR("failed to create qcmap_nas_sockfd ", 0, 0, 0);
    return QCMAP_CM_ERROR;
  }

  rcv_timeo.tv_sec = 0;
  rcv_timeo.tv_usec = 100000;
  setsockopt(qcmap_timer_sockfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&rcv_timeo, sizeof(rcv_timeo));
  val = fcntl(qcmap_timer_sockfd, F_GETFL, 0);
  fcntl(qcmap_timer_sockfd, F_SETFL, val | O_NONBLOCK);
  /* set bit in os_params */
  FD_SET(qcmap_timer_sockfd, &os_params.fds);
  os_params.max_fd = MAX(os_params.max_fd, qcmap_timer_sockfd);

  qcmap_timer.sun_family = AF_UNIX;
  strcpy(qcmap_timer.sun_path, QCMAP_TIMER_UDS_FILE);
  unlink(qcmap_timer.sun_path);
  len = strlen(qcmap_timer.sun_path) + sizeof(qcmap_timer.sun_family);
  if (bind(qcmap_timer_sockfd, (struct sockaddr *)&qcmap_timer, len) == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR("Error binding the socket, errno: %d", errno, 0, 0);
    return QCMAP_CM_ERROR;
  }
  return QCMAP_CM_ENOERROR;
}

int create_qcmap_sta_socket()
{
  int val, rval;
  struct sockaddr_un qcmap_sta;
  int len;
  struct timeval rcv_timeo;

  rval = create_socket(&qcmap_sta_sockfd);
  if( rval == QCMAP_CM_ERROR || qcmap_sta_sockfd < 0 )
  {
    LOG_MSG_ERROR("failed to create qcmap_nas_sockfd ", 0, 0, 0);
    return QCMAP_CM_ERROR;
  }

  rcv_timeo.tv_sec = 0;
  rcv_timeo.tv_usec = 100000;
  setsockopt(qcmap_sta_sockfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&rcv_timeo, sizeof(rcv_timeo));
  val = fcntl(qcmap_sta_sockfd, F_GETFL, 0);
  fcntl(qcmap_sta_sockfd, F_SETFL, val | O_NONBLOCK);
  /* set bit in os_params */
  FD_SET(qcmap_sta_sockfd, &os_params.fds);
  os_params.max_fd = MAX(os_params.max_fd, qcmap_sta_sockfd);

  qcmap_sta.sun_family = AF_UNIX;
  strcpy(qcmap_sta.sun_path, QCMAP_STA_UDS_FILE);
  unlink(qcmap_sta.sun_path);
  len = strlen(qcmap_sta.sun_path) + sizeof(qcmap_sta.sun_family);
  if (bind(qcmap_sta_sockfd, (struct sockaddr *)&qcmap_sta, len) == QCMAP_CM_ERROR)
  {
    LOG_MSG_ERROR("Error binding the socket, errno: %d", errno, 0, 0);
    return QCMAP_CM_ERROR;
  }
  return QCMAP_CM_ENOERROR;
}

/* start QCMAP netlink socket monitor thread for NETLINK NEIGH Message*/
void* qcmap_netlink_start(void)
{
        qcmap_nl_sk_fd_set_info_t sk_fdset;
        int ret_val = 0;
        unsigned int groups = 0;

        memset(&sk_fdset, 0, sizeof(qcmap_nl_sk_fd_set_info_t));
        LOG_MSG_INFO1("netlink starter memset sk_fdset succeeds\n",0,0,0);
        groups = RTMGRP_NEIGH | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_ROUTE;

        ret_val = qcmap_nl_listener_init(NETLINK_ROUTE,
                                         groups,&sk_fdset,
                                         qcmap_nl_recv_msg);
        if (ret_val != QCMAP_NL_SUCCESS)
        {
           LOG_MSG_ERROR("Failed to initialize QCMAP netlink event listener thread\n",0,0,0);
           return NULL;
        }

        ret_val = qcmap_packet_socket_init(&sk_fdset,
                                         qcmap_packet_sock_recv_msg);
        if (ret_val != QCMAP_NL_SUCCESS)
        {
           LOG_MSG_ERROR("Failed to initialize QCMAP packet socket listener thread\n",0,0,0);
           return NULL;
        }

        ret_val = qcmap_nl_sock_listener_start(&sk_fdset);
        if (ret_val != QCMAP_NL_SUCCESS)
        {
           LOG_MSG_ERROR("Failed to start listening to RTM Socket QCMAP netlink event listener thread\n",0,0,0);
           return NULL;
        }

        LOG_MSG_ERROR(" Unreachable Code -- If reached means something wrong in QCMAP Netlink thread",0,0,0);
        // Cleanup for the thread
        return NULL;
}

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_INIT()

  DESCRIPTION
    Register the QCMAP CM service with QMI Framework

  PARAMETERS
    None

  RETURN VALUE
    None

  DEPENDENCIES
    None

  SIDE EFFECTS
    None
===========================================================================*/
int qmi_qcmap_msgr_init
(
  void
)
{
  qmi_csi_error         rc;
  struct sockaddr_un qcmap_dsi;
  int len;
  struct timeval rcv_timeo;
  int val, rval;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  LOG_MSG_INFO1("QMI QCMAP CM service init %d", qmi_qcmap_msgr_inited,0,0);

  if (!qmi_qcmap_msgr_inited)
    qmi_qcmap_msgr_inited = TRUE;
  else
  {
    LOG_MSG_INFO1("QMI QCMAP CM service already initialized",0,0,0);
    return QCMAP_CM_ENOERROR;
  }

  /* Create dsi -> qcmap client socket */
  if (create_socket(&dsi_qcmap_sockfd) != QCMAP_CM_ENOERROR)
  {
    LOG_MSG_ERROR("qmi_qcmap_msgr_init::error creating dsi_qcmap_sockfd socket", 0, 0, 0);
  }

  /* Create cmdq -> qcmap client socket */
  if (create_socket(&cmdq_qcmap_sockfd) != QCMAP_CM_ENOERROR)
  {
    LOG_MSG_ERROR("qmi_qcmap_msgr_init::error creating cmdq_qcmap_sockfd socket", 0, 0, 0);
  }

  /* Create nas -> qcmap client socket */
  if (create_socket(&nas_qcmap_sockfd) != QCMAP_CM_ENOERROR)
  {
    LOG_MSG_ERROR("qmi_qcmap_msgr_init::error creating nas_qcmap_sockfd socket", 0, 0, 0);
  }

  /* Create timer -> qcmap timer socket */
  if (create_socket(&timer_qcmap_sockfd) != QCMAP_CM_ENOERROR)
  {
    LOG_MSG_ERROR("qmi_qcmap_msgr_init::error creating timer_qcmap_sockfd socket", 0, 0, 0);
  }

  /* Create netlink -> qcmap client socket */
  if (create_socket(&nl_qcmap_sockfd) != QCMAP_CM_ENOERROR)
  {
    LOG_MSG_ERROR("qmi_qcmap_msgr_init::error creating nl_qcmap_sockfd socket", 0, 0, 0);
  }

  /*-------------------------------------------------------------------------
    Register QMI QCMAP CM service with QCSI
  -------------------------------------------------------------------------*/
  rc = qmi_csi_register
       (
       qcmap_msgr_get_service_object_v01(),
       qmi_qcmap_msgr_svc_connect_cb,
       qmi_qcmap_msgr_svc_disconnect_cb,
       qmi_qcmap_msgr_handle_client_req_cb,
       &qmi_qcmap_msgr_state,
       &os_params,
       &qmi_qcmap_msgr_state.service_handle
       );

  if(rc != QMI_CSI_NO_ERR)
  {
    LOG_MSG_ERROR("Unable to register QCMAP CM service! Error %d", rc,0,0);
    ds_assert(0);
    return QCMAP_CM_ERROR;
  }

  LOG_MSG_INFO1("QMI QCMAP CM service registered with QMI Framework",0,0,0);

  /* Create qcmap -> dsi server socket */
  rval = create_qcmap_dsi_socket();
  if (rval != QCMAP_CM_ENOERROR)
  {
    LOG_MSG_ERROR("Unable to create qcmap dsi socket!", 0,0,0);
    return QCMAP_CM_ERROR;
  }

  /* Create qcmap -> cmdq server socket */
  rval = create_qcmap_cmdq_socket();

  if (rval != QCMAP_CM_ENOERROR)
  {
    LOG_MSG_ERROR("Unable to create qcmap cmdq socket!", 0,0,0);
    return QCMAP_CM_ERROR;
  }

  /* Create qcmap -> nas server socket */
  rval = create_qcmap_nas_socket();

  if (rval != QCMAP_CM_ENOERROR)
  {
    LOG_MSG_ERROR("Unable to create qcmap nas socket!", 0,0,0);
    return QCMAP_CM_ERROR;
  }

  /* Create qcmap -> timer server socket */
  rval = create_qcmap_timer_socket();

  if (rval != QCMAP_CM_ENOERROR)
  {
    LOG_MSG_ERROR("Unable to create qcmap nas socket!", 0,0,0);
    return QCMAP_CM_ERROR;
  }

  /* Create qcmap -> netlink server socket */
  rval = create_qcmap_nl_socket();
  if (rval != QCMAP_CM_ENOERROR)
  {
    LOG_MSG_ERROR("Unable to create qcmap netlink socket!", 0,0,0);
    return QCMAP_CM_ERROR;
  }

  /* Create qcmap -> sta server socket */
  rval = create_qcmap_sta_socket();

  if (rval != QCMAP_CM_ENOERROR)
  {
    LOG_MSG_ERROR("Unable to create qcmap sta socket!", 0,0,0);
    return QCMAP_CM_ERROR;
  }

  LOG_MSG_INFO1("Successfully completed initialization",0,0,0);
  return QCMAP_CM_ENOERROR;
} /* qmi_qcmap_msgr_init */


int main(int argc, char **argv)
{

  int test_mode = 0, nbytes=0;
  char buf[MAX_BUF_LEN];
  struct sockaddr_storage their_addr;
  socklen_t addr_len = sizeof(struct sockaddr_storage);
  qcmap_dsi_buffer_t *qcmap_dsi_buffer = NULL;
  qcmap_nas_buffer_t *qcmap_nas_buffer = NULL;
  qcmap_timer_buffer_t *qcmap_timer_buffer = NULL;
  qcmap_sta_buffer_t *qcmap_sta_buffer = NULL;
  qcmap_nl_sock_msg_t *qcmap_nl_buffer = NULL;
  qcmap_nl_sock_msg_t qcmap_nl_wlan_buffer;
  uint8_t             mac_addr[QCMAP_MSGR_MAC_ADDR_LEN_V01];
  fd_set master_fd_set;
  int tmp_handle = 0;
  qmi_error_type_v01 qmi_err_num;
  size_t len = 0;
  int ret;
  boolean bootup_init = false ;
  pthread_t qcmap_netlink_thread = 0;
  uint32 usb_ip=0;
  qcmap_msgr_wlan_mode_enum_v01 wlan_mode = QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01;
  qcmap_msgr_access_profile_v01 guest_access_profile;
  qcmap_msgr_station_mode_config_v01 station_config;
  struct sigaction sa;

  /*Initialize the Diag for QXDM logs*/
  if (TRUE != Diag_LSM_Init(NULL))
  {
     QCMAP_CM_LOG("Diag_LSM_Init failed !!");
  }

  /* Initialize qcmap_cmdq module */
  qmi_qcmap_msgr_init();

  /* Create a thread to monitor Netlink Neighbour*/
  ret = pthread_create(&qcmap_netlink_thread, NULL, qcmap_netlink_start, NULL);
  if (QCMAP_CM_SUCCESS != ret)
  {
     LOG_MSG_ERROR("unable to create netlink thread for QCMAP return reason: %d\n",ret,0,0);
     return ret;
  }
  LOG_MSG_INFO1("created netlink thread for QCMAP\n",0,0,0);

  switch(argc)
  {
    /* If command line parameters were entered, ...*/
    case 2:
      /* ... read the XML file path from argv[1]. */
      LOG_MSG_INFO1("%s XML path %s\n", argv[0], argv[1],0);
      QcMapMgr = new QCMAP_ConnectionManager(argv[1]);
      break;
    case 3:
      if (0 != strncasecmp(argv[2],"d",1))
      {
        exit(1);
      }
      QcMapMgr = new QCMAP_ConnectionManager(argv[1]);
      LOG_MSG_INFO1("Deamon mode %s %s \n",argv[0], argv[1],0);
      break;
    case 4:
      if (0 != strncasecmp(argv[2],"l",1))
      {
        exit(1);
      }
      QcMapMgr = new QCMAP_ConnectionManager(argv[1]);
      modem_loopback_mode = 1;
      LOG_MSG_INFO1("Loopback mode %s %s %s \n",argv[0], argv[1],argv[2]);
      break;
    default:
      /* Else, use default paramters to configure the Mobile AP. */
      QcMapMgr = new QCMAP_ConnectionManager(NULL);
      break;
  }

  if( QcMapMgr == NULL)
  {
    LOG_MSG_ERROR("Insufficient memory to initialize QCMAP object", 0, 0, 0);
    return QCMAP_CM_ERROR;
  }

  if ( QcMapMgr->get_wlan_enable_cfg_flag() )
  {

    if ( QcMapMgr->GetWLANConfig(&wlan_mode, &guest_access_profile, &station_config, &qmi_err_num) )
    {
      QCMAP_CM_LOG("Got WLAN Config successfully.\n");
    }

    /* If wlan mode is ap-sta and mobileap needs to be enabled on bootup,
     * enable mobileap and then enable wlan */
    if( wlan_mode == QCMAP_MSGR_WLAN_MODE_AP_STA_V01 &&
        QcMapMgr->get_mobileap_enable_cfg_flag() )
    {
      if (QcMapMgr->Enable(&tmp_handle, &qcmap_handle,&qmi_err_num))
      {
        LOG_MSG_INFO1("Enable Mobile AP on Bootup succeeds.\n",0,0,0);
        qcmap_handle.handle = tmp_handle;
      }
      else
      {
        LOG_MSG_ERROR("Enable Mobile AP On Bootup FAILS!!! errno:%d.\n",qmi_err_num,0,0);
      }
    }

    /* For WLAN bootup KPI log */
    ds_system_call("echo QCMAP:WLAN mode > /dev/kmsg",
            strlen("echo QCMAP:WLAN mode > /dev/kmsg"));

    if ( QcMapMgr->EnableWLAN(&qmi_err_num,true) )
    {
      LOG_MSG_INFO1("Enable WLAN on Bootup succeeds.\n",0,0,0);
    }
    else
    {
      LOG_MSG_INFO1("Enable WLAN on Bootup Failsi errno:%d.\n",qmi_err_num,0,0);
    }
  }

  /* If Wlan is not in AP-STA mode, wlan is enabled first so that wlan
   * service is available at the earliest. After enabling wlan, mobileap is
   * enabled */
  if( wlan_mode != QCMAP_MSGR_WLAN_MODE_AP_STA_V01 &&
      QcMapMgr->get_mobileap_enable_cfg_flag() )
  {
    if ( QcMapMgr->Enable(&tmp_handle, &qcmap_handle,&qmi_err_num ) )
    {
      LOG_MSG_INFO1("Enable Mobile AP on Bootup succeeds.\n",0,0,0);
      qcmap_handle.handle = tmp_handle;
    }
    else
    {
      LOG_MSG_ERROR("Enable Mobile AP On Bootup FAILS!!! errno:%d.\n",qmi_err_num,0,0);
    }
  }

  /* Register the sighandlers, so the app may be shutdown with a
     kill command.*/
  signal(SIGTERM, sigHandler);
  signal(SIGINT, sigHandler);
  signal(SIGUSR1, SIG_IGN);

  /* Register User Signal Handler used for QCMAP Timers */
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = qcmap_timer_handler;
  sigemptyset(&sa.sa_mask);
  sigaction(SIG, &sa, NULL);
  /* initilize qcril library  */
  if (DSI_SUCCESS != dsi_init_ex(DSI_MODE_GENERAL, qcmap_cm_dsi_net_init_cb, NULL))
  {
    LOG_MSG_ERROR("dsi_init failed !!",0,0,0);
    return -1;
  }
  /*KPI log message*/
  ds_system_call("echo QCMAP:Init Complete > /dev/kmsg",
          strlen("echo QCMAP:Init Complete > /dev/kmsg"));

    while(1)
  {
    master_fd_set = os_params.fds;
    ret = select(os_params.max_fd+1, &master_fd_set, NULL, NULL, NULL);
    if (ret < 0)
    {
      LOG_MSG_ERROR("Error in select, errno:%d", errno, 0, 0);
      if( errno == EINTR )
       continue;
      else
       return -1;
    }

    for (int i = 0; ( i <= os_params.max_fd ); i++)
    {
      if (FD_ISSET(i, &master_fd_set))
      {
           LOG_MSG_INFO1("some fd set, %d, qcmap_dsi_sockfd:%d, qcmap_nl_sockfd:%d \n", i, qcmap_dsi_sockfd, qcmap_nl_sockfd);
        if ( i == qcmap_dsi_sockfd )
        {
           LOG_MSG_INFO1("Received Message from dsi_callback\n\n",0,0,0);
           if ( ( nbytes = recvfrom(i, buf, MAX_BUF_LEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) <= 0 )
           {
             if ( nbytes == 0 )
             {
               LOG_MSG_INFO1("Completed full recv from dsi callback", 0, 0, 0);
             }
	     else
	     {
               LOG_MSG_ERROR("recvfrom returned error, errno:%d", errno, 0, 0);
             }
           }
           else
           {
             qcmap_dsi_buffer = (qcmap_dsi_buffer_t *)buf;
             if (qcmap_dsi_buffer->dsi_nethandle != NULL)
             {
               qcmap_cm_process_dsi_net_evt( qcmap_dsi_buffer->dsi_nethandle,
                                        qcmap_dsi_buffer->user_data,
                                        qcmap_dsi_buffer->evt,
                                        qcmap_dsi_buffer->payload_ptr );
             }
             else if(qcmap_dsi_buffer->user_data != NULL)
             {
               /*Indicates that dsi_net is initialized*/
               if (*((uint32_t *)(qcmap_dsi_buffer->user_data)) == DSI_INITED)
               {
                 LOG_MSG_INFO1("DSI Init Callback Received", 0,0,0);
                 free(qcmap_dsi_buffer->user_data);
                 qcmap_dsi_buffer->user_data = NULL;
                 qcmap_cm_process_dsi_init_ind();
               }
             }
           }
        }
        else if ( i == qcmap_cmdq_sockfd )
        {
           LOG_MSG_INFO1("\n\nReceived Message from cmdq_context\n\n",0,0,0);
           if ( (nbytes = recvfrom(i, buf, MAX_BUF_LEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) <= 0 )
           {
             if ( nbytes == 0 )
             {
               LOG_MSG_INFO1("Completed full recv from cmdq context", 0, 0, 0);
             }
	     else
	     {
               LOG_MSG_ERROR("recvfrom returned error, errno:%d", errno, 0, 0);
             }
           }
           else
           {
             qcmap_dsi_buffer = (qcmap_dsi_buffer_t *)buf;
             if (qcmap_dsi_buffer->dsi_nethandle != NULL)
             {
               qcmap_cm_process_dsi_net_evt( qcmap_dsi_buffer->dsi_nethandle,
                                        qcmap_dsi_buffer->user_data,
                                        qcmap_dsi_buffer->evt,
                                        qcmap_dsi_buffer->payload_ptr );
             }
           }
        }
        else if ( i == qcmap_nas_sockfd )
        {
           LOG_MSG_INFO1("\n\nReceived Message from nas_context\n\n",0,0,0);
           if ( (nbytes = recvfrom(i, buf, MAX_BUF_LEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) <= 0 )
           {
             if ( nbytes == 0 )
             {
               LOG_MSG_INFO1("Completed full recv from nas context", 0, 0, 0);
             }
	     else
	     {
               LOG_MSG_ERROR("recvfrom returned error, errno:%d", errno, 0, 0);
             }
           }
           else
           {
             qcmap_nas_buffer = (qcmap_nas_buffer_t *)buf;

             qcmap_cm_process_qmi_nas_ind (
             qcmap_nas_buffer->user_handle,
             qcmap_nas_buffer->msg_id,
             qcmap_nas_buffer->ind_buf,
             qcmap_nas_buffer->ind_buf_len,
             qcmap_nas_buffer->ind_cb_data );

             free(qcmap_nas_buffer->ind_buf);
           }
        }
        else if ( i == qcmap_timer_sockfd )
        {
           LOG_MSG_INFO1("\n\nReceived Message from timer_context\n\n",0,0,0);
           if ( (nbytes = recvfrom(i, buf, MAX_BUF_LEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) <= 0 )
           {
             if ( nbytes == 0 )
             {
               LOG_MSG_INFO1("Completed full recv from qcmap_timer_sockfd context", 0, 0, 0);
             }
	     else
	     {
               LOG_MSG_ERROR("recvfrom returned error, errno:%d", errno, 0, 0);
             }
           }
           else
           {
             qcmap_timer_buffer = (qcmap_timer_buffer_t *)buf;
             qcmap_cm_process_qmi_timer_ind(qcmap_timer_buffer->msg_id);
           }
        }
        else if ( i == qcmap_sta_sockfd )
        {
           LOG_MSG_INFO1("\nReceived Message from sta_context\n", 0, 0, 0);
           addr_len = sizeof(struct sockaddr_storage);
           memset(buf,0,MAX_BUF_LEN);
           len = sizeof(qcmap_sta_buffer_t);
           if ( (nbytes = recvfrom(i, buf, len , 0, (struct sockaddr *)&their_addr, &addr_len)) <= 0 )
           {
             if ( nbytes == 0 )
             {
               LOG_MSG_INFO1("Completed full recv from sta context", 0, 0, 0);
             }
	     else
	     {
               LOG_MSG_ERROR("recvfrom returned error, errno:%d", errno, 0, 0);
             }
           }
           else
           {
             qcmap_sta_buffer = (qcmap_sta_buffer_t *)buf;
             LOG_MSG_INFO1("Received event %d from sta context",
                           qcmap_sta_buffer->event, 0, 0);
             if (qcmap_sta_buffer->event == AP_STA_CONNECTED)
             {
               memset(&qcmap_nl_wlan_buffer, 0, sizeof(qcmap_nl_wlan_buffer));
               memset(mac_addr, 0, QCMAP_MSGR_MAC_ADDR_LEN_V01);
               ds_mac_addr_pton(qcmap_sta_buffer->mac_addr, mac_addr);
               memcpy(qcmap_nl_wlan_buffer.nl_addr.mac_addr, mac_addr, QCMAP_MSGR_MAC_ADDR_LEN_V01);
               if(!(QcMapMgr->MatchMacAddrInList(&(qcmap_nl_wlan_buffer.nl_addr))))
               {
                 LOG_MSG_INFO1("No match found for the WLAN MAC,"
                               "so add a linked list node\n",0, 0, 0);
                 if (!(QcMapMgr->AddNewDeviceEntry(
                     (void*)mac_addr,
                     qcmap_sta_buffer->device_type,
                     NULL, NULL)))
                 {
                   LOG_MSG_ERROR("Error in adding a new device entry ", 0, 0, 0);
                 }
                 else
                 {
                   if (qcmap_nl_send_getneigh_event() != QCMAP_NL_SUCCESS)
                     LOG_MSG_ERROR("Error sending GETNEIGH message",0,0,0);
                 }
               }
             }
             else if (qcmap_sta_buffer->event == AP_STA_DISCONNECTED)
             {
               if(!(QcMapMgr->DeleteDeviceEntryInfo(
                  (void*)qcmap_sta_buffer->mac_addr,
                  qcmap_sta_buffer->device_type)))
               {
                 LOG_MSG_ERROR("Error in deleting a device entry ", 0, 0, 0);
               }
             }
             else
             {
               qcmap_cm_process_sta_ind (qcmap_sta_buffer->sta_cookie,
                                         qcmap_sta_buffer->event);
             }
           }
        }
        else if ( i == qcmap_nl_sockfd )
        {
           nbytes = recvfrom(i, buf, MAX_BUF_LEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len);
           if (nbytes > 0)
           {
              LOG_MSG_INFO1("Completed full recv from netlink_thread context Recvd bytes# %d ",nbytes, 0, 0);
           }
           else
           {
              LOG_MSG_ERROR("recvfrom returned error, errno:%d", errno, 0, 0);
              break;
           }
           qcmap_nl_buffer = (qcmap_nl_sock_msg_t *)buf;
           LOG_MSG_INFO1(" Netlink Message Info: Event = %d, Interface = %d",
                         qcmap_nl_buffer->nl_event,qcmap_nl_buffer->nl_iface,0);

           if ((qcmap_nl_buffer->nl_iface == QCMAP_NL_USB) && (QcMapMgr))
           {
             if(qcmap_nl_buffer->nl_event == QCMAP_NL_NEWNEIGH)
             {
               if (QcMapMgr->GetUSBEnable() == true)
               {
                 usb_ip = QcMapMgr->GetUSBNeighIP();
                 if((usb_ip != qcmap_nl_buffer->nl_addr.ip_addr) &&
                    (qcmap_nl_buffer->nl_addr.isValidIPv4address))
                 {
                   if(usb_ip != 0)
                   {
                     QcMapMgr->DelUsbConntrack();
                   }
                   QcMapMgr->SetUSBNeighIP(qcmap_nl_buffer->nl_addr.ip_addr);
                 }
                 /* Check if an entry already exists. If not add the entry. */
                 if(!(QcMapMgr->MatchMacAddrInList(&(qcmap_nl_buffer->nl_addr))))
                 {
                   LOG_MSG_INFO1("No match found for the USB MAC,"
                                 "so add a linked list node\n",0, 0, 0);

                   QcMapMgr->SetUSBMac(qcmap_nl_buffer->nl_addr.mac_addr);
                   if(!(QcMapMgr->AddNewDeviceEntry(
                      (void*)qcmap_nl_buffer->nl_addr.mac_addr,
                      QCMAP_MSGR_DEVICE_TYPE_USB_V01,
                      &qcmap_nl_buffer->nl_addr.ip_addr,
                      qcmap_nl_buffer->nl_addr.ip_v6_addr,
                      qcmap_nl_buffer->nl_addr.isValidIPv4address,
                      qcmap_nl_buffer->nl_addr.isValidIPv6address)))
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
               else
               {
                 LOG_MSG_INFO1("Recieved NEWNEIGH Event before USB is enabled from QTI --- Ignore it",0,0,0);
               }
             }
             else if(qcmap_nl_buffer->nl_event == QCMAP_NL_NEWADDR)
             {
               LOG_MSG_INFO1("Received RTM_NEWADDR for IPv4 on ecm0",0,0,0);
               QcMapMgr->ProcessCradleAddrAssign(&qcmap_handle);
             }
           }
           //If netlink event is received from WLAN interface
           else if((qcmap_nl_buffer->nl_iface == QCMAP_NL_WLAN) && (QcMapMgr))
           {
             if (qcmap_nl_buffer->nl_event == QCMAP_NL_NEWNEIGH)
             {
               //Store the MAC and IP address in QCMAP Manager context
               LOG_MSG_INFO1("Received RTM_NEWNEIGH for WLAN Client",0, 0, 0);
               if(!(QcMapMgr->MatchMacAddrInList(&(qcmap_nl_buffer->nl_addr))))
               {
                 LOG_MSG_ERROR("No match found for the WLAN MAC -"
                               "Recived a NEWNEIGH Event before AP-STA-CONNECTED",0, 0, 0);
               }
             }
             else if (qcmap_nl_buffer->nl_event == QCMAP_NL_NEWADDR)
             {
               LOG_MSG_INFO1("Received RTM_NEWADDR for IPv4 on wlan0",0,0,0);
               QcMapMgr->ProcessAddrAssign(&qcmap_handle, WLAN0);
             }
             else
             {
               LOG_MSG_ERROR("Invalid Event received from Netlink Thread"
                             "Event =%d", qcmap_nl_buffer->nl_event, 0, 0);
             }
           }
           else if ((qcmap_nl_buffer->nl_iface == QCMAP_NL_PPP) && (QcMapMgr))
           {
             if (qcmap_nl_buffer->nl_event == QCMAP_NL_PPP_IPV6_ROUTE)
             {
               QcMapMgr->StorePPPIPv6IID(qcmap_nl_buffer->nl_addr.ip_v6_addr);
             }
             else
             {
               LOG_MSG_ERROR("Invalid Event received from Netlink Thread Event =%d",
                             qcmap_nl_buffer->nl_event, 0, 0);
             }
           }
           // If netlink event is recieved on Bridge interface when we are in AP-STA Mode
           else if((qcmap_nl_buffer->nl_iface == QCMAP_NL_BRIDGE) && (QcMapMgr))
           {
             if (qcmap_nl_buffer->nl_event == QCMAP_NL_NEWADDR)
             {
               LOG_MSG_INFO1("Received RTM_NEWADDR for IPv4 on bridge0",0,0,0);
               if (QcMapMgr->IsAPSTABridgeActivated())
                 QcMapMgr->ProcessAddrAssign(&qcmap_handle, BRIDGE_IFACE);
               else
                 LOG_MSG_INFO1(" Ignorning RTM_NEWADDR for IPv4 on bridge0 since we are in Router Mode",0,0,0);
             }
             else
             {
               LOG_MSG_ERROR("Invalid Event received from Netlink Thread"
                             "Event =%d", qcmap_nl_buffer->nl_event, 0, 0);
             }
           }
           else if ((qcmap_nl_buffer->nl_iface == QCMAP_NL_ANY) && (QcMapMgr))
           {
             if (qcmap_nl_buffer->nl_event == QCMAP_NL_RA)
             {
               QcMapMgr->UpdateGlobalV6addr(&(qcmap_nl_buffer->nl_addr));
             }
             else
             {
               LOG_MSG_ERROR("Invalid Event received from Netlink Thread Event =%d",
                             qcmap_nl_buffer->nl_event, 0, 0);
             }
           }
        }
        else /* QMI message from the client */
        {
          LOG_MSG_INFO1("\n\nReceived QMI Message\n\n",0,0,0);
          qmi_csi_handle_event(qmi_qcmap_msgr_state.service_handle, &os_params);
        }
      }
    }
  }
  qmi_csi_unregister(qmi_qcmap_msgr_state.service_handle);
  pthread_join(qcmap_netlink_thread, NULL);
  LOG_MSG_INFO1("QMI QCMAP CM service exiting",0,0,0);
  return 0;
}


static qmi_csi_cb_error qmi_qcmap_msgr_svc_connect_cb
(
  qmi_client_handle          client_handle,
  void                      *service_handle,
  void                     **connection_handle
)
{
  qmi_qcmap_msgr_client_info_type    *clnt_info_ptr;
  qmi_qcmap_msgr_state_info_type     *qcmap_svc_ptr;
  unsigned int                     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(connection_handle != NULL);
  ds_assert(service_handle != NULL);

  qcmap_svc_ptr = (qmi_qcmap_msgr_state_info_type *) service_handle;

  for (index=0; index < QCMAP_MSGR_MAX_CLIENT_HANDLES; index++)
  {
    if (NULL == qcmap_svc_ptr->client_handle_list[index])
    {
      LOG_MSG_INFO1("\n qmi_qcmap_msgr_svc_connect_cb: index=%d\n",index,0,0);
      clnt_info_ptr = (qmi_qcmap_msgr_client_info_type *)malloc(
                        sizeof(qmi_qcmap_msgr_client_info_type));
      if(!clnt_info_ptr)
      {
        LOG_MSG_INFO1("qcmap_msgr_svc_connect_cb: Out of mem",0,0,0);
        return QMI_CSI_CB_CONN_REFUSED;
      }
      else
        break;
    }
  }

  if (index == QCMAP_MSGR_MAX_CLIENT_HANDLES)
  {
    LOG_MSG_ERROR("Client handle slots exhausted", 0, 0, 0);
    return QMI_CSI_CB_NO_MEM;
  }

  memset(clnt_info_ptr, 0, sizeof(qmi_qcmap_msgr_client_info_type));
  clnt_info_ptr->clnt = client_handle;
  qcmap_svc_ptr->num_clients++;
  qcmap_svc_ptr->client_handle_list[index] = *connection_handle = clnt_info_ptr;
  LOG_MSG_INFO1("qcmap_msgr_svc_connect_cb: Alloc client 0x%p",
                qcmap_svc_ptr->client_handle_list[index],0,0);

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_svc_connect_cb() */

static void qmi_qcmap_msgr_svc_disconnect_cb
(
  void                      *connection_handle,
  void                      *service_handle
)
{
  qmi_qcmap_msgr_state_info_type       *qcmap_svc_ptr;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  unsigned int                         client_index;
  unsigned int                         index;
  int                                  ret_val,err_num;
  short                                ps_errno;
  boolean                              last_client=TRUE;
  qmi_qcmap_msgr_client_info_type      *clnt_info;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_upnp_mode_enum_v01        status = 0;
  int                client_count = 1;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(service_handle != NULL);
  qcmap_svc_ptr = (qmi_qcmap_msgr_state_info_type *)service_handle;

  for (client_index=0;client_index < QCMAP_MSGR_MAX_CLIENT_HANDLES; client_index++)
  {
    if (qcmap_svc_ptr->client_handle_list[client_index] == connection_handle)
      break;
  }
  if (client_index == QCMAP_MSGR_MAX_CLIENT_HANDLES)
  {
    LOG_MSG_ERROR("qcmap_msgr_svc_disconnect_cb: Invalid Handle %p", connection_handle,0,0);
    return;
  }

  /* Increment Client count to 2 if UPnP is running since it has it's own mobile_ap client */
  if (QcMapMgr->GetUPNPStatus(&status, &qmi_err_num) && status == QCMAP_MSGR_UPNP_MODE_UP_V01)
  {
    client_count = 2;
  }

  clnt_info = (qmi_qcmap_msgr_client_info_type *)qcmap_svc_ptr->client_handle_list[client_index];

  if(clnt_info->client_enabled)
  {
    if(qcmap_svc_ptr->client_ref_count > client_count)
    {
      qcmap_svc_ptr->client_ref_count--;
      clnt_info->client_enabled = FALSE;
    }
    else if(qcmap_svc_ptr->client_ref_count == client_count)
    {
     if (ret_val=QcMapMgr->Disable(&err_num, &qmi_err_num))
      {
        if (err_num == QCMAP_CM_EALDDISCONN)
        {
          qcmap_svc_ptr->client_ref_count -= client_count;
          clnt_info->client_enabled = FALSE;
          LOG_MSG_INFO1("MobileAP Disable succeeds.",0,0,0);
        }
        else
        {
          qcmap_svc_ptr->client_ref_count -= client_count;
          clnt_info->client_enabled = FALSE;
          LOG_MSG_INFO1("MobileAP Disable in progress.",0,0,0);
        }
      }
      else
      {
        LOG_MSG_ERROR("MobileAP Disable request fails.",0,0,0);
      }
    }
  }

  qcmap_svc_ptr->num_clients--;
  LOG_MSG_INFO1("qcmap_msgr_svc_disconnect_cb: Releasing client %p, num_clients: %d",
                qcmap_svc_ptr->client_handle_list[client_index], qcmap_svc_ptr->num_clients,0);
  free(qcmap_svc_ptr->client_handle_list[client_index]);
  qcmap_svc_ptr->client_handle_list[client_index] = NULL;


  for (index=0; (index < QCMAP_MSGR_MAX_CLIENT_HANDLES); index++)
  {
    if (qcmap_svc_ptr->client_handle_list[index] != NULL)
    {
      last_client=FALSE;
      break;
    }
  }

  LOG_MSG_INFO1("\nqcmap_msgr_svc_disconnect_cb: last_client: %d", last_client,0,0);
  if (last_client)
  {
    LOG_MSG_INFO1("qcmap_msgr_svc_disconnect_cb: Last Client True!!",0,0,0);
    /* Terminate SoftAP handle, if it still exists, since the last client has disconnected */
    map_handle = &qcmap_handle;
    if (map_handle->handle != 0)
    {
      if ((map_handle->cb_ptr) && (QcMapMgr->qcmap_tear_down_in_progress == false))
      {
        LOG_MSG_INFO1("\nqcmap_msgr_svc_disconnect_cb: Freeing cbPtr!!",0,0,0);
        free(map_handle->cb_ptr);
        map_handle->cb_ptr = NULL;
      }
      map_handle->handle = 0;
    }
  }
  return;

} /* qmi_qcmap_msgr_svc_disconnect_cb() */


static qmi_csi_cb_error qmi_qcmap_msgr_handle_client_req_cb
(
  void                     *connection_handle,
  qmi_req_handle           req_handle,
  unsigned int             msg_id,
  void                     *req_c_struct,
  unsigned int             req_c_struct_len,
  void                     *service_handle
)
{
  qmi_csi_cb_error                rc;
  qmi_qcmap_msgr_client_info_type   *clnt_info;
  qmi_qcmap_msgr_state_info_type    *qcmap_svc_ptr;
  unsigned int                    client_index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(connection_handle != NULL);
  ds_assert(service_handle != NULL);

  rc = QMI_CSI_CB_INTERNAL_ERR;
  clnt_info = (qmi_qcmap_msgr_client_info_type*)connection_handle;
  qcmap_svc_ptr = (qmi_qcmap_msgr_state_info_type*) service_handle;

  LOG_MSG_INFO1("\nEntered qmi_qcmap_msgr_handle_client_req_cb",0,0,0);

  for (client_index=0;client_index < QCMAP_MSGR_MAX_CLIENT_HANDLES; client_index++)
  {
    LOG_MSG_INFO1("\nLooking for client %p...", clnt_info,0,0);
    if(qcmap_svc_ptr->client_handle_list[client_index] == clnt_info)
    {
      LOG_MSG_INFO1("Found",0,0,0);
      break;
    }
  }
  if (client_index == QCMAP_MSGR_MAX_CLIENT_HANDLES)
  {
      LOG_MSG_INFO1("qcmap_msgr_handle_req_cb: Invalid clnt handle %p",
                    clnt_info,0,0);
      return rc;
  }

  if(msg_id < (sizeof(req_handle_table) / sizeof(*req_handle_table)))
  {
    if(req_handle_table[msg_id])
    {
      rc = req_handle_table[msg_id] (clnt_info, req_handle, msg_id,
                                     req_c_struct, req_c_struct_len,
                                     service_handle);
    }
    else
    {
      LOG_MSG_INFO1("qcmap_msgr_handle_req_cb: NULL message ID handler: %d",
                    msg_id,0,0);
    }
  }
  else
  {
    LOG_MSG_INFO1("qcmap_msgr_handle_req_cb: Invalid message ID: %d", msg_id,0,0);
  }

  return rc;
} /* qmi_qcmap_msgr_handle_client_req_cb() */


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_WWAN_STATUS_IND()

  DESCRIPTION
    Dispatches a unicast indication to notify registered QCMobileAP client
      about WWAN status.

  RETURN VALUE
    None

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
void qmi_qcmap_msgr_wwan_status_ind
(
  void                           *user_data,
  qcmap_msgr_wwan_status_enum_v01  conn_status,
  qcmap_msgr_wwan_call_end_type_enum_v01 call_end_type,
  int call_end_reason_code
)
{
  qmi_qcmap_msgr_client_info_type      *qcmap_cp=NULL;
  qcmap_msgr_wwan_status_ind_msg_v01     ind_msg;
  qmi_qcmap_msgr_status_cb_data   *cb_data;
  qmi_csi_error                     rc;
  unsigned int                      client_index=0;
  unsigned int                      softAp_index;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qmi_qcmap_msgr_state_info_type *qcmap_sp;
  union
  {
    qcmap_msgr_bring_up_wwan_ind_msg_v01   bring_up_ind;
    qcmap_msgr_tear_down_wwan_ind_msg_v01  tear_down_ind;
  }wwan_op_ind_msg;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(user_data != NULL);
  LOG_MSG_INFO1("\n qmi_qcmap_msgr_wwan_status_ind \n",0,0,0);
  cb_data = (qmi_qcmap_msgr_status_cb_data*)user_data;
  qcmap_sp = cb_data->svc_cb;
  qcmap_cp = (qmi_qcmap_msgr_client_info_type *)(qcmap_sp->client_handle_list[0]);

  //go through all the handles and see which client is waiting for wwan status indication
  for (client_index=0;client_index < QCMAP_MSGR_MAX_CLIENT_HANDLES; client_index++)
  {
    qcmap_cp = (qmi_qcmap_msgr_client_info_type *)(qcmap_sp->client_handle_list[client_index]);
    if ( qcmap_cp && (qcmap_cp->wwan_op_ind_pending || qcmap_cp->wwan_status_ind_regd) )
    {
      if (qcmap_cp->wwan_op_ind_pending)
      {
        if (qcmap_cp->wwan_op_ind_msg_id == QMI_QCMAP_MSGR_BRING_UP_WWAN_IND_V01)
        {
          wwan_op_ind_msg.bring_up_ind.conn_status = conn_status;
          wwan_op_ind_msg.bring_up_ind.mobile_ap_handle = cb_data->map_instance;

          if ( conn_status == QCMAP_MSGR_WWAN_STATUS_CONNECTING_FAIL_V01 ||
               conn_status == QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING_FAIL_V01)
          {
            wwan_op_ind_msg.bring_up_ind.wwan_call_end_reason_valid = TRUE;
            wwan_op_ind_msg.bring_up_ind.wwan_call_end_reason.wwan_call_end_reason_type = call_end_type;
            wwan_op_ind_msg.bring_up_ind.wwan_call_end_reason.wwan_call_end_reason_code = call_end_reason_code;
            LOG_MSG_INFO1("\nSending WWAN status ind_msg.wwan_call_end_reason_type =%d ind_msg.wwan_call_end_reason_code=%d ",
                              call_end_type,call_end_reason_code,0);
          }
          LOG_MSG_INFO1("\nSending WWAN status %d to client %p", conn_status, qcmap_cp,0);
          rc = qmi_csi_send_ind(qcmap_cp->clnt, QMI_QCMAP_MSGR_BRING_UP_WWAN_IND_V01,
                                &wwan_op_ind_msg.bring_up_ind,
                                sizeof(qcmap_msgr_bring_up_wwan_ind_msg_v01));
        }
	else if (qcmap_cp->wwan_op_ind_msg_id == QMI_QCMAP_MSGR_TEAR_DOWN_WWAN_IND_V01)
        {
          wwan_op_ind_msg.tear_down_ind.conn_status = conn_status;
          wwan_op_ind_msg.tear_down_ind.mobile_ap_handle = cb_data->map_instance;

          if ( conn_status == QCMAP_MSGR_WWAN_STATUS_DISCONNECTED_V01 ||
               conn_status == QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTED_V01)
          {
            wwan_op_ind_msg.tear_down_ind.wwan_call_end_reason_valid = TRUE;
            wwan_op_ind_msg.tear_down_ind.wwan_call_end_reason.wwan_call_end_reason_type = call_end_type;
            wwan_op_ind_msg.tear_down_ind.wwan_call_end_reason.wwan_call_end_reason_code = call_end_reason_code;
            LOG_MSG_INFO1("\nSending WWAN status call_end_reason_type =%d call_end_reason_code=%d ",
                            call_end_type,call_end_reason_code,0);
          }
          LOG_MSG_INFO1("\nSending WWAN status %d to client %p", conn_status, qcmap_cp,0);
          rc = qmi_csi_send_ind(qcmap_cp->clnt, QMI_QCMAP_MSGR_TEAR_DOWN_WWAN_IND_V01,
                               &wwan_op_ind_msg.tear_down_ind,
                               sizeof(qcmap_msgr_tear_down_wwan_ind_msg_v01));
        }
        if ( rc != QMI_CSI_NO_ERR )
        {
          LOG_MSG_ERROR("Cannot send WWAN op pending ind to client %p, error %d",
                        qcmap_cp, rc,0);
        }
        else
        {
          LOG_MSG_INFO1("Sent WWAN op pending indication %d to client %p",
                        qcmap_cp->wwan_op_ind_msg_id, qcmap_cp,0);
        }

        qcmap_cp->wwan_op_ind_pending = FALSE;
      }
      else if (qcmap_cp->wwan_status_ind_regd)
      {
        memset(&ind_msg, 0, sizeof(qcmap_msgr_wwan_status_ind_msg_v01));

        ind_msg.wwan_status = conn_status;
        if ( conn_status == QCMAP_MSGR_WWAN_STATUS_CONNECTING_FAIL_V01 ||
             conn_status == QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING_FAIL_V01 ||
             conn_status == QCMAP_MSGR_WWAN_STATUS_DISCONNECTED_V01 ||
             conn_status == QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTED_V01)
        {
          ind_msg.wwan_call_end_reason_valid = TRUE;
          ind_msg.wwan_call_end_reason.wwan_call_end_reason_type = call_end_type;
          ind_msg.wwan_call_end_reason.wwan_call_end_reason_code = call_end_reason_code;
        }
        LOG_MSG_INFO1("\nSending WWAN status %d to client %p", conn_status, qcmap_cp,0);
        rc = qmi_csi_send_ind(qcmap_cp->clnt, QMI_QCMAP_MSGR_WWAN_STATUS_IND_V01, &ind_msg, sizeof(qcmap_msgr_wwan_status_ind_msg_v01));
        if ( rc != QMI_CSI_NO_ERR )
        {
          LOG_MSG_ERROR("\nCannot send WWAN ind to client %p, error %d", qcmap_cp, rc, 0);
        }
        else
        {
          LOG_MSG_INFO1("\nSent WWAN ind to client %p, error %d", qcmap_cp, rc, 0);
        }
      }
    }
  }
} /* qmi_qcmap_msgr_wwan_status_ind() */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_WWAN_STATUS_IND()

  DESCRIPTION
    Dispatches a unicast indication to notify registered QCMobileAP client
      about Mobile AP status.

  RETURN VALUE
    None

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
void qmi_qcmap_msgr_mobile_ap_status_ind
(
  void                           *user_data,
  qcmap_msgr_mobile_ap_status_enum_v01  conn_status
)
{
  qmi_qcmap_msgr_client_info_type      *qcmap_cp=NULL;
  qcmap_msgr_mobile_ap_status_ind_msg_v01     ind_msg;
  qmi_qcmap_msgr_status_cb_data   *cb_data;
  qmi_csi_error                     rc;
  unsigned int                      client_index=0;
  unsigned int                      softAp_index;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qmi_qcmap_msgr_state_info_type *qcmap_sp;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(user_data != NULL);

  cb_data = (qmi_qcmap_msgr_status_cb_data*)user_data;
  qcmap_sp = cb_data->svc_cb;
  qcmap_cp = (qmi_qcmap_msgr_client_info_type *)(qcmap_sp->client_handle_list[0]);
  //go through all the handles and see which client is waiting for wwan status indication
  for (client_index=0;client_index < QCMAP_MSGR_MAX_CLIENT_HANDLES; client_index++)
  {
    qcmap_cp = (qmi_qcmap_msgr_client_info_type *)(qcmap_sp->client_handle_list[client_index]);
    if (qcmap_cp && qcmap_cp->qcmap_status_ind_regd)
    {
      memset(&ind_msg, 0, sizeof(qcmap_msgr_mobile_ap_status_ind_msg_v01));
      ind_msg.mobile_ap_status = conn_status;
      LOG_MSG_INFO1("\nSending Mobile AP status %d to client %p", conn_status, qcmap_cp,0);
      rc = qmi_csi_send_ind(qcmap_cp->clnt, QMI_QCMAP_MSGR_MOBILE_AP_STATUS_IND_V01, &ind_msg, sizeof(qcmap_msgr_mobile_ap_status_ind_msg_v01));
      if ( rc != QMI_CSI_NO_ERR )
      {
        LOG_MSG_ERROR("\nCannot send Mobile AP status to client %p, error %d", qcmap_cp, rc, 0);
      }
      else
      {
        LOG_MSG_INFO1("\nSent Mobile AP status to client %p, error %d", qcmap_cp, rc, 0);
      }
    }
  }
} /* qmi_qcmap_msgr_mobile_ap_status_ind() */

/*===========================================================================
  FUNCTION QMI_QCMAP_STATION_MODE_STATUS_IND()

  DESCRIPTION
    Dispatches a unicast indication to notify registered QCMobileAP client
      about station mode.

  RETURN VALUE
    None

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
void qmi_qcmap_msgr_station_mode_status_ind
(
  void                           *user_data,
  qcmap_msgr_station_mode_status_enum_v01  conn_status
)
{
  qmi_qcmap_msgr_client_info_type      *qcmap_cp=NULL;
  qcmap_msgr_station_mode_status_ind_msg_v01     ind_msg;
  qmi_qcmap_msgr_status_cb_data   *cb_data;
  qmi_csi_error                     rc;
  unsigned int                      client_index=0;
  unsigned int                      softAp_index;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qmi_qcmap_msgr_state_info_type *qcmap_sp;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(user_data != NULL);

  cb_data = (qmi_qcmap_msgr_status_cb_data*)user_data;
  qcmap_sp = cb_data->svc_cb;
  qcmap_cp = (qmi_qcmap_msgr_client_info_type *)(qcmap_sp->client_handle_list[0]);

  //go through all the handles and see which client is waiting for station mode status indication
  for (client_index=0;client_index < QCMAP_MSGR_MAX_CLIENT_HANDLES; client_index++)
  {
    qcmap_cp = (qmi_qcmap_msgr_client_info_type *)(qcmap_sp->client_handle_list[client_index]);
    if (qcmap_cp && qcmap_cp->station_mode_status_ind_regd)
    {
      memset(&ind_msg, 0, sizeof(qcmap_msgr_station_mode_status_ind_msg_v01));
      ind_msg.station_mode_status = conn_status;
      LOG_MSG_INFO1("\nSending Station mode status %d to client %p", conn_status, qcmap_cp,0);
      rc = qmi_csi_send_ind(qcmap_cp->clnt, QMI_QCMAP_MSGR_STATION_MODE_STATUS_IND_V01, &ind_msg, sizeof(qcmap_msgr_station_mode_status_ind_msg_v01));
      if ( rc != QMI_CSI_NO_ERR )
      {
        LOG_MSG_ERROR("\nCannot send Station mode ind to client %p, error %d", qcmap_cp, rc, 0);
      }
      else
      {
        LOG_MSG_INFO1("\nSent station mode ind to client %p, error %d", qcmap_cp, rc, 0);
      }
    }
  }
} /* qmi_qcmap_msgr_station_mode_status_ind() */


/*===========================================================================
  FUNCTION QMI_QCMAP_CRADLE_MODE_STATUS_IND()

  DESCRIPTION
    Dispatches a unicast indication to notify registered QCMobileAP client
      about cradle mode.

  RETURN VALUE
    None

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
void qmi_qcmap_msgr_cradle_mode_status_ind
(
  void                           *user_data,
  qcmap_msgr_cradle_status_enum_v01     state
)
{
  qmi_qcmap_msgr_client_info_type      *qcmap_cp=NULL;
  qcmap_msgr_cradle_mode_status_ind_msg_v01     ind_msg;
  qmi_qcmap_msgr_status_cb_data   *cb_data;
  qmi_csi_error                     rc;
  unsigned int                      client_index=0;
  unsigned int                      softAp_index;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qmi_qcmap_msgr_state_info_type *qcmap_sp;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ASSERT(user_data);

  cb_data = (qmi_qcmap_msgr_status_cb_data*)user_data;
  qcmap_sp = cb_data->svc_cb;
  qcmap_cp = (qmi_qcmap_msgr_client_info_type *)(qcmap_sp->client_handle_list[0]);

  //go through all the handles and see which client is waiting for cradle mode status indication
  for (client_index=0;client_index < QCMAP_MSGR_MAX_CLIENT_HANDLES; client_index++)
  {
    qcmap_cp = (qmi_qcmap_msgr_client_info_type *)(qcmap_sp->client_handle_list[client_index]);
    if (qcmap_cp && qcmap_cp->station_mode_status_ind_regd)
    {
      memset(&ind_msg, 0, sizeof(qcmap_msgr_cradle_mode_status_ind_msg_v01));
      ind_msg.cradle_status = state;
      LOG_MSG_INFO1("\nSending Cradle mode status %d to client %p", state, qcmap_cp,0);
      rc = qmi_csi_send_ind(qcmap_cp->clnt, QMI_QCMAP_MSGR_CRADLE_MODE_STATUS_IND_V01, &ind_msg, sizeof(qcmap_msgr_cradle_mode_status_ind_msg_v01));
      if ( rc != QMI_CSI_NO_ERR )
      {
        LOG_MSG_ERROR("\nCannot send Cradle mode ind to client %p, error %d", qcmap_cp, rc, 0);
      }
      else
      {
        LOG_MSG_INFO1("\nSent Cradle mode ind to client %p, error %d", qcmap_cp, rc, 0);
      }
    }
  }
} /* qmi_qcmap_msgr_cradle_mode_status_ind() */


static qmi_csi_cb_error  qmi_qcmap_msgr_wwan_status_ind_reg
(
  qmi_qcmap_msgr_client_info_type      *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                     *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle)
{
  qmi_qcmap_msgr_state_info_type                    *qcmap_sp;
  qcmap_msgr_wwan_status_ind_register_req_msg_v01   *req_ptr;
  qcmap_msgr_wwan_status_ind_register_resp_msg_v01  resp_msg;
  qmi_csi_cb_error   rc;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type*)service_handle;
  req_ptr = (qcmap_msgr_wwan_status_ind_register_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_wwan_status_ind_register_resp_msg_v01));
  rc = QMI_CSI_CB_INTERNAL_ERR;

  clnt_info->wwan_status_ind_regd = req_ptr->register_indication;
  LOG_MSG_INFO1 ("Client %p, reg %d for WWAN status", clnt_info, clnt_info->wwan_status_ind_regd,0);
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg, sizeof(qcmap_msgr_wwan_status_ind_register_resp_msg_v01));
  return QMI_CSI_CB_NO_ERR;
}

static qmi_csi_cb_error  qmi_qcmap_msgr_mobile_ap_status_ind_reg
(
  qmi_qcmap_msgr_client_info_type      *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                     *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle)
{
  qmi_qcmap_msgr_state_info_type                    *qcmap_sp;
  qcmap_msgr_mobile_ap_status_ind_register_req_msg_v01   *req_ptr;
  qcmap_msgr_mobile_ap_status_ind_register_resp_msg_v01  resp_msg;
  qmi_csi_cb_error   rc;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type*)service_handle;
  req_ptr = (qcmap_msgr_mobile_ap_status_ind_register_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_mobile_ap_status_ind_register_resp_msg_v01));
  rc = QMI_CSI_CB_INTERNAL_ERR;

  clnt_info->qcmap_status_ind_regd = req_ptr->register_indication;
  LOG_MSG_INFO1 ("Client %p, reg %d for Mobile AP status", clnt_info, clnt_info->qcmap_status_ind_regd,0);
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg, sizeof(qcmap_msgr_mobile_ap_status_ind_register_resp_msg_v01));
  return QMI_CSI_CB_NO_ERR;
}

static qmi_csi_cb_error  qmi_qcmap_msgr_station_mode_status_ind_reg
(
  qmi_qcmap_msgr_client_info_type      *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                     *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle)
{
  qmi_qcmap_msgr_state_info_type                    *qcmap_sp;
  qcmap_msgr_station_mode_status_ind_register_req_msg_v01   *req_ptr;
  qcmap_msgr_station_mode_status_ind_register_resp_msg_v01  resp_msg;
  qmi_csi_cb_error   rc;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type*)service_handle;
  req_ptr = (qcmap_msgr_station_mode_status_ind_register_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_station_mode_status_ind_register_resp_msg_v01));
  rc = QMI_CSI_CB_INTERNAL_ERR;

  clnt_info->station_mode_status_ind_regd = req_ptr->register_indication;
  LOG_MSG_INFO1 ("Client %p, reg %d for STATION Mode status", clnt_info, clnt_info->station_mode_status_ind_regd,0);
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg, sizeof(qcmap_msgr_station_mode_status_ind_register_resp_msg_v01));
  return QMI_CSI_CB_NO_ERR;
}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_MOBILE_AP_ENABLE()

  DESCRIPTION
    Enable SoftAP functionality on modem.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_mobile_ap_enable
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_mobile_ap_enable_resp_msg_v01   resp_msg;
  qmi_qcmap_msgr_status_cb_data    *cb_data_ptr=NULL;

  int              handle;
  int              ret_val;
  unsigned int     index;
  qmi_error_type_v01 qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;

  LOG_MSG_INFO1 ("Client %p enabling Mobile AP", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_mobile_ap_enable_resp_msg_v01));

  /*---------------------------------------------------------------------
    Enable SoftAP
  ---------------------------------------------------------------------*/

  if (qcmap_handle.cb_ptr == NULL)
  {
    cb_data_ptr = (qmi_qcmap_msgr_status_cb_data*)malloc(
                         sizeof(qmi_qcmap_msgr_status_cb_data));

    if (cb_data_ptr == NULL)
    {
      LOG_MSG_ERROR("Out of mem for WWAN cb data", 0, 0, 0);
      return QMI_CSI_CB_NO_MEM;
    }

    cb_data_ptr->svc_cb = qcmap_sp;
    cb_data_ptr->map_instance = QCMAP_MSGR_SOFTAP_HANDLE;
    cb_data_ptr->wwan_cb  = qmi_qcmap_msgr_wwan_status_ind;
    cb_data_ptr->qcmap_cb = qmi_qcmap_msgr_mobile_ap_status_ind;
    cb_data_ptr->sta_cb = qmi_qcmap_msgr_station_mode_status_ind;
    cb_data_ptr->cradle_cb = qmi_qcmap_msgr_cradle_mode_status_ind;

    qcmap_handle.cb_ptr = cb_data_ptr;
  }

  if(!clnt_info->client_enabled)
  {
    if(qcmap_sp->client_ref_count > 0)
    {
      clnt_info->client_enabled = TRUE;
      qcmap_sp->client_ref_count++;
      index = QCMAP_MSGR_SOFTAP_HANDLE;
      LOG_MSG_INFO1("Enabled MobileAP inst %d",
                  index,0,0);
      resp_msg.mobile_ap_handle_valid = TRUE;
      resp_msg.mobile_ap_handle = index;
      qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                        sizeof(qcmap_msgr_mobile_ap_enable_resp_msg_v01));

      return QMI_CSI_CB_NO_ERR;
    }
    else if(qcmap_sp->client_ref_count == 0)
    {

      if (QcMapMgr->Enable(&handle, &qcmap_handle, &qmi_err_num))
      {
        clnt_info->client_enabled = TRUE;
        qcmap_sp->client_ref_count++;
        LOG_MSG_INFO1("MobileAP Enable succeeds.\n",0,0,0);
      }
      else
      {
        LOG_MSG_INFO1("MobileAP Enable fails.\n",0,0,0);
        resp_msg.mobile_ap_handle_valid = FALSE;
        resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
        resp_msg.resp.error = qmi_err_num;
        qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                        sizeof(qcmap_msgr_mobile_ap_enable_resp_msg_v01));
        return QMI_CSI_CB_NO_ERR;
      }


      qcmap_handle.handle = handle;

      index = QCMAP_MSGR_SOFTAP_HANDLE;
      LOG_MSG_INFO1("Enabled MobileAP inst %d",
                    index,0,0);
      resp_msg.mobile_ap_handle_valid = TRUE;
      resp_msg.mobile_ap_handle = index;
      qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                        sizeof(qcmap_msgr_mobile_ap_enable_resp_msg_v01));

      return QMI_CSI_CB_NO_ERR;
    }
  }
  else
  {
    LOG_MSG_ERROR("The client has already enabled MobileAP", 0, 0, 0);
    resp_msg.mobile_ap_handle_valid = FALSE;
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_NO_EFFECT_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_mobile_ap_enable_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

} /* qmi_qcmap_msgr_mobile_ap_enable() */


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_MOBILE_AP_DISABLE()

  DESCRIPTION
    Disable SoftAP functionality on modem.

  PARAMETERS
    sp:          QMI_QCMAP's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_mobile_ap_disable
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type            *qcmap_sp;
  qcmap_msgr_mobile_ap_disable_req_msg_v01   *req_ptr;
  qcmap_msgr_mobile_ap_disable_resp_msg_v01   resp_msg;
  qmi_qcmap_msgr_softap_handle_type         *map_handle;
  qcmap_msgr_upnp_mode_enum_v01             status = 0;
  int16              ps_errno;
  int                return_val, err_num=0;
  unsigned int             index;
  qmi_error_type_v01 qmi_err_num;
  int                client_count = 1;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type*)service_handle;
  req_ptr = (qcmap_msgr_mobile_ap_disable_req_msg_v01*)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_mobile_ap_disable_resp_msg_v01));

  LOG_MSG_INFO1("qmi_qcmap_msgr_mobile_ap_disable::enter",0,0,0);
  /*---------------------------------------------------------------------
    Disable SOftAP. This would trigger all RmNet calls to stop.
  ---------------------------------------------------------------------*/
  index = req_ptr->mobile_ap_handle;
  if (index != QCMAP_MSGR_SOFTAP_HANDLE)
  {
    LOG_MSG_ERROR("Incorrect handle passed %d",req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_mobile_ap_disable_resp_msg_v01));

    return QMI_CSI_CB_NO_ERR;
  }

  /* Increment Client count to 2 if UPnP is running since it has it's own mobile_ap client */
  if (QcMapMgr->GetUPNPStatus(&status, &qmi_err_num) && status == QCMAP_MSGR_UPNP_MODE_UP_V01)
  {
    client_count = 2;
  }

  map_handle = &qcmap_handle;
  if (map_handle->handle == 0)
  {
    LOG_MSG_ERROR("MobileAP inst %d already disabled",index,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_NO_EFFECT_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_mobile_ap_disable_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1 ("Client %p disabling MobileAP inst %d",
                 clnt_info, index,0);
  LOG_MSG_INFO1("qcmap_sp->client_ref_count = %d , client_count = %d ",qcmap_sp->client_ref_count,client_count,0);

  // If client is enabled but ref count = 0, implies mobileap disable is already called on the server side
  if ((clnt_info->client_enabled) && (qcmap_sp->client_ref_count == 0))
  {
    clnt_info->client_enabled = FALSE;
    resp_msg.resp.result = QMI_RESULT_SUCCESS_V01;
    resp_msg.resp.error = QMI_ERR_NONE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                            sizeof(qcmap_msgr_mobile_ap_disable_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if(clnt_info->client_enabled)
  {
    if(qcmap_sp->client_ref_count > client_count)
    {
      qcmap_sp->client_ref_count--;
      clnt_info->client_enabled = FALSE;
      resp_msg.resp.result = QMI_RESULT_SUCCESS_V01;

      /* Set the error to QMI_ERR_NO_EFFECT_V01 to indicate the clients
         (esp. IoE Clients) that MobileAP is disabled. */
      resp_msg.resp.error = QMI_ERR_NO_EFFECT_V01;
      qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                            sizeof(qcmap_msgr_mobile_ap_disable_resp_msg_v01));
      return QMI_CSI_CB_NO_ERR;
    }
    else if(qcmap_sp->client_ref_count == client_count)
    {
      // Check if Activate LAN is in progress
      if (QcMapMgr->is_activate_lan_in_progress())
      {
        QcMapMgr->set_activate_lan_in_progress(false);
        LOG_MSG_ERROR("Activate LAN in progess, so disable MoblieAP will not effect", 0, 0, 0);
        resp_msg.resp.result = QMI_RESULT_SUCCESS_V01;
        resp_msg.resp.error = QMI_ERR_NONE_V01;
        qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                          sizeof(qcmap_msgr_mobile_ap_disable_resp_msg_v01));
        return QMI_CSI_CB_NO_ERR;
      }

      if (return_val=QcMapMgr->Disable(&err_num, &qmi_err_num))
      {
        if (err_num == QCMAP_CM_EALDDISCONN)
        {
          qcmap_sp->client_ref_count = qcmap_sp->client_ref_count - client_count;
          LOG_MSG_INFO1("Client reference count %d",qcmap_sp->client_ref_count,0,0);
          clnt_info->client_enabled = FALSE;
          resp_msg.resp.result = QMI_RESULT_SUCCESS_V01;
          resp_msg.resp.error = QMI_ERR_NONE_V01;
          qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                            sizeof(qcmap_msgr_mobile_ap_disable_resp_msg_v01));
          map_handle->handle = 0;
          return QMI_CSI_CB_NO_ERR;
        }
        LOG_MSG_INFO1("MobileAP Disable in progress.",0,0,0);
        resp_msg.resp.result = QMI_RESULT_SUCCESS_V01;
        resp_msg.resp.error = QMI_ERR_NONE_V01;

        /*.If backhaul is not connected, Mobileap will be disabled instantly.
           And since call back function is being called much before the response
           pending flag is set to TRUE, responses are not sent to the client.
           Hence, we set qcmap_disable_resp_msg_v01.resp.error to
           QMI_ERR_NO_EFFECT_V01, so that the caller of this function sends a
           response back to the client. Used for IoE 9x25.
          */
        if (err_num == QCMAP_CM_DISCONNECTED)
          resp_msg.resp.error = QMI_ERR_NO_EFFECT_V01;
      }
      else
      {
        LOG_MSG_ERROR("MobileAP Disable request fails.",0,0,0);
        resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
        resp_msg.resp.error = qmi_err_num;
        qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                          sizeof(qcmap_msgr_mobile_ap_disable_resp_msg_v01));
        return QMI_CSI_CB_NO_ERR;
      }

      qcmap_sp->client_ref_count = qcmap_sp->client_ref_count - client_count;
      LOG_MSG_INFO1("Client reference count %d",qcmap_sp->client_ref_count,0,0);
      clnt_info->client_enabled = FALSE;
      qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                        sizeof(qcmap_msgr_mobile_ap_disable_resp_msg_v01));
      map_handle->handle = 0;
      return QMI_CSI_CB_NO_ERR;
    }
  }
  else
  {
    LOG_MSG_ERROR("MobileAP not enabled by client, but trying to disable!!", 0, 0, 0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_mobile_ap_disable_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
} /* qmi_qcmap_msgr_mobile_ap_disable() */

/*===========================================================================
  FUNCTION QMI_QCMAPI_ENABLE_IPV4()

  DESCRIPTION
    Enable IPV4 Functionality.

  PARAMETERS
    sp:          QMI_QCMAP's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_enable_ipv4
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type       *qcmap_sp;
  qcmap_msgr_enable_ipv4_req_msg_v01 *req_ptr;
  qcmap_msgr_enable_ipv4_resp_msg_v01 resp_msg;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  boolean            retval;
  int16              ps_errno;
  int                err_num=0;
  unsigned int             index;
  qmi_error_type_v01 qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type*)service_handle;
  req_ptr = (qcmap_msgr_enable_ipv4_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_enable_ipv4_resp_msg_v01));

  /*---------------------------------------------------------------------
      Enable IPV4
  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_enable_ipv4_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1 ("Client %p Enabling IPV4 in MobileAP inst %d", clnt_info, index,0);

  if (!(retval=QcMapMgr->EnableIPV4(&qmi_err_num)))
  {
    LOG_MSG_ERROR("Enabling IPV4 failed",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_enable_ipv4_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_enable_ipv4_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_enable_ipv4() */

/*===========================================================================
  FUNCTION QMI_QCMAPI_DISABLE_IPV4()

  DESCRIPTION
    Disable IPV4 Functionality.

  PARAMETERS
    sp:          QMI_QCMAP's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_disable_ipv4
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type       *qcmap_sp;
  qcmap_msgr_disable_ipv4_req_msg_v01 *req_ptr;
  qcmap_msgr_disable_ipv4_resp_msg_v01 resp_msg;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  boolean            retval;
  int16              ps_errno;
  int                err_num=0;
  unsigned int             index;
  qmi_error_type_v01 qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type*)service_handle;
  req_ptr = (qcmap_msgr_disable_ipv4_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_disable_ipv4_resp_msg_v01));

  /*---------------------------------------------------------------------
      Disable IPV4
  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_disable_ipv4_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1 ("Client %p Disabling IPV4 in MobileAP inst %d", clnt_info, index,0);

  if (!(retval=QcMapMgr->DisableIPV4(&qmi_err_num)))
  {
    LOG_MSG_ERROR("Disabling IPV4 failed",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_disable_ipv4_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_disable_ipv4_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_disable_ipv4() */

/*===========================================================================
  FUNCTION QMI_QCMAPI_ENABLE_IPV6()

  DESCRIPTION
    Enable IPV6 Functionality.

  PARAMETERS
    sp:          QMI_QCMAP's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_enable_ipv6
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type       *qcmap_sp;
  qcmap_msgr_enable_ipv6_req_msg_v01 *req_ptr;
  qcmap_msgr_enable_ipv6_resp_msg_v01 resp_msg;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  boolean            retval;
  int16              ps_errno;
  int                err_num=0;
  unsigned int             index;
  qmi_error_type_v01 qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type*)service_handle;
  req_ptr = (qcmap_msgr_enable_ipv6_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_enable_ipv6_resp_msg_v01));

  /*---------------------------------------------------------------------
      Disable IPV6
  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_enable_ipv6_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1 ("Client %p Enabling IPV6 in MobileAP inst %d", clnt_info, index,0);

  if (!(retval=QcMapMgr->EnableIPV6(&qmi_err_num)))
  {
    LOG_MSG_ERROR("Enabling IPV6 failed",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_enable_ipv6_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_enable_ipv6_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_enable_ipv6() */

/*===========================================================================
  FUNCTION QMI_QCMAPI_DISABLE_IPV6()

  DESCRIPTION
    Disable IPV6 Functionality.

  PARAMETERS
    sp:          QMI_QCMAP's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_disable_ipv6
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type       *qcmap_sp;
  qcmap_msgr_disable_ipv6_req_msg_v01 *req_ptr;
  qcmap_msgr_disable_ipv6_resp_msg_v01 resp_msg;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  boolean            retval;
  int16              ps_errno;
  int                err_num=0;
  unsigned int             index;
  qmi_error_type_v01 qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type*)service_handle;
  req_ptr = (qcmap_msgr_disable_ipv6_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_disable_ipv6_resp_msg_v01));

  /*---------------------------------------------------------------------
      Disable IPV6
  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_disable_ipv6_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1 ("Client %p Disabling IPV6 in MobileAP inst %d", clnt_info, index,0);

  if (!(retval=QcMapMgr->DisableIPV6(&qmi_err_num)))
  {
    LOG_MSG_ERROR("Disabling IPV6 failed",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_disable_ipv6_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_disable_ipv6_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_disable_ipv6() */


/*===========================================================================
  FUNCTION QMI_QCMAPI_TEAR_DOWN_WWAN()

  DESCRIPTION
    Tear down currently active WWAN connection.

  PARAMETERS
    sp:          QMI_QCMAP's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_tear_down_wwan
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type       *qcmap_sp;
  qcmap_msgr_tear_down_wwan_req_msg_v01 *req_ptr;
  qcmap_msgr_tear_down_wwan_resp_msg_v01 resp_msg;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qcmap_msgr_wwan_call_type_v01 call_type = QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01;
  qmi_csi_cb_error   rc;
  boolean            retval;
  int16              ps_errno;
  int                err_num=0;
  unsigned int             index;
  qmi_error_type_v01 qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type*)service_handle;
  req_ptr = (qcmap_msgr_tear_down_wwan_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_tear_down_wwan_resp_msg_v01));
  rc = QMI_CSI_CB_INTERNAL_ERR;

  /*---------------------------------------------------------------------
      Tear down WWAN for this client
  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_tear_down_wwan_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if ( req_ptr->call_type_valid )
  {
    call_type = req_ptr->call_type;
  }

  LOG_MSG_INFO1 ("Client %p ending WWAN call in MobileAP inst %d", clnt_info, index,0);

  if (!(retval=QcMapMgr->DisconnectBackHaul(call_type, &err_num, &qmi_err_num)))
  {
    LOG_MSG_ERROR("WWAN teardown failed",0,0,0);
    if ( call_type == QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01 )
    {
      resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING_FAIL_V01;
    }
    else
    {
      resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_FAIL_V01;
    }
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_tear_down_wwan_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if (err_num == QCMAP_CM_EALDDISCONN)
  {
    LOG_MSG_INFO1("WWAN Already disconnected!!",0,0,0);
    if ( call_type == QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01 )
    {
      resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTED_V01;
    }
    else
    {
      resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_DISCONNECTED_V01;
    }
    resp_msg.conn_status_valid = TRUE;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_tear_down_wwan_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  if ( call_type == QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01 )
  {
    resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING_V01;
  }
  else
  {
    resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_V01;
  }
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_tear_down_wwan_resp_msg_v01));

  /* Set pending to TRUE to notify client with indication
   * once final WWAN state is received
   *  */
  clnt_info->wwan_op_ind_pending = TRUE;
  clnt_info->wwan_op_ind_msg_id = QMI_QCMAP_MSGR_TEAR_DOWN_WWAN_IND_V01;

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_tear_down_wwan() */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_CM_BRING_UP_WWAN()

  DESCRIPTION
    Bring up WWAN connection in MobileAP mode.

  PARAMETERS
    sp:          QMI_QCMAP's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_bring_up_wwan
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type       *qcmap_sp;
  qcmap_msgr_bring_up_wwan_req_msg_v01  *req_ptr;
  qcmap_msgr_bring_up_wwan_resp_msg_v01  resp_msg;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qcmap_msgr_wwan_call_type_v01 call_type = QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01;
  qmi_csi_cb_error   rc;
  boolean            retval;
  int16              ps_errno;
  unsigned int       index;
  int                err_num=0;
  qmi_error_type_v01 qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type*)service_handle;
  req_ptr = (qcmap_msgr_bring_up_wwan_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_bring_up_wwan_resp_msg_v01));
  rc = QMI_CSI_CB_INTERNAL_ERR;

  /*---------------------------------------------------------------------
        Start the network interface for this client
  ---------------------------------------------------------------------*/
  index = req_ptr->mobile_ap_handle;
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_ERROR("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_bring_up_wwan_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if ( req_ptr->call_type_valid )
  {
    call_type = req_ptr->call_type;
  }

  LOG_MSG_INFO1 ("Client %p starting WWAN call in MobileAP inst %d",
                 clnt_info, index,0);

  if (!(retval=QcMapMgr->ConnectBackHaul(call_type, &err_num, &qmi_err_num)))
  {
    LOG_MSG_ERROR("WWAN bringup failed %d",0,0,0);
    if ( call_type == QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01 )
    {
      resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING_FAIL_V01;
    }
    else
    {
      resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_CONNECTING_FAIL_V01;
    }
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_bring_up_wwan_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if (err_num == QCMAP_CM_EALDCONN)
  {
    LOG_MSG_INFO1("WWAN Already connected!!",0,0,0);
    if ( call_type == QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01 )
    {
      resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTED_V01;
    }
    else
    {
      resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_CONNECTED_V01;
    }
    resp_msg.conn_status_valid = TRUE;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_bring_up_wwan_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if ( call_type == QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01 )
  {
    resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING_V01;
  }
  else
  {
    resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_CONNECTING_V01;
  }
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_bring_up_wwan_resp_msg_v01));

  /* Set pending to TRUE to notify client with indication
   * once final WWAN state is received
   *  */
  clnt_info->wwan_op_ind_pending = TRUE;
  clnt_info->wwan_op_ind_msg_id = QMI_QCMAP_MSGR_BRING_UP_WWAN_IND_V01;
  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_bring_up_wwan() */


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_ENABLE_WLAN()

  DESCRIPTION
    Enable WLAN functionality on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_enable_wlan
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qcmap_msgr_enable_wlan_req_msg_v01    *req_ptr;
  qcmap_msgr_enable_wlan_resp_msg_v01   resp_msg;

  int              ret_val;
  unsigned int     index;
  qmi_error_type_v01 qmi_err_num;
  boolean privileged_client = false;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_enable_wlan_resp_msg_v01));

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_enable_wlan_req_msg_v01 *)req_c_struct;

  if ( req_ptr->privileged_client_valid )
  {
    privileged_client = req_ptr->privileged_client;
  }

  /* Skip the validation for privileged client. */
  index = (req_ptr->mobile_ap_handle);
  if (!privileged_client &&
      (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0))
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_enable_wlan_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1 ("Client %p enabling WLAN", clnt_info,0,0);

  /*---------------------------------------------------------------------
    Enable WLAN
  ---------------------------------------------------------------------*/
  if (QcMapMgr->EnableWLAN(&qmi_err_num, false, privileged_client))
  {
    LOG_MSG_INFO1("Enable WLAN succeeds.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Enable WLAN fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_enable_wlan_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1("\nEnabled WLAN",0,0,0);
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_enable_wlan_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;

} /* qmi_qcmap_msgr_enable_wlan() */


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_DISABLE_WLAN()

  DESCRIPTION
    Disable WLAN functionality on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_disable_wlan
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type        *qcmap_sp;
  qmi_qcmap_msgr_softap_handle_type     *map_handle;
  qcmap_msgr_disable_wlan_req_msg_v01   *req_ptr;
  qcmap_msgr_disable_wlan_resp_msg_v01   resp_msg;

  int              ret_val;
  unsigned int     index;
  qmi_error_type_v01 qmi_err_num;
  boolean privileged_client = false;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_disable_wlan_resp_msg_v01));

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_disable_wlan_req_msg_v01 *)req_c_struct;

  if ( req_ptr->privileged_client_valid )
  {
    privileged_client = req_ptr->privileged_client;
  }

  /* Skip the validation for privileged client. */
  index = (req_ptr->mobile_ap_handle);
  if (!privileged_client &&
      (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0))
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_disable_wlan_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1 ("Client %p disabling WLAN", clnt_info,0,0);



  /*---------------------------------------------------------------------
    Disable WLAN
  ---------------------------------------------------------------------*/

  if (QcMapMgr->DisableWLAN( &qmi_err_num, privileged_client ))
  {
    LOG_MSG_INFO1("Disable WLAN succeeds.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Disable WLAN fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_disable_wlan_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1("\nDisabled WLAN",0,0,0);
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_disable_wlan_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;

} /* qmi_qcmap_msgr_disable_wlan() */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_ADD_STATIC_NAT_ENTRY()

  DESCRIPTION
    Add Static NAT entry on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_add_static_nat_entry
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_add_static_nat_entry_req_msg_v01 *req_ptr;
  qcmap_msgr_add_static_nat_entry_resp_msg_v01 resp_msg;
  qcmap_cm_port_fwding_entry_conf_t nat_entry;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qmi_error_type_v01 qmi_err_num;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_add_static_nat_entry_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p static NAT entry", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_add_static_nat_entry_resp_msg_v01));

  /*---------------------------------------------------------------------
   Add Static NAT Entry
  ---------------------------------------------------------------------*/

  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_add_static_nat_entry_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  nat_entry.port_fwding_private_ip = req_ptr->snat_entry_config.private_ip_addr;
  nat_entry.port_fwding_private_port = req_ptr->snat_entry_config.private_port;
  nat_entry.port_fwding_global_port = req_ptr->snat_entry_config.global_port;
  nat_entry.port_fwding_protocol = req_ptr->snat_entry_config.protocol;

  if (QcMapMgr->AddStaticNatEntry(&nat_entry, &qmi_err_num))
  {
    LOG_MSG_INFO1("Added SNAT entry successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Static NAT entry add failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_add_static_nat_entry_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_add_static_nat_entry_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_add_static_nat_entry() */


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_DELETE_STATIC_NAT_ENTRY()

  DESCRIPTION
    Delete Static NAT entry on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_delete_static_nat_entry
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_delete_static_nat_entry_req_msg_v01 *req_ptr;
  qcmap_msgr_delete_static_nat_entry_resp_msg_v01 resp_msg;
  qcmap_cm_port_fwding_entry_conf_t nat_entry;
 qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qmi_error_type_v01 qmi_err_num;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_delete_static_nat_entry_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p deleting static NAT entry", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_delete_static_nat_entry_resp_msg_v01));

  /*---------------------------------------------------------------------
   Delete Static NAT Entry
  ---------------------------------------------------------------------*/

 index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_delete_static_nat_entry_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  nat_entry.port_fwding_private_ip = req_ptr->snat_entry_config.private_ip_addr;
  nat_entry.port_fwding_private_port = req_ptr->snat_entry_config.private_port;
  nat_entry.port_fwding_global_port = req_ptr->snat_entry_config.global_port;
  nat_entry.port_fwding_protocol = req_ptr->snat_entry_config.protocol;

  if (QcMapMgr->DeleteStaticNatEntry(&nat_entry, &qmi_err_num))
  {
    LOG_MSG_INFO1("Deleted SNAT entry successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Static NAT entry delete failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_delete_static_nat_entry_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_delete_static_nat_entry_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_delete_static_nat_entry() */


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_STATIC_NAT_ENTRY()

  DESCRIPTION
    Get Static NAT entries on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_get_static_nat_entries
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_get_static_nat_entries_req_msg_v01 *req_ptr;
  qcmap_msgr_get_static_nat_entries_resp_msg_v01 resp_msg;
  qcmap_msgr_snat_entry_config_v01 nat_entry;
  qmi_error_type_v01 qmi_err_num;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_get_static_nat_entries_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p get static NAT entries", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_static_nat_entries_resp_msg_v01));

  /*---------------------------------------------------------------------
   Get Static NAT Entries
  ---------------------------------------------------------------------*/

  if (QcMapMgr->GetStaticNatEntries((qcmap_msgr_snat_entry_config_v01 *)&resp_msg.snat_config[0], &resp_msg.snat_config_len, &qmi_err_num))
  {
    LOG_MSG_INFO1("Get SNAT entry succeeded.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("No Static NAT entries configured",0,0,0);

    resp_msg.snat_config_valid = false;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_static_nat_entries_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  resp_msg.snat_config_valid = true;
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_static_nat_entries_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_static_nat_entries() */


/*===========================================================================
  FUNCTION qmi_qcmap_msgr_set_dmz()

  DESCRIPTION
    Add DMZ on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_set_dmz
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_set_dmz_req_msg_v01 *req_ptr;
  qcmap_msgr_set_dmz_resp_msg_v01 resp_msg;
  uint32 dmz_ip=0;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qmi_error_type_v01 qmi_err_num;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_set_dmz_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p dmz", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_dmz_resp_msg_v01));

  /*---------------------------------------------------------------------
   Add DMZ
  ---------------------------------------------------------------------*/

 index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_dmz_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  dmz_ip = req_ptr->dmz_ip_addr;

  if (QcMapMgr->AddDMZ(dmz_ip, &qmi_err_num))
  {
    LOG_MSG_INFO1("Added DMZ successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("DMZ add failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_dmz_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_dmz_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_set_dmz() */




/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_DELETE_DMZ()

  DESCRIPTION
    Delete DMZ on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_delete_dmz
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_delete_dmz_req_msg_v01 *req_ptr;
  qcmap_msgr_delete_dmz_resp_msg_v01 resp_msg;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qmi_error_type_v01 qmi_err_num;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_delete_dmz_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p deleting dmz", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_delete_dmz_resp_msg_v01));

  /*---------------------------------------------------------------------
   Delete DMZ
  ---------------------------------------------------------------------*/

 index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_delete_dmz_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if (QcMapMgr->DeleteDMZ(&qmi_err_num))
  {
    LOG_MSG_INFO1("Deleted DMZ successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("DMZ delete failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_delete_dmz_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_delete_dmz_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_delete_dmz() */


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_DMZ()

  DESCRIPTION
    Get DMZ on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_get_dmz
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_get_dmz_req_msg_v01 *req_ptr;
  qcmap_msgr_get_dmz_resp_msg_v01 resp_msg;
  uint32 dmz_ip=0;
  qmi_error_type_v01 qmi_err_num;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_get_dmz_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p get dmz", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_dmz_resp_msg_v01));

  /*---------------------------------------------------------------------
   Get DMZ
  ---------------------------------------------------------------------*/

  if (QcMapMgr->GetDMZ(&resp_msg.dmz_ip_addr, &qmi_err_num))
  {
    LOG_MSG_INFO1("Get DMZ succeeded.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Get DMZ failed!!",0,0,0);
    resp_msg.dmz_ip_addr_valid = false;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_dmz_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  resp_msg.dmz_ip_addr_valid = true;
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_dmz_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_dmz() */

/*===========================================================================

  FUNCTION QMI_QCMAP_MSGR_GET_WWAN_STATS()

  DESCRIPTION
    Get wwan stats on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_get_wwan_stats
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_get_wwan_stats_req_msg_v01 *req_ptr;
  qcmap_msgr_get_wwan_stats_resp_msg_v01 resp_msg;
  qcmap_msgr_ip_family_enum_v01 ip_family;
  qcmap_msgr_wwan_statistics_type_v01 wwan_stats;
  qmi_error_type_v01 qmi_err_num;
  int              ret_val;
  unsigned int     index;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_get_wwan_stats_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p get wwan stats", clnt_info,0,0);

  memset(&wwan_stats, 0, sizeof(qcmap_msgr_wwan_statistics_type_v01));
  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_wwan_stats_resp_msg_v01));

  /*---------------------------------------------------------------------

  ---------------------------------------------------------------------*/

 index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_get_wwan_stats_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  ip_family = req_ptr->ip_family;

  if (QcMapMgr->GetWWANStatistics(ip_family, &wwan_stats, &qmi_err_num))
  {
    LOG_MSG_INFO1("Got WWAN Stats successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Get WWAN Stats get failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_wwan_stats_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  resp_msg.wwan_stats_valid = TRUE;
  memcpy((void *)&resp_msg.wwan_stats, (void *)&wwan_stats, sizeof(qcmap_msgr_wwan_statistics_type_v01));

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_wwan_stats_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_wwan_stats() */


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_RESET_WWAN_STATS()

  DESCRIPTION
    Reset wwan stats on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_reset_wwan_stats
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_reset_wwan_stats_req_msg_v01 *req_ptr;
  qcmap_msgr_reset_wwan_stats_resp_msg_v01 resp_msg;
  qcmap_msgr_ip_family_enum_v01 ip_family;
  qmi_error_type_v01 qmi_err_num;
  int              ret_val;
  unsigned int     index;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_reset_wwan_stats_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p reset wwan stats", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_reset_wwan_stats_resp_msg_v01));

  /*---------------------------------------------------------------------

  ---------------------------------------------------------------------*/

 index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_reset_wwan_stats_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  ip_family = req_ptr->ip_family;

  if (QcMapMgr->ResetWWANStatistics(ip_family, &qmi_err_num))
  {
    LOG_MSG_INFO1("Reset WWAN Stats successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Reset WWAN Stats failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_reset_wwan_stats_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_reset_wwan_stats_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_reset_wwan_stats() */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_IPSEC_VPN_PT()

  DESCRIPTION
    Get IPSEC VPN Passthrough flag value on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_ipsec_vpn_pt
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_get_ipsec_vpn_pass_through_req_msg_v01 *req_ptr;
  qcmap_msgr_get_ipsec_vpn_pass_through_resp_msg_v01 resp_msg;
  int err_num=0;
  int              ret_val;
  uint8    ipsec_vpn_pt_flag=0;
  qmi_error_type_v01 qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_get_ipsec_vpn_pass_through_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p get ipsec vpn passthrough flag", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_ipsec_vpn_pass_through_resp_msg_v01));

  /*---------------------------------------------------------------------

  ---------------------------------------------------------------------*/
  if (QcMapMgr->GetIpsecVpnPassthroughFlag(&ipsec_vpn_pt_flag, &qmi_err_num))
  {
    LOG_MSG_INFO1("Got IPSEC VPN Passthrough successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Get IPSEC VPN Passthrough failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_ipsec_vpn_pass_through_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  resp_msg.vpn_pass_through_value_valid = TRUE;
  resp_msg.vpn_pass_through_value = ipsec_vpn_pt_flag;
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_ipsec_vpn_pass_through_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_ipsec_vpn_pt */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_SET_IPSEC_VPN_PT()

  DESCRIPTION
    Set IPSEC VPN Passthrough flag value on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_set_ipsec_vpn_pt
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_set_ipsec_vpn_pass_through_req_msg_v01 *req_ptr;
  qcmap_msgr_set_ipsec_vpn_pass_through_resp_msg_v01 resp_msg;
  int err_num=0;
  int              ret_val;
  uint8    ipsec_vpn_pt_flag=0;
  unsigned int index;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qmi_error_type_v01 qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_set_ipsec_vpn_pass_through_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p set ipsec vpn passthrough flag", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_ipsec_vpn_pass_through_resp_msg_v01));

  /*---------------------------------------------------------------------

  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_ipsec_vpn_pass_through_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  if (QcMapMgr->SetIPSECVpnPassThrough(req_ptr->vpn_pass_through_value, &qmi_err_num))
  {
    LOG_MSG_INFO1("Set IPSEC VPN Passthrough successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Set IPSEC VPN Passthrough failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_ipsec_vpn_pass_through_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_ipsec_vpn_pass_through_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_set_ipsec_vpn_pt */


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_PPTP_VPN_PT()

  DESCRIPTION
    Get PPTP VPN Passthrough flag value on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_pptp_vpn_pt
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_get_pptp_vpn_pass_through_req_msg_v01 *req_ptr;
  qcmap_msgr_get_pptp_vpn_pass_through_resp_msg_v01 resp_msg;
  int err_num=0;
  int              ret_val;
  uint8    pptp_vpn_pt_flag=0;
  qmi_error_type_v01 qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_get_pptp_vpn_pass_through_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p get pptp vpn passthrough flag", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_pptp_vpn_pass_through_resp_msg_v01));

  /*---------------------------------------------------------------------

  ---------------------------------------------------------------------*/
  if (QcMapMgr->GetPptpVpnPassthroughFlag(&pptp_vpn_pt_flag, &qmi_err_num))
  {
    LOG_MSG_INFO1("Got PPTP VPN Passthrough successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Get PPTP VPN Passthrough failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_pptp_vpn_pass_through_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  resp_msg.vpn_pass_through_value_valid = TRUE;
  resp_msg.vpn_pass_through_value = pptp_vpn_pt_flag;
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_pptp_vpn_pass_through_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_pptp_vpn_pt */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_SET_PPTP_VPN_PT()

  DESCRIPTION
    Set PPTP VPN Passthrough flag value on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_set_pptp_vpn_pt
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_set_pptp_vpn_pass_through_req_msg_v01 *req_ptr;
  qcmap_msgr_set_pptp_vpn_pass_through_resp_msg_v01 resp_msg;
  int err_num=0;
  int              ret_val;
  uint8    pptp_vpn_pt_flag=0;
  qmi_error_type_v01 qmi_err_num;
  unsigned int index;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_set_pptp_vpn_pass_through_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p set pptp vpn passthrough flag", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_pptp_vpn_pass_through_resp_msg_v01));

  /*---------------------------------------------------------------------

  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_pptp_vpn_pass_through_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  if (QcMapMgr->SetPPTPVpnPassThrough(req_ptr->vpn_pass_through_value, &qmi_err_num))
  {
    LOG_MSG_INFO1("Set PPTP VPN Passthrough successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Set PPTP VPN Passthrough failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_pptp_vpn_pass_through_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_pptp_vpn_pass_through_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_set_pptp_vpn_pt */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_L2TP_VPN_PT()

  DESCRIPTION
    Get L2TP VPN Passthrough flag value on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_l2tp_vpn_pt
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_get_l2tp_vpn_pass_through_req_msg_v01 *req_ptr;
  qcmap_msgr_get_l2tp_vpn_pass_through_resp_msg_v01 resp_msg;
  int err_num=0;
  int              ret_val;
  uint8    l2tp_vpn_pt_flag=0;
  qmi_error_type_v01 qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_get_l2tp_vpn_pass_through_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p get l2tp vpn passthrough flag", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_l2tp_vpn_pass_through_resp_msg_v01));

  /*---------------------------------------------------------------------

  ---------------------------------------------------------------------*/
  if (QcMapMgr->GetL2tpVpnPassthroughFlag(&l2tp_vpn_pt_flag, &qmi_err_num))
  {
    LOG_MSG_INFO1("Got L2TP VPN Passthrough successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Get L2TP VPN Passthrough failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_l2tp_vpn_pass_through_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  resp_msg.vpn_pass_through_value_valid = TRUE;
  resp_msg.vpn_pass_through_value = l2tp_vpn_pt_flag;
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_l2tp_vpn_pass_through_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_l2tp_vpn_pt */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_SET_L2TP_VPN_PT()

  DESCRIPTION
    Set L2TP VPN Passthrough flag value on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_set_l2tp_vpn_pt
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_set_l2tp_vpn_pass_through_req_msg_v01 *req_ptr;
  qcmap_msgr_set_l2tp_vpn_pass_through_resp_msg_v01 resp_msg;
  int err_num=0;
  int              ret_val;
  uint8    l2tp_vpn_pt_flag=0;
  unsigned int index;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qmi_error_type_v01 qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_set_l2tp_vpn_pass_through_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p set l2tp vpn passthrough flag", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_l2tp_vpn_pass_through_resp_msg_v01));

  /*---------------------------------------------------------------------

  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_l2tp_vpn_pass_through_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  if (QcMapMgr->SetL2TPVpnPassThrough(req_ptr->vpn_pass_through_value, &qmi_err_num))
  {
    LOG_MSG_INFO1("Set L2TP VPN Passthrough successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Set L2TP VPN Passthrough failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_l2tp_vpn_pass_through_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_l2tp_vpn_pass_through_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_set_l2tp_vpn_pt */


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_NAT_TYPE()

  DESCRIPTION
    Get NAT Type value on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_nat_type
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_get_nat_type_resp_msg_v01 resp_msg;
  qcmap_msgr_nat_enum_v01 cur_nat_type =
                                       QCMAP_MSGR_NAT_ENUM_MIN_ENUM_VAL_V01;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;

  printf ("Client %p get nat type", clnt_info);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_nat_type_resp_msg_v01));

  /*---------------------------------------------------------------------
   Get NAT Type.
  ---------------------------------------------------------------------*/
  if (QcMapMgr->GetNatType(&cur_nat_type, &qmi_err_num))
  {
    QCMAP_CM_LOG("Got NAT type value successfully.\n");
  }
  else
  {
    QCMAP_CM_LOG("Get NAT type failed!!");
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_nat_type_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  resp_msg.nat_type_valid = TRUE;
  resp_msg.nat_type = cur_nat_type;
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_nat_type_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_nat_type */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_SET_NAT_TYPE()

  DESCRIPTION
    Set NAT Type on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_set_nat_type
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_set_nat_type_req_msg_v01 *req_ptr;
  qcmap_msgr_set_nat_type_resp_msg_v01 resp_msg;
  unsigned int index;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_set_nat_type_req_msg_v01 *)req_c_struct;

  printf ("Client %p set nat type", clnt_info);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_nat_type_resp_msg_v01));

  /*---------------------------------------------------------------------
   Set NAT Type.
  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_nat_type_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  if (QcMapMgr->SetNatType(req_ptr->nat_type, &qmi_err_num))
  {
    QCMAP_CM_LOG("Set NAT Type successfully.\n");
  }
  else
  {
    QCMAP_CM_LOG("Set NAT Type failed!!");
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_nat_type_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_nat_type_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_set_nat_type */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_AUTO_CONNECT()

  DESCRIPTION
    Get Autoconnect flag value on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_auto_connect
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_get_auto_connect_req_msg_v01 *req_ptr;
  qcmap_msgr_get_auto_connect_resp_msg_v01 resp_msg;
  int err_num=0;
  int              ret_val;
  int              index;
  boolean    auto_connect_flag;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_get_auto_connect_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p get auto_connect flag", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_auto_connect_resp_msg_v01));
  /*---------------------------------------------------------------------

  ---------------------------------------------------------------------*/
  auto_connect_flag =  QcMapMgr->GetAutoconnect();

  resp_msg.auto_conn_flag_valid = TRUE;
  resp_msg.auto_conn_flag = auto_connect_flag;

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_auto_connect_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_auto_connect */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_SET_AUTO_CONNECT()

  DESCRIPTION
    Set auto connect flag value on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_set_auto_connect
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_set_auto_connect_req_msg_v01 *req_ptr;
  qcmap_msgr_set_auto_connect_resp_msg_v01 resp_msg;
  int err_num=0;
  int              ret_val;
  uint8    auto_connect_flag=0;
  unsigned int index;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qmi_error_type_v01 qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_set_auto_connect_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p set auto_connect flag", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_auto_connect_resp_msg_v01));

  /*---------------------------------------------------------------------

  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_auto_connect_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  if (QcMapMgr->SetAutoconnect(req_ptr->enable, &qmi_err_num))
  {
    LOG_MSG_INFO1("Set Auto connect flag successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Set Auto connect failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_auto_connect_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_auto_connect_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_set_auto_connect */



/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_ADD_FIREWALL_ENTRY_REQ_V01()

  DESCRIPTION
    Add Firewall entry on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_add_firewall_entry
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
 )
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qcmap_msgr_add_firewall_entry_req_msg_v01 *req_ptr;
  qcmap_msgr_add_firewall_entry_resp_msg_v01 resp_msg;
  qcmap_msgr_firewall_conf_t  firewall_conf;
  int              ret_val,next_hdr_prot,mask;
  unsigned int     index;
  qmi_error_type_v01  qmi_err_num;

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_add_firewall_entry_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_add_firewall_entry_resp_msg_v01));
  memset(&firewall_conf, 0, sizeof(qcmap_msgr_firewall_conf_t));

  if(req_ptr->ip_version == QCMAP_MSGR_IP_FAMILY_V4_V01)
  {
    firewall_conf.extd_firewall_entry.filter_spec.ip_vsn = IP_V4;
  }
  else if (req_ptr->ip_version == QCMAP_MSGR_IP_FAMILY_V6_V01)
  {
    firewall_conf.extd_firewall_entry.filter_spec.ip_vsn = IP_V6;
  }
  else
  {
    LOG_MSG_ERROR("Unsupported IP Version %d",
                   req_ptr->ip_version , 0, 0);
    qmi_err_num = QCMAP_MSGR_INVALID_PARAM;
    goto firewall_add_error;
  }

  switch (firewall_conf.extd_firewall_entry.filter_spec.ip_vsn)
  {
    case IP_V4:
      if( req_ptr->ip4_src_addr_valid )
      {
        firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v4.src.addr.ps_s_addr =
                                        (req_ptr->ip4_src_addr.addr);
        firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v4.src.subnet_mask.ps_s_addr =
                                        (req_ptr->ip4_src_addr.subnet_mask);
        firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask |= IPFLTR_MASK_IP4_SRC_ADDR;
        LOG_MSG_INFO1("IP4 source address is:",0,0,0);
        IPV4_ADDR_MSG(firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v4.src.addr.ps_s_addr);
        LOG_MSG_INFO1("IP4 source subnet mask is:", 0, 0, 0);
        IPV4_ADDR_MSG(firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v4.src.subnet_mask.ps_s_addr);
      }

      if(req_ptr->ip4_tos_valid)
      {
        firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v4.tos.val = req_ptr->ip4_tos.value;
        firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v4.tos.mask = req_ptr->ip4_tos.mask;
        firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask |= IPFLTR_MASK_IP4_TOS;
        LOG_MSG_INFO1("IP4  TOS value %d mask %d ",
                      firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v4.tos.val,
                      firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v4.tos.mask, 0);
      }

      if(req_ptr->next_hdr_prot_valid)
      {
       firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v4.next_hdr_prot=req_ptr->next_hdr_prot;
       firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v4.field_mask |= IPFLTR_MASK_IP4_NEXT_HDR_PROT;
      }
      break;

    case IP_V6:
      if (req_ptr->ip6_src_addr_valid)
      {
        memcpy(firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr8,
               req_ptr->ip6_src_addr.addr,
               QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8));

        firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v6.src.prefix_len =
                                 req_ptr->ip6_src_addr.prefix_len;
        firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask |= IPFLTR_MASK_IP6_SRC_ADDR;

        LOG_MSG_INFO1("IPV6 src_addr is:",0, 0, 0);
        IPV6_ADDR_MSG(firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr64);
        LOG_MSG_INFO1("IPV6 prefix len %d",
                      firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v6.src.prefix_len , 0, 0);
      }

      if(req_ptr->ip6_trf_cls_valid)
      {
        firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.val = req_ptr->ip6_trf_cls.value;
        firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.mask = req_ptr->ip6_trf_cls.mask;
        firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask |= IPFLTR_MASK_IP6_TRAFFIC_CLASS;
        LOG_MSG_INFO1("IPV6 trf class value %d mask %d",
                      firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.val ,
                      firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v6.trf_cls.mask, 0);
      }

      if(req_ptr->next_hdr_prot_valid)
      {
        firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v6.next_hdr_prot=req_ptr->next_hdr_prot;
        firewall_conf.extd_firewall_entry.filter_spec.ip_hdr.v6.field_mask |= IPFLTR_MASK_IP6_NEXT_HDR_PROT ;
      }
      break;

    default:
      LOG_MSG_ERROR("Bad IP version %d", firewall_conf.extd_firewall_entry.filter_spec.ip_vsn, 0, 0);
  }

  LOG_MSG_INFO1("Next header protocol %d  ",
                req_ptr->next_hdr_prot, 0, 0);
  if(req_ptr->next_hdr_prot_valid)
  {
   switch(req_ptr->next_hdr_prot)
   {
    case PS_IPPROTO_TCP:
     next_hdr_prot = PS_IPPROTO_TCP;

     if (req_ptr->tcp_udp_src_valid)
     {
       firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.field_mask |= IPFLTR_MASK_TCP_SRC_PORT;
       firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.src.port =
                                         req_ptr->tcp_udp_src.port;
       firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.src.range = req_ptr->tcp_udp_src.range;
       LOG_MSG_INFO1("TCP src port %d range %d",
                      req_ptr->tcp_udp_src.port,
                      req_ptr->tcp_udp_src.range, 0);
     }
     if(req_ptr->tcp_udp_dst_valid)
     {
       firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.field_mask |= IPFLTR_MASK_TCP_DST_PORT;
       firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.port =
                                         req_ptr->tcp_udp_dst.port;
       firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.tcp.dst.range = req_ptr->tcp_udp_dst.range;
       LOG_MSG_INFO1("TCP dst port %d range %d",
                        req_ptr->tcp_udp_dst.port,
                        req_ptr->tcp_udp_dst.range, 0);
     }
        break;

     case PS_IPPROTO_UDP:
      next_hdr_prot = PS_IPPROTO_UDP;

      if (req_ptr->tcp_udp_src_valid)
      {
        firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.udp.field_mask |= IPFLTR_MASK_UDP_SRC_PORT;
        firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.udp.src.port =
                                         req_ptr->tcp_udp_src.port;
        firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.udp.src.range = req_ptr->tcp_udp_src.range;
        LOG_MSG_INFO1("UDP src port %d range %d",
                       req_ptr->tcp_udp_src.port,
                       req_ptr->tcp_udp_src.range, 0);
      }

      if(req_ptr->tcp_udp_dst_valid)
      {
        firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.udp.field_mask |= IPFLTR_MASK_UDP_DST_PORT;
        firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.port =
                                         req_ptr->tcp_udp_dst.port;
        firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.udp.dst.range = req_ptr->tcp_udp_dst.range;
        LOG_MSG_INFO1("UDP dst port %d range %d",
                       req_ptr->tcp_udp_dst.port,
                       req_ptr->tcp_udp_dst.range, 0);
        }

       break;

      case PS_IPPROTO_ICMP:
      case PS_IPPROTO_ICMP6:
      if(req_ptr->next_hdr_prot == PS_IPPROTO_ICMP)
      {
        next_hdr_prot = PS_IPPROTO_ICMP;
      }
      else
      {
        next_hdr_prot = PS_IPPROTO_ICMP6;
      }
      if (req_ptr->icmp_type_valid)
      {
        firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.icmp.field_mask |=
                                                IPFLTR_MASK_ICMP_MSG_TYPE;
        firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.icmp.type = req_ptr->icmp_type;
        LOG_MSG_INFO1("ICMP type %d ",
                       firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.icmp.type , 0, 0);
      }
      if (req_ptr->icmp_code_valid)
      {
         firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.icmp.field_mask |=
                                                 IPFLTR_MASK_ICMP_MSG_CODE;
         firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.icmp.code = req_ptr->icmp_code;
         LOG_MSG_INFO1("ICMP code %d ",
                        firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.icmp.code, 0, 0);
       }
        break;
      case PS_IPPROTO_ESP:
        next_hdr_prot = PS_IPPROTO_ESP;
        if (req_ptr->esp_spi_valid)
        {
          firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.esp.field_mask |=
          IPFLTR_MASK_ESP_SPI;
          firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.esp.spi = req_ptr->esp_spi;
          LOG_MSG_INFO1("ESP SPI %d  ",req_ptr->esp_spi, 0, 0);
        }
         break;
      case PS_IPPROTO_TCP_UDP:
        next_hdr_prot = PS_IPPROTO_TCP_UDP;

        if (req_ptr->tcp_udp_src_valid)
        {
          firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask |=
                                               IPFLTR_MASK_TCP_UDP_SRC_PORT;
          firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.src.port =
                                     req_ptr->tcp_udp_src.port;
          firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.src.range =
                                                req_ptr->tcp_udp_src.range;
          LOG_MSG_INFO1("TCP_UDP src port %d range %d",
                         req_ptr->tcp_udp_src.port,
                         req_ptr->tcp_udp_src.range, 0);
        }

        if(req_ptr->tcp_udp_dst_valid)
        {
          firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask |=
                                                  IPFLTR_MASK_TCP_UDP_DST_PORT;
          firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.port =
                                       req_ptr->tcp_udp_dst.port;
          firewall_conf.extd_firewall_entry.filter_spec.next_prot_hdr.tcp_udp_port_range.dst.range =
                                                req_ptr->tcp_udp_dst.range;
          LOG_MSG_INFO1("TCP_UDP dst port %d range %d",
                        req_ptr->tcp_udp_dst.port,
                        req_ptr->tcp_udp_dst.range, 0);
        }
          break;
      default:
        LOG_MSG_ERROR("Unsupported next header protocol %d",
                       req_ptr->next_hdr_prot , 0, 0);
        qmi_err_num = QCMAP_MSGR_INVALID_PARAM;
        goto firewall_add_error;
      }
   }

  ret_val = QcMapMgr->AddFireWallEntry(&firewall_conf, &qmi_err_num);
  if(ret_val == QCMAP_CM_SUCCESS)
  {
    LOG_MSG_INFO1("Added Firewall entry successfully.\n",0,0,0);
    memset(&resp_msg, 0, sizeof(qcmap_msgr_add_firewall_entry_resp_msg_v01));
    resp_msg.firewall_handle_valid=true;
    resp_msg.firewall_handle=firewall_conf.extd_firewall_entry.firewall_handle;
  }
  else
  {
    firewall_add_error:
    LOG_MSG_ERROR("Add Firewall entry failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE;
    if(ret_val == QCMAP_MSGR_ENTRY_PRESENT)
      resp_msg.resp.error = qmi_err_num;
    else if(ret_val == QCMAP_MSGR_ENTRY_FULL)
      resp_msg.resp.error = qmi_err_num ;
    else
      resp_msg.resp.error =  qmi_err_num;

    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
        sizeof(qcmap_msgr_add_firewall_entry_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
      sizeof(qcmap_msgr_add_firewall_entry_resp_msg_v01));
  return QMI_CSI_CB_NO_ERR;
}

/*==========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_FIREWALL_ENTRIES_HANDLE_LIST_REQ_V01

  DESCRIPTION
    Get Firewall handle list .

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_firewall_handles_list
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
 )
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qcmap_msgr_get_firewall_entries_handle_list_req_msg_v01 *req_ptr;
  qcmap_msgr_get_firewall_entries_handle_list_resp_msg_v01  resp_msg;
  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_get_firewall_entries_handle_list_req_msg_v01 *)req_c_struct;
  qcmap_msgr_firewall_conf_t firewall_entry;
  qmi_error_type_v01 qmi_err_num;
  int i=0;
  if ( req_ptr->ip_version == QCMAP_MSGR_IP_FAMILY_V4_V01 )
  {
    firewall_entry.extd_firewall_handle.ip_family=IP_V4;
  }
  else if ( req_ptr->ip_version == QCMAP_MSGR_IP_FAMILY_V6_V01 )
  {
    firewall_entry.extd_firewall_handle.ip_family=IP_V6;
  }
  else
  {
    LOG_MSG_ERROR("Ip version %d not supported",req_ptr->ip_version,0,0);
    qmi_err_num = QCMAP_MSGR_INVALID_PARAM;
    goto get_handlist_error;
  }

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_firewall_entries_handle_list_resp_msg_v01));

  if (QcMapMgr->GetFireWallHandleList(&firewall_entry, &qmi_err_num))
  {
    LOG_MSG_INFO1("Get Firewall Handle list successfully.\n",0,0,0);
    resp_msg.firewall_handle_list_valid= TRUE;
    resp_msg.firewall_handle_list_len=firewall_entry.extd_firewall_handle_list.num_of_entries;
    for( i=0; i<resp_msg.firewall_handle_list_len ; i++)
    {
      resp_msg.firewall_handle_list[i]=firewall_entry.extd_firewall_handle_list.handle_list[i];
    }
  }
  else
  {
    get_handlist_error:
    LOG_MSG_ERROR("Get Firewall Handle list Failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
        sizeof(qcmap_msgr_get_firewall_entries_handle_list_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
      sizeof(qcmap_msgr_get_firewall_entries_handle_list_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;

}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_DELETE_FIREWALL_ENTRY_REQ_V01()

  DESCRIPTION
    Add Firewall entry on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_del_firewall_entry
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
 )
{
  qmi_qcmap_msgr_state_info_type    *qcmap_s;
  qcmap_msgr_delete_firewall_entry_req_msg_v01 *req_ptr;
  qcmap_msgr_delete_firewall_entry_resp_msg_v01 resp_msg;
  qcmap_s = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_delete_firewall_entry_req_msg_v01 *)req_c_struct;
  qcmap_msgr_firewall_conf_t firewall_entry;
  qmi_error_type_v01  qmi_err_num;

  firewall_entry.extd_firewall_entry.firewall_handle=req_ptr->firewall_handle;

  memset(&resp_msg, 0, sizeof(qcmap_msgr_delete_firewall_entry_resp_msg_v01));

  if (QcMapMgr->DeleteFireWallEntry(&firewall_entry,&qmi_err_num))
  {
    LOG_MSG_INFO1("Delete entry successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Delete entry failed ",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
        sizeof(qcmap_msgr_delete_firewall_entry_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
      sizeof(qcmap_msgr_delete_firewall_entry_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;

}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_FIREWALL_ENTRY_REQ_V01()

  DESCRIPTION
    Get Firewall entry on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
    ===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_get_firewall_entry
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
 )
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_get_firewall_entry_req_msg_v01 *req_ptr;
  qcmap_msgr_get_firewall_entry_resp_msg_v01 resp_msg;

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_get_firewall_entry_req_msg_v01 *)req_c_struct;

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_firewall_entry_resp_msg_v01));

  if (QcMapMgr->GetFireWallEntry(&resp_msg,req_ptr->firewall_handle, &qmi_err_num))
  {
    LOG_MSG_INFO1("\n Entries filled successfully for display NUM entry ",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("No Entries found !!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
        sizeof(qcmap_msgr_get_firewall_entry_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
      sizeof(qcmap_msgr_get_firewall_entry_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_MOBILEAP_STATUS_REQ_V01()

  DESCRIPTION
    Gets MobileAP Status.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
    ===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_mobileap_status
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
 )
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qcmap_msgr_mobile_ap_status_req_v01 *req_ptr;
  qcmap_msgr_mobile_ap_status_resp_v01 resp_msg;
  uint8_t status;
  qmi_error_type_v01                   qmi_err_num;
  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_mobile_ap_status_req_v01 *)req_c_struct;

  memset(&resp_msg, 0, sizeof(qcmap_msgr_mobile_ap_status_resp_v01));

  resp_msg.mobile_ap_status_valid = TRUE;
  if( QcMapMgr->GetMobileAPhandle(&qmi_err_num) != 0 )
  resp_msg.mobile_ap_status = QCMAP_MSGR_MOBILE_AP_STATUS_CONNECTED_V01;
  else
  resp_msg.mobile_ap_status = QCMAP_MSGR_MOBILE_AP_STATUS_DISCONNECTED_V01;

  LOG_MSG_INFO1("Sending Mobile ap status ",0,0,0);
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
      sizeof(qcmap_msgr_mobile_ap_status_resp_v01));

  return QMI_CSI_CB_NO_ERR;
}

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_WWAN_STATUS_REQ_V01()

  DESCRIPTION
    Get WWAN status

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
    ===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_wwan_status
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
 )
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qcmap_msgr_wwan_status_req_msg_v01 *req_ptr;
  qcmap_msgr_wwan_status_resp_msg_v01 resp_msg;
  uint8_t status = 0;
  unsigned int index;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qcmap_msgr_wwan_call_type_v01 call_type = QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01;
  qmi_error_type_v01                   qmi_err_num;
/*-----------------------------------------------------------------------------*/
  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_wwan_status_req_msg_v01 *)req_c_struct;

  memset(&resp_msg, 0, sizeof(qcmap_msgr_wwan_status_resp_msg_v01));

  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_wwan_status_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if ( req_ptr->call_type_valid )
  {
    call_type = req_ptr->call_type;
  }
  if (QcMapMgr->GetWWANStatus(call_type, &status, &qmi_err_num))
  {
    resp_msg.conn_status_valid = TRUE;
    if ( call_type == QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01 )
    {
      if(status == QCMAP_CM_WAN_CONNECTING)
        resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_CONNECTING_V01;
      else if (status == QCMAP_CM_WAN_DISCONNECTING )
        resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_V01;
      else if (status == QCMAP_CM_WAN_CONNECTED)
        resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_CONNECTED_V01;
      else
        resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_DISCONNECTED_V01;
    }
    else
    {
      if ( status == QCMAP_CM_V6_WAN_CONNECTING )
        resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTING_V01;
      else if ( status == QCMAP_CM_V6_WAN_DISCONNECTING )
        resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTING_V01;
      else if ( status == QCMAP_CM_V6_WAN_CONNECTED )
        resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_IPV6_CONNECTED_V01;
      else
        resp_msg.conn_status = QCMAP_MSGR_WWAN_STATUS_IPV6_DISCONNECTED_V01;
    }
    LOG_MSG_INFO1("\n Entries filled successfully for display NUM entry ",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("No Entries found !!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
        sizeof(qcmap_msgr_wwan_status_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
      sizeof(qcmap_msgr_wwan_status_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_STATION_MODE_STATUS_REQ_V01()

  DESCRIPTION
    Gets Station Mode Status.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
    ===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_station_mode_status
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
 )
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qcmap_msgr_get_station_mode_status_resp_msg_v01 resp_msg;
  qmi_error_type_v01                   qmi_err_num;
  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_station_mode_status_resp_msg_v01));

  resp_msg.conn_status_valid = TRUE;

  if( QcMapMgr->InStaMode())
  resp_msg.conn_status = QCMAP_MSGR_STATION_MODE_CONNECTED_V01;
  else
  resp_msg.conn_status = QCMAP_MSGR_STATION_MODE_DISCONNECTED_V01;

  LOG_MSG_INFO1("Sending Station Mode status ",0,0,0);
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
      sizeof(qcmap_msgr_get_station_mode_status_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_WWAN_CONFIG()

  DESCRIPTION
    Get wwan config on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_get_wwan_config
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_get_wwan_config_req_msg_v01 *req_ptr;
  qcmap_msgr_get_wwan_config_resp_msg_v01 resp_msg;
  int err_num=0;
  int              ret_val;
  unsigned int     index;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  in_addr public_ip;
  in_addr primary_dns;
  in_addr secondary_dns;
  uint8_t public_ip_v6[QCMAP_MSGR_IPV6_ADDR_LEN_V01];
  uint8_t primary_dns_v6[QCMAP_MSGR_IPV6_ADDR_LEN_V01];
  uint8_t secondary_dns_v6[QCMAP_MSGR_IPV6_ADDR_LEN_V01];

  qmi_error_type_v01 qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  memset(&public_ip,0,sizeof(in_addr));
  memset(&primary_dns,0,sizeof(in_addr));
  memset(&secondary_dns,0,sizeof(in_addr));

  memset(public_ip_v6,0,QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8));
  memset(primary_dns_v6,0,QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8));
  memset(secondary_dns_v6,0,QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8));

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_get_wwan_config_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p get wwan stats", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_wwan_config_resp_msg_v01));

  /*---------------------------------------------------------------------

  ---------------------------------------------------------------------*/

  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_get_wwan_config_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if(req_ptr->addr_type_op == (QCMAP_MSGR_MASK_V4_ADDR_V01 |
                              QCMAP_MSGR_MASK_V4_DNS_ADDR_V01))
  {
    if (QcMapMgr->GetNetworkConfig((in_addr_t *)&public_ip.s_addr,
                                       (uint32 *)&primary_dns.s_addr,
                                       (in_addr_t *)&secondary_dns.s_addr,
                                       &qmi_err_num))
    {
      LOG_MSG_INFO1("Got WWAN Config successfully.\n",0,0,0);
    }
    else
    {
      LOG_MSG_ERROR("Get WWAN Config get failed!!",0,0,0);
      resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
      resp_msg.resp.error = qmi_err_num;
      qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_get_wwan_config_resp_msg_v01));
      return QMI_CSI_CB_NO_ERR;
    }

    if ( public_ip.s_addr != 0 )
    {
      resp_msg.v4_addr_valid = TRUE;
      resp_msg.v4_addr = htonl(public_ip.s_addr);
    }
    if ( primary_dns.s_addr != 0 )
    {
      resp_msg.v4_prim_dns_addr_valid = TRUE;
      resp_msg.v4_prim_dns_addr = htonl(primary_dns.s_addr);
    }
    if ( secondary_dns.s_addr != 0 )
    {
      resp_msg.v4_sec_dns_addr_valid = TRUE;
      resp_msg.v4_sec_dns_addr = htonl(secondary_dns.s_addr);
    }
  }

  if(req_ptr->addr_type_op == (QCMAP_MSGR_MASK_V6_ADDR_V01 |
                              QCMAP_MSGR_MASK_V6_DNS_ADDR_V01))
  {
    if (QcMapMgr->GetIPv6NetworkConfig(public_ip_v6,
                                       primary_dns_v6,
                                       secondary_dns_v6,
                                       &qmi_err_num))
    {
      LOG_MSG_INFO1("Got IPv6 WWAN Config successfully.\n",0,0,0);
    }
    else
    {
      LOG_MSG_ERROR("Get WWAN Config get failed!!",0,0,0);
      resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
      resp_msg.resp.error = qmi_err_num;
      qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_get_wwan_config_resp_msg_v01));
      return QMI_CSI_CB_NO_ERR;
    }

    if (!PS_IN6_IS_ADDR_UNSPECIFIED(public_ip_v6))
    {
      resp_msg.v6_addr_valid = TRUE;
      memcpy(resp_msg.v6_addr, public_ip_v6,
             QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8));
    }
    if (!PS_IN6_IS_ADDR_UNSPECIFIED(primary_dns_v6))
    {
      resp_msg.v6_prim_dns_addr_valid = TRUE;
      memcpy(resp_msg.v6_prim_dns_addr, primary_dns_v6,
             QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8));
    }
    if (!PS_IN6_IS_ADDR_UNSPECIFIED(secondary_dns_v6))
    {
      resp_msg.v6_sec_dns_addr_valid = TRUE;
      memcpy(resp_msg.v6_sec_dns_addr, secondary_dns_v6,
             QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8));
    }
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_wwan_config_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_wwan_config() */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_NAT_TIMEOUT()

  DESCRIPTION
    Get NAT Timeout value of a particular type on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_nat_timeout
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qmi_error_type_v01  qmi_err_num;
  qcmap_msgr_get_nat_timeout_req_msg_v01 *req_ptr;
  qcmap_msgr_get_nat_timeout_resp_msg_v01 resp_msg;
  uint32 timeout_value = 0;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_get_nat_timeout_req_msg_v01 *)req_c_struct;

  printf ("Client %p Get Nat Timeout\n", clnt_info);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_nat_type_resp_msg_v01));

  /*---------------------------------------------------------------------
   Get NAT Timeout.
  ---------------------------------------------------------------------*/
  if (QcMapMgr->GetNatTimeout(req_ptr->nat_timeout_type, &timeout_value, &qmi_err_num))
  {
    QCMAP_CM_LOG("Got NAT timeout value successfully.\n");
  }
  else
  {
    QCMAP_CM_LOG("Get NAT timeout failed!!");
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_nat_timeout_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  resp_msg.timeout_value_valid = TRUE;
  resp_msg.timeout_value = timeout_value;
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_nat_timeout_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_nat_timeout */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_SET_NAT_TIMEOUT()

  DESCRIPTION
    Set NAT Timeout value on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_set_nat_timeout
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  unsigned int     index;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qmi_error_type_v01  qmi_err_num;
  qcmap_msgr_set_nat_timeout_req_msg_v01 *req_ptr;
  qcmap_msgr_set_nat_timeout_resp_msg_v01 resp_msg;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_set_nat_timeout_req_msg_v01 *)req_c_struct;

  printf ("Client %p set nat timeout", clnt_info);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_nat_timeout_resp_msg_v01));

  /*---------------------------------------------------------------------
   Set NAT Timeout.
  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_nat_timeout_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  if (QcMapMgr->SetNatTimeout(req_ptr->nat_timeout_type, req_ptr->timeout_value, &qmi_err_num))
  {
    QCMAP_CM_LOG("Set NAT Timeout successfully.\n");
  }
  else
  {
    QCMAP_CM_LOG("Set NAT Timeout failed!!");
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_nat_timeout_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_nat_timeout_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_set_nat_timeout */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_WLAN_STATUS()

  DESCRIPTION
    Get the current WLAN status.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_wlan_status
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_get_wlan_status_resp_msg_v01 resp_msg;
  unsigned int     index;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qcmap_msgr_get_wlan_status_req_msg_v01  *req_ptr = NULL;
  qcmap_msgr_wlan_mode_enum_v01 wlan_mode =
                                    QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);
  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_get_wlan_status_req_msg_v01 *)req_c_struct;

  printf ("Client %p Get WLAN Status\n", clnt_info);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_wlan_status_resp_msg_v01));

  /*---------------------------------------------------------------------
   Get WLAN Status.
  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
     ( map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_ERROR("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_get_wlan_status_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  if (QcMapMgr->GetWLANStatus(&wlan_mode, &qmi_err_num))
  {
    QCMAP_CM_LOG("Got WLAN Status successfully.\n");
  }
  else
  {
    QCMAP_CM_LOG("Get WLAN Status failed!!");
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_wlan_status_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  resp_msg.wlan_mode_valid = TRUE;
  resp_msg.wlan_mode = wlan_mode;
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_wlan_status_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_wlan_status */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_LAN_CONFIG()

  DESCRIPTION
    Get the configured LAN Config including USB TE config

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_lan_config
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_get_lan_config_resp_msg_v01 resp_msg;
  qcmap_msgr_lan_config_v01 lan_config;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;

  printf ("Client %p Get LAN Config\n", clnt_info);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_lan_config_resp_msg_v01));
  memset(&lan_config,0,sizeof(qcmap_msgr_lan_config_v01));

  /*---------------------------------------------------------------------
   Get LAN Config.
  ---------------------------------------------------------------------*/
  if (QcMapMgr->GetLANConfig(&lan_config, &qmi_err_num))
  {
    QCMAP_CM_LOG("Got LAN Config successfully.\n");
  }
  else
  {
    QCMAP_CM_LOG("Get LAN Config failed!!");
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_lan_config_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  resp_msg.lan_config_valid = TRUE;
  resp_msg.lan_config = lan_config;

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_lan_config_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_lan_config */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_WLAN_CONFIG()

  DESCRIPTION
    Get the configured WLAN Mode, Guest AP Access profile and Station mode config

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_wlan_config
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_get_wlan_config_resp_msg_v01 resp_msg;
  qcmap_msgr_wlan_mode_enum_v01 wlan_mode =
                                    QCMAP_MSGR_WLAN_MODE_ENUM_MIN_ENUM_VAL_V01;
  qcmap_msgr_access_profile_v01 guest_access_profile;
  qcmap_msgr_station_mode_config_v01 station_config;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;

  printf ("Client %p Get WLAN Config\n", clnt_info);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_wlan_config_resp_msg_v01));
  memset(&station_config,0,sizeof(qcmap_msgr_station_mode_config_v01));

  /*---------------------------------------------------------------------
   Get WLAN Config.
  ---------------------------------------------------------------------*/
  if (QcMapMgr->GetWLANConfig(&wlan_mode, &guest_access_profile, &station_config, &qmi_err_num))
  {
    QCMAP_CM_LOG("Got WLAN Config successfully.\n");
  }
  else
  {
    QCMAP_CM_LOG("Get WLAN Config failed!!");
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_wlan_config_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  resp_msg.wlan_mode_valid = TRUE;
  resp_msg.wlan_mode = wlan_mode;

  resp_msg.guest_ap_access_profile_valid = TRUE;
  resp_msg.guest_ap_access_profile = guest_access_profile;

  resp_msg.station_config_valid = TRUE;
  resp_msg.station_config = station_config;

  LOG_MSG_INFO1(" qmi_qcmap_msgr_get_wlan_config ap_sta_bridge_mode=%d ",resp_msg.station_config.ap_sta_bridge_mode,0,0);

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_wlan_config_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_wlan_config */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_SET_LAN_CONFIG()

  DESCRIPTION
    Set LAN Config on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_set_lan_config
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_set_lan_config_req_msg_v01 *req_ptr;
  qcmap_msgr_set_lan_config_resp_msg_v01 resp_msg;
  qcmap_msgr_lan_config_v01 *lan_cfg = NULL;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_set_lan_config_req_msg_v01 *)req_c_struct;

  printf ("Client %p set  LAN config", clnt_info);
  if ( req_ptr->lan_config_valid )
  {
    lan_cfg = &req_ptr->lan_config;
  }

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_lan_config_resp_msg_v01));

  /*---------------------------------------------------------------------
   Set  LAN Config.
  ---------------------------------------------------------------------*/
  if (QcMapMgr->SetLANConfig(lan_cfg, &qmi_err_num))
  {
    QCMAP_CM_LOG("Set  LAN Config successfully.\n");
  }
  else
  {
    QCMAP_CM_LOG("Set  LAN Config failed!! with error = %d",qmi_err_num);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_lan_config_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_lan_config_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_set_lan_config */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_SET_WLAN_CONFIG()

  DESCRIPTION
    Set WLAN Config on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_set_wlan_config
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
  {

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_set_wlan_config_req_msg_v01 *req_ptr;
  qcmap_msgr_set_wlan_config_resp_msg_v01 resp_msg;
  unsigned int     index;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qcmap_msgr_access_profile_v01 guest_ap_profile= QCMAP_MSGR_ACCESS_PROFILE_MIN_ENUM_VAL_V01;
  qcmap_msgr_station_mode_config_v01 *station_cfg = NULL;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_set_wlan_config_req_msg_v01 *)req_c_struct;

  printf ("Client %p set WLAN config", clnt_info);

  if (req_ptr->guest_ap_acess_profile_valid) {
     guest_ap_profile = req_ptr->guest_ap_acess_profile;
  }

  if ( req_ptr->station_config_valid )
  {
    station_cfg = &req_ptr->station_config;
  }

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_wlan_config_resp_msg_v01));

  /*---------------------------------------------------------------------
   Set WLAN Config.
  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
     ( map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_ERROR("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_wlan_config_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  if (QcMapMgr->SetWLANConfig(req_ptr->wlan_mode, guest_ap_profile, station_cfg, &qmi_err_num))
  {
    QCMAP_CM_LOG("Set WLAN Config successfully.\n");
  }
  else
  {
    QCMAP_CM_LOG("Set WLAN Config failed!!");
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_wlan_config_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_wlan_config_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_set_wlan_config */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_ACTIVATE_LAN()

  DESCRIPTION
    Activates LAN with available config.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_activate_lan
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_activate_lan_resp_msg_v01   resp_msg;
  qcmap_msgr_activate_lan_req_msg_v01    *req_ptr = NULL;
  int              ret_val;
  unsigned int     index;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;

  LOG_MSG_INFO1 ("Client %p activating LAN", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_activate_lan_resp_msg_v01));
  req_ptr = (qcmap_msgr_activate_lan_req_msg_v01 *)req_c_struct;

  /*---------------------------------------------------------------------
   Activate LAN with the current configuration.
  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_activate_lan_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if (QcMapMgr->ActivateLAN(&qmi_err_num))
  {
    LOG_MSG_INFO1("Activate LAN succeeds.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Activate LAN fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_activate_lan_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1("\nActivated LAN",0,0,0);
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_activate_lan_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;

} /* qmi_qcmap_msgr_activate_lan() */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_ACTIVATE_WLAN()

  DESCRIPTION
    Activates WLAN with available config.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_activate_wlan
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_activate_wlan_resp_msg_v01   resp_msg;
  qcmap_msgr_activate_wlan_req_msg_v01     *req_ptr;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_activate_wlan_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p activating WLAN", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_activate_wlan_resp_msg_v01));

  /*---------------------------------------------------------------------
   Activate WLAN with the current configuration.
  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_activate_wlan_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if (QcMapMgr->ActivateWLAN(&qmi_err_num))
  {
    LOG_MSG_INFO1("Activate WLAN succeeds.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Activate WLAN fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_activate_wlan_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1("\nActivated WLAN",0,0,0);
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_activate_wlan_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;

} /* qmi_qcmap_msgr_activate_wlan() */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_ACTIVATE_HOSTAPD_CONFIG()

  DESCRIPTION
    Activates Hostapd with available config.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_activate_hostapd_config
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_activate_hostapd_config_resp_msg_v01   resp_msg;
  qcmap_msgr_activate_hostapd_config_req_msg_v01     *req_ptr;
  qcmap_msgr_activate_hostapd_ap_enum_v01 ap_type = QCMAP_MSGR_PRIMARY_AP_V01;
  qcmap_msgr_activate_hostapd_action_enum_v01 action_type = QCMAP_MSGR_ACTIVATE_HOSTAPD_ACTION_ENUM_MIN_ENUM_VAL_V01;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_activate_hostapd_config_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p activating Hostapd", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_activate_hostapd_config_resp_msg_v01));

  /*---------------------------------------------------------------------
   Activate Hostapd with the current configuration.
  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_activate_hostapd_config_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if ( req_ptr->ap_type_valid )
  {
    ap_type = req_ptr->ap_type;
  }

  if ( req_ptr->action_type_valid )
  {
    action_type = req_ptr->action_type;
  }

  if (QcMapMgr->ActivateHostapdConfig(ap_type, action_type, &qmi_err_num))
  {
    LOG_MSG_INFO1("Activate Hostapd succeeds.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Activate Hostapd fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_activate_hostapd_config_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1("\nActivated Hostapd",0,0,0);
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_activate_hostapd_config_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;

} /* qmi_qcmap_msgr_activate_hostapd_config() */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_ACTIVATE_SUPPLICANT_CONFIG()

  DESCRIPTION
    Activates Supplicant with available config.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_activate_supplicant_config
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_activate_supplicant_config_resp_msg_v01   resp_msg;
  qcmap_msgr_activate_supplicant_config_req_msg_v01     *req_ptr;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_activate_supplicant_config_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p activating Supplicant", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_activate_supplicant_config_resp_msg_v01));

  /*---------------------------------------------------------------------
   Activate Supplicant with the current configuration.
  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_activate_supplicant_config_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if (QcMapMgr->ActivateSupplicantConfig(&qmi_err_num))
  {
    LOG_MSG_INFO1("Activate Supplicant succeeds.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Activate Supplicant fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_activate_supplicant_config_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1("\nActivated Supplicant",0,0,0);
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_activate_supplicant_config_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;

} /* qmi_qcmap_msgr_activate_supplicant_config() */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_ROAMING()

  DESCRIPTION
    Gets Roaming flag value on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_roaming
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_get_roaming_pref_req_msg_v01 *req_ptr;
  qcmap_msgr_get_roaming_pref_resp_msg_v01 resp_msg;
  int err_num=0;
  int              ret_val;
  boolean    auto_connect_flag;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_get_roaming_pref_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p get roaming flag", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_roaming_pref_resp_msg_v01));

  /*---------------------------------------------------------------------
       Get Roaming flag
  ---------------------------------------------------------------------*/
  resp_msg.allow_wwan_calls_while_roaming_valid = TRUE;
  resp_msg.allow_wwan_calls_while_roaming = QcMapMgr->GetRoaming();
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_roaming_pref_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_SET_ROAMING()

  DESCRIPTION
    Set roaming flag value on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_set_roaming
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_set_roaming_pref_req_msg_v01 *req_ptr;
  qcmap_msgr_set_roaming_pref_resp_msg_v01 resp_msg;
  int err_num=0;
  int              ret_val;
  uint8    roaming_flag=0;
  unsigned int     index;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qmi_error_type_v01 qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_set_roaming_pref_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p set roaming flag", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_roaming_pref_resp_msg_v01));

  /*---------------------------------------------------------------------

  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_roaming_pref_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  if (QcMapMgr->SetRoaming(req_ptr->allow_wwan_calls_while_roaming, &qmi_err_num))
  {
    LOG_MSG_INFO1("Set Roaming flag successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Set Roaming flag failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_roaming_pref_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_roaming_pref_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_USB_LINK_UP()

  DESCRIPTION
    Sets up the RNDIS/ECM USB tethered link.

  PARAMETERS
    sp:          QMI_QCMAP's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_usb_link_up
(
  qmi_qcmap_msgr_client_info_type    *clnt_info,
  qmi_req_handle                     req_handle,
  int                                msg_id,
  void                               *req_c_struct,
  int                                req_c_struct_len,
  void                               *service_handle
)
{
  qmi_qcmap_msgr_state_info_type        *qcmap_sp;
  qcmap_msgr_usb_link_up_req_msg_v01    *req_ptr;
  qcmap_msgr_usb_link_up_resp_msg_v01   resp_msg;
  qmi_qcmap_msgr_softap_handle_type     *map_handle;
  qmi_csi_cb_error                      rc;
  boolean                               retval;
  int16                                 ps_errno;
  unsigned int                          index;
  int                                   err_num=0;
  qcmap_qti_usb_link_type               usb_link;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  LOG_MSG_INFO1 ("USB link up message received", 0, 0, 0);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type*)service_handle;
  req_ptr = (qcmap_msgr_usb_link_up_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_usb_link_up_resp_msg_v01));
  rc = QMI_CSI_CB_INTERNAL_ERR;

/*-------------------------------------------------------------------------
  Sanity check for qcmap handle
--------------------------------------------------------------------------*/
  index = req_ptr->mobile_ap_handle;
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_ERROR("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_usb_link_up_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1 ("Client %p bringing up USB link for QTI in MobileAP inst %d",
                 clnt_info, index,0);

  if(req_ptr->usb_link == QCMAP_MSGR_USB_LINK_RNDIS_V01)
  {
    usb_link = QCMAP_QTI_USB_LINK_RNDIS;
  }
  else if(req_ptr->usb_link == QCMAP_MSGR_USB_LINK_ECM_V01)
  {
    usb_link = QCMAP_QTI_USB_LINK_ECM;
  }
  else if(req_ptr->usb_link == QCMAP_MSGR_USB_LINK_PPP_V01)
  {
    usb_link = QCMAP_QTI_USB_LINK_PPP;
  }
  else
  {
    LOG_MSG_ERROR("Setup of USB link failed for QTI",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_NO_EFFECT_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_usb_link_up_resp_msg_v01));
  }

  if (!(retval=QcMapMgr->SetupUSBLink(usb_link,&err_num)))
  {
    LOG_MSG_ERROR("Setup of USB link failed for QTI",0,0,0);

    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_NO_EFFECT_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_usb_link_up_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_usb_link_up_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_USB_LINK_DOWN()

  DESCRIPTION
    Brings down the RNDIS/ECM USB tethered link.

  PARAMETERS

  RETURN VALUE

  DEPENDENCIES

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_usb_link_down(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle                         req_handle,
  int                                    msg_id,
  void                                   *req_c_struct,
  int                                    req_c_struct_len,
  void                                   *service_handle
)
{
  qmi_qcmap_msgr_state_info_type        *qcmap_sp;
  qcmap_msgr_usb_link_down_req_msg_v01  *req_ptr;
  qcmap_msgr_usb_link_down_resp_msg_v01 resp_msg;
  qmi_qcmap_msgr_softap_handle_type     *map_handle;
  qmi_csi_cb_error                      rc;
  boolean                               retval;
  int16                                 ps_errno;
  unsigned int                          index;
  int                                   err_num=0;
  qcmap_qti_usb_link_type               usb_link;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  LOG_MSG_INFO1 ("USB link down message received", 0, 0, 0);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type*)service_handle;
  req_ptr = (qcmap_msgr_usb_link_down_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_usb_link_down_resp_msg_v01));
  rc = QMI_CSI_CB_INTERNAL_ERR;

/*-------------------------------------------------------------------------
  Sanity check for qcmap handle
--------------------------------------------------------------------------*/
  index = req_ptr->mobile_ap_handle;
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_ERROR("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_usb_link_down_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1 ("Client %p bringing down USB link for QTI in MobileAP inst %d",
                 clnt_info, index,0);

  if(req_ptr->usb_link == QCMAP_MSGR_USB_LINK_RNDIS_V01)
  {
    usb_link = QCMAP_QTI_USB_LINK_RNDIS;
    LOG_MSG_INFO1 ("USB link RNDIS", 0, 0, 0);
  }
  else if(req_ptr->usb_link == QCMAP_MSGR_USB_LINK_ECM_V01)
  {
    usb_link = QCMAP_QTI_USB_LINK_ECM;
    LOG_MSG_INFO1 ("USB link ECM", 0, 0, 0);
  }
  else if(req_ptr->usb_link == QCMAP_MSGR_USB_LINK_PPP_V01)
  {
    usb_link = QCMAP_QTI_USB_LINK_PPP;
    LOG_MSG_INFO1 ("USB link PPP", 0, 0, 0);
  }
  else
  {
    LOG_MSG_ERROR("Bring down of USB link failed for QTI",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_NO_EFFECT_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_usb_link_down_resp_msg_v01));

  }

  if (!(retval=QcMapMgr->BringDownUSBLink(usb_link, map_handle, &err_num)))
  {
    LOG_MSG_ERROR("Bring down of USB link failed for QTI",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_NO_EFFECT_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_usb_link_down_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_usb_link_down_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}



static qmi_csi_cb_error qmi_qcmap_msgr_set_firewall_config
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_set_firewall_config_req_msg_v01 *req_ptr;
  qcmap_msgr_set_firewall_config_resp_msg_v01 resp_msg;
  int err_num=0;
  int ret_val;
  uint8    roaming_flag=0;
  boolean enable_firewall, pkts_allowed;
  qmi_error_type_v01  qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_set_firewall_config_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p set firewall config", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_firewall_config_resp_msg_v01));

  /*---------------------------------------------------------------------

  ---------------------------------------------------------------------*/
  enable_firewall = req_ptr->firewall_enabled;
  if( req_ptr->pkts_allowed_valid )
  {
   pkts_allowed = req_ptr->pkts_allowed;
  }
  if (QcMapMgr->SetFirewallConfig( enable_firewall, pkts_allowed, &qmi_err_num))
  {
    LOG_MSG_INFO1("Set firewall config successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Set firewall config failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_firewall_config_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_firewall_config_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_FIREWALL_CONFIG()

  DESCRIPTION
    Gets Roaming flag value on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_firewall_config
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qcmap_msgr_get_firewall_config_req_msg_v01 *req_ptr;
  qcmap_msgr_get_firewall_config_resp_msg_v01 resp_msg;
  qmi_error_type_v01  qmi_err_num;
  int err_num=0;
  int ret_val;
  boolean enable_firewall, pkts_allowed;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);
  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_get_firewall_config_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p get firewall flag", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_firewall_config_resp_msg_v01));

  /*---------------------------------------------------------------------

  ---------------------------------------------------------------------*/

  if(QcMapMgr->GetFirewallConfig(&enable_firewall, &pkts_allowed, &qmi_err_num))
  {
    resp_msg.firewall_enabled_valid = TRUE;
    resp_msg.firewall_enabled = enable_firewall;
    if(enable_firewall)
    {
      resp_msg.pkts_allowed_valid = TRUE;
      resp_msg.pkts_allowed = pkts_allowed;
    }
    LOG_MSG_INFO1("get firewall config successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("get firewall config failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_get_firewall_config_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_firewall_config_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}

/*===========================================================================
FUNCTION QMI_QCMAP_MSGR_GET_WWAN_POLICY()

  DESCRIPTION
    Gets wwan profile..

  PARAMETERS

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_wwan_policy
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qcmap_msgr_get_wwan_policy_resp_msg_v01 resp_msg;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_net_policy_info_v01 WWAN_policy;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;

  LOG_MSG_INFO1("Client %p Get WWAN Config\n", clnt_info, 0, 0);

  memset( &resp_msg, 0, sizeof(qcmap_msgr_get_wwan_policy_resp_msg_v01 ) );
  memset( &WWAN_policy, 0, sizeof(qcmap_msgr_net_policy_info_v01 ) );
  /*---------------------------------------------------------------------
   Get WWAN Config.
  ---------------------------------------------------------------------*/
  if (QcMapMgr->GetWWANPolicy(&WWAN_policy, &qmi_err_num ))
  {
    LOG_MSG_INFO1("Get WWAN Config success.\n", 0, 0, 0);
  }
  else
  {
    LOG_MSG_ERROR("Get WWAN Config failed: %d", qmi_err_num, 0, 0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_wwan_policy_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  resp_msg.wwan_policy_valid = TRUE;
  resp_msg.wwan_policy = WWAN_policy;
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_wwan_policy_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}
/*===========================================================================
FUNCTION QMI_QCMAP_MSGR_SET_WWAN_POLICY()

  DESCRIPTION
    Sets wwan profile.

  PARAMETERS

  RETURN VALUE

  DEPENDENCIES

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_set_wwan_policy
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
 )
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qcmap_msgr_set_wwan_policy_resp_msg_v01 resp_msg;
  qcmap_msgr_set_wwan_policy_req_msg_v01 *req_ptr;
  qmi_error_type_v01 qmi_err_num;
  unsigned int                          index;
  qmi_qcmap_msgr_softap_handle_type     *map_handle;
  qcmap_msgr_net_policy_info_v01 WWAN_policy ;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  LOG_MSG_INFO1("Client %p Set WWAN Config\n", clnt_info, 0, 0);
  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_set_wwan_policy_req_msg_v01 *)req_c_struct;
  memset( &resp_msg, 0, sizeof(qcmap_msgr_set_wwan_policy_resp_msg_v01 ) );
  WWAN_policy = req_ptr->wwan_policy;
  /*-------------------------------------------------------------------------
  Sanity check for qcmap handle
--------------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if ( index != QCMAP_MSGR_SOFTAP_HANDLE ||
     ( map_handle = &qcmap_handle)->handle == 0 )
  {
    LOG_MSG_ERROR("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_wwan_policy_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  /*---------------------------------------------------------------------
   Set WWAN Config.
  ---------------------------------------------------------------------*/
  if (QcMapMgr->SetWWANPolicy(WWAN_policy, &qmi_err_num))
  {
    LOG_MSG_INFO1("Set WWAN Config successfully.\n", 0, 0, 0);
  }
  else
  {
    LOG_MSG_ERROR("Set WWAN Config failed: %d", qmi_err_num, 0, 0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_wwan_policy_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
   qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_wwan_policy_resp_msg_v01));

   return QMI_CSI_CB_NO_ERR;
}
/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_IPV4_STATE()

  DESCRIPTION
    Gets the state of ipv4.

  PARAMETERS

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_ipv4_state
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
 )
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_get_ipv4_state_resp_msg_v01 resp_msg;
  uint8_t status;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);
  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  memset( &resp_msg, 0, sizeof(qcmap_msgr_get_ipv4_state_resp_msg_v01 ) );
  if (QcMapMgr->GetIPv4State(&status, &qmi_err_num))
  {
    resp_msg.ipv4_state_valid = TRUE;
    resp_msg.ipv4_state = status;
    LOG_MSG_INFO1("\n ipv4 state successfully determined ", 0, 0, 0);
  }
  else
  {
    LOG_MSG_ERROR("Get IPV4 state failed: %d", qmi_err_num, 0, 0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof( qcmap_msgr_get_ipv4_state_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
      sizeof(qcmap_msgr_get_ipv4_state_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}
/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_IPV6_STATE()

  DESCRIPTION
    Gets the state of ipv6.

  PARAMETERS

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_ipv6_state
(
 qmi_qcmap_msgr_client_info_type        *clnt_info,
 qmi_req_handle           req_handle,
 int                      msg_id,
 void                    *req_c_struct,
 int                      req_c_struct_len,
 void                     *service_handle
 )
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_get_ipv6_state_resp_msg_v01 resp_msg;
  uint8_t status;
  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);
  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  memset( &resp_msg, 0, sizeof(qcmap_msgr_get_ipv6_state_resp_msg_v01 ) );
  if (QcMapMgr->GetIPv6State(&status, &qmi_err_num))
  {
    resp_msg.ipv6_state_valid = TRUE;
    resp_msg.ipv6_state = status;
    LOG_MSG_INFO1("\n ipv6 state successfully determined ", 0, 0, 0);
  }
  else
  {
    LOG_MSG_ERROR("Get IPV6 state failed: %d", qmi_err_num, 0, 0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof( qcmap_msgr_get_ipv6_state_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
      sizeof(qcmap_msgr_get_ipv6_state_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_ENABLE_UPNP()

  DESCRIPTION
    Enable UPnP daemon.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_enable_upnp
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qcmap_msgr_enable_upnp_resp_msg_v01   resp_msg;
  qmi_error_type_v01 qmi_err_num;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;

  LOG_MSG_INFO1 ("Client %p enabling UPNP", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_enable_upnp_resp_msg_v01));

  /*---------------------------------------------------------------------
    Enable UPnP
  ---------------------------------------------------------------------*/

  if (QcMapMgr->EnableUPNP(&qmi_err_num))
  {
    LOG_MSG_INFO1("Enable UPNP succeeds.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Enable UPNP fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
	resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_enable_upnp_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1("\nEnabled UPNP",0,0,0);
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_enable_upnp_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_DISABLE_UPNP()

  DESCRIPTION
    Enable UPnP.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_disable_upnp
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qcmap_msgr_disable_upnp_resp_msg_v01   resp_msg;
  qmi_error_type_v01 qmi_err_num;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;

  LOG_MSG_INFO1 ("Client %p disabling UPNP", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_disable_upnp_resp_msg_v01));

  /*---------------------------------------------------------------------
    Disable UPnP
  ---------------------------------------------------------------------*/

  if (QcMapMgr->DisableUPNP(&qmi_err_num))
  {
    LOG_MSG_INFO1("Disable UPNP succeeds.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Disable UPNP fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
	resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_disable_upnp_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1("\nDisabled UPNP",0,0,0);
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_disable_upnp_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_UPNP_STATUS()

  DESCRIPTION
    Returns the status of UPnP.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_get_upnp_status
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qcmap_msgr_get_upnp_status_resp_msg_v01   resp_msg;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_upnp_mode_enum_v01 status = 0;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  LOG_MSG_INFO1 ("Client %p returning UPNP status", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_upnp_status_resp_msg_v01));

  /*---------------------------------------------------------------------
    Enable UPnP
  ---------------------------------------------------------------------*/

  if (QcMapMgr->GetUPNPStatus(&status, &qmi_err_num))
  {
    resp_msg.upnp_mode_valid = true;
    resp_msg.upnp_mode = status;
  }
  else
  {
    LOG_MSG_ERROR("Get UPnP status fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
	  resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_upnp_status_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_upnp_status_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_ENABLE_DLNA()

  DESCRIPTION
    Enable DLNA.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_enable_dlna
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qcmap_msgr_enable_dlna_resp_msg_v01   resp_msg;
  qmi_error_type_v01 qmi_err_num;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;

  LOG_MSG_INFO1 ("Client %p enabling DLNA", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_enable_dlna_resp_msg_v01));

  /*---------------------------------------------------------------------
    Enable DLNA
  ---------------------------------------------------------------------*/

  if (QcMapMgr->EnableDLNA(&qmi_err_num))
  {
    LOG_MSG_INFO1("Enable DLNA succeeds.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Enable DLNA fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
	resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_enable_dlna_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1("\nEnabled DLNA",0,0,0);
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_enable_dlna_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_DISABLE_DLNA()

  DESCRIPTION
    Enable DLNA.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_disable_dlna
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;
  qcmap_msgr_disable_dlna_resp_msg_v01   resp_msg;
  qmi_error_type_v01 qmi_err_num;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;

  LOG_MSG_INFO1 ("Client %p disabling DLNA", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_disable_dlna_resp_msg_v01));

  /*---------------------------------------------------------------------
    Disable DLNA
  ---------------------------------------------------------------------*/

  if (QcMapMgr->DisableDLNA(&qmi_err_num))
  {
    LOG_MSG_INFO1("Disable DLNA succeeds.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Disable DLNA fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
	resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_disable_dlna_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1("\nDisabled DLNA",0,0,0);
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_disable_dlna_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_DLNA_STATUS()

  DESCRIPTION
    Returns the status of DLNA.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_get_dlna_status
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qcmap_msgr_get_dlna_status_resp_msg_v01   resp_msg;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_dlna_mode_enum_v01 status = 0;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  LOG_MSG_INFO1 ("Client %p returning DLNA status", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_dlna_status_resp_msg_v01));

  /*---------------------------------------------------------------------
    Get DLNA Status
  ---------------------------------------------------------------------*/

  if (QcMapMgr->GetDLNAStatus(&status, &qmi_err_num))
  {
    resp_msg.dlna_mode_valid = true;
    resp_msg.dlna_mode = status;
  }
  else
  {
    LOG_MSG_ERROR("Get DLNA status fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
	  resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_dlna_status_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_dlna_status_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_SET_DLNA_MEDIA_DIR()

  DESCRIPTION
    Configures the DLNA media directory.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_set_dlna_media_dir
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qcmap_msgr_set_dlna_media_dir_resp_msg_v01   resp_msg;
  qcmap_msgr_set_dlna_media_dir_req_msg_v01    *req_msg;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_dlna_mode_enum_v01 status = 0;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  LOG_MSG_INFO1 ("Client %p setting DLNA media directory", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_dlna_media_dir_resp_msg_v01));

  req_msg = (qcmap_msgr_set_dlna_media_dir_req_msg_v01*) req_c_struct;

  /*---------------------------------------------------------------------
    Set DLNA Media Directories
  ---------------------------------------------------------------------*/

  if (QcMapMgr->SetDLNAMediaDir(req_msg->media_dir, &qmi_err_num))
  {
    LOG_MSG_INFO1("Set DLNA Media Dir success.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Set DLNA Media Dir fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
	  resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_dlna_media_dir_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_dlna_media_dir_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_DLNA_MEDIA_DIR()

  DESCRIPTION
    Returns the DLNA media directory.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_get_dlna_media_dir
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qcmap_msgr_get_dlna_media_dir_resp_msg_v01   resp_msg;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_dlna_mode_enum_v01 status = 0;
  char             media_dir[QCMAP_MSGR_MAX_DLNA_DIR_LEN_V01] = "";

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  LOG_MSG_INFO1 ("Client %p returning DLNA media directory", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_dlna_media_dir_resp_msg_v01));

  /*---------------------------------------------------------------------
    Retreive DLNA Media Directory
  ---------------------------------------------------------------------*/

  if (QcMapMgr->GetDLNAMediaDir( media_dir, &qmi_err_num))
  {
    strlcpy(resp_msg.media_dir, media_dir, sizeof(resp_msg.media_dir));
    resp_msg.media_dir_valid = true;
    resp_msg.media_dir_len = sizeof(resp_msg.media_dir);
  }
  else
  {
    LOG_MSG_ERROR("Get DLNA status fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
	  resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_dlna_media_dir_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_dlna_media_dir_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_ENABLE_MULTICAST_DNS_RESPONDER()

  DESCRIPTION
    Enable M-DNS Daemon

  PARAMETERS
    sp:         QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_enable_multicast_dns_responder
(
  qmi_qcmap_msgr_client_info_type    *clnt_info,
  qmi_req_handle                     req_handle,
  int                                msg_id,
  void                               *req_c_struct,
  int                                req_c_struct_len,
  void                               *service_handle
)
{
  qmi_qcmap_msgr_state_info_type                *qcmap_sp;
  qcmap_msgr_enable_multicast_dns_resp_msg_v01  resp_msg;
  qmi_error_type_v01                            qmi_err_num;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;

  LOG_MSG_INFO1 ("Client %p enabling M-DNS", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_enable_multicast_dns_resp_msg_v01));

  /*---------------------------------------------------------------------
    Enable M-DNS Daemon
  ---------------------------------------------------------------------*/

  if (QcMapMgr->EnableMDNS(&qmi_err_num))
  {
    LOG_MSG_INFO1("Enable M-DNS succeeds.\n",0,0,0);
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_enable_multicast_dns_resp_msg_v01));

    return QMI_CSI_CB_NO_ERR;
  }
  else
  {
    LOG_MSG_ERROR("Enable M-DNS fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
	resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_enable_multicast_dns_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_DISABLE_MULTICAST_DNS_RESPONDER()

  DESCRIPTION
    Enable M-DNS Daemon.

  PARAMETERS
    sp:         QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_disable_multicast_dns_responder
(
  qmi_qcmap_msgr_client_info_type    *clnt_info,
  qmi_req_handle                     req_handle,
  int                                msg_id,
  void                               *req_c_struct,
  int                                req_c_struct_len,
  void                               *service_handle
)
{
  qmi_qcmap_msgr_state_info_type                 *qcmap_sp;
  qcmap_msgr_disable_multicast_dns_resp_msg_v01  resp_msg;
  qmi_error_type_v01                             qmi_err_num;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;

  LOG_MSG_INFO1 ("Client %p disabling M-DNS", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_disable_multicast_dns_resp_msg_v01));

  /*---------------------------------------------------------------------
    Disable M-DNS
  ---------------------------------------------------------------------*/

  if (QcMapMgr->DisableMDNS(&qmi_err_num))
  {
    LOG_MSG_INFO1("Disable M-DNS succeeds.\n",0,0,0);
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_disable_multicast_dns_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  else
  {
    LOG_MSG_ERROR("Disable M-DNS fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
	resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_disable_multicast_dns_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_MULTICAST_DNS_RESPONDER_STATUS()

  DESCRIPTION
    Returns the status of MDNS.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_get_multicast_dns_responder_status
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qcmap_msgr_get_multicast_dns_status_resp_msg_v01   resp_msg;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_mdns_mode_enum_v01 status;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  LOG_MSG_INFO1 ("Client %p returning MDNS status", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_multicast_dns_status_resp_msg_v01));

  /*---------------------------------------------------------------------
    Enable UPnP
  ---------------------------------------------------------------------*/

  if (QcMapMgr->GetMDNSStatus(&status, &qmi_err_num))
  {
    resp_msg.mdns_mode_valid = true;
    resp_msg.mdns_mode = status;
  }
  else
  {
    LOG_MSG_ERROR("Get MDNS status fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
	  resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_multicast_dns_status_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_multicast_dns_status_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_SET_QCMAP_BOOTUP_CFG()

  DESCRIPTION
  Set the bootup configuration for QCMAP componets

  PARAMETERS
    sp:         QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_set_qcmap_bootup_cfg
(
  qmi_qcmap_msgr_client_info_type    *clnt_info,
  qmi_req_handle                     req_handle,
  int                                msg_id,
  void                               *req_c_struct,
  int                                req_c_struct_len,
  void                               *service_handle
)
{
  qmi_qcmap_msgr_state_info_type               *qcmap_sp;
  qcmap_msgr_set_qcmap_bootup_cfg_resp_msg_v01 resp_msg;
  qcmap_msgr_set_qcmap_bootup_cfg_req_msg_v01  *req_ptr = NULL;
  qmi_error_type_v01                           qmi_err_num;
  qcmap_msgr_bootup_flag_v01            mobileap_enable = QCMAP_MSGR_BOOTUP_FLAG_MIN_ENUM_VAL_V01;
  qcmap_msgr_bootup_flag_v01            wlan_enable = QCMAP_MSGR_BOOTUP_FLAG_MIN_ENUM_VAL_V01;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);
  ds_assert(req_c_struct != NULL);

  if (req_ptr = (qcmap_msgr_set_qcmap_bootup_cfg_req_msg_v01*)req_c_struct) {
     // Mobile AP Bootup Flag
     if (req_ptr->mobileap_bootup_flag_valid) {
        mobileap_enable = req_ptr->mobileap_bootup_flag;
     }
     // WLAN Bootup Flag
     if (req_ptr->wlan_bootup_flag_valid) {
        wlan_enable = req_ptr->wlan_bootup_flag;
     }
  }

   LOG_MSG_INFO1 ("qmi_qcmap_msgr_set_qcmap_bootup_cfg mobileap_enable = %d mobileap_enable_vaildi =%d ",
                   req_ptr->mobileap_bootup_flag,
                   req_ptr->mobileap_bootup_flag_valid,0);
   LOG_MSG_INFO1 ("qmi_qcmap_msgr_set_qcmap_bootup_cfg wlan_enable = %d wlan_enable_vaild =%d ",
                   req_ptr->wlan_bootup_flag,
                   req_ptr->wlan_bootup_flag_valid,0);

  LOG_MSG_INFO1 ("Client %p setting booutp configuration for QCMAP", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_qcmap_bootup_cfg_resp_msg_v01));

  /*---------------------------------------------------------------------
    Set QCMAP Bootup CFG
  ---------------------------------------------------------------------*/

  if (QcMapMgr->SetQCMAPBootupConfig(mobileap_enable, wlan_enable, &qmi_err_num))
  {
    LOG_MSG_INFO1("Set QCMAP Bootup CFG succeeds.\n",0,0,0);
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_qcmap_bootup_cfg_resp_msg_v01));

    return QMI_CSI_CB_NO_ERR;
  }
  else
  {
    LOG_MSG_ERROR(" Set QCMAP Bootup CFG Fails \n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_qcmap_bootup_cfg_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
}

/*===========================================================================
  FUNCTION  QMI_QCMAP_MSGR_GET_QCMAP_BOOTUP_CFG()

  DESCRIPTION
  Get QCMAP Bootup Config

  PARAMETERS
    sp:         QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_get_qcmap_bootup_cfg
(
  qmi_qcmap_msgr_client_info_type    *clnt_info,
  qmi_req_handle                     req_handle,
  int                                msg_id,
  void                               *req_c_struct,
  int                                req_c_struct_len,
  void                               *service_handle
)
{
  qmi_qcmap_msgr_state_info_type                *qcmap_sp;
  qcmap_msgr_get_qcmap_bootup_cfg_resp_msg_v01  resp_msg;
  qmi_error_type_v01                            qmi_err_num;
  qcmap_msgr_bootup_flag_v01             mobileap_enable, wlan_enable;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  LOG_MSG_INFO1 ("Client %p Getting QCMAP Bootup Cfg", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_qcmap_bootup_cfg_resp_msg_v01));

  /*---------------------------------------------------------------------
    Get QCMAP Bootup Config
  ---------------------------------------------------------------------*/

  if (QcMapMgr->GetQCMAPBootupConfig(&mobileap_enable, &wlan_enable, &qmi_err_num)) {
    LOG_MSG_INFO1(" GetQCMAPBootupConfig succeeds.\n",0,0,0);
    resp_msg.mobileap_bootup_flag         = mobileap_enable;
    resp_msg.mobileap_bootup_flag_valid = true;
    resp_msg.wlan_bootup_flag                = wlan_enable;
    resp_msg.wlan_bootup_flag_valid        = true;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,sizeof(qcmap_msgr_get_qcmap_bootup_cfg_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  } else {
    LOG_MSG_ERROR("GetQCMAPBootupConfig fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg, sizeof(qcmap_msgr_get_qcmap_bootup_cfg_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
}

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_DATA_BITRATE()

  DESCRIPTION
    Get the current and max data bitrates

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_data_bitrate
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qcmap_msgr_data_bitrate_v01 data_rate;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_get_data_bitrate_resp_msg_v01 resp_msg;
  qcmap_msgr_get_data_bitrate_req_msg_v01 *req_ptr;
  unsigned int             index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  printf ("Client %p Get Data Bitrate\n", clnt_info);

  req_ptr = (qcmap_msgr_get_data_bitrate_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_data_bitrate_resp_msg_v01));
  memset(&data_rate, 0, sizeof(qcmap_msgr_data_bitrate_v01));
  /*---------------------------------------------------------------------
   Get Data bitrates.
  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (&qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_get_data_bitrate_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if (QcMapMgr->GetDataBitrate(&data_rate, &qmi_err_num))
  {
    QCMAP_CM_LOG("Got Data Bitrate successfully.\n");
  }
  else
  {
    QCMAP_CM_LOG("Get Data Bitrate failed!!");
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_data_bitrate_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  resp_msg.data_rate_valid = TRUE;
  resp_msg.data_rate = data_rate;

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_data_bitrate_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_data_bitrate */


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_SET_UPNP_NOTIFY_INTERVAL()

  DESCRIPTION
    Configures the UPnP notify interval.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_set_upnp_notify_interval
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qcmap_msgr_set_upnp_notify_interval_resp_msg_v01   resp_msg;
  qcmap_msgr_set_upnp_notify_interval_req_msg_v01    *req_msg;
  qmi_error_type_v01 qmi_err_num;

  int              ret_val;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);
  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);

  LOG_MSG_INFO1 ("Client %p setting UPnP notify interval", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_upnp_notify_interval_resp_msg_v01));

  req_msg = (qcmap_msgr_set_upnp_notify_interval_req_msg_v01*) req_c_struct;

  /*---------------------------------------------------------------------
    Set UPnP notify interval
  ---------------------------------------------------------------------*/

  if (QcMapMgr->SetUPNPNotifyInterval(req_msg->notify_interval, &qmi_err_num))
  {
    LOG_MSG_INFO1("Set UPnP notify interval success.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Set UPnP notify interval failsed: %d", qmi_err_num,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_upnp_notify_interval_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_upnp_notify_interval_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_UPNP_NOTIFY_INTERVAL()

  DESCRIPTION
    Returns the UPnP notify interval.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_get_upnp_notify_interval
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qcmap_msgr_get_upnp_notify_interval_resp_msg_v01   resp_msg;
  qmi_error_type_v01 qmi_err_num;

  int              notify_int;
  int              ret_val;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  LOG_MSG_INFO1 ("Client %p returning UPnP notify interval", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_upnp_notify_interval_resp_msg_v01));

  /*---------------------------------------------------------------------
    Retreive UPnP notify interval
  ---------------------------------------------------------------------*/

  if (QcMapMgr->GetUPNPNotifyInterval(&notify_int, &qmi_err_num))
  {
    resp_msg.notify_interval = notify_int;
    resp_msg.notify_interval_valid = true;
  }
  else
  {
    LOG_MSG_ERROR("Get UPnP notify interval failed: %d", qmi_err_num,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_upnp_notify_interval_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_upnp_notify_interval_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_SET_DLNA_NOTIFY_INTERVAL()

  DESCRIPTION
    Configures the DLNA notify interval.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_set_dlna_notify_interval
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qcmap_msgr_set_dlna_notify_interval_resp_msg_v01   resp_msg;
  qcmap_msgr_set_dlna_notify_interval_req_msg_v01    *req_msg;
  qmi_error_type_v01 qmi_err_num;

  int              ret_val;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);
  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);

  LOG_MSG_INFO1 ("Client %p setting DLNA notify interval", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_dlna_notify_interval_resp_msg_v01));

  req_msg = (qcmap_msgr_set_dlna_notify_interval_req_msg_v01*) req_c_struct;

  /*---------------------------------------------------------------------
    Set DLNA notify interval
  ---------------------------------------------------------------------*/

  if (QcMapMgr->SetDLNANotifyInterval(req_msg->notify_interval, &qmi_err_num))
  {
    LOG_MSG_INFO1("Set DLNA notify interval success.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Set DLNA notify interval failed: %d", qmi_err_num,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_dlna_notify_interval_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_dlna_notify_interval_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_DLNA_NOTIFY_INTERVAL()

  DESCRIPTION
    Returns the DLNA notify interval.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_get_dlna_notify_interval
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qcmap_msgr_get_dlna_notify_interval_resp_msg_v01   resp_msg;
  qmi_error_type_v01 qmi_err_num;

  int              notify_int;
  int              ret_val;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  LOG_MSG_INFO1 ("Client %p returning DLNA notify interval", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_dlna_notify_interval_resp_msg_v01));

  /*---------------------------------------------------------------------
    Retreive DLNA notify interval
  ---------------------------------------------------------------------*/

  if (QcMapMgr->GetDLNANotifyInterval(&notify_int, &qmi_err_num))
  {
    resp_msg.notify_interval = notify_int;
    resp_msg.notify_interval_valid = true;
  }
  else
  {
    LOG_MSG_ERROR("Get DLNA notify interval failed: %d", qmi_err_num,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_dlna_notify_interval_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_dlna_notify_interval_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}
/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_ADD_DHCP_RESERVATION_RECORD()

  DESCRIPTION
    Add DHCP Reservation Record.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_add_dhcp_reservation_record
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_add_dhcp_reservation_record_req_msg_v01 *req_ptr;
  qcmap_msgr_add_dhcp_reservation_record_resp_msg_v01 resp_msg;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qmi_error_type_v01 qmi_err_num;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_add_dhcp_reservation_record_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p requesting to add DHCP Reservation Record", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_add_dhcp_reservation_record_resp_msg_v01));

  /*---------------------------------------------------------------------
   Add DHCP Reservation Record
  ---------------------------------------------------------------------*/

  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_add_dhcp_reservation_record_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if (QcMapMgr->AddDHCPReservRecord(&(req_ptr->dhcp_reservation_record), &qmi_err_num))
  {
    LOG_MSG_INFO1("Added DHCP Reservation Record successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Adding DHCP Reservation Record failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_add_dhcp_reservation_record_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_add_dhcp_reservation_record_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_add_dhcp_reservation_record() */


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_DHCP_RESERVATION_RECORDS()

  DESCRIPTION
    Get DHCP Reservation Records for AP

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error    qmi_qcmap_msgr_get_dhcp_reservation_records
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_get_dhcp_reservation_records_req_msg_v01 *req_ptr;
  qcmap_msgr_get_dhcp_reservation_records_resp_msg_v01 resp_msg;
  qmi_error_type_v01 qmi_err_num;

  int              ret_val;
  unsigned int     index;
 /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_get_dhcp_reservation_records_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p get DHCP Reservation Records", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_dhcp_reservation_records_resp_msg_v01));

  /*---------------------------------------------------------------------
   Get DHCP Reservation Records
  ---------------------------------------------------------------------*/

  if (QcMapMgr->GetDHCPReservRecords(&resp_msg.dhcp_reservation_records[0],\
                                     &resp_msg.dhcp_reservation_records_len, &qmi_err_num))
  {
    LOG_MSG_INFO1("Get DHCP Reservation Records succeeded.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Get DHCP Reservation records failed!!",0,0,0);

    resp_msg.dhcp_reservation_records_valid = false;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_dhcp_reservation_records_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  resp_msg.dhcp_reservation_records_valid = true;
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_dhcp_reservation_records_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_dhcp_reservation_records() */



/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_EDIT_DHCP_RESERVATION_RECORD()

  DESCRIPTION
    Edits a DHCP record based on MAC or IP address

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_edit_dhcp_reservation_record
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_edit_dhcp_reservation_record_req_msg_v01 *req_ptr;
  qcmap_msgr_edit_dhcp_reservation_record_resp_msg_v01 resp_msg;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qmi_error_type_v01 qmi_err_num;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_edit_dhcp_reservation_record_req_msg_v01*)req_c_struct;
  qcmap_msgr_dhcp_reservation_v01 dhcp_reservation_record;

  LOG_MSG_INFO1 ("Client %p Editing a DHCP Reservation Record", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_edit_dhcp_reservation_record_resp_msg_v01));
  memset(&dhcp_reservation_record,0,sizeof(qcmap_msgr_dhcp_reservation_v01));

  /*---------------------------------------------------------------------
   Edit DHCP Reservation Record
  ---------------------------------------------------------------------*/

 index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_edit_dhcp_reservation_record_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  memcpy(&dhcp_reservation_record,&req_ptr->dhcp_reservation_record,\
         sizeof(qcmap_msgr_dhcp_reservation_v01));
  if (QcMapMgr->EditDHCPReservRecord(&req_ptr->client_reserved_ip,\
                                     &dhcp_reservation_record, &qmi_err_num))
  {
    LOG_MSG_INFO1("Edited DHCP Reservation Record successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Editing DHCP Reservation Record Failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_edit_dhcp_reservation_record_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_edit_dhcp_reservation_record_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_edit_dhcp_reservation_record() */


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_DELETE_DHCP_RESERVATION_RECORD()

  DESCRIPTION
    Deletes DHCP record based on MAC or IP address

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_delete_dhcp_reservation_record
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_delete_dhcp_reservation_record_req_msg_v01  *req_ptr;
  qcmap_msgr_delete_dhcp_reservation_record_resp_msg_v01 resp_msg;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  qmi_error_type_v01 qmi_err_num;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_delete_dhcp_reservation_record_req_msg_v01*)req_c_struct;

  LOG_MSG_INFO1 ("Client %p Deleting a DHCP Reservation Record", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_delete_dhcp_reservation_record_resp_msg_v01));

  /*---------------------------------------------------------------------
   Delete the DHCP Reservation Record
  ---------------------------------------------------------------------*/

 index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_delete_dhcp_reservation_record_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if (QcMapMgr->DeleteDHCPReservRecord(&req_ptr->client_reserved_ip, &qmi_err_num))
  {
    LOG_MSG_INFO1("Deleted DHCP Reservation Record successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Deleting DHCP Reservation Record Failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_delete_dhcp_reservation_record_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_delete_dhcp_reservation_record_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_delete_dhcp_reservation_record() */
/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_WEBSERVER_WWAN_ACCESS()

  DESCRIPTION
    Get webserver wwan access flag value on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_webserver_wwan_access_flag
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_get_webserver_wwan_access_req_msg_v01 *req_ptr;
  qcmap_msgr_get_webserver_wwan_access_resp_msg_v01 resp_msg;
  int err_num=0;
  int              ret_val;
  uint8    webserver_wwan_access_flag=0;
  qmi_error_type_v01 qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_get_webserver_wwan_access_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p get webserver wwan access flag", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_webserver_wwan_access_resp_msg_v01));

  /*---------------------------------------------------------------------

  ---------------------------------------------------------------------*/
  if (QcMapMgr->GetWebserverWWANAccessFlag(&webserver_wwan_access_flag, &qmi_err_num))
  {
    LOG_MSG_INFO1("Got Webserver WWAN Access Flag successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Get Webserver WWAN Access Flag failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_webserver_wwan_access_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  resp_msg.webserver_wwan_access_valid = TRUE;
  resp_msg.webserver_wwan_access = webserver_wwan_access_flag;
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_webserver_wwan_access_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_webserver_wwan_access_flag */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_SET_WEBSERVER_WWAN_ACCESS()

  DESCRIPTION
    Set Webserver WWAN Access flag value on Apps.

  PARAMETERS
    sp:          QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_set_webserver_wwan_access_flag
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type    *qcmap_sp;

  qcmap_msgr_set_webserver_wwan_access_req_msg_v01 *req_ptr;
  qcmap_msgr_set_webserver_wwan_access_resp_msg_v01 resp_msg;
  int err_num=0;
  int              ret_val;
  uint8    pptp_vpn_pt_flag=0;
  qmi_error_type_v01 qmi_err_num;
  unsigned int index;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_set_webserver_wwan_access_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p set webserver wwan access flag", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_webserver_wwan_access_resp_msg_v01));

  /*---------------------------------------------------------------------

  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_webserver_wwan_access_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  if (QcMapMgr->SetWebserverWWANAccess(req_ptr->webserver_wwan_access, &qmi_err_num))
  {
    LOG_MSG_INFO1("Set Webserver WWAN access successfully.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Set webserver wwan access failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_webserver_wwan_access_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_webserver_wwan_access_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_set_webserver_wwan_access_flag */
/*===========================================================================
  FUNCTION QMI_QCMAPI_ENABLE_ALG()

  DESCRIPTION
    Enable  ALGs Functionality.

  PARAMETERS
    sp:          QMI_QCMAP's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_enable_alg
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type       *qcmap_sp;
  qcmap_msgr_enable_alg_req_msg_v01 *req_ptr;
  qcmap_msgr_enable_alg_resp_msg_v01 resp_msg;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  int16              ps_errno;
  int                err_num=0;
  unsigned int             index;
  qmi_error_type_v01 qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type*)service_handle;
  req_ptr = (qcmap_msgr_enable_alg_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_enable_alg_resp_msg_v01));

  /*---------------------------------------------------------------------
      Enable ALGs
  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
     (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                   req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_enable_alg_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1 ("Client %p Enabling ALGs in MobileAP inst %d mask %d",
                  clnt_info, index, req_ptr->alg_type_op);

  if (!(QcMapMgr->EnableAlg(req_ptr->alg_type_op, &qmi_err_num)))
  {
    LOG_MSG_ERROR("Enabling ALGs %d failed Error: %d", req_ptr->alg_type_op,
                   qmi_err_num, 0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_enable_alg_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_enable_alg_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_enable_alg() */

/*===========================================================================
  FUNCTION QMI_QCMAPI_DISABLE_ALG()

  DESCRIPTION
    Disable ALGs Functionality.

  PARAMETERS
    sp:          QMI_QCMAP's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_disable_alg
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type       *qcmap_sp;
  qcmap_msgr_disable_alg_req_msg_v01 *req_ptr;
  qcmap_msgr_disable_alg_resp_msg_v01 resp_msg;
  qmi_qcmap_msgr_softap_handle_type    *map_handle;
  int16              ps_errno;
  int                err_num=0;
  unsigned int             index;
  qmi_error_type_v01 qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type*)service_handle;
  req_ptr = (qcmap_msgr_disable_alg_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_disable_alg_resp_msg_v01));

  /*---------------------------------------------------------------------
      Disable ALGs
  ---------------------------------------------------------------------*/
  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_disable_alg_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1 ("Client %p Disabling ALGs in MobileAP inst %d mask %d",
                  clnt_info, index, req_ptr->alg_type_op);

  if (!(QcMapMgr->DisableAlg(req_ptr->alg_type_op, &qmi_err_num)))
  {
    LOG_MSG_ERROR("Disabling ALGs %d failed Error: %d ", req_ptr->alg_type_op,
                   qmi_err_num, 0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_disable_alg_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_disable_alg_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_disable_alg() */


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_SET_SIP_SERVER_INFO

  DESCRIPTION
    -Sets the default user configured SIP server information.

  DEPENDENCIES
    qmi_qcmap_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_set_sip_server_info
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type              *qcmap_sp;
  qcmap_msgr_set_sip_server_info_req_msg_v01  *req_ptr;
  qcmap_msgr_set_sip_server_info_resp_msg_v01 resp_msg;
  qmi_qcmap_msgr_softap_handle_type           *map_handle;
  boolean                                     retval;
  int16                                       ps_errno;
  int                                         err_num=0;
  unsigned int                                index;
  qmi_error_type_v01                          qmi_err_num;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type*)service_handle;
  req_ptr = (qcmap_msgr_set_sip_server_info_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_sip_server_info_resp_msg_v01));


  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_sip_server_info_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1 ("Client %p MobileAP inst %d set SIP server info type %d",
                  clnt_info, index, req_ptr->default_sip_server_info.pcscf_info_type);

  if (!(retval=QcMapMgr->SetSIPServerInfo(req_ptr->default_sip_server_info, &qmi_err_num)))
  {
    LOG_MSG_ERROR("Set SIP server info type %d failed Error: %d ",
                  req_ptr->default_sip_server_info.pcscf_info_type,
                  qmi_err_num,
                  0);

    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_sip_server_info_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_sip_server_info_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_set_sip_server_info() */


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_SIP_SERVER_INFO

  DESCRIPTION
   - Retrieves default user configured SIP server information.
   - Retrieves network assigned SIP server information.

  DEPENDENCIES
    qmi_qcmap_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_get_sip_server_info
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type              *qcmap_sp;
  qcmap_msgr_get_sip_server_info_req_msg_v01  *req_ptr;
  qcmap_msgr_get_sip_server_info_resp_msg_v01 resp_msg;
  qmi_qcmap_msgr_softap_handle_type           *map_handle;
  boolean                                     retval;
  int16                                       ps_errno;
  int                                         err_num=0;
  unsigned int                                index;
  unsigned int                                count=0;
  qmi_error_type_v01                          qmi_err_num;
  qcmap_msgr_sip_server_info_v01              default_sip_server_info;
  qcmap_msgr_sip_server_info_v01
            network_sip_server_info[QCMAP_MSGR_MAX_SIP_SERVER_ENTRIES_V01];
  int                                         count_network_sip_server_info=0;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type*)service_handle;
  req_ptr = (qcmap_msgr_get_sip_server_info_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_sip_server_info_resp_msg_v01));


  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_get_sip_server_info_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1 ("Client %p MobileAP inst %d",
                  clnt_info, index, 0);

  memset(&default_sip_server_info, 0, sizeof(default_sip_server_info));
  memset(network_sip_server_info, 0, sizeof(network_sip_server_info));
  if (!(retval=QcMapMgr->GetSIPServerInfo(&default_sip_server_info,
                                          network_sip_server_info,
                                          &count_network_sip_server_info,
                                          &qmi_err_num)))
  {
    LOG_MSG_ERROR("Get SIP server failed Error: %d ",
                  qmi_err_num,
                  0, 0);

    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_get_sip_server_info_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if (default_sip_server_info.pcscf_info_type != QCMAP_CM_SIP_SERVER_MIN)
  {
    resp_msg.default_sip_server_info_valid = TRUE;
    memcpy(&(resp_msg.default_sip_server_info),
           &default_sip_server_info,
           sizeof(default_sip_server_info));
  }

  if (count_network_sip_server_info > 0)
  {
    LOG_MSG_INFO1("Found %d network assigned SIP servers", count_network_sip_server_info, 0, 0);
    resp_msg.network_sip_server_info_valid = TRUE;
    resp_msg.network_sip_server_info_len = count_network_sip_server_info;
    memcpy(resp_msg.network_sip_server_info,
           network_sip_server_info,
           count_network_sip_server_info*sizeof(qcmap_msgr_sip_server_info_v01));
  }
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_sip_server_info_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_sip_server_info() */


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_IPV6_SIP_SERVER_INFO

  DESCRIPTION
   - Retrieves network assigned IPV6 SIP server information.

  DEPENDENCIES
    qmi_qcmap_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_get_ipv6_sip_server_info
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type                   *qcmap_sp;
  qcmap_msgr_get_ipv6_sip_server_info_req_msg_v01  *req_ptr;
  qcmap_msgr_get_ipv6_sip_server_info_resp_msg_v01 resp_msg;
  qmi_qcmap_msgr_softap_handle_type                *map_handle;
  boolean                                          retval;
  int16                                            ps_errno;
  int                                              err_num = 0;
  unsigned int                                     index;
  unsigned int                                     count = 0;
  qmi_error_type_v01                               qmi_err_num;
  qcmap_msgr_ipv6_sip_server_info_v01
            network_v6_sip_server_info[QCMAP_MSGR_MAX_SIP_SERVER_ENTRIES_V01];
  int                                      count_network_v6_sip_server_info=0;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type*)service_handle;
  req_ptr = (qcmap_msgr_get_ipv6_sip_server_info_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_ipv6_sip_server_info_resp_msg_v01));


  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
     (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_get_ipv6_sip_server_info_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1 ("Client %p MobileAP inst %d",
                  clnt_info, index, 0);


  memset(network_v6_sip_server_info, 0, sizeof(network_v6_sip_server_info));
  if (!(QcMapMgr->GetV6SIPServerInfo(network_v6_sip_server_info,
                                     &count_network_v6_sip_server_info,
                                     &qmi_err_num)))
  {
    LOG_MSG_ERROR("Get IPV6 SIP server failed Error: %d ",
                  qmi_err_num,
                  0, 0);

    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_get_ipv6_sip_server_info_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if (count_network_v6_sip_server_info > 0)
  {
    LOG_MSG_INFO1("Found %d network assigned IPV6 SIP servers",
                  count_network_v6_sip_server_info, 0, 0);
    resp_msg.network_ipv6_sip_server_info_valid = TRUE;
    resp_msg.network_ipv6_sip_server_info_len = count_network_v6_sip_server_info;
    memcpy(resp_msg.network_ipv6_sip_server_info,
           network_v6_sip_server_info,
           count_network_v6_sip_server_info*sizeof(qcmap_msgr_ipv6_sip_server_info_v01));
  }
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_ipv6_sip_server_info_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_ipv6_sip_server_info() */


/*===========================================================================
  FUNCTION  QMI_QCMAP_MSGR_RESTORE_FACTORY_CONFIG()

  DESCRIPTION
  Restore factory default configuration.

  PARAMETERS
    sp:         QMI_QCMAP MSGR's service instance state pointer for this qmi link
    cmd_buf_p:   Message Header
    cl_sp:       Coresponding client state pointer
    sdu_in:      input command data

  RETURN VALUE
    dsm * pointing to the response to be sent to host
    NULL if no response was generated or command was discarded

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_restore_factory_config
(
  qmi_qcmap_msgr_client_info_type    *clnt_info,
  qmi_req_handle                     req_handle,
  int                                msg_id,
  void                               *req_c_struct,
  int                                req_c_struct_len,
  void                               *service_handle
)
{
  qcmap_msgr_restore_factory_config_req_msg_v01   *req_ptr;
  qcmap_msgr_restore_factory_config_resp_msg_v01  resp_msg;
  qmi_error_type_v01                              qmi_err_num;
  qmi_qcmap_msgr_softap_handle_type               *map_handle;
  unsigned int                                    index;

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  LOG_MSG_INFO1 ("Client %p Restore factory default configuration", clnt_info,0,0);

  req_ptr = ( qcmap_msgr_restore_factory_config_req_msg_v01 *)req_c_struct;
  memset( &resp_msg, 0, sizeof( qcmap_msgr_restore_factory_config_resp_msg_v01 ));
  /*---------------------------------------------------------------------
    Reset to factory configuration.
   ---------------------------------------------------------------------*/
  index = req_ptr->mobile_ap_handle;
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
     (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_ERROR("Incorrect handle passed %d",req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp( req_handle, msg_id, &resp_msg,
                       sizeof(qcmap_msgr_restore_factory_config_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  /*Send response to client that the device will be resetting */
  qmi_csi_send_resp( req_handle, msg_id, &resp_msg,
                     sizeof(qcmap_msgr_restore_factory_config_resp_msg_v01));

  if( QcMapMgr->RestoreFactoryConfig( &qmi_err_num ) )
  {
    LOG_MSG_INFO1(" RestoreFactoryConfig done.\n",0,0,0);
    return QMI_CSI_CB_NO_ERR;
  }
   else
  {
    LOG_MSG_ERROR(" RestoreFactoryConfig fails.\n",0,0,0);
    return QMI_CSI_CB_NO_ERR;
  }
}


/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_SET_ODU_MODE

  DESCRIPTION
   - Sets the ODU mode

  DEPENDENCIES
    qmi_qcmap_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_set_odu_mode(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qcmap_msgr_set_odu_mode_resp_msg_v01   resp_msg;
  qcmap_msgr_set_odu_mode_req_msg_v01    *req_msg;
  qmi_error_type_v01 qmi_err_num;

  int              ret_val;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);
  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);

  LOG_MSG_INFO1 ("Client %p setting ODU mode", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_odu_mode_resp_msg_v01));

  req_msg = (qcmap_msgr_set_odu_mode_req_msg_v01*) req_c_struct;

  /*---------------------------------------------------------------------
    Set ODU mode
  ---------------------------------------------------------------------*/

  if (QcMapMgr->SetODUMode(req_msg->mode, &qmi_err_num))
  {
    LOG_MSG_INFO1("Set ODU mode success. mode = ", req_msg->mode,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Set ODU mode failed: %d", qmi_err_num,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_odu_mode_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_set_odu_mode() */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_ODU_MODE

  DESCRIPTION
   - Retrieves the current ODU mode.

  DEPENDENCIES
    qmi_qcmap_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_get_odu_mode(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qcmap_msgr_get_odu_mode_resp_msg_v01   resp_msg;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_odu_mode_v01 mode;

  int              ret_val;
  unsigned int     index;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  LOG_MSG_INFO1 ("Client %p returning ODU mode", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_odu_mode_resp_msg_v01));

  /*---------------------------------------------------------------------
    Retreive DLNA Media Directory
  ---------------------------------------------------------------------*/

  if (QcMapMgr->GetODUMode( &mode, &qmi_err_num))
  {
    LOG_MSG_INFO1 ("Valid mode %d", mode,0,0);
    resp_msg.mode = mode;
    resp_msg.mode_valid = true;
  }
  else
  {
    LOG_MSG_ERROR("Get DLNA status fails.\n",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_odu_mode_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_odu_mode() */

/*===========================================================================
  FUNCTION QMI_QCMAP_MSGR_GET_CONNECTED_DEVICES_INFO

  DESCRIPTION
   - Fetches information regarding the devices connected to SoftAP device

  DEPENDENCIES
    qmi_qcmap_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error  qmi_qcmap_msgr_get_connected_devices_info
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qmi_qcmap_msgr_state_info_type              *qcmap_sp;
  qcmap_msgr_get_connected_devices_info_req_msg_v01  *req_ptr;
  qcmap_msgr_get_connected_devices_info_resp_msg_v01 resp_msg;
  qmi_qcmap_msgr_softap_handle_type           *map_handle;
  boolean                                     retval;
  unsigned int                                index;
  qmi_error_type_v01                          qmi_err_num;
  int                                         count_connected_devices_info=0;

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type*)service_handle;
  req_ptr = (qcmap_msgr_get_connected_devices_info_req_msg_v01 *)req_c_struct;
  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_connected_devices_info_resp_msg_v01));

  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
      (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_get_connected_devices_info_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  LOG_MSG_INFO1 ("Client %p MobileAP inst %d",
                 clnt_info, index, 0);

/*---------------------------------------------------------------------
   Get Connected Device Info
  ---------------------------------------------------------------------*/

  if (QcMapMgr->GetConnectedDevicesInfo(resp_msg.connected_devices_info,
                                        &resp_msg.connected_devices_info_len,
                                        &qmi_err_num))
  {
    LOG_MSG_INFO1("Get Connected Devices Information succeeded.\n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("No Connected Devices Information available currently",0,0,0);

    resp_msg.connected_devices_info_valid = false;
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_get_connected_devices_info_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  resp_msg.connected_devices_info_valid = true;
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_connected_devices_info_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
}

/*===========================================================================
  FUNCTION qmi_qcmap_msgr_set_supplicant_config()

  DESCRIPTION
    Activate/Deactivate the WPA Supplicant config.

  PARAMETERS
    clnt_info        : Client Info
    req_handle       : Request handle
    msg_id           : Message ID corresponding to the API
    req_c_struct     : Request structure
    req_c_struct_len : Length of the request structure
    service_handle   : Service handle

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_set_supplicant_config
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{

  qmi_qcmap_msgr_state_info_type *qcmap_sp;
  qcmap_msgr_set_supplicant_config_req_msg_v01 *req_ptr;
  qcmap_msgr_set_supplicant_config_resp_msg_v01 resp_msg;
  qmi_error_type_v01 qmi_err_num;
  unsigned int index;
  qmi_qcmap_msgr_softap_handle_type *map_handle;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ds_assert(req_c_struct != NULL);
  ds_assert(req_c_struct_len > 0);
  ds_assert(clnt_info != NULL);
  ds_assert(service_handle != NULL);

  qcmap_sp = (qmi_qcmap_msgr_state_info_type *)service_handle;
  req_ptr = (qcmap_msgr_set_supplicant_config_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p set supplicant config ", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_supplicant_config_resp_msg_v01));

  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
     (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_supplicant_config_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }
  if (QcMapMgr->SetWPASupplicantConfig(req_ptr->supplicant_config_status, &qmi_err_num))
  {
    LOG_MSG_INFO1("Station config applied successfully. \n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Station config applying failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_supplicant_config_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_supplicant_config_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_set_supplicant_config */

/*===========================================================================
  FUNCTION qmi_qcmap_msgr_get_cradle_mode()

  DESCRIPTION
    Retreives the usb cradle mode.

  PARAMETERS
    clnt_info        : Client Info
    req_handle       : Request handle
    msg_id           : Message ID corresponding to the API
    req_c_struct     : Request structure
    req_c_struct_len : Length of the request structure
    service_handle   : Service handle

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_cradle_mode
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qcmap_msgr_get_cradle_mode_req_msg_v01 *req_ptr;
  qcmap_msgr_get_cradle_mode_resp_msg_v01 resp_msg;
  qmi_error_type_v01 qmi_err_num;
  qcmap_msgr_cradle_mode_v01 mode;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ASSERT(req_c_struct);
  ASSERT(req_c_struct_len > 0);
  ASSERT(clnt_info);
  ASSERT(service_handle);

  req_ptr = (qcmap_msgr_get_cradle_mode_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p get cradle mode ", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_cradle_mode_resp_msg_v01));

  if (QcMapMgr->GetCradleMode(&mode, &qmi_err_num))
  {
    LOG_MSG_INFO1("Cradle mode retreived successfully. \n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Cradle mode retreive failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_get_cradle_mode_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  resp_msg.mode_valid = true;
  resp_msg.mode = mode;
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_cradle_mode_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_cradle_mode */

/*===========================================================================
  FUNCTION qmi_qcmap_msgr_set_cradle_mode()

  DESCRIPTION
    Sets the usb cradle mode.

  PARAMETERS
    clnt_info        : Client Info
    req_handle       : Request handle
    msg_id           : Message ID corresponding to the API
    req_c_struct     : Request structure
    req_c_struct_len : Length of the request structure
    service_handle   : Service handle

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_set_cradle_mode
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qcmap_msgr_set_cradle_mode_req_msg_v01 *req_ptr;
  qcmap_msgr_set_cradle_mode_resp_msg_v01 resp_msg;
  qmi_error_type_v01 qmi_err_num;
  unsigned int index;
  qmi_qcmap_msgr_softap_handle_type *map_handle;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ASSERT(req_c_struct);
  ASSERT(req_c_struct_len > 0);
  ASSERT(clnt_info);
  ASSERT(service_handle);

  req_ptr = (qcmap_msgr_set_cradle_mode_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p set cradle mode", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_cradle_mode_resp_msg_v01));

  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
     (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_cradle_mode_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if (QcMapMgr->SetCradleMode(req_ptr->mode, map_handle, &qmi_err_num))
  {
    LOG_MSG_INFO1("Cradle mode applied successfully. \n",0,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Cradle mode applying failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_cradle_mode_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_cradle_mode_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_set_cradle_mode */

/*===========================================================================
  FUNCTION qmi_qcmap_msgr_get_prefix_delgation_config()

  DESCRIPTION
    Retreives the prefix delegation config.

  PARAMETERS
    clnt_info        : Client Info
    req_handle       : Request handle
    msg_id           : Message ID corresponding to the API
    req_c_struct     : Request structure
    req_c_struct_len : Length of the request structure
    service_handle   : Service handle

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_prefix_delegation_config
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qcmap_msgr_get_prefix_delegation_config_req_msg_v01 *req_ptr;
  qcmap_msgr_get_prefix_delegation_config_resp_msg_v01 resp_msg;
  qmi_error_type_v01 qmi_err_num;
  boolean prefix_delegation;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ASSERT(req_c_struct);
  ASSERT(req_c_struct_len > 0);
  ASSERT(clnt_info);
  ASSERT(service_handle);

  req_ptr = (qcmap_msgr_get_prefix_delegation_config_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p get prefix delegation config ", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_prefix_delegation_config_resp_msg_v01));

  if (QcMapMgr->GetPrefixDelegationConfig(&prefix_delegation, &qmi_err_num))
  {
    LOG_MSG_INFO1("Prefix delegation mode retreived successfully. Config %d\n",prefix_delegation,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Prefix Delegation config retreive failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_get_prefix_delegation_config_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  resp_msg.prefix_delegation_valid = true;
  resp_msg.prefix_delegation = prefix_delegation;
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_prefix_delegation_config_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_prefix_delegation_config */

/*===========================================================================
  FUNCTION qmi_qcmap_msgr_set_prefix_delegation_config()

  DESCRIPTION
    Enable/disable prefix delegation config.

  PARAMETERS
    clnt_info        : Client Info
    req_handle       : Request handle
    msg_id           : Message ID corresponding to the API
    req_c_struct     : Request structure
    req_c_struct_len : Length of the request structure
    service_handle   : Service handle

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_set_prefix_delegation_config
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qcmap_msgr_set_prefix_delegation_config_req_msg_v01 *req_ptr;
  qcmap_msgr_set_prefix_delegation_config_resp_msg_v01 resp_msg;
  qmi_error_type_v01 qmi_err_num;
  unsigned int index;
  qmi_qcmap_msgr_softap_handle_type *map_handle;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ASSERT(req_c_struct);
  ASSERT(req_c_struct_len > 0);
  ASSERT(clnt_info);
  ASSERT(service_handle);

  req_ptr = (qcmap_msgr_set_prefix_delegation_config_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p set prefix delegation config", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_set_prefix_delegation_config_resp_msg_v01));

  index = (req_ptr->mobile_ap_handle);
  if (index != QCMAP_MSGR_SOFTAP_HANDLE ||
     (map_handle = &qcmap_handle)->handle == 0)
  {
    LOG_MSG_INFO1("Incorrect handle %d passed",
                  req_ptr->mobile_ap_handle,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = QMI_ERR_INVALID_HANDLE_V01;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_prefix_delegation_config_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  if (QcMapMgr->SetPrefixDelegationConfig(req_ptr->prefix_delegation, &qmi_err_num))
  {
    LOG_MSG_INFO1("Prefix delegation config %d applied successfully. \n",req_ptr->prefix_delegation,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Prefix delegation config applying failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_set_prefix_delegation_config_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_set_prefix_delegation_config_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_set_prefix_delegation_config */

/*===========================================================================
  FUNCTION qmi_qcmap_msgr_get_prefix_delgation_status()

  DESCRIPTION
    Retreives the current prefix delegation mode.

  PARAMETERS
    clnt_info        : Client Info
    req_handle       : Request handle
    msg_id           : Message ID corresponding to the API
    req_c_struct     : Request structure
    req_c_struct_len : Length of the request structure
    service_handle   : Service handle

  DEPENDENCIES
    qmi_qcmap_msgr_init() must have been called

  SIDE EFFECTS
    None
===========================================================================*/
static qmi_csi_cb_error qmi_qcmap_msgr_get_prefix_delegation_status
(
  qmi_qcmap_msgr_client_info_type        *clnt_info,
  qmi_req_handle           req_handle,
  int                      msg_id,
  void                    *req_c_struct,
  int                      req_c_struct_len,
  void                     *service_handle
)
{
  qcmap_msgr_get_prefix_delegation_status_req_msg_v01 *req_ptr;
  qcmap_msgr_get_prefix_delegation_status_resp_msg_v01 resp_msg;
  qmi_error_type_v01 qmi_err_num;
  boolean prefix_delegation;
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  ASSERT(req_c_struct);
  ASSERT(req_c_struct_len > 0);
  ASSERT(clnt_info);
  ASSERT(service_handle);

  req_ptr = (qcmap_msgr_get_prefix_delegation_status_req_msg_v01 *)req_c_struct;

  LOG_MSG_INFO1 ("Client %p get prefix delegation config ", clnt_info,0,0);

  memset(&resp_msg, 0, sizeof(qcmap_msgr_get_prefix_delegation_status_resp_msg_v01));

  if (QcMapMgr->GetPrefixDelegationStatus(&prefix_delegation, &qmi_err_num))
  {
    LOG_MSG_INFO1("Prefix delegation mode retreived successfully. Status %d\n",prefix_delegation,0,0);
  }
  else
  {
    LOG_MSG_ERROR("Prefix Delegation config retreive failed!!",0,0,0);
    resp_msg.resp.result = QMI_RESULT_FAILURE_V01;
    resp_msg.resp.error = qmi_err_num;
    qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                      sizeof(qcmap_msgr_get_prefix_delegation_status_resp_msg_v01));
    return QMI_CSI_CB_NO_ERR;
  }

  resp_msg.prefix_delegation_valid = true;
  resp_msg.prefix_delegation = prefix_delegation;
  qmi_csi_send_resp(req_handle, msg_id, &resp_msg,
                    sizeof(qcmap_msgr_get_prefix_delegation_status_resp_msg_v01));

  return QMI_CSI_CB_NO_ERR;
} /* qmi_qcmap_msgr_get_prefix_delegation_status */
