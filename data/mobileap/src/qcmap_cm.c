/******************************************************************************

                           QCMAP_CM.C

******************************************************************************/

/******************************************************************************

  @file    qcmap_cm.c
  @brief   Mobile AP Connection Manager Lib Implementation

  DESCRIPTION
  Mobile AP Connection Manager Lib Implementation.

  ---------------------------------------------------------------------------
  Copyright (c) 2011-2014 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        --------------------------------------------------------
01/28/14   rk         KW fixes.
10/04/13   pm         Added fix to backhaul issue by reading state variable
                      under mutex lock.
04/11/13   mp         Added support for DUN call over MobileAP.
03/29/13   sb         QCMAP boot up optimizations for RNDIS.
12/25/12   mp         Added flag to check roaming indicator validity.
10/03/12   mp         Fix to prevent QCMAP starting call before QTI Link UP.
10/16/12   mp         Fix to start autoconnect backhaul in all cases.
09/11/12   mp         Added support for dualAPN in MobileAP.
08/03/12   mp         Added support for Port Restricted Cone NAT.
07/20/12   cp         Embedded data call fixes.
07/02/12   vb         Fixed bug in Embedded Call Handling.
06/25/12   mp         Added support to propagate config parameters while
                      switching from AP+STA to AP mode.
06/14/12   rk         Fixed compilation warnings and Embedded call issues.
05/18/12   vb         Added support for embedded call bringup.
05/17/12   cp         Added support for data transfer between tethered USB
                      client and stations connected to SSID2.
05/04/12   cp         AP-STA mode fixes.
05/03/12   sb         Coex service change
04/16/12   sb         IPV6 state machine fix
03/09/12   sb         Adding extended firewall support
03/02/12   ss         Adding support for Concurrent STA+AP mode.
03/01/12   SDT        Adding support for Dual AP mode.
02/20/12   SDT        Adding support for IPv6.

******************************************************************************/

/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/un.h>
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
#include "customer.h"
#include "comdef.h"
#include "amssassert.h"
#include "msg.h"
#include "err.h"
#include "ds_util.h"
#include "ds_Utils_DebugMsg.h"
#include "qmi_client.h"
#include "qualcomm_mobile_access_point_v01.h"
#include "network_access_service_v01.h"
#include "qcmap_cm_api.h"
#include "coexistence_service_v01.h"
#include "ds_cmdq.h"
#include "qcmap_cmdq.h"
#ifdef USE_GLIB
#include <glib.h>
#define strlcpy g_strlcpy
#endif


/*===========================================================================
                     FORWARD DECLARATIONS
===========================================================================*/

/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/
#define QCMAP_DEFAULT_DSS_INIT_TIME    3

/*---------------------------------------------------------------------------
   Type representing an QCMAP CM CB (QCMAP CM control block); this captures
   all state and other data related to the QCMAP CM
---------------------------------------------------------------------------*/
typedef struct
{
    qcmap_cm_state_e          state;
    int                       handle;
    qcmap_cm_state_e          ipv6_state;

    /* MobileAP config read from XML file or default.*/
    qcmap_cm_conf_t          *cfg;

    /* libqcmap_cm library callback info */
    qcmap_cm_cb_fcn           qcmap_cm_callback;
    void                     *qcmap_cm_callback_user_data;
    pthread_mutex_t           qcmap_cm_mutex;

    dsi_call_info_t dsi_net_hndl;
    char dsi_device_name[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 2];

    /* IPv6 DSI */
    dsi_call_info_t ipv6_dsi_net_hndl;
    char ipv6_dsi_device_name[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 2];

    /* QMI QCMAP service info */
    qmi_client_type           qmi_qcmap_handle;
    qmi_client_type           qmi_qcmap_notifier;
    qmi_cci_os_signal_type    qmi_qcmap_os_params;

    /* QMI NAS service info */
    qmi_client_type           qmi_nas_handle;
    qmi_client_type           qmi_nas_notifier;
    qmi_cci_os_signal_type    qmi_nas_os_params;

    /* QMI QCMAP IPv4v6 handles */
    uint32_t                  mobile_ap_handle;
    uint32_t                  ipv6_mobile_ap_handle;

    /* Whether it is init */
    boolean                   init;

    /*Whether disable is in progress*/
    boolean                   disable_in_process;

    /* Whether it has backhaul service  */
    boolean                   backhaul_service;
    uint8                     roaming_indicator;
    boolean                   roaming_indicator_valid;
    uint8                     eri_roam_data[QCMAP_WAN_MAX_ERI_DATA_SIZE];
    uint16                    eri_roam_data_len;
} qcmap_cm_cb_t;

/*---------------------------------------------------------------------------
   Mobile AP CM control block
---------------------------------------------------------------------------*/
static qcmap_cm_cb_t qcmap_cm_cb = {
  .state                       = QCMAP_CM_DISABLE,
  .ipv6_state                  = QCMAP_CM_V6_DISABLE,
  .handle                      = 1,
  .mobile_ap_handle            = QCMAP_WAN_INVALID_QCMAP_HANDLE,
  .ipv6_mobile_ap_handle       = QCMAP_WAN_INVALID_QCMAP_HANDLE,
  .qcmap_cm_callback           = NULL,
  .qcmap_cm_callback_user_data = NULL,
  .init                        = FALSE,
  .backhaul_service            = FALSE,
  .roaming_indicator           = 0,
  .roaming_indicator_valid     = FALSE,
  .eri_roam_data_len           = 0,
  .disable_in_process          = FALSE
};

extern unsigned int cmdq_qcmap_sockfd;
extern unsigned int nas_qcmap_sockfd;
extern unsigned int timer_qcmap_sockfd;


/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  qcmap_cm_lock
===========================================================================*/
/*!
@brief
  Wrapper function for acquiring the shared lock used for serializing access
  to QCMAP CM CB.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static void
qcmap_cm_lock (void)
{
  ds_assert(pthread_mutex_lock(&qcmap_cm_cb.qcmap_cm_mutex) == 0);
}

/*===========================================================================
  FUNCTION  qcmap_cm_unlock
===========================================================================*/
/*!
@brief
  Wrapper function for releasing the shared lock used for serializing access
  to QCMAP CM CB.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static void
qcmap_cm_unlock (void)
{
  ds_assert(pthread_mutex_unlock(&qcmap_cm_cb.qcmap_cm_mutex) == 0);
}

/*===========================================================================
  FUNCTION  qcmap_cm_embd_lock
===========================================================================*/
/*!
@brief
  Wrapper function for acquiring the shared lock used for serializing access
  to QCMAP CM EMBD CB.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/


static int
qcmap_cm_firewall_entry_op_delete_all
(
  int                            *qcmap_cm_errno        /* Error condition */
);

/*===========================================================================
  FUNCTION  qcmap_cm_config_init
===========================================================================*/
/*!
@brief
    QCMAP CM config init function

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static void qcmap_cm_config_init
(
  int    qcmap_cm_handle,                           /* Handle for MobileAP CM  */
  int    *qcmap_cm_errno                            /* Error condition value  */
);


/*===========================================================================
  FUNCTION  qcmap_cm_process_dsi_net_evt
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

void qcmap_cm_process_dsi_net_evt
(
dsi_hndl_t hndl,
void * user_data,
dsi_net_evt_t evt,
dsi_evt_payload_t *payload_ptr
)
{
  qcmap_cm_event_e                     event;
  boolean                              callback = FALSE;
  int                                  i;
  int                                  err;
  int                                  *qcmap_cm_errno = &err;
  qmi_client_error_type                qmi_error;
  qcmap_cm_state_e                     state,ipv6_state;
  nas_indication_register_req_msg_v01  qcmap_nas_indication_register_req_msg_v01;
  nas_indication_register_resp_msg_v01 qcmap_nas_indication_register_resp_msg_v01;

  LOG_MSG_INFO1_6("qcmap_cm_process_dsi_net_evt: dsi_handle %d "
                "qcmap_cm_cb.state (V4) %d qcmap_cm_cb.ipv6_state (V6) %d", hndl,
                qcmap_cm_cb.state, qcmap_cm_cb.ipv6_state,0,0,0);

  if(hndl == qcmap_cm_cb.ipv6_dsi_net_hndl.handle)
  {
    qcmap_cm_lock();
    ipv6_state = qcmap_cm_cb.ipv6_state;
    qcmap_cm_unlock();
    if(ipv6_state == QCMAP_CM_V6_LAN_CONNECTING)
    {
        if(evt == DSI_EVT_NET_IS_CONN)
        {
          int rval;
          char device[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 2];

          memset(device, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);
          qcmap_cm_lock();
          qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_LAN_CONNECTED;
          qcmap_cm_unlock();
          event = QCMAP_CM_EVENT_LAN_IPv6_CONNECTED;
          callback = TRUE;
          LOG_MSG_INFO1("Received DSI_EVT_NET_IS_CONN in state V6_LAN_CONNECTING",0,0,0);
          LOG_MSG_INFO1("Transition from state V6_LAN_CONNECTING to V6_LAN_CONNECTED",0,0,0);

	  rval = dsi_get_device_name(qcmap_cm_cb.ipv6_dsi_net_hndl.handle, device, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1);

          if(rval != DSI_SUCCESS)
          {
            LOG_MSG_ERROR ("Couldn't get ipv6 rmnet name. rval %d ", rval, 0, 0);
            strlcpy((char*) &device, QCMAP_V6_DEFAULT_DEVICE_NAME, 16);
          }

          LOG_MSG_INFO1("device_name:%s ", device, 0, 0);

          memcpy(&qcmap_cm_cb.ipv6_dsi_device_name, &device, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN);
    
          LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv6 LAN Connected",
                        qcmap_cm_cb.handle, 0, 0);
        }
        else if (evt == DSI_EVT_NET_NO_NET)
        {
          qcmap_cm_lock();
          qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_ENABLE;
          qcmap_cm_unlock();
          event = QCMAP_CM_EVENT_LAN_IPv6_CONNECTING_FAIL;
          callback = TRUE;
          LOG_MSG_INFO1("Received DSI_EVT_NET_NO_NET in state V6_LAN_CONNECTING",0,0,0);
          LOG_MSG_INFO1("Transition from state V6_LAN_CONNECTING to V6_ENABLE",0,0,0);
          LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv6 LAN Connecting Fail",
                        qcmap_cm_cb.handle, 0, 0);
        }
        else if(evt == QCMAP_CM_IND_V6_WAN_CONNECTED)
        {
          LOG_MSG_INFO1("Received IND_V6_WAN_CONNECTED in state V6_LAN_CONNECTING",0,0,0);
          LOG_MSG_INFO1("Ignore event IND_V6_WAN_CONNECTED in state V6_LAN_CONNECTING",0,0,0);
        }
        else if(evt == QCMAP_CM_IND_V6_WAN_DISCONNECTED)
        {
          LOG_MSG_INFO1("Received IND_V6_WAN_DISCONNECTED in state V6_LAN_CONNECTING",0,0,0);
          LOG_MSG_INFO1("Ignore event IND_V6_WAN_DISCONNECTED in state V6_LAN_CONNECTING",0,0,0);
        }
    }
    else if (ipv6_state == QCMAP_CM_V6_LAN_CONNECTED)
    {
      if (evt == DSI_EVT_NET_IS_CONN)
      {
          LOG_MSG_INFO1("Received DSI_EVT_NET_IS_CONN in state V6_LAN_CONNECTED",0,0,0);
          LOG_MSG_INFO1("Ignore event DSI_EVT_NET_IS_CONN in state V6_LAN_CONNECTED",0,0,0);
      }
      else if(evt == DSI_EVT_NET_NO_NET)
      {
        qcmap_cm_lock();
        qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_ENABLE;
        qcmap_cm_unlock();
        event = QCMAP_CM_EVENT_LAN_IPv6_DISCONNECTED;
        callback = TRUE;
        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv6 LAN Disconnected",
                        qcmap_cm_cb.handle, 0, 0);
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in state V6_LAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Transition from V6_LAN_CONNECTED to V6_ENABLE",0,0,0);
        if(qcmap_cm_cb.disable_in_process)
        {
          qcmap_cm_disable(qcmap_cm_cb.handle, qcmap_cm_errno);
        }
      }
      else if (evt == QCMAP_CM_IND_V6_WAN_CONNECTED)
      {
        qcmap_cm_lock();
        qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_WAN_CONNECTED;
        qcmap_cm_unlock();
        event = QCMAP_CM_EVENT_WAN_IPv6_CONNECTED;
        callback = TRUE;
        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv6 WAN Connected",
                        qcmap_cm_cb.handle, 0, 0);
        LOG_MSG_INFO1("Received event IND_V6_WAN_CONNECTED in state V6_LAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Transition from V6_LAN_CONNECTED to V6_WAN_CONNECTED",0,0,0);
        
        /*deregister from serving system NAS indication to save power*/
        memset(&qcmap_nas_indication_register_req_msg_v01, 0, sizeof(nas_indication_register_req_msg_v01));

        qcmap_nas_indication_register_req_msg_v01.req_serving_system_valid = TRUE;
        qcmap_nas_indication_register_req_msg_v01.req_serving_system = 0x00;

        LOG_MSG_INFO1("Deregistering from serving system NAS indications",0,0,0);

        qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_nas_handle,
                                       QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01,
                                       &qcmap_nas_indication_register_req_msg_v01,
                                       sizeof(qcmap_nas_indication_register_req_msg_v01),
                                       &qcmap_nas_indication_register_resp_msg_v01,
                                       sizeof(qcmap_nas_indication_register_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
        if(qmi_error != QMI_NO_ERR)
        {
          LOG_MSG_ERROR("Failed to deregister from NAS indications %d", qmi_error,0,0);
          return;
        }

      }
      else if (evt == QCMAP_CM_IND_V6_WAN_DISCONNECTED)
      {
        LOG_MSG_INFO1("Received event IND_V6_WAN_DISCONNECTED in state V6_LAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Ignore event IND_V6_WAN_CONNECTED in state V6_LAN_CONNECTED",0,0,0);
      }
    }
    else if(ipv6_state == QCMAP_CM_V6_LAN_DISCONNECTING)
    {
      if(evt == DSI_EVT_NET_IS_CONN)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_IS_CONN in state V6_LAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event  DSI_EVT_NET_IS_CONN in state V6_LAN_DISCONNECTING",0,0,0);
      }
      else if(evt == DSI_EVT_NET_NO_NET)
      {
        qcmap_cm_lock();
        qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_ENABLE;
        qcmap_cm_unlock();
        event = QCMAP_CM_EVENT_LAN_IPv6_DISCONNECTED;
        callback = TRUE;
        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv6 LAN Disconnected",
                        qcmap_cm_cb.handle, 0, 0);
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in state V6_LAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Transition from V6_LAN_DISCONNECTING to V6_ENABLE",0,0,0);
        if(qcmap_cm_cb.disable_in_process)
        {
          qcmap_cm_disable(qcmap_cm_cb.handle, qcmap_cm_errno);
        }
      }
      else if (evt == QCMAP_CM_IND_V6_WAN_CONNECTED)
      {
        LOG_MSG_INFO1("Received event IND_V6_WAN_CONNECTED in V6_LAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event IND_V6_WAN_CONNECTED in state V6_LAN_DISCONNECTING",0,0,0);
      }
      else if (evt == QCMAP_CM_IND_V6_WAN_DISCONNECTED)
      {
        LOG_MSG_INFO1("Received event IND_V6_WAN_DISCONNECTED in V6_LAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event IND_V6_WAN_DISCONNECTED in state V6_LAN_DISCONNECTING",0,0,0);
      }
    }
    else if(ipv6_state == QCMAP_CM_V6_WAN_CONNECTING)
    {
      if(evt == DSI_EVT_NET_IS_CONN)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_IS_CONN in V6_WAN_CONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_IS_CONN in state V6_WAN_CONNECTING",0,0,0);
      }
      else if(evt == DSI_EVT_NET_NO_NET)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in V6_WAN_CONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_NO_NET in state V6_WAN_CONNECTING",0,0,0);
      }
      else if (evt == QCMAP_CM_IND_V6_WAN_CONNECTED)
      {
        qcmap_cm_lock();
        qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_WAN_CONNECTED;
        qcmap_cm_unlock();
        event = QCMAP_CM_EVENT_WAN_IPv6_CONNECTED;
        callback = TRUE;
        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv6 WAN Connected",
                        qcmap_cm_cb.handle, 0, 0);
        LOG_MSG_INFO1("Received event IND_V6_WAN_CONNECTED in V6_WAN_CONNECTING",0,0,0);
        LOG_MSG_INFO1("Transition from V6_WAN_CONNECTING to V6_WAN_CONNECTED",0,0,0);

        /*deregister from serving system NAS indication to save power*/
        memset(&qcmap_nas_indication_register_req_msg_v01, 0, sizeof(nas_indication_register_req_msg_v01));

        qcmap_nas_indication_register_req_msg_v01.req_serving_system_valid = TRUE;
        qcmap_nas_indication_register_req_msg_v01.req_serving_system = 0x00;

        LOG_MSG_INFO1("Deregistering from serving system NAS indications",0,0,0);

        qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_nas_handle,
                                       QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01,
                                       &qcmap_nas_indication_register_req_msg_v01,
                                       sizeof(qcmap_nas_indication_register_req_msg_v01),
                                       &qcmap_nas_indication_register_resp_msg_v01,
                                       sizeof(qcmap_nas_indication_register_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
        if(qmi_error != QMI_NO_ERR)
        {
          LOG_MSG_ERROR("Failed to deregister from NAS indications %d", qmi_error,0,0);
          return;
        }

      }
      else if (evt == QCMAP_CM_IND_V6_WAN_DISCONNECTED)
      {
        qcmap_cm_lock();
        qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_LAN_CONNECTED;
        qcmap_cm_unlock();
        event = QCMAP_CM_EVENT_WAN_IPv6_DISCONNECTED;
        callback = TRUE;
        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv6 WAN Disconnected",
                        qcmap_cm_cb.handle, 0, 0);
        LOG_MSG_INFO1("Received event IND_V6_WAN_DISCONNECTED in V6_WAN_CONNECTING",0,0,0);
        LOG_MSG_INFO1("Transition from V6_WAN_CONNECTING to V6_LAN_CONNECTED",0,0,0);

        if(qcmap_cm_cb.disable_in_process)
        {
          qcmap_cm_disable(qcmap_cm_cb.handle, qcmap_cm_errno);
        }
        else if(qcmap_cm_cb.cfg->wan_config.auto_connect)
        {
          /*register for serving system NAS indication*/
          memset(&qcmap_nas_indication_register_req_msg_v01, 0,
                       sizeof(nas_indication_register_req_msg_v01));
          
          qcmap_nas_indication_register_req_msg_v01.req_serving_system_valid =
                                                                          TRUE;
          qcmap_nas_indication_register_req_msg_v01.req_serving_system = 0x01;

          LOG_MSG_INFO1("Registering for serving system NAS indication",0,0,0);

          qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_nas_handle,
                         QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01,
                         &qcmap_nas_indication_register_req_msg_v01,
                         sizeof(qcmap_nas_indication_register_req_msg_v01),
                         &qcmap_nas_indication_register_resp_msg_v01,
                         sizeof(qcmap_nas_indication_register_resp_msg_v01),
                                QCMAP_CM_QMI_TIMEOUT_VALUE);
          if(qmi_error != QMI_NO_ERR)
          {
            LOG_MSG_ERROR("Failed to register to NAS indications %d",
                                                         qmi_error,0,0);
            return;
          }

          LOG_MSG_INFO1("Start autoconnect",0,0,0);
          /* Set backhual service status to false. We don't know the reason
             for the disconnect. Try to reconnect the backhaul. */
          qcmap_cm_cb.backhaul_service = FALSE;
          qcmap_cm_connect_backhaul(qcmap_cm_cb.handle, qcmap_cm_errno);
        }
      }
    }
    else if(ipv6_state == QCMAP_CM_V6_WAN_CONNECTED)
    {
      if(evt == DSI_EVT_NET_IS_CONN)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_IS_CONN in V6_WAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_IS_CONN in state V6_WAN_CONNECTED",0,0,0);
      }
      else if(evt == DSI_EVT_NET_NO_NET)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in V6_WAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_NO_NET in state V6_WAN_CONNECTED",0,0,0);
      }
      else if (evt == QCMAP_CM_IND_V6_WAN_CONNECTED)
      {
        LOG_MSG_INFO1("Received event IND_V6_WAN_CONNECTED in V6_WAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Ignore event IND_V6_WAN_CONNECTED in state V6_WAN_CONNECTED",0,0,0);
      }
      else if (evt == QCMAP_CM_IND_V6_WAN_DISCONNECTED)
      {
        qcmap_cm_lock();
        qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_LAN_CONNECTED;
        qcmap_cm_unlock();
        event = QCMAP_CM_EVENT_WAN_IPv6_DISCONNECTED;
        callback = TRUE;
        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv6 WAN Disconnected",
                      qcmap_cm_cb.handle, 0, 0);
        LOG_MSG_INFO1("Received event IND_V6_WAN_DISCONNECTED in V6_WAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Transition from V6_WAN_CONNECTED to V6_LAN_CONNECTED",0,0,0);
        if(qcmap_cm_cb.disable_in_process)
        {
          qcmap_cm_disable(qcmap_cm_cb.handle, qcmap_cm_errno);
        }
        else if(qcmap_cm_cb.cfg->wan_config.auto_connect)
        {
          /*register for serving system NAS indication*/
          memset(&qcmap_nas_indication_register_req_msg_v01, 0, sizeof(nas_indication_register_req_msg_v01));
          
          qcmap_nas_indication_register_req_msg_v01.req_serving_system_valid = TRUE;
          qcmap_nas_indication_register_req_msg_v01.req_serving_system = 0x01;

          LOG_MSG_INFO1("Registering for serving system NAS indications",0,0,0);

          qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_nas_handle,
                                         QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01,
                                         &qcmap_nas_indication_register_req_msg_v01,
                                         sizeof(qcmap_nas_indication_register_req_msg_v01),
                                         &qcmap_nas_indication_register_resp_msg_v01,
                                         sizeof(qcmap_nas_indication_register_resp_msg_v01),
                                         QCMAP_CM_QMI_TIMEOUT_VALUE);
          if(qmi_error != QMI_NO_ERR)
          {
            LOG_MSG_ERROR("Failed to register to NAS indications %d", qmi_error,0,0);
            return;
          }

            LOG_MSG_INFO1("Start autoconnect",0,0,0);
            /* Set backhual service status to false. We don't know the reason
               for the disconnect. Try to reconnect the backhaul. */
            qcmap_cm_cb.backhaul_service = FALSE;
            qcmap_cm_connect_backhaul(qcmap_cm_cb.handle, qcmap_cm_errno);
        }
      }
    }
    else if(ipv6_state == QCMAP_CM_V6_WAN_DISCONNECTING)
    {
      if(evt == DSI_EVT_NET_IS_CONN)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_IS_CONN in V6_WAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_IS_CONN in V6_WAN_DISCONNECTING",0,0,0);
      }
      else if(evt == DSI_EVT_NET_NO_NET)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in V6_WAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore DSI_EVT_NET_NO_NET in  V6_WAN_DISCONNECTING",0,0,0);
      }
      else if (evt == QCMAP_CM_IND_V6_WAN_CONNECTED)
      {
        LOG_MSG_INFO1("Received event IND_V6_WAN_CONNECTED in V6_WAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore IND_V6_WAN_CONNECTED in  V6_WAN_DISCONNECTING",0,0,0);
      }
      else if (evt == QCMAP_CM_IND_V6_WAN_DISCONNECTED)
      {
          qcmap_cm_lock();
          qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_LAN_CONNECTED;
          qcmap_cm_unlock();
          event = QCMAP_CM_EVENT_WAN_IPv6_DISCONNECTED;
          LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv6 Backhaul Disconnected",
                        qcmap_cm_cb.handle, 0, 0);
          callback = TRUE;
          LOG_MSG_INFO1("Received event IND_V6_WAN_DISCONNECTED in V6_WAN_DISCONNECTING",0,0,0);
          LOG_MSG_INFO1("Transition from V6_WAN_DISCONNECTING to V6_LAN_CONNECTED",0,0,0);
          if(qcmap_cm_cb.disable_in_process)
          {
            qcmap_cm_disable(qcmap_cm_cb.handle, qcmap_cm_errno);
          }
      }
    }
    else if(ipv6_state == QCMAP_CM_V6_ENABLE)
    {
      if(evt == DSI_EVT_NET_IS_CONN)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_IS_CONN in V6_ENABLE",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_IS_CONN in V6_ENABLE",0,0,0);
      }
      else if(evt == DSI_EVT_NET_NO_NET)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in V6_ENABLE",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_NO_NET in V6_ENABLE",0,0,0);
      }
      else if (evt == QCMAP_CM_IND_V6_WAN_CONNECTED)
      {
        LOG_MSG_INFO1("Received event V6_WAN_CONNECTED in V6_ENABLE",0,0,0);
        LOG_MSG_INFO1("Ignore event V6_WAN_CONNECTED in V6_ENABLE",0,0,0);
      }
      else if (evt == QCMAP_CM_IND_V6_WAN_DISCONNECTED)
      {
        LOG_MSG_INFO1("Received event V6_WAN_DISCONNECTED in V6_ENABLE",0,0,0);
        LOG_MSG_INFO1("Ignore event V6_WAN_DISCONNECTED in V6_ENABLE",0,0,0);
      }
    }
  }

  if (hndl == qcmap_cm_cb.dsi_net_hndl.handle)
  {
    qcmap_cm_lock();
    state = qcmap_cm_cb.state;
    qcmap_cm_unlock();
    if (state == QCMAP_CM_LAN_CONNECTING)
    {
      if (evt == DSI_EVT_NET_IS_CONN)
      {
        int rval;
        char device[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 2];
        memset(device, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);


        qcmap_cm_lock();  
        qcmap_cm_cb.state = QCMAP_CM_LAN_CONNECTED;
        qcmap_cm_unlock();
        event = QCMAP_CM_EVENT_LAN_CONNECTED;
        callback = TRUE;

        LOG_MSG_INFO1("Received DSI_EVT_NET_IS_CONN in state LAN_CONNECTING",0,0,0);
        LOG_MSG_INFO1("Transition from state LAN_CONNECTING to LAN_CONNECTED",0,0,0);

        qcmap_cm_config_init(qcmap_cm_cb.handle, qcmap_cm_errno);

        rval = dsi_get_device_name(qcmap_cm_cb.dsi_net_hndl.handle, device, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1);

        if ( rval != DSI_SUCCESS )
        {
          LOG_MSG_ERROR ("Couldn't get ipv4 rmnet name. rval %d",
                         rval, 0, 0);
          strlcpy((char *)device, QCMAP_V4_DEFAULT_DEVICE_NAME, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1);
        }

        LOG_MSG_INFO1("device_name:%s ", device, 0, 0);

        memcpy(&qcmap_cm_cb.dsi_device_name, device, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);

        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv4 LAN Connected", qcmap_cm_cb.handle, 0, 0);
      }

      else if (evt == DSI_EVT_NET_NO_NET)
      {
        qcmap_cm_lock();
        qcmap_cm_cb.state = QCMAP_CM_ENABLE;
        qcmap_cm_unlock();
        event = QCMAP_CM_EVENT_LAN_CONNECTING_FAIL;
        callback = TRUE;
        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv4 LAN Connecting Fail",
                      qcmap_cm_cb.handle, 0, 0);
        LOG_MSG_INFO1("Received DSI_EVT_NET_NO_NET in state LAN_CONNECTING",0,0,0);
        LOG_MSG_INFO1("Transition from state LAN_CONNECTING to ENABLE",0,0,0);
      }
      else if(evt == QCMAP_CM_IND_V4_WAN_CONNECTED)
      {
        LOG_MSG_INFO1("Received IND_V4_WAN_CONNECTED in state LAN_CONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event IND_V4_WAN_CONNECTED in state LAN_CONNECTING",0,0,0);
      }
      else if(evt == QCMAP_CM_IND_V4_WAN_DISCONNECTED)
      {
        LOG_MSG_INFO1("Received IND_V4_WAN_DISCONNECTED in state LAN_CONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event IND_V4_WAN_DISCONNECTED in state LAN_CONNECTING",0,0,0);
      }
    }
    else if (state == QCMAP_CM_LAN_CONNECTED)
    {
      if (evt == DSI_EVT_NET_IS_CONN)
      {
        LOG_MSG_INFO1("Received DSI_EVT_NET_IS_CONN in state LAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_IS_CONN in state LAN_CONNECTED",0,0,0);
      }
      else if (evt == DSI_EVT_NET_NO_NET)
      {
        qcmap_cm_lock();
        qcmap_cm_cb.state = QCMAP_CM_ENABLE;
        qcmap_cm_unlock();
        event = QCMAP_CM_EVENT_LAN_DISCONNECTED;
        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv4 LAN Disconnected",
                      qcmap_cm_cb.handle, 0, 0);
        callback = TRUE;
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in state LAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Transition from LAN_CONNECTED to ENABLE",0,0,0);
        if(qcmap_cm_cb.disable_in_process)
        {
          qcmap_cm_disable(qcmap_cm_cb.handle, qcmap_cm_errno);
        }
      }
      else if(evt == QCMAP_CM_IND_V4_WAN_CONNECTED)
      {
        qcmap_cm_lock();
        qcmap_cm_cb.state = QCMAP_CM_WAN_CONNECTED;
        qcmap_cm_unlock();
        event = QCMAP_CM_EVENT_WAN_CONNECTED;
        callback = TRUE;
        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv4 WAN Connected",
                      qcmap_cm_cb.handle, 0, 0);
        LOG_MSG_INFO1("Received event IND_V4_WAN_CONNECTED in state LAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Transition from LAN_CONNECTED to WAN_CONNECTED",0,0,0);

        /*deregister from serving system NAS indication to save power*/
        memset(&qcmap_nas_indication_register_req_msg_v01, 0, sizeof(nas_indication_register_req_msg_v01));

        qcmap_nas_indication_register_req_msg_v01.req_serving_system_valid = TRUE;
        qcmap_nas_indication_register_req_msg_v01.req_serving_system = 0x00;

        LOG_MSG_INFO1("Deregistering from serving system NAS indications",0,0,0);

        qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_nas_handle,
                                       QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01,
                                       &qcmap_nas_indication_register_req_msg_v01,
                                       sizeof(qcmap_nas_indication_register_req_msg_v01),
                                       &qcmap_nas_indication_register_resp_msg_v01,
                                       sizeof(qcmap_nas_indication_register_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
        if(qmi_error != QMI_NO_ERR)
        {
          LOG_MSG_ERROR("Failed to deregister from NAS indications %d", qmi_error,0,0);
          return;
        }

      }
      else if(evt == QCMAP_CM_IND_V4_WAN_DISCONNECTED)
      {
        LOG_MSG_INFO1("Received event IND_V4_WAN_DISCONNECTED in state LAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Ignore event IND_V4_WAN_CONNECTED in state LAN_CONNECTED",0,0,0);
      }
    }
    else if (state == QCMAP_CM_LAN_DISCONNECTING)
    {
      if(evt == DSI_EVT_NET_IS_CONN)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_IS_CONN in state LAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_IS_CONN in state LAN_DISCONNECTING",0,0,0);
      }
      else if (evt == DSI_EVT_NET_NO_NET)
      {
        qcmap_cm_lock();
        qcmap_cm_cb.state = QCMAP_CM_ENABLE;
        qcmap_cm_unlock();
        event = QCMAP_CM_EVENT_LAN_DISCONNECTED;
        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv4 LAN Disconnected",
                      qcmap_cm_cb.handle, 0, 0);
        callback = TRUE;
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in state LAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Transition from LAN_DISCONNECTING to ENABLE",0,0,0);
        if(qcmap_cm_cb.disable_in_process)
        {
          qcmap_cm_disable(qcmap_cm_cb.handle, qcmap_cm_errno);
        }
      }
      else if(evt == QCMAP_CM_IND_V4_WAN_CONNECTED)
      {
        LOG_MSG_INFO1("Received event IND_V4_WAN_CONNECTED in LAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event IND_V4_WAN_CONNECTED in state LAN_DISCONNECTING",0,0,0);
      }
      else if(evt == QCMAP_CM_IND_V4_WAN_DISCONNECTED)
      {
        LOG_MSG_INFO1("Received event IND_V4_WAN_DISCONNECTED in LAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event IND_V4_WAN_DISCONNECTED in state LAN_DISCONNECTING",0,0,0);
      }
    }
    else if (state == QCMAP_CM_WAN_CONNECTING)
    {
      if (evt == DSI_EVT_NET_IS_CONN)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_IS_CONN in WAN_CONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_IS_CONN in state WAN_CONNECTING",0,0,0);
      }      
      else if (evt == DSI_EVT_NET_NO_NET)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in WAN_CONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_NO_NET in state WAN_CONNECTING",0,0,0);
      }
      else if (evt == QCMAP_CM_IND_V4_WAN_CONNECTED)
      {
        qcmap_cm_lock();
        qcmap_cm_cb.state = QCMAP_CM_WAN_CONNECTED;
        qcmap_cm_unlock();
        event = QCMAP_CM_EVENT_WAN_CONNECTED;
        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv4 Backhaul Connected",
                      qcmap_cm_cb.handle, 0, 0);
        callback = TRUE;
        LOG_MSG_INFO1("Received event IND_V4_WAN_CONNECTED in WAN_CONNECTING",0,0,0);
        LOG_MSG_INFO1("Transition from WAN_CONNECTING to WAN_CONNECTED",0,0,0);

        /*deregister from serving system NAS indication to save power*/
        memset(&qcmap_nas_indication_register_req_msg_v01, 0, sizeof(nas_indication_register_req_msg_v01));

        qcmap_nas_indication_register_req_msg_v01.req_serving_system_valid = TRUE;
        qcmap_nas_indication_register_req_msg_v01.req_serving_system = 0x00;

        LOG_MSG_INFO1("Deregistering from serving system NAS indications",0,0,0);

        qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_nas_handle,
                                       QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01,
                                       &qcmap_nas_indication_register_req_msg_v01,
                                       sizeof(qcmap_nas_indication_register_req_msg_v01),
                                       &qcmap_nas_indication_register_resp_msg_v01,
                                       sizeof(qcmap_nas_indication_register_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
        if(qmi_error != QMI_NO_ERR)
        {
          LOG_MSG_ERROR("Failed to deregister from NAS indications %d", qmi_error,0,0);
          return;
        }

      }
      else if (evt == QCMAP_CM_IND_V4_WAN_DISCONNECTED)
      {
          qcmap_cm_lock();
          qcmap_cm_cb.state = QCMAP_CM_LAN_CONNECTED;
          qcmap_cm_unlock();
          event = QCMAP_CM_EVENT_WAN_CONNECTING_FAIL;
          LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv4 Backhaul Connecting Fail",
                      qcmap_cm_cb.handle, 0, 0);
          callback = TRUE;
          LOG_MSG_INFO1("Received event IND_V4_WAN_DISCONNECTED in WAN_CONNECTING",0,0,0);
          LOG_MSG_INFO1("Transition from WAN_CONNECTING to LAN_CONNECTED",0,0,0);

          if(qcmap_cm_cb.disable_in_process)
          {
            qcmap_cm_disable(qcmap_cm_cb.handle, qcmap_cm_errno);
          }
          else if (qcmap_cm_cb.cfg->wan_config.auto_connect)
          {
            /*register for serving system NAS indication*/
            memset(&qcmap_nas_indication_register_req_msg_v01, 0,
                         sizeof(nas_indication_register_req_msg_v01));
            qcmap_nas_indication_register_req_msg_v01.req_serving_system_valid=
                                                                          TRUE;
            qcmap_nas_indication_register_req_msg_v01.req_serving_system = 0x01;

            LOG_MSG_INFO1("Registering for serving system NAS indications",0,0,0);

            qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_nas_handle,
                           QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01,
                           &qcmap_nas_indication_register_req_msg_v01,
                           sizeof(qcmap_nas_indication_register_req_msg_v01),
                           &qcmap_nas_indication_register_resp_msg_v01,
                           sizeof(qcmap_nas_indication_register_resp_msg_v01),
                           QCMAP_CM_QMI_TIMEOUT_VALUE);
            if(qmi_error != QMI_NO_ERR)
            {
              LOG_MSG_ERROR("Failed to register to NAS indications %d",
                                                            qmi_error,0,0);
              return;
            }

            LOG_MSG_INFO1("Start autoconnect",0,0,0);
            /* Set backhual service status to false. We don't know the reason
               for the disconnect. Try to reconnect the backhaul. */
            qcmap_cm_cb.backhaul_service = FALSE;
            qcmap_cm_connect_backhaul(qcmap_cm_cb.handle, qcmap_cm_errno);
          }
      }
    }
    else if (state == QCMAP_CM_WAN_CONNECTED)
    {
      if(evt == DSI_EVT_NET_IS_CONN)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_IS_CONN in WAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_IS_CONN in state WAN_CONNECTED",0,0,0);
      }
      else if(evt == DSI_EVT_NET_NO_NET)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in WAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_NO_NET in state WAN_CONNECTED",0,0,0);
      }
      else if (evt == QCMAP_CM_IND_V4_WAN_CONNECTED)
      {
        LOG_MSG_INFO1("Received event QCMAP_CM_IND_V4_WAN_CONNECTED in WAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Ignore event QCMAP_CM_IND_V4_WAN_CONNECTED in state WAN_CONNECTED",0,0,0);
      }
      else if (evt == QCMAP_CM_IND_V4_WAN_DISCONNECTED)
      {
        qcmap_cm_lock();
        qcmap_cm_cb.state = QCMAP_CM_LAN_CONNECTED;
        qcmap_cm_unlock();
        event = QCMAP_CM_EVENT_WAN_DISCONNECTED;
        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv4 Backhaul Disconnected",
                      qcmap_cm_cb.handle, 0, 0);
        callback = TRUE;
        LOG_MSG_INFO1("Received event IND_V4_WAN_DISCONNECTED in WAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Transition from WAN_CONNECTED to LAN_CONNECTED",0,0,0);

        if(qcmap_cm_cb.disable_in_process)
        {
          qcmap_cm_disable(qcmap_cm_cb.handle, qcmap_cm_errno);
        }
        else if (qcmap_cm_cb.cfg->wan_config.auto_connect)
        {
          /*register for serving system NAS indication*/
          memset(&qcmap_nas_indication_register_req_msg_v01, 0, sizeof(nas_indication_register_req_msg_v01));
          qcmap_nas_indication_register_req_msg_v01.req_serving_system_valid = TRUE;
          qcmap_nas_indication_register_req_msg_v01.req_serving_system = 0x01;

          LOG_MSG_INFO1("Registering for serving system NAS indications",0,0,0);

          qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_nas_handle,
                                         QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01,
                                         &qcmap_nas_indication_register_req_msg_v01,
                                         sizeof(qcmap_nas_indication_register_req_msg_v01),
                                         &qcmap_nas_indication_register_resp_msg_v01,
                                         sizeof(qcmap_nas_indication_register_resp_msg_v01),
                                         QCMAP_CM_QMI_TIMEOUT_VALUE);
          if(qmi_error != QMI_NO_ERR)
          {
            LOG_MSG_ERROR("Failed to register to NAS indications %d", qmi_error,0,0);
            return;
          }

          LOG_MSG_INFO1("Start autoconnect",0,0,0);
          /* Set backhual service status to false. We don't know the reason
             for the disconnect. Try to reconnect the backhaul. */
          qcmap_cm_cb.backhaul_service = FALSE;
          qcmap_cm_connect_backhaul(qcmap_cm_cb.handle, qcmap_cm_errno);
        }
      }
    }
    else if (state == QCMAP_CM_WAN_DISCONNECTING)
    {
      if(evt == DSI_EVT_NET_IS_CONN)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_IS_CONN in WAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_IS_CONN in WAN_DISCONNECTING",0,0,0);
      }
      else if(evt == DSI_EVT_NET_NO_NET)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in WAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_NO_NET in WAN_DISCONNECTING",0,0,0);
      }
      else if (evt == QCMAP_CM_IND_V4_WAN_CONNECTED)
      {
        LOG_MSG_INFO1("Received event QCMAP_CM_IND_V4_WAN_CONNECTED in WAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event QCMAP_CM_IND_V4_WAN_CONNECTED in WAN_DISCONNECTING",0,0,0);
      }
      else if (evt == QCMAP_CM_IND_V4_WAN_DISCONNECTED)
      {
        qcmap_cm_lock();
        qcmap_cm_cb.state = QCMAP_CM_LAN_CONNECTED;
        qcmap_cm_unlock();
        event = QCMAP_CM_EVENT_WAN_DISCONNECTED;
        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv4 Backhaul Disconnected",
                      qcmap_cm_cb.handle, 0, 0);
        callback = TRUE;
        LOG_MSG_INFO1("Received event IND_V4_WAN_DISCONNECTED in WAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Transition from WAN_DISCONNECTING to LAN_CONNECTED",0,0,0);
          
        if(qcmap_cm_cb.disable_in_process)
        {
          qcmap_cm_disable(qcmap_cm_cb.handle, qcmap_cm_errno);
        }
      }
    }
    else if (state == QCMAP_CM_ENABLE)
    {
      if(evt == DSI_EVT_NET_IS_CONN)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_IS_CONN in ENABLE",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_IS_CONN in ENABLE",0,0,0);
      }
      else if(evt == DSI_EVT_NET_NO_NET)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in ENABLE",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_NO_NET in ENABLE",0,0,0);
      }
      else if (evt == QCMAP_CM_IND_V4_WAN_CONNECTED)
      {
        LOG_MSG_INFO1("Received event QCMAP_CM_IND_V4_WAN_CONNECTED in ENABLE",0,0,0);
        LOG_MSG_INFO1("Ignore event QCMAP_CM_IND_V4_WAN_CONNECTED in ENABLE",0,0,0);
      }
      else if (evt == QCMAP_CM_IND_V4_WAN_DISCONNECTED)
      {
        LOG_MSG_INFO1("Received event QCMAP_CM_IND_V4_WAN_DISCONNECTED in ENABLE",0,0,0);
        LOG_MSG_INFO1("Ignore event QCMAP_CM_IND_V4_WAN_DISCONNECTED in ENABLE",0,0,0);
      }
    }
  }

  if (callback)
    qcmap_cm_cb.qcmap_cm_callback(qcmap_cm_cb.handle, event,
                              qcmap_cm_cb.qcmap_cm_callback_user_data);

  return;
}

 /*
    TODO: This can be removed.
  */
/*===========================================================================
  FUNCTION  qcmap_cm_qmi_qcmap_ind
===========================================================================*/
/*!
@brief
 Processes an incoming QMI QCMAP Indication.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static void
qcmap_cm_qmi_qcmap_ind
(
  qmi_client_type user_handle,                    /* QMI user handle       */
  unsigned int    msg_id,                         /* Indicator message ID  */
  void           *ind_buf,                        /* Raw indication data   */
  unsigned int    ind_buf_len,                    /* Raw data length       */
  void           *ind_cb_data                     /* User call back handle */
)
{
  qcmap_cmdq_cmd_t * cmd_buf = NULL;
  qmi_client_error_type qmi_error;

  LOG_MSG_INFO1("qcmap_cm_qmi_qcmap_ind: user_handle %X msg_id %d ind_buf_len %d.",
                user_handle, msg_id, ind_buf_len);

  switch (msg_id)
  {
    case QMI_QCMAP_WWAN_STATUS_IND_V01:
    {
      qcmap_wwan_status_ind_msg_v01 ind_data;

      qmi_error = qmi_client_message_decode(user_handle,
                                            QMI_IDL_INDICATION,
                                            msg_id,
                                            ind_buf,
                                            ind_buf_len,
                                            &ind_data,
                                            sizeof(qcmap_wwan_status_ind_msg_v01));
      if (qmi_error != QMI_NO_ERR)
      {
        LOG_MSG_ERROR("qcmap_cm_qmi_qcmap_ind: qmi_client_message_decode error %d",
                      qmi_error, 0, 0);
        break;
      }

      if (ind_data.call_end_reason_valid)
      {
        LOG_MSG_INFO1("qcmap_cm_qmi_qcmap_ind call_end_reason %d",
                      ind_data.call_end_reason, 0, 0);
      }

      if (ind_data.verbose_call_end_reason_valid)
      {
        LOG_MSG_INFO1("qcmap_cm_qmi_qcmap_ind verbose call_end_reason %lx",
                      ind_data.verbose_call_end_reason,
                      ind_data.verbose_call_end_reason, 0);
      }

      /* Process packet service status indication for WWAN for QCMAP*/
      if (ind_data.wwan_status == QCMAP_WWAN_CONNECTED_V01)
      {
        if (ind_data.mobile_ap_handle == qcmap_cm_cb.mobile_ap_handle)
        {
          LOG_MSG_INFO1("qcmap_cm_qmi_qcmap_ind: IPv4 WWAN Connected reconfig_required %d",
                        ind_data.reconfig_required, 0, 0);
          cmd_buf = qcmap_cmdq_get_cmd();
          if (cmd_buf == NULL)
          {
            LOG_MSG_ERROR("qcmap_cm_qmi_qcmap_ind : failed in qcmap_cmdq_get_cmd",0,0,0);
            return;
          }
          cmd_buf->data.dsi_nethandle = qcmap_cm_cb.dsi_net_hndl.handle;
          cmd_buf->data.evt = QCMAP_CM_IND_V4_WAN_CONNECTED;
          LOG_MSG_INFO1("qcmap_cmdq: queue commmand",0,0,0);
          /* Post command for processing in the command thread context */
          if( QCMAP_CMDQ_SUCCESS != qcmap_cmdq_put_cmd( cmd_buf ) )
          {
            qcmap_cmdq_release_cmd(cmd_buf);
            LOG_MSG_INFO1("qcmap_cmdq: failed to put commmand",0,0,0);
            return;
          } 
        }
        else if (ind_data.mobile_ap_handle == qcmap_cm_cb.ipv6_mobile_ap_handle)
        {
          LOG_MSG_INFO1("qcmap_cm_qmi_qcmap_ind: IPv6 WWAN Connected reconfig_required %d",
                        ind_data.reconfig_required, 0, 0);
          cmd_buf = qcmap_cmdq_get_cmd();
          if (cmd_buf == NULL)
          {
            LOG_MSG_ERROR("qcmap_cm_qmi_qcmap_ind : failed in qcmap_cmdq_get_cmd",0,0,0);
            return;
          }
          cmd_buf->data.dsi_nethandle = qcmap_cm_cb.ipv6_dsi_net_hndl.handle;
          /*cmd_buf->data.iface_id = DSS_IFACE_ANY;*/
          cmd_buf->data.evt = QCMAP_CM_IND_V6_WAN_CONNECTED;
          LOG_MSG_INFO1("qcmap_cmdq: queue commmand",0,0,0);
          /* Post command for processing in the command thread context */
          if( QCMAP_CMDQ_SUCCESS != qcmap_cmdq_put_cmd( cmd_buf ) ) 
          {
            qcmap_cmdq_release_cmd(cmd_buf);
            LOG_MSG_INFO1("qcmap_cmdq: failed to put commmand",0,0,0);
            return;
          } 
        }
      }
      else if (ind_data.wwan_status == QCMAP_WWAN_DISCONNECTED_V01)
      {
        if (ind_data.mobile_ap_handle == qcmap_cm_cb.mobile_ap_handle)
        {
          LOG_MSG_INFO1("qcmap_cm_qmi_qcmap_ind: IPv4 WWAN Disconnected reconfig_required %d",
                        ind_data.reconfig_required, 0, 0);
          cmd_buf = qcmap_cmdq_get_cmd();
          if (cmd_buf == NULL)
          {
            LOG_MSG_ERROR("qcmap_cm_qmi_qcmap_ind : failed in qcmap_cmdq_get_cmd",0,0,0);
            return;
          }
          cmd_buf->data.dsi_nethandle = qcmap_cm_cb.dsi_net_hndl.handle;
          cmd_buf->data.evt = QCMAP_CM_IND_V4_WAN_DISCONNECTED;
          LOG_MSG_INFO1("qcmap_cmdq: queue commmand",0,0,0);
          /* Post command for processing in the command thread context */
          if( QCMAP_CMDQ_SUCCESS != qcmap_cmdq_put_cmd( cmd_buf ) ) 
          {
            qcmap_cmdq_release_cmd(cmd_buf);
            LOG_MSG_INFO1("qcmap_cmdq: failed to put commmand",0,0,0);
            return;
          } 
        }
        else if (ind_data.mobile_ap_handle == qcmap_cm_cb.ipv6_mobile_ap_handle)
        {
          LOG_MSG_INFO1("qcmap_cm_qmi_qcmap_ind: IPv6 WWAN Disconnected reconfig_required %d",
                        ind_data.reconfig_required, 0, 0);
          cmd_buf = qcmap_cmdq_get_cmd();
          if (cmd_buf == NULL)
          {
            LOG_MSG_ERROR("qcmap_cm_qmi_qcmap_ind : failed in qcmap_cmdq_get_cmd",0,0,0);
            return;
          }
          cmd_buf->data.dsi_nethandle = qcmap_cm_cb.ipv6_dsi_net_hndl.handle;
          cmd_buf->data.evt = QCMAP_CM_IND_V6_WAN_DISCONNECTED;
          LOG_MSG_INFO1("qcmap_cmdq: queue commmand",0,0,0);
          /* Post command for processing in the command thread context */
          if( QCMAP_CMDQ_SUCCESS != qcmap_cmdq_put_cmd( cmd_buf ) ) 
          {
            qcmap_cmdq_release_cmd(cmd_buf);
            LOG_MSG_INFO1("qcmap_cmdq: failed to put commmand",0,0,0);
            return;
          }  
        }
      }
      break;
    }
    default:
      break;
  }
  return;
}

/*===========================================================================
  FUNCTION  qcmap_cm_qmi_nas_ind
===========================================================================*/
/*!
@brief
 Processes an incoming QMI NAS Indication.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static void
qcmap_cm_qmi_nas_ind
(
  qmi_client_type user_handle,                    /* QMI user handle       */
  unsigned int    msg_id,                         /* Indicator message ID  */
  void           *ind_buf,                        /* Raw indication data   */
  unsigned int    ind_buf_len,                    /* Raw data length       */
  void           *ind_cb_data                     /* User call back handle */
)
{
  qmi_client_error_type qmi_error = QMI_NO_ERR;
  nas_serving_system_ind_msg_v01 serving_system_info;
  int               err;
  int              *qcmap_cm_errno = &err;

  LOG_MSG_INFO1("qcmap_cm_qmi_nas_ind: user_handle %d msg_id %d ind_buf_len %d.",
                user_handle,
                msg_id,
                ind_buf_len);

  /* Process based on indication type */
  switch (msg_id)
  {
    case QMI_NAS_SERVING_SYSTEM_IND_MSG_V01:
      qmi_error = qmi_client_message_decode(user_handle,
                                            QMI_IDL_INDICATION,
                                            msg_id,
                                            ind_buf,
                                            ind_buf_len,
                                            &serving_system_info,
                                            sizeof(nas_serving_system_ind_msg_v01));
      if (qmi_error == QMI_NO_ERR)
      {
        LOG_MSG_INFO1("qcmap_cm_qmi_nas_ind: reg_state %s ps_attach_state %d",
                      serving_system_info.serving_system.registration_state,
                      serving_system_info.serving_system.ps_attach_state, 0);

        LOG_MSG_INFO1("qcmap_cm_qmi_nas_ind: roaming_indicator_valid %d roaming_indicator %d",
                      serving_system_info.roaming_indicator_valid,
                      serving_system_info.roaming_indicator, 0);

        if (serving_system_info.roaming_indicator_valid)
        {
          /* The above roaming indicator valid flag is set when CM SS EVENTs
             update roaming info to QMI and there is a change detected in
             roaming_indicator value from the last sent value.
          */
          qcmap_cm_cb.roaming_indicator_valid =
                             serving_system_info.roaming_indicator_valid;
          qcmap_cm_cb.roaming_indicator = serving_system_info.roaming_indicator;
        }

        if ((serving_system_info.serving_system.registration_state == NAS_REGISTERED_V01) &&
            (serving_system_info.serving_system.ps_attach_state == NAS_PS_ATTACHED_V01))
        {
          if (qcmap_cm_cb.cfg->wan_config.auto_connect && qcmap_cm_cb.state == QCMAP_CM_LAN_CONNECTED && 
              !qcmap_cm_cb.disable_in_process)
          {
            LOG_MSG_INFO1("qcmap_cm_qmi_nas_ind: Auto connect start", 0, 0, 0);
            qcmap_cm_cb.backhaul_service = TRUE;
            qcmap_cm_connect_backhaul(qcmap_cm_cb.handle, qcmap_cm_errno);
          }
          if(qcmap_cm_cb.cfg->wan_config.auto_connect && qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_LAN_CONNECTED &&
             !qcmap_cm_cb.disable_in_process)
          {
            LOG_MSG_INFO1("qcmap_cm_qmi_nas_ind: Auto connect start", 0, 0, 0);
            qcmap_cm_cb.backhaul_service = TRUE;
            qcmap_cm_connect_backhaul(qcmap_cm_cb.handle, qcmap_cm_errno);
          }
        }
      }
      else
      {
        LOG_MSG_INFO1("qcmap_cm_qmi_nas_ind: could not decode messgae %d", qmi_error, 0, 0);
      }
      break;
    default:
      /* Ignore all other indications */
      LOG_MSG_ERROR("Got a NAS indication apart from serving system indication. msg_id =%d",msg_id,0,0);
      break;
  }

  return;
}

/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================

FUNCTION QCMAP_CM_ENABLE()

DESCRIPTION

  Enable MobileAP CM based on the config.
  It will register MobileAP event callback.
  It will configure Modem in MobileAP Mode and bring up RmNet between Q6 and A5.
  It will also bring up LAN if it is config.

DEPENDENCIES
  None.

RETURN VALUE
  Returns MobileAP CM application ID on success.

  On error, return 0 and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
int qcmap_cm_enable
(
  qcmap_cm_conf_t *qcmap_cm_cfg,                  /* Config for QCMAP CM   */
  qcmap_cm_cb_fcn  qcmap_cm_callback,             /* Callback function     */
  void            *qcmap_cm_callback_user_data,   /* Callback user data    */
  int             *qcmap_cm_errno                 /* Error condition value */
)
{
  qcmap_cmdq_cmd_t * cmd_buf = NULL;
  char c;
  int qcmap_cm_handle = 0;

  sint15 rval;
  int qmi_err;
  int i,count = 0;
  pthread_mutexattr_t attr;
  qmi_idl_service_object_type qcmap_qmi_idl_service_object;
  qmi_idl_service_object_type nas_qmi_idl_service_object;
  uint32_t num_services = 0, num_entries = 0;
  qmi_service_info info[10];
  qmi_client_error_type qmi_error, qmi_err_code = QMI_NO_ERR;
  qcmap_mobile_ap_enable_req_msg_v01 qcmap_enable_req_msg_v01;
  qcmap_mobile_ap_enable_resp_msg_v01 qcmap_enable_resp_msg_v01;
  qcmap_wwan_status_ind_register_req_msg_v01 wwan_status_ind_register_req_msg_v01;
  qcmap_wwan_status_ind_register_resp_msg_v01 wwan_status_ind_register_resp_msg_v01;
  nas_indication_register_req_msg_v01 qcmap_nas_indication_register_req_msg_v01;
  nas_indication_register_resp_msg_v01 qcmap_nas_indication_register_resp_msg_v01;
  dsi_call_param_value_t param_info, param_info_profile;

  printf("QCMAP Connection Manager: In qcmap_cm_enable \n");

  ds_assert(qcmap_cm_callback != NULL);
  ds_assert(qcmap_cm_cfg != NULL);
  ds_assert(qcmap_cm_errno != NULL);

  if (qcmap_cm_cb.init == FALSE)
  {
    LOG_MSG_INFO1("qcmap_cm_enable: init", 0, 0, 0);

    /* If it is the first time call to QCMAP CM */
    pthread_mutexattr_init(&attr);

    /* Set mutex to be reentry */
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&qcmap_cm_cb.qcmap_cm_mutex, &attr);
    qcmap_cm_cb.init = TRUE;
  }

  qcmap_cm_lock();

  qcmap_cm_cb.cfg = qcmap_cm_cfg;

  if((qcmap_cm_cb.cfg->wan_config.ip_family == QCMAP_IP_V4_V01) && 
       (qcmap_cm_cb.state > QCMAP_CM_DISABLE))
  {
    *qcmap_cm_errno = QCMAP_CM_ENOERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "qcmap_cm_enable : already enabled", 0, 0, 0);
    return qcmap_cm_cb.handle;
  }
  else if((qcmap_cm_cb.cfg->wan_config.ip_family == QCMAP_IP_V4V6_V01) && 
          (qcmap_cm_cb.state > QCMAP_CM_DISABLE) && 
          (qcmap_cm_cb.ipv6_state > QCMAP_CM_V6_DISABLE))
  {
    *qcmap_cm_errno = QCMAP_CM_ENOERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "qcmap_cm_enable : already enabled", 0, 0, 0);
    return qcmap_cm_cb.handle;
  }


  if (qcmap_cm_cb.disable_in_process)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "qcmap_cm_enable fail because disable still in progress", 0, 0, 0);
    return qcmap_cm_handle;
  }

  qcmap_cm_cb.qcmap_cm_callback = qcmap_cm_callback;
  qcmap_cm_cb.qcmap_cm_callback_user_data = qcmap_cm_callback_user_data;

  if (qcmap_cm_cb.ipv6_dsi_net_hndl.handle != NULL)
  {
    dsi_rel_data_srvc_hndl(qcmap_cm_cb.ipv6_dsi_net_hndl.handle);
    qcmap_cm_cb.ipv6_dsi_net_hndl.handle = NULL;
    memset(&qcmap_cm_cb.ipv6_dsi_net_hndl, 0x0, sizeof(qcmap_cm_cb.ipv6_dsi_net_hndl));
  }

  if ( qcmap_cm_cb.dsi_net_hndl.handle != NULL )
  {
    dsi_rel_data_srvc_hndl(qcmap_cm_cb.dsi_net_hndl.handle);
    qcmap_cm_cb.dsi_net_hndl.handle = NULL;
    memset(&qcmap_cm_cb.dsi_net_hndl, 0x0, sizeof(qcmap_cm_cb.dsi_net_hndl));
  }

  memset( &qcmap_cm_cb.dsi_net_hndl, 0x0, sizeof(qcmap_cm_cb.dsi_net_hndl) );
  memset( &qcmap_cm_cb.ipv6_dsi_net_hndl, 0x0, sizeof(qcmap_cm_cb.ipv6_dsi_net_hndl) );

  param_info_profile.buf_val = NULL;
  {
    qcmap_cm_cb.dsi_net_hndl.handle = dsi_get_data_srvc_hndl(qcmap_cm_process_dsi_net_evt, (void*) &qcmap_cm_cb.dsi_net_hndl);

    if (qcmap_cm_cb.dsi_net_hndl.handle == NULL)
    {
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      qcmap_cm_unlock();
      LOG_MSG_INFO1("qcmap_cm_enable: Can not get dsi net handle",0,0,0);
      printf("QCMAP Connection Manager: In qcmap_cm_enable %d \n", __LINE__);
      return qcmap_cm_handle;
    }

    param_info_profile.buf_val = NULL;
  switch (qcmap_cm_cb.cfg->wan_config.tech)
  {
    case QCMAP_WAN_TECH_3GPP:
        qcmap_cm_cb.dsi_net_hndl.tech = "UMTS";
        qcmap_cm_cb.dsi_net_hndl.profile = qcmap_cm_cb.cfg->wan_config.profile_id.v4.umts_profile_index;;
        param_info.num_val = DSI_RADIO_TECH_UMTS;
        param_info_profile.num_val = qcmap_cm_cb.cfg->wan_config.profile_id.v4.umts_profile_index;;
        dsi_set_data_call_param(qcmap_cm_cb.dsi_net_hndl.handle, DSI_CALL_INFO_UMTS_PROFILE_IDX, &param_info_profile);
      break;
    case QCMAP_WAN_TECH_3GPP2:
        qcmap_cm_cb.dsi_net_hndl.tech = "CDMA";
        qcmap_cm_cb.dsi_net_hndl.profile =  qcmap_cm_cb.cfg->wan_config.profile_id.v4.cdma_profile_index;
        param_info.num_val = DSI_RADIO_TECH_CDMA;
        param_info_profile.num_val = qcmap_cm_cb.cfg->wan_config.profile_id.v4.cdma_profile_index;
        dsi_set_data_call_param(qcmap_cm_cb.dsi_net_hndl.handle, DSI_CALL_INFO_CDMA_PROFILE_IDX, &param_info_profile);
      break;
    case QCMAP_WAN_TECH_ANY:
    default:
        qcmap_cm_cb.dsi_net_hndl.tech = "AUTOMATIC";
        param_info.num_val = DSI_RADIO_TECH_UNKNOWN;

        param_info_profile.num_val = qcmap_cm_cb.cfg->wan_config.profile_id.v4.umts_profile_index;;
        dsi_set_data_call_param(qcmap_cm_cb.dsi_net_hndl.handle, DSI_CALL_INFO_UMTS_PROFILE_IDX, &param_info_profile);

        param_info_profile.num_val = qcmap_cm_cb.cfg->wan_config.profile_id.v4.cdma_profile_index;
        dsi_set_data_call_param(qcmap_cm_cb.dsi_net_hndl.handle, DSI_CALL_INFO_CDMA_PROFILE_IDX, &param_info_profile);

      break;
  }

    /* set data call param */
    param_info.buf_val = NULL;

    dsi_set_data_call_param(qcmap_cm_cb.dsi_net_hndl.handle, DSI_CALL_INFO_TECH_PREF, &param_info);

    /* For DSI_NETCTRL, see if we need to set APN in case of UMTS/LTE and Auth profile in case of CDMA */

    qcmap_cm_cb.dsi_net_hndl.family = DSI_IP_FAMILY_4;
    param_info.buf_val = NULL;
    param_info.num_val = DSI_IP_VERSION_4;
    dsi_set_data_call_param(qcmap_cm_cb.dsi_net_hndl.handle, DSI_CALL_INFO_IP_VERSION, &param_info);

  }



    if (qcmap_cm_cb.cfg->wan_config.ip_family == QCMAP_IP_V4V6_V01)
  {
    qcmap_cm_cb.ipv6_dsi_net_hndl.handle = dsi_get_data_srvc_hndl(qcmap_cm_process_dsi_net_evt, (void*) &qcmap_cm_cb.ipv6_dsi_net_hndl);
    if (qcmap_cm_cb.ipv6_dsi_net_hndl.handle == NULL)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    qcmap_cm_unlock();
    LOG_MSG_INFO1("qcmap_cm_enable: Can not get ipv6 dsi net handle",0,0,0);
    printf("QCMAP Connection Manager: In qcmap_cm_enable %d \n", __LINE__);
    return qcmap_cm_handle;
  }

    switch (qcmap_cm_cb.cfg->wan_config.tech)
    {
      case QCMAP_WAN_TECH_3GPP:
        qcmap_cm_cb.ipv6_dsi_net_hndl.tech = "UMTS";
        qcmap_cm_cb.ipv6_dsi_net_hndl.profile = qcmap_cm_cb.cfg->wan_config.profile_id.v6.umts_profile_index;
        param_info.num_val = DSI_RADIO_TECH_UMTS;
        param_info_profile.num_val = qcmap_cm_cb.cfg->wan_config.profile_id.v6.umts_profile_index;
        dsi_set_data_call_param(qcmap_cm_cb.ipv6_dsi_net_hndl.handle, DSI_CALL_INFO_UMTS_PROFILE_IDX, &param_info_profile);
        break;
      case QCMAP_WAN_TECH_3GPP2:
        qcmap_cm_cb.ipv6_dsi_net_hndl.tech = "CDMA";
        qcmap_cm_cb.ipv6_dsi_net_hndl.profile =  qcmap_cm_cb.cfg->wan_config.profile_id.v6.cdma_profile_index;
        param_info.num_val = DSI_RADIO_TECH_CDMA;
        param_info_profile.num_val = qcmap_cm_cb.cfg->wan_config.profile_id.v6.cdma_profile_index;
        dsi_set_data_call_param(qcmap_cm_cb.ipv6_dsi_net_hndl.handle, DSI_CALL_INFO_CDMA_PROFILE_IDX, &param_info_profile);
        break;
      case QCMAP_WAN_TECH_ANY:
      default:
        qcmap_cm_cb.ipv6_dsi_net_hndl.tech = "AUTOMATIC";
        param_info.num_val = DSI_RADIO_TECH_UNKNOWN;
        param_info_profile.num_val = qcmap_cm_cb.cfg->wan_config.profile_id.v6.umts_profile_index;
        dsi_set_data_call_param(qcmap_cm_cb.ipv6_dsi_net_hndl.handle, DSI_CALL_INFO_UMTS_PROFILE_IDX, &param_info_profile);

        param_info_profile.num_val = qcmap_cm_cb.cfg->wan_config.profile_id.v6.cdma_profile_index;
        dsi_set_data_call_param(qcmap_cm_cb.ipv6_dsi_net_hndl.handle, DSI_CALL_INFO_CDMA_PROFILE_IDX, &param_info_profile);

        break;
    }

    /* set data call param */
    param_info.buf_val = NULL;

    dsi_set_data_call_param(qcmap_cm_cb.ipv6_dsi_net_hndl.handle, DSI_CALL_INFO_TECH_PREF, &param_info);

    /* For DSI_NETCTRL, see if we need to set APN in case of UMTS/LTE and Auth profile in case of CDMA */

    qcmap_cm_cb.ipv6_dsi_net_hndl.family = DSI_IP_FAMILY_6;
    param_info.buf_val = NULL;
    param_info.num_val = DSI_IP_VERSION_6;
    dsi_set_data_call_param(qcmap_cm_cb.ipv6_dsi_net_hndl.handle, DSI_CALL_INFO_IP_VERSION, &param_info);

    LOG_MSG_INFO1("qcmap_cm_enable: ipv6_dsi_net_handle %X", qcmap_cm_cb.ipv6_dsi_net_hndl.handle,0,0);

  }

  qcmap_qmi_idl_service_object = qcmap_get_service_object_v01();
  if (qcmap_qmi_idl_service_object == NULL)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    qcmap_cm_unlock();
    LOG_MSG_ERROR("qcpmap service object(qcmap) not available.", 0, 0, 0);
    return qcmap_cm_handle;
  }

  LOG_MSG_INFO1("qcmap_cm_enable: qcmap_get_service_object_v01", 0, 0, 0);

  qmi_error = qmi_client_notifier_init(qcmap_qmi_idl_service_object,
                                       &qcmap_cm_cb.qmi_qcmap_os_params,
                                       &qcmap_cm_cb.qmi_qcmap_notifier);

  LOG_MSG_INFO1("qcmap_cm_enable: qmi_client_notifier_init(qcmap): %d", qmi_error, 0, 0);

  if (qmi_error < 0)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    qcmap_cm_unlock();
    LOG_MSG_ERROR("qmi_client_notifier_init(qcmap) returned %d",
                  qmi_error, 0, 0);
    return qcmap_cm_handle;
  }

  /* Check if the service is up, if not wait on a signal */
  while(1)
  {
    qmi_error = qmi_client_get_service_list(qcmap_qmi_idl_service_object,
                                            NULL,
                                            NULL,
                                            &num_services);

    LOG_MSG_INFO1("qcmap_cm_enable: qmi_client_get_service_list: %d",
                  qmi_error, 0, 0);

    if(qmi_error == QMI_NO_ERR)
      break;
    /* wait for server to come up */
    QMI_CCI_OS_SIGNAL_WAIT(&qcmap_cm_cb.qmi_qcmap_os_params, 0);
  }

  num_entries = num_services;

  LOG_MSG_INFO1("qcmap_cm_enable: qmi_client_get_service_list: num_e %d num_s %d",
                num_entries, num_services, 0);
  /* The server has come up, store the information in info variable */
  qmi_error = qmi_client_get_service_list(qcmap_qmi_idl_service_object,
                                          info,
                                          &num_entries,
                                          &num_services);

  LOG_MSG_INFO1("qcmap_cm_enable: qmi_client_get_service_list: num_e %d num_s %d error %d",
                num_entries, num_services, qmi_error);

  if (qmi_error != QMI_NO_ERR)
  {
    qmi_client_release(qcmap_cm_cb.qmi_qcmap_notifier);
    qcmap_cm_cb.qmi_qcmap_notifier = NULL;
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Can not get qcmap service list %d",
                   qmi_error, 0, 0);
    return qcmap_cm_handle;
  }

  qmi_error = qmi_client_init(&info[0],
                              qcmap_qmi_idl_service_object,
                              qcmap_cm_qmi_qcmap_ind,
                              NULL,
                              NULL,
                              &qcmap_cm_cb.qmi_qcmap_handle);

  LOG_MSG_INFO1("qcmap_cm_enable: qmi_client_init: %d",
                qmi_error, 0, 0);

  if (qmi_error != QMI_NO_ERR)
  {
    qmi_client_release(qcmap_cm_cb.qmi_qcmap_notifier);
    qcmap_cm_cb.qmi_qcmap_notifier = NULL;
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Can not init qcmap client %d",
                   qmi_error, 0, 0);
    return qcmap_cm_handle;
  }

  LOG_MSG_INFO1("Releasing QMI QCMAP notifier ",0,0,0);
  qmi_error = qmi_client_release(qcmap_cm_cb.qmi_qcmap_notifier);
  qcmap_cm_cb.qmi_qcmap_notifier = NULL;
  if (qmi_error != QMI_NO_ERR)
  {
    LOG_MSG_ERROR( "Can not release client qcmap notifier %d",
                 qmi_error, 0, 0);
  }

  nas_qmi_idl_service_object = nas_get_service_object_v01();
  if (nas_qmi_idl_service_object == NULL)
  {
    qmi_client_release(qcmap_cm_cb.qmi_qcmap_handle);
    qcmap_cm_cb.qmi_qcmap_handle = NULL;
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    qcmap_cm_unlock();
    LOG_MSG_ERROR("qcpmap service object(nas) not available.",
                  0, 0, 0);
    return qcmap_cm_handle;
  }

  LOG_MSG_INFO1("qcmap_cm_enable: nas_get_service_object_v01",
                0, 0, 0);

  qmi_error = qmi_client_notifier_init(nas_qmi_idl_service_object,
                                       &qcmap_cm_cb.qmi_nas_os_params,
                                       &qcmap_cm_cb.qmi_nas_notifier);

  LOG_MSG_INFO1("qcmap_cm_enable: qmi_client_notifier_init(nas): %d",
                qmi_error, 0, 0);

  if (qmi_error < 0)
  {
    qmi_client_release(qcmap_cm_cb.qmi_qcmap_handle);
    qcmap_cm_cb.qmi_qcmap_handle = NULL;
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    qcmap_cm_unlock();
    LOG_MSG_ERROR("qmi_client_notifier_init(nas) returned %d",
                  qmi_error, 0, 0);
    return qcmap_cm_handle;
  }

  /* Check if the service is up, if not wait on a signal */
  while(1)
  {
    qmi_error = qmi_client_get_service_list(nas_qmi_idl_service_object,
                                            NULL,
                                            NULL,
                                            &num_services);
    LOG_MSG_INFO1("qcmap_cm_enable: qmi_client_get_service_list: %d", qmi_error, 0, 0);
    if(qmi_error == QMI_NO_ERR)
      break;
    /* wait for server to come up */
    QMI_CCI_OS_SIGNAL_WAIT(&qcmap_cm_cb.qmi_nas_os_params, 0);
  }

  num_entries = num_services;

  LOG_MSG_INFO1("qcmap_cm_enable: qmi_client_get_service_list: num_e %d num_s %d",
                num_entries, num_services, 0);

  /* The server has come up, store the information in info variable */
  qmi_error = qmi_client_get_service_list(nas_qmi_idl_service_object,
                                          info,
                                          &num_entries,
                                          &num_services);

  LOG_MSG_INFO1("qcmap_cm_enable: qmi_client_get_service_list: num_e %d num_s %d error %d",
                num_entries, num_services, qmi_error);

  if (qmi_error != QMI_NO_ERR)
  {
    qmi_client_release(qcmap_cm_cb.qmi_nas_notifier);
    qcmap_cm_cb.qmi_nas_notifier = NULL;
    qmi_client_release(qcmap_cm_cb.qmi_qcmap_handle);
    qcmap_cm_cb.qmi_qcmap_handle = NULL;
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    qcmap_cm_unlock();
    LOG_MSG_ERROR("Can not get nas service list %d", qmi_error, 0, 0);
    return qcmap_cm_handle;
  }

  LOG_MSG_INFO1("qcmap_cm_enable: calling qmi_client_init",
                0, 0, 0);

  qmi_error = qmi_client_init(&info[0],
                              nas_qmi_idl_service_object,
                              qcmap_cm_qmi_nas_ind,
                              NULL,
                              NULL,
                              &qcmap_cm_cb.qmi_nas_handle);

  LOG_MSG_INFO1("qcmap_cm_enable: qmi_client_init: %d",
                qmi_error, 0, 0);

  if (qmi_error != QMI_NO_ERR)
  {
    qmi_client_release(qcmap_cm_cb.qmi_nas_notifier);
    qcmap_cm_cb.qmi_nas_notifier = NULL;
    qmi_client_release(qcmap_cm_cb.qmi_qcmap_handle);
    qcmap_cm_cb.qmi_qcmap_handle = NULL;
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Can not init nas client %d",
                   qmi_error, 0, 0);
    return qcmap_cm_handle;
  }

  /*deregister from all NAS indications*/
  memset(&qcmap_nas_indication_register_req_msg_v01, 0, sizeof(nas_indication_register_req_msg_v01));

  qcmap_nas_indication_register_req_msg_v01.dual_standby_pref_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.dual_standby_pref = 0x00;
  qcmap_nas_indication_register_req_msg_v01.err_rate_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.err_rate = 0x00;
  qcmap_nas_indication_register_req_msg_v01.network_reject_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.network_reject.reg_network_reject = 0x00;
  qcmap_nas_indication_register_req_msg_v01.reg_csp_plmn_mode_bit_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.reg_csp_plmn_mode_bit =0x00;
  qcmap_nas_indication_register_req_msg_v01.reg_current_plmn_name_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.reg_current_plmn_name = 0x00;
  qcmap_nas_indication_register_req_msg_v01.reg_ddtm_events_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.reg_ddtm_events = 0x00;
  qcmap_nas_indication_register_req_msg_v01.reg_embms_status_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.reg_embms_status = 0x00;
  qcmap_nas_indication_register_req_msg_v01.reg_hdr_session_close_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.reg_hdr_session_close = 0x00;
  qcmap_nas_indication_register_req_msg_v01.reg_hdr_uati_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.reg_hdr_uati = 0x00;
  qcmap_nas_indication_register_req_msg_v01.reg_managed_roaming_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.reg_managed_roaming = 0x00;
  qcmap_nas_indication_register_req_msg_v01.reg_network_time_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.reg_network_time = 0x00;
  qcmap_nas_indication_register_req_msg_v01.reg_operator_name_data_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.reg_operator_name_data = 0x00;
  qcmap_nas_indication_register_req_msg_v01.reg_rf_band_info_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.reg_rf_band_info = 0x00;
  qcmap_nas_indication_register_req_msg_v01.reg_rtre_cfg_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.reg_rtre_cfg = 0x00;
  qcmap_nas_indication_register_req_msg_v01.reg_sys_sel_pref_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.reg_sys_sel_pref = 0x00;
  qcmap_nas_indication_register_req_msg_v01.sig_info_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.sig_info = 0x00;
  qcmap_nas_indication_register_req_msg_v01.subscription_info_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.subscription_info = 0x00;
  qcmap_nas_indication_register_req_msg_v01.sys_info_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.sys_info = 0x00;

  /*register for serving system NAS indication if autoconnect is enabled*/
  if(qcmap_cm_cb.cfg->wan_config.auto_connect)
  {
    LOG_MSG_INFO1("Registering for serving system NAS indications",0,0,0);

     qcmap_nas_indication_register_req_msg_v01.req_serving_system_valid = TRUE;
     qcmap_nas_indication_register_req_msg_v01.req_serving_system = 0x01;     
   }
   else
   {
     LOG_MSG_INFO1("Deregistering from serving system NAS indications",0,0,0);

     qcmap_nas_indication_register_req_msg_v01.req_serving_system_valid = TRUE;
     qcmap_nas_indication_register_req_msg_v01.req_serving_system = 0x00;
   }
   qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_nas_handle,
                                         QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01,
                                         &qcmap_nas_indication_register_req_msg_v01,
                                         sizeof(qcmap_nas_indication_register_req_msg_v01),
                                         &qcmap_nas_indication_register_resp_msg_v01,
                                         sizeof(qcmap_nas_indication_register_resp_msg_v01),
                                         QCMAP_CM_QMI_TIMEOUT_VALUE);
   if (qmi_error != QMI_NO_ERR)
   {
     qmi_client_release(qcmap_cm_cb.qmi_nas_notifier);
     qcmap_cm_cb.qmi_nas_notifier = NULL;
     qmi_client_release(qcmap_cm_cb.qmi_nas_handle);
     qcmap_cm_cb.qmi_nas_handle = NULL;
     qmi_client_release(qcmap_cm_cb.qmi_qcmap_handle);
     qcmap_cm_cb.qmi_qcmap_handle = NULL;
     *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
     qcmap_cm_unlock();
     LOG_MSG_ERROR( "Can not perform NAS indication register %d",
                   qmi_error, 0, 0);
     return qcmap_cm_handle;
   }

  LOG_MSG_INFO1("Releasing QMI NAS notifier",0,0,0);
  qmi_error = qmi_client_release(qcmap_cm_cb.qmi_nas_notifier);
  qcmap_cm_cb.qmi_nas_notifier = NULL;

   if (qmi_error != QMI_NO_ERR)
   {
     LOG_MSG_ERROR("Can not release client nas notifier %d",
                 qmi_error, 0, 0);
   }
  /*Set NAT Type*/
  qcmap_enable_req_msg_v01.qcmap_nat_type_info =
                          qcmap_cm_cb.cfg->nat_config.nat_type;
  qcmap_enable_req_msg_v01.qcmap_nat_type_info_valid = TRUE;

  /* Pouplate MobileAP Param */
  qcmap_enable_req_msg_v01.ip_addr_info.nat_dns_addr = qcmap_cm_cb.cfg->lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].a5_ip_addr;
  qcmap_enable_req_msg_v01.ip_addr_info.subnet_mask = qcmap_cm_cb.cfg->lan_config.interface[QCMAP_CM_INTF_PRIMARY_INDEX].sub_net_mask;

  /* Populate SSID2 info if enabled. */
  if ( qcmap_cm_cb.cfg->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].enable == TRUE &&
       qcmap_cm_cb.cfg->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].devmode == QCMAP_CM_DEVMODE_AP &&
       qcmap_cm_cb.cfg->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].access_profile == QCMAP_CM_PROFILE_FULL_ACCESS )
  {
    qcmap_enable_req_msg_v01.ssid2_ip_addr_info.addr =
            qcmap_cm_cb.cfg->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].a5_ip_addr;
    qcmap_enable_req_msg_v01.ssid2_ip_addr_info.subnet_mask =
            qcmap_cm_cb.cfg->lan_config.interface[QCMAP_CM_INTF_SSID2_INDEX].sub_net_mask;
    qcmap_enable_req_msg_v01.ssid2_ip_addr_info_valid = TRUE;
  }
  else
  {
    qcmap_enable_req_msg_v01.ssid2_ip_addr_info_valid = FALSE;
  }

  qcmap_enable_req_msg_v01.ip_addr_info.nat_ip_addr = qcmap_cm_cb.cfg->lan_config.nat_ip_addr;
  qcmap_enable_req_msg_v01.ip_addr_info.apps_rmnet_gateway_addr = qcmap_cm_cb.cfg->lan_config.q6_ip_addr_facing_a5;
  qcmap_enable_req_msg_v01.ip_addr_info.apps_rmnet_ip_addr = qcmap_cm_cb.cfg->lan_config.a5_rmnet_ip_addr;

  qcmap_enable_req_msg_v01.ip_addr_info.usb_rmnet_gateway_addr = qcmap_cm_cb.cfg->lan_config.q6_ip_addr_facing_usb_rmnet;
  qcmap_enable_req_msg_v01.ip_addr_info.usb_rmnet_ip_addr = qcmap_cm_cb.cfg->lan_config.usb_rmnet_ip_addr;

  qcmap_enable_req_msg_v01.ip_addr_info_valid = TRUE;

  qcmap_enable_req_msg_v01.dun_client_ip_addr_valid = TRUE;
  qcmap_enable_req_msg_v01.dun_client_ip_addr =
                              qcmap_cm_cb.cfg->lan_config.dun_client_ip_addr;

  qcmap_enable_req_msg_v01.ip_family = QCMAP_IP_V4_V01;
  qcmap_enable_req_msg_v01.net_policy_info.profile_id_3gpp =
       qcmap_cm_cb.cfg->wan_config.profile_id.v4.umts_profile_index;
  qcmap_enable_req_msg_v01.net_policy_info.profile_id_3gpp2 =
       qcmap_cm_cb.cfg->wan_config.profile_id.v4.cdma_profile_index;
  qcmap_enable_req_msg_v01.net_policy_info.tech_pref = qcmap_cm_cb.cfg->wan_config.tech;

  qcmap_enable_req_msg_v01.net_policy_info_valid = TRUE;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_MOBILE_AP_ENABLE_REQ_V01,
                                       (void*)&qcmap_enable_req_msg_v01,
                                       sizeof(qcmap_enable_req_msg_v01),
                                       (void*)&qcmap_enable_resp_msg_v01,
                                       sizeof(qcmap_enable_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);

  LOG_MSG_INFO1("qcmap_cm_enable: qmi_client_send_msg_sync(enable): error %d result %d valid %d",
                qmi_error, qcmap_enable_resp_msg_v01.resp.result, qcmap_enable_resp_msg_v01.mobile_ap_handle_valid);

  if ((qmi_error != QMI_NO_ERR) ||
      (qcmap_enable_resp_msg_v01.resp.result != QMI_NO_ERR) ||
      (qcmap_enable_resp_msg_v01.mobile_ap_handle_valid != TRUE))
  {
    qmi_client_release(qcmap_cm_cb.qmi_nas_handle);
    qcmap_cm_cb.qmi_nas_handle = NULL;
    qmi_client_release(qcmap_cm_cb.qmi_qcmap_handle);
    qcmap_cm_cb.qmi_qcmap_handle = NULL;
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Can not enable qcmap %d : %d",
                   qmi_error, qcmap_enable_resp_msg_v01.resp.error, 0);
    return qcmap_cm_handle;
  }

  qcmap_cm_cb.mobile_ap_handle = qcmap_enable_resp_msg_v01.mobile_ap_handle;

  if (qcmap_cm_cb.ipv6_dsi_net_hndl.handle != NULL)
  {
    qcmap_enable_req_msg_v01.ip_family = QCMAP_IP_V6_V01;
    qcmap_enable_req_msg_v01.ip_addr_info_valid = FALSE;

    qcmap_enable_req_msg_v01.net_policy_info.profile_id_3gpp =
                    qcmap_cm_cb.cfg->wan_config.profile_id.v6.umts_profile_index;
    qcmap_enable_req_msg_v01.net_policy_info.profile_id_3gpp2 =
                    qcmap_cm_cb.cfg->wan_config.profile_id.v6.cdma_profile_index;

    qcmap_enable_req_msg_v01.net_policy_info_valid = TRUE;

    qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_MOBILE_AP_ENABLE_REQ_V01,
                                       (void*)&qcmap_enable_req_msg_v01,
                                       sizeof(qcmap_enable_req_msg_v01),
                                       (void*)&qcmap_enable_resp_msg_v01,
                                       sizeof(qcmap_enable_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);


    LOG_MSG_INFO1("qcmap_cm_enable: V6 qmi_client_send_msg_sync(enable): error %d result %d valid %d",
                  qmi_error, qcmap_enable_resp_msg_v01.resp.result,
                  qcmap_enable_resp_msg_v01.mobile_ap_handle_valid);

    if ((qmi_error != QMI_NO_ERR) ||
        (qcmap_enable_resp_msg_v01.resp.result != QMI_NO_ERR) ||
        (qcmap_enable_resp_msg_v01.mobile_ap_handle_valid != TRUE))
    {
      qmi_client_release(qcmap_cm_cb.qmi_nas_handle);
      qcmap_cm_cb.qmi_nas_handle = NULL;
      qmi_client_release(qcmap_cm_cb.qmi_qcmap_handle);
      qcmap_cm_cb.qmi_qcmap_handle = NULL;
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      qcmap_cm_unlock();
      LOG_MSG_ERROR( "Can not enable qcmap V6 %d : %d",
                     qmi_error, qcmap_enable_resp_msg_v01.resp.error, 0);
      return qcmap_cm_handle;
    }
    else
    {
      qcmap_cm_cb.ipv6_mobile_ap_handle = qcmap_enable_resp_msg_v01.mobile_ap_handle;
    }
  }

  qcmap_cm_cb.state = QCMAP_CM_ENABLE;
  if (qcmap_cm_cb.ipv6_dsi_net_hndl.handle != NULL)
  {
    qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_ENABLE;
  }

  LOG_MSG_INFO1("Enable: STATES V4 %d V6 %d", qcmap_cm_cb.state, qcmap_cm_cb.ipv6_state, 0);
  qcmap_cm_handle = qcmap_cm_cb.handle;

  LOG_MSG_INFO1("QCMAP AP Handle 0x%04x Enabled", qcmap_cm_handle, 0, 0);
  qcmap_cm_cb.qcmap_cm_callback(qcmap_cm_cb.handle,
                                QCMAP_CM_EVENT_ENABLED,
                                qcmap_cm_cb.qcmap_cm_callback_user_data);

  wwan_status_ind_register_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;
  wwan_status_ind_register_req_msg_v01.register_indication = 1;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_WWAN_STATUS_IND_REG_REQ_V01,
                                       (void*)&wwan_status_ind_register_req_msg_v01,
                                       sizeof(wwan_status_ind_register_req_msg_v01),
                                       (void*)&wwan_status_ind_register_resp_msg_v01,
                                       sizeof(wwan_status_ind_register_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);

  LOG_MSG_INFO1("qcmap_cm_enable: qmi_client_send_msg_sync(wwan): error %d result %d",
                qmi_error, wwan_status_ind_register_resp_msg_v01.resp.result, 0);

  if ((qmi_error != QMI_NO_ERR) ||
      (wwan_status_ind_register_resp_msg_v01.resp.result != QMI_NO_ERR))
  {
    LOG_MSG_ERROR("Can not enable wwan status ind %d : %d",
                  qmi_error, qcmap_enable_resp_msg_v01.resp.error, 0);
  }

   if (qcmap_cm_cb.ipv6_dsi_net_hndl.handle != NULL)
  {
    wwan_status_ind_register_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.ipv6_mobile_ap_handle;
    wwan_status_ind_register_req_msg_v01.register_indication = 1;

    qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                         QMI_QCMAP_WWAN_STATUS_IND_REG_REQ_V01,
                                         (void*)&wwan_status_ind_register_req_msg_v01,
                                         sizeof(wwan_status_ind_register_req_msg_v01),
                                         (void*)&wwan_status_ind_register_resp_msg_v01,
                                         sizeof(wwan_status_ind_register_resp_msg_v01),
                                         QCMAP_CM_QMI_TIMEOUT_VALUE);

    LOG_MSG_INFO1("qcmap_cm_enable: IPv6 qmi_client_send_msg_sync(wwan): error %d result %d",
                  qmi_error, wwan_status_ind_register_resp_msg_v01.resp.result, 0);

    if ((qmi_error != QMI_NO_ERR) ||
        (wwan_status_ind_register_resp_msg_v01.resp.result != QMI_NO_ERR))
    {
      LOG_MSG_ERROR("Can not enable IPv6 wwan status ind %d : %d",
                    qmi_error, qcmap_enable_resp_msg_v01.resp.error, 0);
    }
  }

  qcmap_cm_cb.state = QCMAP_CM_LAN_CONNECTING;
   if (qcmap_cm_cb.ipv6_dsi_net_hndl.handle != NULL)
  {
    qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_LAN_CONNECTING;
  }

  LOG_MSG_INFO1("Enable:LAN connecting: STATES V4 %d V6 %d", 
                qcmap_cm_cb.state, qcmap_cm_cb.ipv6_state, 0);
  LOG_MSG_INFO1("QCMAP AP Handle 0x%04x LAN Connecting",
                qcmap_cm_handle, 0, 0);
  qcmap_cm_cb.qcmap_cm_callback(qcmap_cm_cb.handle,
                                QCMAP_CM_EVENT_LAN_CONNECTING,
                                qcmap_cm_cb.qcmap_cm_callback_user_data);

  qcmap_cm_unlock();

  /*-------------------------------------------------------------------
   Connecting LAN. In order to allow DSS init to complete try 3 times in
   3 seconds.
  -------------------------------------------------------------------*/
  while(1 && count < 3)
  {
    rval = dsi_start_data_call(qcmap_cm_cb.dsi_net_hndl.handle);
    if ( rval == DSI_SUCCESS)
      break;
    sleep(1);
    count++;
  }

  if ( rval != DSI_SUCCESS)
  {
    LOG_MSG_INFO1("qcmap_cm_enable: dsi_start_data_call rval %d ",
                  rval, 0, 0);
    cmd_buf = qcmap_cmdq_get_cmd();
    cmd_buf->data.dsi_nethandle = qcmap_cm_cb.dsi_net_hndl.handle;
    cmd_buf->data.evt = DSI_EVT_NET_NO_NET;
    LOG_MSG_INFO1("qcmap_cmdq: queue commmand",0,0,0);
    /* Post command for processing in the command thread context */
    if( QCMAP_CMDQ_SUCCESS != qcmap_cmdq_put_cmd( cmd_buf ) ) 
    {
      qcmap_cmdq_release_cmd(cmd_buf);
      LOG_MSG_INFO1("qcmap_cmdq: failed to put commmand",0,0,0);
      return;
    } 
  }

  /* Connecting IPv6 LAN */
  if (qcmap_cm_cb.ipv6_dsi_net_hndl.handle != NULL)
  {
    rval = dsi_start_data_call(qcmap_cm_cb.ipv6_dsi_net_hndl.handle);

    if ( rval != DSI_SUCCESS)
    {
      LOG_MSG_INFO1("qcmap_cm_enable: dsi_start_data_call ipv6 rval %d ",
                    rval, 0, 0);
      // Fake a NONET
      cmd_buf = qcmap_cmdq_get_cmd(); 
      cmd_buf->data.dsi_nethandle = qcmap_cm_cb.ipv6_dsi_net_hndl.handle;
      cmd_buf->data.evt = DSI_EVT_NET_NO_NET;
      LOG_MSG_INFO1("qcmap_cmdq: queue commmand",0,0,0);
      /* Post command for processing in the command thread context */
      if( QCMAP_CMDQ_SUCCESS != qcmap_cmdq_put_cmd( cmd_buf ) ) 
      {
        qcmap_cmdq_release_cmd(cmd_buf);
        LOG_MSG_INFO1("qcmap_cmdq: failed to put commmand",0,0,0);
        return;
      } 
    }
  }

  return qcmap_cm_handle;
}

/*===========================================================================

FUNCTION QCMAP_CM_DISABLE()

DESCRIPTION

  Disable MobileAP CM.
  It will teardown LAN.
  It will configure Modem in non-MobileAP mode.

DEPENDENCIES
  None.

RETURN VALUE

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EBADAPP           invalid application ID specified

SIDE EFFECTS

===========================================================================*/
int qcmap_cm_disable
(
  int  qcmap_cm_handle,                          /* Handle for MobileAP CM */
  int *qcmap_cm_errno                            /* Error condition value  */
)
{
  int ret = QCMAP_CM_SUCCESS;
  int qmi_err, i, qcmap_cm_err, ret_val;
  qcmap_mobile_ap_disable_req_msg_v01 qcmap_disable_req_msg_v01;
  qcmap_mobile_ap_disable_resp_msg_v01 qcmap_disable_resp_msg_v01;
  qmi_client_error_type qmi_error;

  LOG_MSG_INFO1("qcmap_cm_disable: enter", 0, 0, 0);
  ds_assert(qcmap_cm_errno != NULL);

  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle",
                   0, 0, 0);
    return ret;
  }

  if(qcmap_cm_cb.state == QCMAP_CM_DISABLE &&
     qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_DISABLE)
  {
    *qcmap_cm_errno = QCMAP_CM_ENOERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "qcmap_cm_enable : already disabled", 0, 0, 0);
    return QCMAP_CM_SUCCESS;
  }

  if(qcmap_cm_cb.state > QCMAP_CM_LAN_CONNECTED || qcmap_cm_cb.ipv6_state > QCMAP_CM_V6_LAN_CONNECTED)
  {

    qcmap_cm_cb.disable_in_process = TRUE;
    ret_val = qcmap_cm_disconnect_backhaul(qcmap_cm_cb.handle, qcmap_cm_errno);

    if (ret_val == QCMAP_CM_SUCCESS)
    {
      qcmap_cm_unlock();
      return ret;
    }
    if (ret_val == QCMAP_CM_ERROR && *qcmap_cm_errno == QCMAP_CM_EWOULDBLOCK)
    {
      /* Backhaul disconnecting in progress */
      qcmap_cm_unlock();
      return ret;
    }
    else
    {
      qcmap_cm_unlock();
      return QCMAP_CM_ERROR;
    }
  }
  else if (qcmap_cm_cb.state == QCMAP_CM_LAN_CONNECTED || qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_LAN_CONNECTED)
  {
    qcmap_cm_cb.disable_in_process = TRUE;
    qcmap_cm_cb.state = QCMAP_CM_LAN_DISCONNECTING;
    dsi_stop_data_call(qcmap_cm_cb.dsi_net_hndl.handle);

    if (qcmap_cm_cb.ipv6_dsi_net_hndl.handle != NULL)
    {
      qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_LAN_DISCONNECTING;
      dsi_stop_data_call(qcmap_cm_cb.ipv6_dsi_net_hndl.handle);
    }
    qcmap_cm_unlock();

  }
  else if(qcmap_cm_cb.state <= QCMAP_CM_ENABLE && qcmap_cm_cb.ipv6_state <= QCMAP_CM_V6_ENABLE)
  {
    qcmap_cm_cb.state = QCMAP_CM_DISABLE;
    if(qcmap_cm_cb.ipv6_dsi_net_hndl.handle != NULL)
    {
      qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_DISABLE;
    }
    LOG_MSG_INFO1("Disable: STATES V4 %d V6 %d", qcmap_cm_cb.state, qcmap_cm_cb.ipv6_state, 0);
    qcmap_cm_unlock();

    qmi_error = qcmap_cm_firewall_entry_op_delete_all(&qcmap_cm_err);
    if (qmi_error != QCMAP_CM_SUCCESS)
    {
      LOG_MSG_INFO1("QCMAP CM could not delete all firewall entries. err %d cm_err %d",
                  qmi_error, qcmap_cm_err, 0);
    }

    if (qcmap_cm_cb.ipv6_dsi_net_hndl.handle != NULL)
    {
      /* Call QMI API to disable MobileAP */
      qcmap_disable_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.ipv6_mobile_ap_handle;
      qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                         QMI_QCMAP_MOBILE_AP_DISABLE_REQ_V01,
                                         &qcmap_disable_req_msg_v01,
                                         sizeof(qcmap_disable_req_msg_v01),
                                         &qcmap_disable_resp_msg_v01,
                                         sizeof(qcmap_disable_resp_msg_v01),
                                         QCMAP_CM_QMI_TIMEOUT_VALUE);

      if ((qmi_error != QMI_NO_ERR) ||
          (qcmap_disable_resp_msg_v01.resp.result != QMI_NO_ERR))
      {
        LOG_MSG_ERROR( "Can not disable IPv6 qcmap %d : %d",
                     qmi_error, qcmap_disable_resp_msg_v01.resp.error, 0);
      }
    }
    qcmap_disable_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;
    qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_MOBILE_AP_DISABLE_REQ_V01,
                                       &qcmap_disable_req_msg_v01,
                                       sizeof(qcmap_disable_req_msg_v01),
                                       &qcmap_disable_resp_msg_v01,
                                       sizeof(qcmap_disable_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);

    if ((qmi_error != QMI_NO_ERR) ||
        (qcmap_disable_resp_msg_v01.resp.result != QMI_NO_ERR))
    {
      LOG_MSG_ERROR( "Can not disable IPv4 qcmap %d : %d",
                   qmi_error, qcmap_disable_resp_msg_v01.resp.error, 0);
    }

    qmi_error = qmi_client_release(qcmap_cm_cb.qmi_nas_handle);
    qcmap_cm_cb.qmi_nas_handle = NULL;

    if (qmi_error != QMI_NO_ERR)
    {
      LOG_MSG_ERROR("Can not release client nas handle %d",
                  qmi_error, 0, 0);
    }

    qmi_error = qmi_client_release(qcmap_cm_cb.qmi_qcmap_handle);
    qcmap_cm_cb.qmi_qcmap_handle = NULL;

    if (qmi_error != QMI_NO_ERR)
    {
      LOG_MSG_ERROR( "Can not release client qcmap handle %d",
                   qmi_error, 0, 0);
    }

    qcmap_cm_cb.disable_in_process = FALSE;

    qcmap_cm_cb.qcmap_cm_callback(qcmap_cm_cb.handle,
                                QCMAP_CM_EVENT_DISABLED,
                                qcmap_cm_cb.qcmap_cm_callback_user_data);

    LOG_MSG_INFO1("QCMAP AP Handle 0x%04x Disabled",
                qcmap_cm_handle, 0, 0);

  }
  else
  {
    qcmap_cm_unlock();
  }
  
  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_CONNECT_BACKHAUL()

DESCRIPTION

  It will bringup WWAN.

DEPENDENCIES
  None.

RETURN VALUE
  If WAN is already connected, returns QCMAP_CM_SUCCESS.
  Otherwise, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EBADAPP           invalid application ID specified
  QCMAP_CM_EWOULDBLOCK       the operation would block
  QCMAP_CM_EOPNOTSUPP        backhaul bringup/teardown in progress or LAN is not connected yet


SIDE EFFECTS

===========================================================================*/
int qcmap_cm_connect_backhaul
(
  int  qcmap_cm_handle,                          /* Handle for MobileAP CM */
  int *qcmap_cm_errno                            /* Error condition value  */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qcmap_bring_up_wwan_req_msg_v01 qcmap_bring_up_wwan_req_msg;
  qcmap_bring_up_wwan_resp_msg_v01 qcmap_bring_up_wwan_resp_msg;
  qmi_client_error_type qmi_error;

  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_connect_backhaul: enter",
                0, 0, 0);

  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR("Wrong QCMAP CM Handle",
                  0, 0, 0);
    return ret;
  }

  if((qcmap_cm_cb.cfg->wan_config.ip_family == QCMAP_IP_V4_V01) &&
       (qcmap_cm_cb.state == QCMAP_CM_WAN_CONNECTED))
  {
    *qcmap_cm_errno = QCMAP_CM_ENOERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "qcmap_cm_enable : already enabled", 0, 0, 0);
    return qcmap_cm_cb.handle;
  }
  else if((qcmap_cm_cb.cfg->wan_config.ip_family == QCMAP_IP_V4V6_V01) &&
          (qcmap_cm_cb.state == QCMAP_CM_WAN_CONNECTED) &&
          (qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_WAN_CONNECTED))
  {
    *qcmap_cm_errno = QCMAP_CM_ENOERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "QCMAP CM Backhaul already connected", 0, 0, 0);
    return qcmap_cm_cb.handle;
  }

  if (qcmap_cm_cb.state < QCMAP_CM_LAN_CONNECTED &&
      qcmap_cm_cb.ipv6_state < QCMAP_CM_V6_LAN_CONNECTED)
  {
    /* We can connect WAN only after LAN is connected */
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR("QCMAP CM LAN is not connected",
                  0, 0, 0);
    return ret;
  }

  if(qcmap_cm_cb.cfg->wan_config.ip_family == QCMAP_IP_V4_V01)
  {
    if (qcmap_cm_cb.state == QCMAP_CM_WAN_CONNECTING ||
        qcmap_cm_cb.state == QCMAP_CM_WAN_DISCONNECTING)
    {
      /* We have some outstanding WAN request */
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      qcmap_cm_unlock();
      LOG_MSG_ERROR("QCMAP CM has outstanding backhaul request",
                    0, 0, 0);
      return ret;
    }
  }
  else if(qcmap_cm_cb.cfg->wan_config.ip_family == QCMAP_IP_V6_V01)
  {
    if (qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_WAN_CONNECTING ||
        qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_WAN_DISCONNECTING)
    {
      /* We have some outstanding WAN request */
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      qcmap_cm_unlock();
      LOG_MSG_ERROR("QCMAP CM has outstanding backhaul request",
                    0, 0, 0);
      return ret;
    }
  }
  else if(qcmap_cm_cb.cfg->wan_config.ip_family == QCMAP_IP_V4V6_V01)
  {
    if ((qcmap_cm_cb.state == QCMAP_CM_WAN_CONNECTING ||
        qcmap_cm_cb.state == QCMAP_CM_WAN_DISCONNECTING)&&
        (qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_WAN_CONNECTING ||
         qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_WAN_DISCONNECTING))
    {
      /* We have some outstanding WAN request */
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      qcmap_cm_unlock();
      LOG_MSG_ERROR("QCMAP CM has outstanding backhaul request",
                    0, 0, 0);
      return ret;
    }
  }

  if(qcmap_cm_cb.cfg->nat_config.tethering_only && 
     (!qcmap_cm_cb.cfg->lan_config.qti_link_up))
  {
    /* We are in tethering only mode and QTI LINK is down.
       In this QCMAP does not bringup backhaul */
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR("QTI LINK DOWN state. Cannot bring up WWAN",
                  0, 0, 0);
    return ret;
  }

  // check whether had backhaul service
  if (!qcmap_cm_cb.backhaul_service)
  {
    nas_get_serving_system_resp_msg_v01 get_serving_system_resp_msg_v01;
    qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_nas_handle,
                                         QMI_NAS_GET_SERVING_SYSTEM_REQ_MSG_V01,
                                         NULL,
                                         0,
                                         &get_serving_system_resp_msg_v01,
                                         sizeof(get_serving_system_resp_msg_v01),
                                         QCMAP_CM_QMI_TIMEOUT_VALUE);
    if (qmi_error != QMI_NO_ERR)
    {
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      qcmap_cm_unlock();
      LOG_MSG_ERROR( "QCMAP CM nas get serving system fail %d",
                     qmi_error, 0, 0);
      return ret;
    }
    else
    {
      if ((get_serving_system_resp_msg_v01.serving_system.registration_state != NAS_REGISTERED_V01) ||
          (get_serving_system_resp_msg_v01.serving_system.ps_attach_state != NAS_PS_ATTACHED_V01))
      {
        *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
        ret = QCMAP_CM_ERROR;
        qcmap_cm_unlock();
        LOG_MSG_ERROR("QCMAP CM has no backhaul service",
                      0, 0, 0);
        return ret;
      }
    }
    LOG_MSG_INFO1("qcmap_cm_connect_backhaul: roaming_indicator_valid %d roaming_indicator %d", get_serving_system_resp_msg_v01.roaming_indicator_valid, get_serving_system_resp_msg_v01.roaming_indicator, 0);
    /* QCMAP is on an active network. */
    qcmap_cm_cb.backhaul_service = TRUE;

    /* If the roaming indicator data is valid... */
    if (get_serving_system_resp_msg_v01.roaming_indicator_valid)
    {
      /* ...update the gobal roaming_indicator validity flag and value. */
      qcmap_cm_cb.roaming_indicator_valid =
              get_serving_system_resp_msg_v01.roaming_indicator_valid;
      qcmap_cm_cb.roaming_indicator = get_serving_system_resp_msg_v01.roaming_indicator;
    }

    /* If roaming is on... */
    if ((qcmap_cm_cb.roaming_indicator_valid) &&
        (qcmap_cm_cb.roaming_indicator == NAS_ROAMING_IND_ON_V01))
    {
      /* ...read the ERI config file for valid roam_status values. */
      if ((ret = qcmap_cm_eri_read_config(qcmap_cm_handle, qcmap_cm_cb.cfg->wan_config.eri_config_file, qcmap_cm_errno))
          != QCMAP_CM_SUCCESS)
      {
        LOG_MSG_ERROR("qcmap_cm_connect_backhaul: qcmap_cm_eri_read_config: qcmap_cm_errno %d", qcmap_cm_errno, 0, 0);
      }
    }
  }

  /* Check to see whether we can connect backhaul during roam. */
  /* If we're connected to a network...*/
  if (qcmap_cm_cb.backhaul_service)
  {
    /* If the config indicates roaming data calls not allowed and we're
       connected to a roaming network...*/
    if ((!qcmap_cm_cb.cfg->wan_config.roaming) &&
        (qcmap_cm_cb.roaming_indicator_valid) &&
        (qcmap_cm_cb.roaming_indicator == NAS_ROAMING_IND_ON_V01))
    {
      LOG_MSG_ERROR("qcmap_cm_connect_backhaul: qcmap_cm_cb.eri_roam_data_len %d", qcmap_cm_cb.eri_roam_data_len, 0, 0)
      ret = QCMAP_CM_ERROR;
      /* If ERI roam data, check if roaming status matches eri data. */
      if (qcmap_cm_cb.eri_roam_data_len > 0)
      {
        /* Call QMI NAS service to get system info. */
        nas_get_sys_info_resp_msg_v01 get_sys_info_resp_msg;
        nas_roam_status_enum_type_v01 roam_status = NAS_ROAM_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;
        uint8 i = qcmap_cm_cb.eri_roam_data_len;

        qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_nas_handle,
                                             QMI_NAS_GET_SYS_INFO_REQ_MSG_V01,
                                             NULL,
                                             0,
                                             &get_sys_info_resp_msg,
                                             sizeof(get_sys_info_resp_msg),
                                             QCMAP_CM_QMI_TIMEOUT_VALUE);
        /* If the call returns system info...*/
        if ((qmi_error == QMI_NO_ERR) &&
            (get_sys_info_resp_msg.resp.result == QMI_NO_ERR))
        {
          /* If the LTE roaming status is valid, check the LTE roam status. */
          if ((get_sys_info_resp_msg.lte_sys_info_valid == TRUE) &&
              (get_sys_info_resp_msg.lte_sys_info.common_sys_info.roam_status_valid == TRUE))
          {
            roam_status = get_sys_info_resp_msg.lte_sys_info.common_sys_info.roam_status;
            LOG_MSG_ERROR("qcmap_cm_connect_backhaul: lte roam status %d", roam_status, 0, 0)
          }
          /* If the HDR roaming status is valid, check the HDR roam status. */
          else if ((get_sys_info_resp_msg.hdr_sys_info_valid == TRUE) &&
                   (get_sys_info_resp_msg.hdr_sys_info.common_sys_info.roam_status_valid == TRUE))
          {
            roam_status = get_sys_info_resp_msg.hdr_sys_info.common_sys_info.roam_status;
            LOG_MSG_ERROR("qcmap_cm_connect_backhaul: hdr roam status %d", roam_status, 0, 0)
          }
          /* If the CDMA roaming status is valid, check the CDMA roam status. */
          else if ((get_sys_info_resp_msg.cdma_sys_info_valid == TRUE) &&
                   (get_sys_info_resp_msg.cdma_sys_info.common_sys_info.roam_status_valid == TRUE))
          {
            roam_status = get_sys_info_resp_msg.cdma_sys_info.common_sys_info.roam_status;
            LOG_MSG_ERROR("qcmap_cm_connect_backhaul: cdma roam status %d", roam_status, 0, 0)
          }
          /* If the WCDMA roaming status is valid, check the WCDMA roam status. */
          else if ((get_sys_info_resp_msg.wcdma_sys_info_valid == TRUE) &&
                   (get_sys_info_resp_msg.wcdma_sys_info.common_sys_info.roam_status_valid == TRUE))
          {
            roam_status = get_sys_info_resp_msg.wcdma_sys_info.common_sys_info.roam_status;
            LOG_MSG_ERROR("qcmap_cm_connect_backhaul: wcdma roam status %d", roam_status, 0, 0)
          }
          /* If the GSM roaming status is valid, check the GSM roam status. */
          else if ((get_sys_info_resp_msg.gsm_sys_info_valid == TRUE) &&
                   (get_sys_info_resp_msg.gsm_sys_info.common_sys_info.roam_status_valid == TRUE))
          {
            roam_status = get_sys_info_resp_msg.gsm_sys_info.common_sys_info.roam_status;
            LOG_MSG_ERROR("qcmap_cm_connect_backhaul: gsm roam status %d", roam_status, 0, 0)
          }
          /* If the TDSCDMA roaming status is valid, check the TDSCDMA roam status. */
          else if ((get_sys_info_resp_msg.tdscdma_sys_info_valid == TRUE) &&
                   (get_sys_info_resp_msg.tdscdma_sys_info.common_sys_info.roam_status_valid == TRUE))
          {
            roam_status = get_sys_info_resp_msg.tdscdma_sys_info.common_sys_info.roam_status;
            LOG_MSG_ERROR("qcmap_cm_connect_backhaul: tdscdma roam status %d", roam_status, 0, 0)
          }
          /* If a valid roam status was found...*/
          if (roam_status != NAS_ROAM_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01)
          {
            /* Look to see if the current roaming status matched an eri data entry. */
            for (i = 0; i < qcmap_cm_cb.eri_roam_data_len; i++ )
            {
              if (get_sys_info_resp_msg.cdma_sys_info.common_sys_info.roam_status ==
                  qcmap_cm_cb.eri_roam_data[i])
              {
                /* We have a match, this is a romaing exception. */
                ret = QCMAP_CM_SUCCESS;
                break;
              }
            }
          }
          else
          {
            LOG_MSG_ERROR("qcmap_cm_connect_backhaul: no valid roam status", 0, 0, 0);
          }
        }
        else
        {
          LOG_MSG_ERROR("qcmap_cm_connect_backhaul: qmi_error %d get_sys_info_resp_msg.resp.error %d", qmi_error, get_sys_info_resp_msg.resp.error, 0);
        }
      }
      /* If a roaming exception was not found, error out. */
      if (ret == QCMAP_CM_ERROR)
      {
        *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
        qcmap_cm_unlock();
        LOG_MSG_ERROR("QCMAP CM can not connect backhaul while it is roaming",
                      0, 0, 0);
        return ret;
      }
    }
  }

  *qcmap_cm_errno = QCMAP_CM_EWOULDBLOCK;
  ret = QCMAP_CM_ERROR;

  if(qcmap_cm_cb.state == QCMAP_CM_LAN_CONNECTED)
  {
    /* Bring up the data call. */
    LOG_MSG_INFO1("Bring up IPV4 wwan",0,0,0);
    qcmap_bring_up_wwan_req_msg.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;

    qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_BRING_UP_WWAN_REQ_V01,
                                       &qcmap_bring_up_wwan_req_msg,
                                       sizeof(qcmap_bring_up_wwan_req_msg),
                                       &qcmap_bring_up_wwan_resp_msg,
                                       sizeof(qcmap_bring_up_wwan_resp_msg),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);

    if ((qmi_error != QMI_NO_ERR) ||
      (qcmap_bring_up_wwan_resp_msg.resp.result != QMI_NO_ERR))
    {
      LOG_MSG_ERROR("Can not bring up wwan qcmap %d : %d",
                  qmi_error, qcmap_bring_up_wwan_resp_msg.resp.error, 0);
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    }
    else
    {
      qcmap_cm_cb.state = QCMAP_CM_WAN_CONNECTING;
    }
  }
  else
  {
    LOG_MSG_ERROR("Cannot bring up IPV4 wwan because of invalid state %d",
                  qcmap_cm_cb.state,0,0);
  }

  if (qcmap_cm_cb.ipv6_dsi_net_hndl.handle != NULL)
  {
    if(qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_LAN_CONNECTED)
    {
      LOG_MSG_INFO1("Bring up IPV6 wwan",0,0,0);

      qcmap_bring_up_wwan_req_msg.mobile_ap_handle = qcmap_cm_cb.ipv6_mobile_ap_handle;

      qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                         QMI_QCMAP_BRING_UP_WWAN_REQ_V01,
                                         &qcmap_bring_up_wwan_req_msg,
                                         sizeof(qcmap_bring_up_wwan_req_msg),
                                         &qcmap_bring_up_wwan_resp_msg,
                                         sizeof(qcmap_bring_up_wwan_resp_msg),
                                         QCMAP_CM_QMI_TIMEOUT_VALUE);

      if ((qmi_error != QMI_NO_ERR) ||
          (qcmap_bring_up_wwan_resp_msg.resp.result != QMI_NO_ERR))
      {
        LOG_MSG_ERROR("Can not bring up IPv6 wwan qcmap %d : %d",
                      qmi_error, qcmap_bring_up_wwan_resp_msg.resp.error, 0);
        /* Return EWOULDBLOCK to make sure that we start DNS even if V6 call fails 
           and V4 call succeeds*/
        *qcmap_cm_errno = QCMAP_CM_EWOULDBLOCK;
      }
      else
      {
        qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_WAN_CONNECTING;
      }
    } 
    else
    {
      LOG_MSG_ERROR("Cannot bring up IPV6 wwan because of invalid state %d",
                  qcmap_cm_cb.ipv6_state,0,0);
    }   
  }
  

  qcmap_cm_unlock();

  LOG_MSG_INFO1("QCMAP AP Handle 0x%04x Connecting Backhaul.",
                qcmap_cm_handle, 0, 0);

  return ret;
}

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
int qcmap_cm_disconnect_backhaul
(
  int  qcmap_cm_handle,                          /* Handle for MobileAP CM */
  int *qcmap_cm_errno                            /* Error condition value  */
)
{
  int qmi_err;
  int ret = QCMAP_CM_SUCCESS;
  qcmap_tear_down_wwan_req_msg_v01 qcmap_tear_down_wwan_req_msg;
  qcmap_tear_down_wwan_resp_msg_v01 qcmap_tear_down_wwan_resp_msg;
  qmi_client_error_type qmi_error;

  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_disconnect_backhaul: enter",
                0, 0, 0);

  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR("Wrong QCMAP CM Handle",
                  0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state <= QCMAP_CM_LAN_CONNECTED &&
      qcmap_cm_cb.ipv6_state <= QCMAP_CM_V6_LAN_CONNECTED)
  {
    /* WAN is not connected */
    ret = QCMAP_CM_SUCCESS;
    *qcmap_cm_errno = QCMAP_CM_ENOERROR;
    qcmap_cm_unlock();
    LOG_MSG_INFO1("QCMAP CM WAN is not connected",
                  0, 0, 0);
    return ret;
  }

  if ((qcmap_cm_cb.state == QCMAP_CM_WAN_CONNECTING ||
      qcmap_cm_cb.state == QCMAP_CM_WAN_DISCONNECTING)&&
      (qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_WAN_CONNECTING ||
       qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_WAN_DISCONNECTING))
  {
    /* We can some outstanding WAN request */
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR("QCMAP CM has outstanding backhaul request",
                  0, 0, 0);
    return ret;
  }
  if(qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_WAN_CONNECTED)
  {
    if (qcmap_cm_cb.ipv6_dsi_net_hndl.handle != NULL)
    {
      LOG_MSG_INFO1("Bring down IPv6 WAN",0,0,0);
      qcmap_tear_down_wwan_req_msg.mobile_ap_handle = qcmap_cm_cb.ipv6_mobile_ap_handle;

      qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                         QMI_QCMAP_TEAR_DOWN_WWAN_REQ_V01,
                                         &qcmap_tear_down_wwan_req_msg,
                                         sizeof(qcmap_tear_down_wwan_req_msg),
                                         &qcmap_tear_down_wwan_resp_msg,
                                         sizeof(qcmap_tear_down_wwan_resp_msg),
                                         QCMAP_CM_QMI_TIMEOUT_VALUE);

      if (( qmi_error != QMI_TIMEOUT_ERR ) && ((qmi_error != QMI_NO_ERR) ||
          (qcmap_tear_down_wwan_resp_msg.resp.result != QMI_NO_ERR)))
      {
        LOG_MSG_ERROR( "Can not tear down wwan qcmap %d : %d", qmi_error,
                     qcmap_tear_down_wwan_resp_msg.resp.error, 0);
        *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
        ret = QCMAP_CM_ERROR;
        qcmap_cm_unlock();
        return ret;
      }

      qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_WAN_DISCONNECTING;
    }
  }
  else
  {
    LOG_MSG_ERROR("Cannot bring down IPV6 wwan because of invalid state %d",
                  qcmap_cm_cb.ipv6_state, 0, 0);
  }

  if(qcmap_cm_cb.state == QCMAP_CM_WAN_CONNECTED)
  {
    LOG_MSG_INFO1("Bring down IPv4 wwan",0,0,0);
    qcmap_tear_down_wwan_req_msg.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;

    qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_TEAR_DOWN_WWAN_REQ_V01,
                                       &qcmap_tear_down_wwan_req_msg,
                                       sizeof(qcmap_tear_down_wwan_req_msg),
                                       &qcmap_tear_down_wwan_resp_msg,
                                       sizeof(qcmap_tear_down_wwan_resp_msg),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);

    if (( qmi_error != QMI_TIMEOUT_ERR ) && ((qmi_error != QMI_NO_ERR) ||
        (qcmap_tear_down_wwan_resp_msg.resp.result != QMI_NO_ERR)))
    {
      LOG_MSG_ERROR( "Can not tear down wwan qcmap %d : %d",
                     qmi_error, qcmap_tear_down_wwan_resp_msg.resp.error, 0);
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      qcmap_cm_unlock();
      return ret;
    }

    qcmap_cm_cb.state = QCMAP_CM_WAN_DISCONNECTING;
  }
  else
  {
    LOG_MSG_ERROR("Cannot bring down IPv4 wwan due to invalid state %d",
                  qcmap_cm_cb.state, 0, 0);
  }

  *qcmap_cm_errno = QCMAP_CM_EWOULDBLOCK;
  ret = QCMAP_CM_ERROR;


  qcmap_cm_unlock();
  LOG_MSG_INFO1("QCMAP AP Handle 0x%04x tear down backhaul.",
                qcmap_cm_handle, 0, 0);

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_STATIC_NAT_OP_ADD()

DESCRIPTION

  Adds a static NAT entry to the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the static NAT entry could be added, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The entry could not be added.


SIDE EFFECTS

===========================================================================*/
static int
qcmap_cm_static_nat_op_add
(
  qcmap_cm_port_fwding_entry_conf_t *snat_entry,        /* SNAT Entry      */
  int                               *qcmap_cm_errno     /* Error conditiom */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error = QMI_NO_ERR;
  qcmap_add_static_nat_entry_req_msg_v01 add_static_nat_entry_req_msg_v01;
  qcmap_add_static_nat_entry_resp_msg_v01 add_static_nat_entry_resp_msg_v01;

  add_static_nat_entry_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;
  add_static_nat_entry_req_msg_v01.snat_entry_config.global_port = snat_entry->port_fwding_global_port;
  add_static_nat_entry_req_msg_v01.snat_entry_config.private_ip_addr = snat_entry->port_fwding_private_ip;
  add_static_nat_entry_req_msg_v01.snat_entry_config.private_port = snat_entry->port_fwding_private_port;
  add_static_nat_entry_req_msg_v01.snat_entry_config.protocol = snat_entry->port_fwding_protocol;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_ADD_STATIC_NAT_ENTRY_REQ_V01,
                                       &add_static_nat_entry_req_msg_v01,
                                       sizeof(add_static_nat_entry_req_msg_v01),
                                       &add_static_nat_entry_resp_msg_v01,
                                       sizeof(add_static_nat_entry_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
  if ((qmi_error != QMI_NO_ERR) ||
      (add_static_nat_entry_resp_msg_v01.resp.result != QMI_NO_ERR))
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR("Add static nat failed %d %d",
                  qmi_error, add_static_nat_entry_resp_msg_v01.resp.error, 0);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_STATIC_NAT_OP_GET()

DESCRIPTION

  Returns the static NAT entries in the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the static NAT entries could be retrieved, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The entries could not be retrieved.


SIDE EFFECTS

===========================================================================*/
static int
qcmap_cm_static_nat_op_get
(
  qcmap_cm_port_fwding_entry_conf_t *snat_entry,        /* SNAT Entries    */
  int                               *qcmap_cm_errno     /* Error condition */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_get_static_nat_entries_req_msg_v01 get_static_nat_entries_req_msg_v01;
  qcmap_get_static_nat_entries_resp_msg_v01 get_static_nat_entries_resp_msg_v01;

  get_static_nat_entries_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_GET_STATIC_NAT_ENTRIES_REQ_V01,
                                       &get_static_nat_entries_req_msg_v01,
                                       sizeof(get_static_nat_entries_req_msg_v01),
                                       &get_static_nat_entries_resp_msg_v01,
                                       sizeof(get_static_nat_entries_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);

  if ((qmi_error == QMI_NO_ERR) &&
      (get_static_nat_entries_resp_msg_v01.resp.result == QMI_NO_ERR) &&
      (get_static_nat_entries_resp_msg_v01.snat_config_valid == TRUE))
  {
    LOG_MSG_INFO1("Get static nat entries %d",
                  get_static_nat_entries_resp_msg_v01.snat_config_len, 0, 0);

    if (get_static_nat_entries_resp_msg_v01.snat_config_len > 0)
    {
      int i = get_static_nat_entries_resp_msg_v01.snat_config_len;
      while (i--)
      {
        snat_entry[i].port_fwding_private_ip   = get_static_nat_entries_resp_msg_v01.snat_config[i].private_ip_addr;
        snat_entry[i].port_fwding_private_port = get_static_nat_entries_resp_msg_v01.snat_config[i].private_port;
        snat_entry[i].port_fwding_global_port  = get_static_nat_entries_resp_msg_v01.snat_config[i].global_port;
        snat_entry[i].port_fwding_protocol     = get_static_nat_entries_resp_msg_v01.snat_config[i].protocol;
      }
    }
    ret = get_static_nat_entries_resp_msg_v01.snat_config_len;
  }
  else
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR("Get static nat failed %d %d",
                  qmi_error, get_static_nat_entries_resp_msg_v01.resp.error, 0);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_STATIC_NAT_OP_DELETE()

DESCRIPTION

  Deletes a static NAT entry in the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the static NAT entry could be deleted, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The entry could not be deleted.


SIDE EFFECTS

===========================================================================*/
static int
qcmap_cm_static_nat_op_delete
(
  qcmap_cm_port_fwding_entry_conf_t *snat_entry,        /* SNAT Entry      */
  int                               *qcmap_cm_errno     /* Error condition */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_delete_static_nat_entry_req_msg_v01 delete_static_nat_entry_req_msg_v01;
  qcmap_delete_static_nat_entry_resp_msg_v01 delete_static_nat_entry_resp_msg_v01;

  delete_static_nat_entry_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;
  delete_static_nat_entry_req_msg_v01.snat_entry_config.global_port = snat_entry->port_fwding_global_port;
  delete_static_nat_entry_req_msg_v01.snat_entry_config.private_ip_addr = snat_entry->port_fwding_private_ip;
  delete_static_nat_entry_req_msg_v01.snat_entry_config.private_port = snat_entry->port_fwding_private_port;
  delete_static_nat_entry_req_msg_v01.snat_entry_config.protocol = snat_entry->port_fwding_protocol;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_DELETE_STATIC_NAT_ENTRY_REQ_V01,
                                       &delete_static_nat_entry_req_msg_v01,
                                       sizeof(delete_static_nat_entry_req_msg_v01),
                                       &delete_static_nat_entry_resp_msg_v01,
                                       sizeof(delete_static_nat_entry_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
  if ((qmi_error != QMI_NO_ERR) ||
      (delete_static_nat_entry_resp_msg_v01.resp.result != QMI_NO_ERR))
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Delete static nat failed %d %d",
                   qmi_error, delete_static_nat_entry_resp_msg_v01.resp.error, 0);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_STATIC_NAT_OP()

DESCRIPTION
  This function adds/deletes/gets static NAT entries.

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
int qcmap_cm_static_nat_op
(
  int    qcmap_cm_handle,                           /* Handle for MobileAP CM  */
  qcmap_cm_snat_req_e snat_op_type,                 /* Add/Delete/Get SNAT   */
  qcmap_cm_port_fwding_entry_conf_t *snat_entry,    /* SNAT Entry            */
  int    *qcmap_cm_errno                         /* Error condition value  */
)
{
  int ret = QCMAP_CM_SUCCESS;

  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_static_nat_op: enter",
                0, 0, 0);
  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle",
                   0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state == QCMAP_CM_DISABLE)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "QCMAP CM disable state", 0, 0, 0);
    return ret;
  }

  qcmap_cm_unlock();

  switch (snat_op_type)
  {
    case QCMAP_CM_ADD_SNAT:
      ret = qcmap_cm_static_nat_op_add(snat_entry, qcmap_cm_errno);
      break;
    case QCMAP_CM_GET_SNAT:
      ret = qcmap_cm_static_nat_op_get(snat_entry, qcmap_cm_errno);
      break;
    case QCMAP_CM_DELETE_SNAT:
      ret = qcmap_cm_static_nat_op_delete(snat_entry, qcmap_cm_errno);
      break;
    default:
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      LOG_MSG_ERROR( "Invalid op ty1pe %d",
                     snat_op_type, 0, 0);
      break;
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_FIREWALL_ENTRY_OP_ADD()

DESCRIPTION

  Adds a firewall entry to the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the firewall entry could be added, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The entry could not be added.


SIDE EFFECTS

===========================================================================*/
static int
qcmap_cm_firewall_entry_op_add
(
  qcmap_cm_firewall_entry_conf_t *firewall_entry,       /* Firewall Entry  */
  int                            *qcmap_cm_errno        /* Error condition */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_add_firewall_config_req_msg_v01 add_firewall_config_req_msg_v01;
  qcmap_add_firewall_config_resp_msg_v01 add_firewall_config_resp_msg_v01;

  add_firewall_config_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;
  add_firewall_config_req_msg_v01.firewall_config.start_dest_port = firewall_entry->firewall_start_dest_port;
  add_firewall_config_req_msg_v01.firewall_config.end_dest_port = firewall_entry->firewall_end_dest_port;
  add_firewall_config_req_msg_v01.firewall_config.protocol = firewall_entry->firewall_protocol;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_ADD_FIREWALL_CONFIG_REQ_V01,
                                       &add_firewall_config_req_msg_v01,
                                       sizeof(add_firewall_config_req_msg_v01),
                                       &add_firewall_config_resp_msg_v01,
                                       sizeof(add_firewall_config_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
  if ((qmi_error == QMI_NO_ERR) &&
      (add_firewall_config_resp_msg_v01.resp.result == QMI_NO_ERR) &&
      (add_firewall_config_resp_msg_v01.firewall_handle_valid == TRUE))
  {
    firewall_entry->firewall_handle = add_firewall_config_resp_msg_v01.firewall_handle;
  }
  else
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR("Add firewall config failed %d %d",
                  qmi_error, add_firewall_config_resp_msg_v01.resp.error, 0);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_FIREWALL_ENTRY_OP_GET()

DESCRIPTION

  Gets firewall entries from the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the firewall entries could be retrieved, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The entries could not be retrieved.


SIDE EFFECTS

===========================================================================*/
static int
qcmap_cm_firewall_entry_op_get
(
  qcmap_cm_firewall_entry_conf_t *firewall_entry,      /* Firewall Entries */
  int                            *qcmap_cm_errno       /* Error condition  */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_get_firewall_config_req_msg_v01 get_firewall_config_req_msg_v01;
  qcmap_get_firewall_config_resp_msg_v01 get_firewall_config_resp_msg_v01;

  get_firewall_config_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_GET_FIREWALL_CONFIG_REQ_V01,
                                       &get_firewall_config_req_msg_v01,
                                       sizeof(get_firewall_config_req_msg_v01),
                                       &get_firewall_config_resp_msg_v01,
                                       sizeof(get_firewall_config_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);

  if ((qmi_error == QMI_NO_ERR) &&
      (get_firewall_config_resp_msg_v01.resp.result == QMI_NO_ERR) &&
      (get_firewall_config_resp_msg_v01.firewall_config_valid == TRUE))
  {
    if (get_firewall_config_resp_msg_v01.firewall_config_len > 0)
    {
      int i = get_firewall_config_resp_msg_v01.firewall_config_len;
      while (i--)
      {
        firewall_entry[i].firewall_handle = get_firewall_config_resp_msg_v01.firewall_config[i].firewall_handle;
        firewall_entry[i].firewall_start_dest_port = get_firewall_config_resp_msg_v01.firewall_config[i].start_dest_port;
        firewall_entry[i].firewall_end_dest_port = get_firewall_config_resp_msg_v01.firewall_config[i].end_dest_port;
        firewall_entry[i].firewall_protocol = get_firewall_config_resp_msg_v01.firewall_config[i].protocol;
      }
    }
    ret = get_firewall_config_resp_msg_v01.firewall_config_len;
  }
  else
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Get firewall config failed %d %d", qmi_error, get_firewall_config_resp_msg_v01.resp.error, 0);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_FIREWALL_ENTRY_OP_DELETE()

DESCRIPTION

  Deletes a firewall entry in the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the firewall entry could be deleted, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The entry could not be deleted.


SIDE EFFECTS

===========================================================================*/
static int
qcmap_cm_firewall_entry_op_delete
(
  qcmap_cm_firewall_entry_conf_t *firewall_entry,       /* Firewall Entry  */
  int                            *qcmap_cm_errno        /* Error condition */
)
{
  int ret = QCMAP_CM_SUCCESS, i;
  qmi_client_error_type qmi_error;
  qcmap_delete_firewall_config_req_msg_v01 delete_firewall_config_req_msg_v01;
  qcmap_delete_firewall_config_resp_msg_v01 delete_firewall_config_resp_msg_v01;

  /* Search for a matching item to obtain the handle used to reference this
     entry. */
  delete_firewall_config_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;
  delete_firewall_config_req_msg_v01.firewall_handle = firewall_entry->firewall_handle;
  LOG_MSG_INFO1("qcmap_cm_firewall_entry_op_delete handle %08X", delete_firewall_config_req_msg_v01.firewall_handle, 0, 0);
  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_DELETE_FIREWALL_CONFIG_REQ_V01,
                                       &delete_firewall_config_req_msg_v01,
                                       sizeof(delete_firewall_config_req_msg_v01),
                                       &delete_firewall_config_resp_msg_v01,
                                       sizeof(delete_firewall_config_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
  if ((qmi_error != QMI_NO_ERR) ||
      (delete_firewall_config_resp_msg_v01.resp.result != QMI_NO_ERR))
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
      LOG_MSG_ERROR( "Delete firewall entry failed %d %d",
                   qmi_error, delete_firewall_config_resp_msg_v01.resp.error, 0);
  }

  return ret;
}


/*===========================================================================

FUNCTION QCMAP_CM_FIREWALL_ENTRY_OP()

DESCRIPTION
  This Function add/delete/get static firewall entry

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
int qcmap_cm_firewall_entry_op
(
  int    qcmap_cm_handle,                           /* Handle for MobileAP CM  */
  qcmap_cm_firewall_entry_req_e firewall_entry_op_type,
                                                  /* Add/Del/Get Firewall  */
  qcmap_cm_firewall_entry_conf_t *firewall_entry,   /* Firewall Entry        */
  int    *qcmap_cm_errno                            /* error condition value */
)
{
  int ret = QCMAP_CM_SUCCESS;

  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_firewall_entry_op: enter",
                0, 0, 0);
  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR("Wrong QCMAP CM Handle",
                  0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state == QCMAP_CM_DISABLE)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR("QCMAP CM disable state",
                  0, 0, 0);
    return ret;
  }

  qcmap_cm_unlock();

  switch (firewall_entry_op_type)
  {
    case QCMAP_CM_ADD_FIREWALL_ENTRY_RULE:
      ret = qcmap_cm_firewall_entry_op_add(firewall_entry, qcmap_cm_errno);
      break;
    case QCMAP_CM_GET_FIREWALL_ENTRY_RULE:
      ret = qcmap_cm_firewall_entry_op_get(firewall_entry, qcmap_cm_errno);
      break;
    case QCMAP_CM_DELETE_FIREWALL_ENTRY_RULE:
      ret = qcmap_cm_firewall_entry_op_delete(firewall_entry, qcmap_cm_errno);
      break;
    default:
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      LOG_MSG_ERROR("Invalid firewall entry op %d",
                    firewall_entry_op_type, 0, 0);
      break;
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_FIREWALL_OP_ENABLE()

DESCRIPTION

  Enable the firewall in the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the firewall could be enabled, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The firewall could not be enabled.


SIDE EFFECTS

===========================================================================*/
static int
qcmap_cm_firewall_op_enable
(
  uint8 *pkts_allowed,                                 /* Packets allowed? */
  int   *qcmap_cm_errno                                /* Error condition  */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_enable_firewall_setting_req_msg_v01 enable_firewall_setting_req_msg;
  qcmap_enable_firewall_setting_resp_msg_v01 enable_firewall_setting_resp_msg;

  ds_assert(pkts_allowed != NULL);
  ds_assert(qcmap_cm_errno != NULL);

  if(qcmap_cm_cb.mobile_ap_handle != QCMAP_WAN_INVALID_QCMAP_HANDLE)
  {
    enable_firewall_setting_req_msg.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;
    enable_firewall_setting_req_msg.pkts_allowed = *pkts_allowed;

    qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                         QMI_QCMAP_ENABLE_FIREWALL_SETTING_REQ_V01,
                                         &enable_firewall_setting_req_msg,
                                         sizeof(enable_firewall_setting_req_msg),
                                         &enable_firewall_setting_resp_msg,
                                         sizeof(enable_firewall_setting_resp_msg),
                                         QCMAP_CM_QMI_TIMEOUT_VALUE);
    if ((qmi_error != QMI_NO_ERR) ||
        (enable_firewall_setting_resp_msg.resp.result != QMI_NO_ERR))
    {
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      LOG_MSG_ERROR("Enable firewall config failed for IPv4 %d %d",
                    qmi_error, enable_firewall_setting_resp_msg.resp.error, 0);
    }
  }

  if(qcmap_cm_cb.ipv6_mobile_ap_handle != QCMAP_WAN_INVALID_QCMAP_HANDLE)
  {
    enable_firewall_setting_req_msg.mobile_ap_handle = qcmap_cm_cb.ipv6_mobile_ap_handle;
    enable_firewall_setting_req_msg.pkts_allowed = *pkts_allowed;

    qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                         QMI_QCMAP_ENABLE_FIREWALL_SETTING_REQ_V01,
                                         &enable_firewall_setting_req_msg,
                                         sizeof(enable_firewall_setting_req_msg),
                                         &enable_firewall_setting_resp_msg,
                                         sizeof(enable_firewall_setting_resp_msg),
                                         QCMAP_CM_QMI_TIMEOUT_VALUE);
    if ((qmi_error != QMI_NO_ERR) ||
        (enable_firewall_setting_resp_msg.resp.result != QMI_NO_ERR))
    {
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      LOG_MSG_ERROR("Enable firewall config failed for IPv6 %d %d",
                    qmi_error, enable_firewall_setting_resp_msg.resp.error, 0);
    }
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_FIREWALL_OP_GET()

DESCRIPTION

  Get the firewall state from the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the firewall could be retrieved, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The firewall could not be retrieved.


SIDE EFFECTS

===========================================================================*/
static int
qcmap_cm_firewall_op_get
(
  uint8 *pkts_allowed,                                 /* Packets allowed? */
  int   *qcmap_cm_errno                                /* Error condition  */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_get_firewall_setting_req_msg_v01 get_firewall_setting_req_msg;
  qcmap_get_firewall_setting_resp_msg_v01 get_firewall_setting_resp_msg;

  ds_assert(pkts_allowed != NULL);

  get_firewall_setting_req_msg.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_GET_FIREWALL_SETTING_REQ_V01,
                                       &get_firewall_setting_req_msg,
                                       sizeof(get_firewall_setting_req_msg),
                                       &get_firewall_setting_resp_msg,
                                       sizeof(get_firewall_setting_resp_msg),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
  if ((qmi_error == QMI_NO_ERR) &&
      (get_firewall_setting_resp_msg.resp.result == QMI_NO_ERR) &&
      (get_firewall_setting_resp_msg.firewall_enabled_valid == TRUE))
  {
    ret = get_firewall_setting_resp_msg.firewall_enabled;

    if (get_firewall_setting_resp_msg.pkts_allowed_valid)
    {
      *pkts_allowed = get_firewall_setting_resp_msg.pkts_allowed;
    }
  }
  else
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR("Get firewall config failed %d %d",
                  qmi_error, get_firewall_setting_resp_msg.resp.error, 0);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_FIREWALL_OP_DISABLE()

DESCRIPTION

  Disable the firewall in the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the firewall could be disabled, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The firewall could not be disabled.


SIDE EFFECTS

===========================================================================*/
int qcmap_cm_firewall_op_disable
(
  int *qcmap_cm_errno                                   /* Error condition */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_disable_firewall_setting_req_msg_v01 disable_firewall_setting_req_msg;
  qcmap_disable_firewall_setting_resp_msg_v01 disable_firewall_setting_resp_msg;

  ds_assert(qcmap_cm_errno != NULL);

  if(qcmap_cm_cb.mobile_ap_handle != QCMAP_WAN_INVALID_QCMAP_HANDLE)
  {

    disable_firewall_setting_req_msg.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;

    qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                         QMI_QCMAP_DISABLE_FIREWALL_SETTING_REQ_V01,
                                         &disable_firewall_setting_req_msg,
                                         sizeof(disable_firewall_setting_req_msg),
                                         &disable_firewall_setting_resp_msg,
                                         sizeof(disable_firewall_setting_resp_msg),
                                         QCMAP_CM_QMI_TIMEOUT_VALUE);

    if ((qmi_error != QMI_NO_ERR) ||
        (disable_firewall_setting_resp_msg.resp.result != QMI_NO_ERR))
    {
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      LOG_MSG_ERROR("Disable firewall config failed for IPv4 %d %d",
                    qmi_error, disable_firewall_setting_resp_msg.resp.error, 0);
    }

  }

  if(qcmap_cm_cb.ipv6_mobile_ap_handle != QCMAP_WAN_INVALID_QCMAP_HANDLE)
  {
    disable_firewall_setting_req_msg.mobile_ap_handle = qcmap_cm_cb.ipv6_mobile_ap_handle;

    qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                         QMI_QCMAP_DISABLE_FIREWALL_SETTING_REQ_V01,
                                         &disable_firewall_setting_req_msg,
                                         sizeof(disable_firewall_setting_req_msg),
                                         &disable_firewall_setting_resp_msg,
                                         sizeof(disable_firewall_setting_resp_msg),
                                         QCMAP_CM_QMI_TIMEOUT_VALUE);

    if ((qmi_error != QMI_NO_ERR) ||
        (disable_firewall_setting_resp_msg.resp.result != QMI_NO_ERR))
    {
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      LOG_MSG_ERROR("Disable firewall config failed for IPv6 %d %d",
                    qmi_error, disable_firewall_setting_resp_msg.resp.error, 0);
    }
  }
  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_FIREWALL_OP()

DESCRIPTION

  Enable/Disable/Get the firewall setting in the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the firewall could be configured, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The firewall could not be configured.


SIDE EFFECTS

===========================================================================*/
int qcmap_cm_firewall_op
(
  int                      qcmap_cm_handle,      /* Handle for MobileAP CM  */
  qcmap_cm_firewall_req_e  firewall_op_type,     /* Enable/Set/Get Firewall */
  uint8                   *pkts_allowed,         /* Packets allowed?        */
  int                     *qcmap_cm_errno        /* Error condition value   */
)
{
  int ret = QCMAP_CM_SUCCESS;

  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_firewall_op: enter",
                0, 0, 0);

  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state == QCMAP_CM_DISABLE)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR("QCMAP CM disable state",
                  0, 0, 0);
    return ret;
  }

  qcmap_cm_unlock();

  switch (firewall_op_type)
  {
    case QCMAP_CM_ENABLE_FIREWALL:
      ret = qcmap_cm_firewall_op_enable(pkts_allowed, qcmap_cm_errno);
      break;
    case QCMAP_CM_GET_FIREWALL:
      ret = qcmap_cm_firewall_op_get(pkts_allowed, qcmap_cm_errno);
      break;
    case QCMAP_CM_DISABLE_FIREWALL:
      ret = qcmap_cm_firewall_op_disable(qcmap_cm_errno);
      break;
    default:
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      LOG_MSG_ERROR("Invalid firewall op %d",
                    firewall_op_type, 0, 0);
      break;
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_DMZ_OP_ADD()

DESCRIPTION

  Add a DMZ to the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the DMZ could be added, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The DMZ could not be added.


SIDE EFFECTS

===========================================================================*/
static int qcmap_cm_dmz_op_add
(
  uint32 *dmz_addr,                               /* DMZ IP address        */
  int    *qcmap_cm_errno                          /* Error condition value */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_set_dmz_req_msg_v01 set_dmz_req_msg_v01;
  qcmap_set_dmz_resp_msg_v01 set_dmz_resp_msg_v01;

  set_dmz_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;
  set_dmz_req_msg_v01.dmz_ip_addr = *dmz_addr;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_SET_DMZ_REQ_V01,
                                       &set_dmz_req_msg_v01,
                                       sizeof(set_dmz_req_msg_v01),
                                       &set_dmz_resp_msg_v01,
                                       sizeof(set_dmz_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);

  if ((qmi_error != QMI_NO_ERR) ||
      (set_dmz_resp_msg_v01.resp.result != QMI_NO_ERR))
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Set DMZ IP failed %d %d", qmi_error, set_dmz_resp_msg_v01.resp.error, 0);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_DMZ_OP_GET()

DESCRIPTION

  Get the DMZ from the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the DMZ could be retrieved, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The DMZ could not be retried.


SIDE EFFECTS

===========================================================================*/
static int qcmap_cm_dmz_op_get
(
  uint32 *dmz_addr,                               /* DMZ IP address        */
  int    *qcmap_cm_errno                          /* Error condition value */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_get_dmz_req_msg_v01 get_dmz_req_msg_v01;
  qcmap_get_dmz_resp_msg_v01 get_dmz_resp_msg_v01;

  get_dmz_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_GET_DMZ_REQ_V01,
                                       &get_dmz_req_msg_v01,
                                       sizeof(get_dmz_req_msg_v01),
                                       &get_dmz_resp_msg_v01,
                                       sizeof(get_dmz_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
  if ((qmi_error == QMI_NO_ERR) &&
      (get_dmz_resp_msg_v01.resp.result == QMI_NO_ERR) &&
      (get_dmz_resp_msg_v01.dmz_ip_addr_valid == TRUE))
  {
    *dmz_addr = get_dmz_resp_msg_v01.dmz_ip_addr;
  }
  else
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Get DMZ IP failed %d %d", qmi_error, get_dmz_resp_msg_v01.resp.error, 0);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_DMZ_OP_DELETE()

DESCRIPTION

  Delete the DMZ from the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the DMZ could be deleted, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The DMZ could not be deleted.


SIDE EFFECTS

===== ======================================================================*/
static int
qcmap_cm_dmz_op_delete
(
  int *qcmap_cm_errno                              /* Error condition value */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_delete_dmz_req_msg_v01 delete_dmz_req_msg_v01;
  qcmap_delete_dmz_resp_msg_v01 delete_dmz_resp_msg_v01;

  delete_dmz_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_DELETE_DMZ_REQ_V01,
                                       &delete_dmz_req_msg_v01,
                                       sizeof(delete_dmz_req_msg_v01),
                                       &delete_dmz_resp_msg_v01,
                                       sizeof(delete_dmz_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);

  if ((qmi_error != QMI_NO_ERR) ||
      (delete_dmz_resp_msg_v01.resp.result != QMI_NO_ERR))
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Delete DMZ IP failed %d %d", qmi_error, delete_dmz_resp_msg_v01.resp.error, 0);
  }

  return ret;
}

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
int qcmap_cm_dmz_op
(
  int                 qcmap_cm_handle,           /* Handle for MobileAP CM */
  qcmap_cm_dmz_req_e  dmz_op_type,               /* Set/Get/Delete DMZ     */
  uint32             *dmz_addr,                  /* DMZ IP address         */
  int                *qcmap_cm_errno             /* Error condition value  */
)
{
  int qmi_err;
  int ret = QCMAP_CM_SUCCESS;
  int dmz_addr_is_valid = FALSE;

  ds_assert(dmz_addr != NULL);
  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_dmz_op: enter", 0, 0, 0);
  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state == QCMAP_CM_DISABLE)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "QCMAP CM disable state", 0, 0, 0);
    return ret;
  }

  qcmap_cm_unlock();

  switch (dmz_op_type)
  {
    case QCMAP_CM_ADD_DMZ:
      ret = qcmap_cm_dmz_op_add(dmz_addr, qcmap_cm_errno);
      break;
    case QCMAP_CM_GET_DMZ:
      ret = qcmap_cm_dmz_op_get(dmz_addr, qcmap_cm_errno);
      break;
    case QCMAP_CM_DELETE_DMZ:
      ret = qcmap_cm_dmz_op_delete(qcmap_cm_errno);
      break;
    default:
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      LOG_MSG_ERROR("Invalid DMZ op type op %d",
                    dmz_op_type, 0, 0);
      break;
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_VPN_OP_GET_IPSEC()

DESCRIPTION

  Get the IPSEC VPN passthrough from the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the IPSEC VPN passthrough setting could be retrieved, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The IPSEC VPN passthrough setting could not be
                             retrieved.


SIDE EFFECTS

===========================================================================*/
int
qcmap_cm_vpn_op_get_ipsec
(
  uint8 *vpn_value,                         /* IPSEC VPN Passthrough value */
  int   *qcmap_cm_errno                     /* Error condition value       */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_get_ipsec_vpn_pass_through_req_msg_v01 get_ipsec_vpn_pass_through_req_msg_v01;
  qcmap_get_ipsec_vpn_pass_through_resp_msg_v01 get_ipsec_vpn_pass_through_resp_msg_v01;

  get_ipsec_vpn_pass_through_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_GET_IPSEC_VPN_PASS_THROUGH_REQ_V01,
                                       &get_ipsec_vpn_pass_through_req_msg_v01,
                                       sizeof(get_ipsec_vpn_pass_through_req_msg_v01),
                                       &get_ipsec_vpn_pass_through_resp_msg_v01,
                                       sizeof(get_ipsec_vpn_pass_through_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
  if ((qmi_error == QMI_NO_ERR) &&
      (get_ipsec_vpn_pass_through_resp_msg_v01.resp.result == QMI_NO_ERR) &&
      (get_ipsec_vpn_pass_through_resp_msg_v01.vpn_pass_through_value_valid == TRUE))
  {
    *vpn_value = get_ipsec_vpn_pass_through_resp_msg_v01.vpn_pass_through_value;
  }
  else
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Get ipsec vpn pass failed %d %d",
                   qmi_error, get_ipsec_vpn_pass_through_resp_msg_v01.resp.error, 0);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_VPN_OP_SET_IPSEC()

DESCRIPTION

  Set the IPSEC VPN passthrough in the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the IPSEC VPN passthrough setting could be set, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The IPSEC VPN passthrough setting could not be
                             set.


SIDE EFFECTS

===========================================================================*/
static int
qcmap_cm_vpn_op_set_ipsec
(
  uint8 *vpn_value,                         /* IPSEC VPN Passthrough value */
  int   *qcmap_cm_errno                     /* Error condition value       */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_set_ipsec_vpn_pass_through_req_msg_v01 set_ipsec_vpn_pass_through_req_msg_v01;
  qcmap_set_ipsec_vpn_pass_through_resp_msg_v01 set_ipsec_vpn_pass_through_resp_msg_v01;

  set_ipsec_vpn_pass_through_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;
  set_ipsec_vpn_pass_through_req_msg_v01.vpn_pass_through_value = *vpn_value;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_SET_IPSEC_VPN_PASS_THROUGH_REQ_V01,
                                       &set_ipsec_vpn_pass_through_req_msg_v01,
                                       sizeof(set_ipsec_vpn_pass_through_req_msg_v01),
                                       &set_ipsec_vpn_pass_through_resp_msg_v01,
                                       sizeof(set_ipsec_vpn_pass_through_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);

  if ((qmi_error != QMI_NO_ERR) ||
      (set_ipsec_vpn_pass_through_resp_msg_v01.resp.result != QMI_NO_ERR))
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Set ipsec vpn pass through failed %d %d",
                   qmi_error, set_ipsec_vpn_pass_through_resp_msg_v01.resp.error, 0);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_VPN_OP_GET_IPSEC()

DESCRIPTION

  Get the L2TP VPN passthrough from the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the L2TP VPN passthrough setting could be retrieved, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The L2TP VPN passthrough setting could not be
                             retrieved.


SIDE EFFECTS

===========================================================================*/
int
qcmap_cm_vpn_op_get_l2tp
(
  uint8 *vpn_value,                          /* L2TP VPN Passthrough value */
  int   *qcmap_cm_errno                      /* Error condition value      */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_get_l2tp_vpn_pass_through_req_msg_v01 get_l2tp_vpn_pass_through_req_msg_v01;
  qcmap_get_l2tp_vpn_pass_through_resp_msg_v01 get_l2tp_vpn_pass_through_resp_msg_v01;

  get_l2tp_vpn_pass_through_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_GET_L2TP_VPN_PASS_THROUGH_REQ_V01,
                                       &get_l2tp_vpn_pass_through_req_msg_v01,
                                       sizeof(get_l2tp_vpn_pass_through_req_msg_v01),
                                       &get_l2tp_vpn_pass_through_resp_msg_v01,
                                       sizeof(get_l2tp_vpn_pass_through_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
  if ((qmi_error == QMI_NO_ERR) &&
      (get_l2tp_vpn_pass_through_resp_msg_v01.resp.result == QMI_NO_ERR) &&
      (get_l2tp_vpn_pass_through_resp_msg_v01.vpn_pass_through_value_valid == TRUE))
  {
    *vpn_value = get_l2tp_vpn_pass_through_resp_msg_v01.vpn_pass_through_value;
  }
  else
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Get l2tp vpn pass failed %d %d",
                   qmi_error, get_l2tp_vpn_pass_through_resp_msg_v01.resp.error, 0);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_VPN_OP_SET_L2TP()

DESCRIPTION

  Set the L2TP VPN passthrough in the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the L2TP VPN passthrough setting could be set, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The L2TP VPN passthrough setting could not be
                             set.


SIDE EFFECTS

===========================================================================*/
static int
qcmap_cm_vpn_op_set_l2tp
(
  uint8 *vpn_value,                          /* L2TP VPN Passthrough value */
  int   *qcmap_cm_errno                      /* Error condition value      */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_set_l2tp_vpn_pass_through_req_msg_v01 set_l2tp_vpn_pass_through_req_msg_v01;
  qcmap_set_l2tp_vpn_pass_through_resp_msg_v01 set_l2tp_vpn_pass_through_resp_msg_v01;

  set_l2tp_vpn_pass_through_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;
  set_l2tp_vpn_pass_through_req_msg_v01.vpn_pass_through_value = *vpn_value;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_SET_L2TP_VPN_PASS_THROUGH_REQ_V01,
                                       &set_l2tp_vpn_pass_through_req_msg_v01,
                                       sizeof(set_l2tp_vpn_pass_through_req_msg_v01),
                                       &set_l2tp_vpn_pass_through_resp_msg_v01,
                                       sizeof(set_l2tp_vpn_pass_through_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);

  if ((qmi_error != QMI_NO_ERR) ||
      (set_l2tp_vpn_pass_through_resp_msg_v01.resp.result != QMI_NO_ERR))
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Set l2tp vpn pass through failed %d %d",
                   qmi_error, set_l2tp_vpn_pass_through_resp_msg_v01.resp.error, 0);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_VPN_OP_GET_PPTP()

DESCRIPTION

  Get the PPTP VPN passthrough from the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the PPTP VPN passthrough setting could be retrieved, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The PPTP VPN passthrough setting could not be
                             retrieved.


SIDE EFFECTS

===========================================================================*/
int
qcmap_cm_vpn_op_get_pptp
(
  uint8  *vpn_value,                         /* PPTP VPN Passthrough value */
  int    *qcmap_cm_errno                     /* Error condition value      */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_get_pptp_vpn_pass_through_req_msg_v01 get_pptp_vpn_pass_through_req_msg_v01;
  qcmap_get_pptp_vpn_pass_through_resp_msg_v01 get_pptp_vpn_pass_through_resp_msg_v01;

  get_pptp_vpn_pass_through_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_GET_PPTP_VPN_PASS_THROUGH_REQ_V01,
                                       &get_pptp_vpn_pass_through_req_msg_v01,
                                       sizeof(get_pptp_vpn_pass_through_req_msg_v01),
                                       &get_pptp_vpn_pass_through_resp_msg_v01,
                                       sizeof(get_pptp_vpn_pass_through_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
  if ((qmi_error == QMI_NO_ERR) &&
      (get_pptp_vpn_pass_through_resp_msg_v01.resp.result == QMI_NO_ERR) &&
      (get_pptp_vpn_pass_through_resp_msg_v01.vpn_pass_through_value_valid == TRUE))
  {
    *vpn_value = get_pptp_vpn_pass_through_resp_msg_v01.vpn_pass_through_value;
  }
  else
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Get pptp vpn pass failed %d %d",
                   qmi_error, get_pptp_vpn_pass_through_resp_msg_v01.resp.error, 0);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_VPN_OP_SET_PPTP()

DESCRIPTION

  Set the PPTP VPN passthrough in the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the PPTP VPN passthrough setting could be set, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The PPTP VPN passthrough setting could not be
                             set.


SIDE EFFECTS

===========================================================================*/
static int
qcmap_cm_vpn_op_set_pptp
(
  uint8  *vpn_value,                         /* PPTP VPN Passthrough value */
  int    *qcmap_cm_errno                     /* Error condition value      */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_set_pptp_vpn_pass_through_req_msg_v01 set_pptp_vpn_pass_through_req_msg_v01;
  qcmap_set_pptp_vpn_pass_through_resp_msg_v01 set_pptp_vpn_pass_through_resp_msg_v01;

  set_pptp_vpn_pass_through_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;
  set_pptp_vpn_pass_through_req_msg_v01.vpn_pass_through_value = *vpn_value;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_SET_PPTP_VPN_PASS_THROUGH_REQ_V01,
                                       &set_pptp_vpn_pass_through_req_msg_v01,
                                       sizeof(set_pptp_vpn_pass_through_req_msg_v01),
                                       &set_pptp_vpn_pass_through_resp_msg_v01,
                                       sizeof(set_pptp_vpn_pass_through_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);

  if ((qmi_error != QMI_NO_ERR) ||
      (set_pptp_vpn_pass_through_resp_msg_v01.resp.result != QMI_NO_ERR))
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Set pptp vpn pass through failed %d %d", qmi_error, set_pptp_vpn_pass_through_resp_msg_v01.resp.error, 0);
  }

  return ret;
}

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
int qcmap_cm_vpn_op
(
  int                 qcmap_cm_handle,           /* Handle for MobileAP CM */
  qcmap_cm_vpn_req_e  vpn_op_type,               /* Set/Get VPN Passthru   */
  uint8              *vpn_value,                 /* VPN Passthrough value  */
  int                *qcmap_cm_errno             /* Error condition value  */
)
{
  int qmi_err;
  int ret = QCMAP_CM_SUCCESS;
  int vpn_value_valid = FALSE;

  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_vpn_op: enter", 0, 0, 0);
  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state == QCMAP_CM_DISABLE)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "QCMAP CM disable state", 0, 0, 0);
    return ret;
  }

  qcmap_cm_unlock();

  switch (vpn_op_type)
  {
    case QCMAP_CM_SET_IPSEC:
      ret = qcmap_cm_vpn_op_set_ipsec(vpn_value,qcmap_cm_errno);
      break;
    case QCMAP_CM_GET_IPSEC:
      ret = qcmap_cm_vpn_op_get_ipsec(vpn_value,qcmap_cm_errno);
      break;
    case QCMAP_CM_SET_L2TP:
      ret = qcmap_cm_vpn_op_set_l2tp(vpn_value,qcmap_cm_errno);
      break;
    case QCMAP_CM_GET_L2TP:
      ret = qcmap_cm_vpn_op_get_l2tp(vpn_value,qcmap_cm_errno);
      break;
    case QCMAP_CM_SET_PPTP:
      ret = qcmap_cm_vpn_op_set_pptp(vpn_value,qcmap_cm_errno);
      break;
    case QCMAP_CM_GET_PPTP:
      ret = qcmap_cm_vpn_op_get_pptp(vpn_value,qcmap_cm_errno);
      break;
    default:
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      LOG_MSG_ERROR("Invalid VPN op %d",
                    vpn_op_type, 0, 0);
      break;
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_NAT_TIMEOUT_OP_GET()

DESCRIPTION
  This Function gets the NAT timeout from the QCMAP CM.

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
static int
qcmap_cm_nat_timeout_op_get
(
  uint16 *timeout_value,                          /* Timeout value         */
  int    *qcmap_cm_errno                          /* Error condition value */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_get_dynamic_nat_entry_timeout_req_msg_v01 get_dynamic_nat_entry_timeout_req_msg_v01;
  qcmap_get_dynamic_nat_entry_timeout_resp_msg_v01 get_dynamic_nat_entry_timeout_resp_msg_v01;

  get_dynamic_nat_entry_timeout_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_GET_DYNAMIC_NAT_ENTRY_TIMEOUT_REQ_V01,
                                       &get_dynamic_nat_entry_timeout_req_msg_v01,
                                       sizeof(get_dynamic_nat_entry_timeout_req_msg_v01),
                                       &get_dynamic_nat_entry_timeout_resp_msg_v01,
                                       sizeof(get_dynamic_nat_entry_timeout_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
  if ((qmi_error == QMI_NO_ERR) &&
      (get_dynamic_nat_entry_timeout_resp_msg_v01.resp.result == QMI_NO_ERR) &&
      (get_dynamic_nat_entry_timeout_resp_msg_v01.timeout_valid == TRUE))
  {
    *timeout_value = get_dynamic_nat_entry_timeout_resp_msg_v01.timeout;
  }
  else
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Get dynamic nat timeout failed %d %d",
                   qmi_error, get_dynamic_nat_entry_timeout_resp_msg_v01.resp.error, 0);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_NAT_TIMEOUT_OP_SET()

DESCRIPTION
  This Function sets the NAT timeout in the QCMAP CM.

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
int qcmap_cm_nat_timeout_op_set
(
  uint16 *timeout_value,                          /* Timeout value to set  */
  int    *qcmap_cm_errno                          /* Error condition value */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_set_dynamic_nat_entry_timeout_req_msg_v01 set_dynamic_nat_entry_timeout_req_msg_v01;
  qcmap_set_dynamic_nat_entry_timeout_resp_msg_v01 set_dynamic_nat_entry_timeout_resp_msg_v01;

  set_dynamic_nat_entry_timeout_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;
  set_dynamic_nat_entry_timeout_req_msg_v01.timeout = *timeout_value;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_SET_DYNAMIC_NAT_ENTRY_TIMEOUT_REQ_V01,
                                       &set_dynamic_nat_entry_timeout_req_msg_v01,
                                       sizeof(set_dynamic_nat_entry_timeout_req_msg_v01),
                                       &set_dynamic_nat_entry_timeout_resp_msg_v01,
                                       sizeof(set_dynamic_nat_entry_timeout_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
  if ((qmi_error != QMI_NO_ERR) ||
      (set_dynamic_nat_entry_timeout_resp_msg_v01.resp.result != QMI_NO_ERR))
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR("Set dynamic nat timeout failed %d %d",
                  qmi_error, set_dynamic_nat_entry_timeout_resp_msg_v01.resp.error, 0);
  }

  return ret;
}
/*===========================================================================

FUNCTION QCMAP_CM_NAT_TIMEOUT_OP()

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
int qcmap_cm_nat_timeout_op
(
  int                         qcmap_cm_handle,   /* Handle for MobileAP CM */
  qcmap_cm_nat_timeout_req_e  nat_timeout_op_type, /* Set/Get NAT Timeout  */
  uint16                     *timeout_value,     /* Timeout value          */
  int                        *qcmap_cm_errno     /* Error condition value */
)
{
  int qmi_err;
  int ret = QCMAP_CM_SUCCESS;
  int timeout_value_valid = FALSE;

  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_nat_timeout_op: enter", 0, 0, 0);
  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state == QCMAP_CM_DISABLE)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "QCMAP CM disable state", 0, 0, 0);
    return ret;
  }

  qcmap_cm_unlock();

  switch (nat_timeout_op_type)
  {
    case QCMAP_CM_SET_NAT_ENTRY_TIMEOUT:
      ret = qcmap_cm_nat_timeout_op_set(timeout_value, qcmap_cm_errno);
      break;
    case QCMAP_CM_GET_NAT_ENTRY_TIMEOUT:
      ret = qcmap_cm_nat_timeout_op_get(timeout_value, qcmap_cm_errno);
      break;
    default:
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      LOG_MSG_ERROR( "Invalid nat timeouot op %d",
                     nat_timeout_op_type, 0, 0);
      break;
  }

  return ret;

}

/*===========================================================================

FUNCTION QCMAP_CM_GET_STATE()

DESCRIPTION

  It will get QCMAP CM current state.

DEPENDENCIES
  None.

SIDE EFFECTS

===========================================================================*/

qcmap_cm_state_e
qcmap_cm_get_state
(
  void
)
{
  return qcmap_cm_cb.state;
}

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
int qcmap_cm_set_auto_connect
(
  int      qcmap_cm_handle,                   /* Handle for MobileAP CM    */
  int     *qcmap_cm_errno,                    /* Error condition value     */
  boolean  auto_connect                       /* Autoconnect Enable or Not */
)
{
  int ret = QCMAP_CM_SUCCESS;
  nas_indication_register_req_msg_v01 qcmap_nas_indication_register_req_msg_v01;
  nas_indication_register_resp_msg_v01 qcmap_nas_indication_register_resp_msg_v01;

  qmi_client_error_type qmi_error;

  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_set_auto_connect: enter", 0, 0, 0);
  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state == QCMAP_CM_DISABLE)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "QCMAP CM disable state", 0, 0, 0);
    return ret;
  }

  qcmap_cm_cb.cfg->wan_config.auto_connect = auto_connect;

  qcmap_cm_unlock();

  /*if autoconnect is enabled register to serving system NAS indication and connect backhaul*/
  if(qcmap_cm_cb.cfg->wan_config.auto_connect && 
    ((qcmap_cm_cb.state == QCMAP_CM_LAN_CONNECTED)|| (qcmap_cm_cb.state == QCMAP_CM_V6_LAN_CONNECTED)))
  {
    memset(&qcmap_nas_indication_register_req_msg_v01, 0, sizeof(nas_indication_register_req_msg_v01));

    qcmap_nas_indication_register_req_msg_v01.req_serving_system_valid = TRUE;
    qcmap_nas_indication_register_req_msg_v01.req_serving_system = 0x01;

    LOG_MSG_INFO1("Registering for serving system NAS indications",0,0,0);

    qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_nas_handle,
                                         QMI_NAS_INDICATION_REGISTER_REQ_MSG_V01,
                                         &qcmap_nas_indication_register_req_msg_v01,
                                         sizeof(qcmap_nas_indication_register_req_msg_v01),
                                         &qcmap_nas_indication_register_resp_msg_v01,
                                         sizeof(qcmap_nas_indication_register_resp_msg_v01),
                                         QCMAP_CM_QMI_TIMEOUT_VALUE);
    if(qmi_error != QMI_NO_ERR)
    {
      LOG_MSG_ERROR("Failed to register to NAS indications %d", qmi_error,0,0);
      return QCMAP_CM_ERROR;
    }

    LOG_MSG_INFO1("Starting backhaul connection due to autoconnect",0,0,0);
    qcmap_cm_cb.backhaul_service = FALSE;
    qcmap_cm_connect_backhaul(qcmap_cm_cb.handle, qcmap_cm_errno);

  }

  return QCMAP_CM_SUCCESS;

}


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
int qcmap_cm_set_roaming
(
  int      qcmap_cm_handle,                      /* Handle for MobileAP CM */
  int     *qcmap_cm_errno,                       /* Error condition value  */
  boolean  roaming                               /* Roaming Enable or Not  */
)
{
  int ret = QCMAP_CM_SUCCESS;

  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_set_roaming: enter", 0, 0, 0);
  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state == QCMAP_CM_DISABLE)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "QCMAP CM disable state", 0, 0, 0);
    return ret;
  }

  qcmap_cm_cb.cfg->wan_config.roaming = roaming;

  qcmap_cm_unlock();
}


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
int qcmap_cm_get_ipv4_net_conf
(
  int    qcmap_cm_handle,                        /* Handle for MobileAP CM */
  uint32 *public_ip,                             /* Public IP for WWAN     */
  uint32 *pri_dns_addr,                         /* Primary DNS IP address  */
  uint32 *sec_dns_addr,                         /* Seconday DNS IP address */
  int    *qcmap_cm_errno                        /* Error condition value   */
)
{
  int qmi_err;
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_get_wwan_config_req_msg_v01 get_wwan_config_req_msg_v01;
  qcmap_get_wwan_config_resp_msg_v01 get_wwan_config_resp_msg_v01;

  ds_assert(public_ip != NULL);
  ds_assert(pri_dns_addr != NULL);
  ds_assert(sec_dns_addr != NULL);
  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_get_ipv4_net_conf: enter", 0, 0, 0);

  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state != QCMAP_CM_WAN_CONNECTED)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "QCMAP CM not in wan connected state", 0, 0, 0);
    return ret;
  }

  qcmap_cm_unlock();

  get_wwan_config_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;
  get_wwan_config_req_msg_v01.addr_type_op = QCMAP_MASK_V4_ADDR_V01 |
                                             QCMAP_MASK_V4_DNS_ADDR_V01;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_GET_WWAN_CONFIG_REQ_V01,
                                       &get_wwan_config_req_msg_v01,
                                       sizeof(get_wwan_config_req_msg_v01),
                                       &get_wwan_config_resp_msg_v01,
                                       sizeof(get_wwan_config_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);

  if ((qmi_error == QMI_NO_ERR) &&
      (get_wwan_config_resp_msg_v01.resp.result == QMI_RESULT_SUCCESS_V01))
  {
    if (get_wwan_config_resp_msg_v01.v4_addr_valid == TRUE)
    {
      *public_ip = get_wwan_config_resp_msg_v01.v4_addr;
    }
    if (get_wwan_config_resp_msg_v01.v4_prim_dns_addr_valid == TRUE)
    {
      *pri_dns_addr = get_wwan_config_resp_msg_v01.v4_prim_dns_addr;
    }
    if (get_wwan_config_resp_msg_v01.v4_sec_dns_addr_valid == TRUE)
    {
      *sec_dns_addr = get_wwan_config_resp_msg_v01.v4_sec_dns_addr;
    }
  }
  else
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Get wan ipv4 net conf failed %d %d", qmi_error, get_wwan_config_resp_msg_v01.resp.error, 0);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_GET_DEV_NAME()

DESCRIPTION
  This Function get RMNET device name.

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
int qcmap_cm_get_dev_name
(
  int     qcmap_cm_handle,                     /* Handle for Mobile AP CM  */
  qcmap_ip_family_enum_v01
          qcmap_dev_type,                      /* Dev type                 */
  char   *qcmap_dev_name,                      /* Device name              */
  int    *qcmap_cm_errno                       /* Error condition value    */
)
{
  ds_assert(qcmap_dev_name != NULL);
  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_get_dev_name: enter", 0, 0, 0);

  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return QCMAP_CM_ERROR;
  }

  if (qcmap_cm_cb.state < QCMAP_CM_LAN_CONNECTED)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "QCMAP CM LAN not connected. %d", qcmap_cm_cb.state, 0, 0);
    return QCMAP_CM_ERROR;
  }

  if (qcmap_dev_type == QCMAP_IP_V4_V01)
  {
    strlcpy(qcmap_dev_name, qcmap_cm_cb.dsi_device_name, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 1);
  }
  else if (qcmap_dev_type == QCMAP_IP_V6_V01)
  {
    if (qcmap_cm_cb.ipv6_dsi_net_hndl.handle != NULL)
    {
      strlcpy(qcmap_dev_name, qcmap_cm_cb.ipv6_dsi_device_name, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 1);
    }
    else
    {
      qcmap_cm_unlock();
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      LOG_MSG_ERROR( "QCMAP CM IPv6 device name not valid", 0, 0, 0);
      return QCMAP_CM_ERROR;
    }
  }

  qcmap_cm_unlock();

  return QCMAP_CM_SUCCESS;
}

/*===========================================================================

FUNCTION QCMAP_CM_ENABLE_STA_MODE()

DESCRIPTION
  Enable MobileAP CM based on the config.
  It will register MobileAP event callback.
  It will configure Modem in MobileAP Mode and bring up RmNet between Q6 and A5.
  It will also bring up LAN if it is config.

DEPENDENCIES
  None.

RETURN VALUE
  Returns MobileAP CM application ID on success.
  On error, return 0 and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS
  None.

===========================================================================*/
int qcmap_cm_enable_sta_mode
(
  int    qcmap_cm_handle,                         /* Handler for SoftAP CM */
  int    *qcmap_cm_errno                          /* error condition value */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_station_mode_enable_req_msg_v01 req_msg;
  qcmap_station_mode_enable_resp_msg_v01 resp_msg;
/*-------------------------------------------------------------------------*/

  /* Validate args */
  ds_assert(qcmap_cm_errno != NULL);

  /* Set request msg parameters */
  req_msg.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;

  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state < QCMAP_CM_LAN_CONNECTED)
  {
    /* We can enable STA mode only after LAN is connected */
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "QCMAP CM LAN is not connected", 0, 0, 0);
    return ret;
  }

  qcmap_cm_unlock();

  /* Send QMI msg to modem with 15 sec timeout */
  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_STATION_MODE_ENABLE_REQ_V01,
                                       &req_msg,
                                       sizeof(qcmap_station_mode_enable_req_msg_v01),
                                       &resp_msg,
                                       sizeof(qcmap_station_mode_enable_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
  if (( qmi_error == QMI_TIMEOUT_ERR ) || ((qmi_error == QMI_NO_ERR) &&
      (resp_msg.resp.result == QMI_RESULT_SUCCESS_V01) &&
      (resp_msg.resp.error == QMI_ERR_NONE_V01)))
  {
    //Success
    *qcmap_cm_errno = QCMAP_CM_ENOERROR;
    ret = QCMAP_CM_SUCCESS;
    //Do we need update state?
    //qcmap_cm_cb.state = QCMAP_CM_WAN_CONNECTING;
  }
  else
  {
    // STA MODE not changed..unlikely!
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    //qcmap_cm_cb.state = QCMAP_CM_WAN_CONNECTING;
    LOG_MSG_ERROR( "QCMAP AP Handle %d STA enable qmi err %d Resp error %d",
                   qcmap_cm_handle, qmi_error, resp_msg.resp.error);
  }

  return ret;
}


/*===========================================================================

FUNCTION QCMAP_CM_DISABLE_STA_MODE()

DESCRIPTION
  Disable MobileAP CM.
  Send disable_sta_mode msg to modem.

DEPENDENCIES
  None.

RETURN VALUE
  Returns MobileAP CM application ID on success.
  On error, return 0 and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS
  None.

===========================================================================*/
int qcmap_cm_disable_sta_mode
(
  int                qcmap_cm_handle, /* Handler for SoftAP CM */
  int                *qcmap_cm_errno  /* error condition value */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_station_mode_disable_req_msg_v01 req_msg;
  qcmap_station_mode_disable_resp_msg_v01 resp_msg;
  qcmap_change_nat_type_req_msg_v01 update_nat_type_req_msg;
  qcmap_change_nat_type_resp_msg_v01 update_nat_type_resp_msg;
/*-------------------------------------------------------------------------*/

  /* Validate args */
  ds_assert(qcmap_cm_errno != NULL);

  /* Set request msg parameters */
  req_msg.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;

  /* This message is needed to update modem about last used NAT type on A5*/
  update_nat_type_req_msg.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;
  update_nat_type_req_msg.nat_type_option = qcmap_cm_cb.cfg->nat_config.nat_type;
  update_nat_type_req_msg.nat_type_option_valid = TRUE;

  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state < QCMAP_CM_LAN_CONNECTED)
  {
    /* We can Disable STA mode only after LAN is connected */
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "QCMAP CM LAN is not connected", 0, 0, 0);
    return ret;
  }

  qcmap_cm_unlock();

  /* Send QMI msg to modem to update NAT type just before disabling STA mode */
  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                    QMI_QCMAP_CHANGE_NAT_TYPE_REQ_V01,
                                    &update_nat_type_req_msg,
                                    sizeof(qcmap_change_nat_type_req_msg_v01),
                                    &update_nat_type_resp_msg,
                                    sizeof(qcmap_change_nat_type_resp_msg_v01),
                                    QCMAP_CM_QMI_TIMEOUT_VALUE);
  if ((qmi_error == QMI_NO_ERR) &&
      (update_nat_type_resp_msg.resp.result == QMI_RESULT_SUCCESS_V01) &&
      (update_nat_type_resp_msg.resp.error == QMI_ERR_NONE_V01))
  {
    //Success - Modem is now aware about the last used NAT type on A5.
    *qcmap_cm_errno = QCMAP_CM_ENOERROR;
    ret = QCMAP_CM_SUCCESS;
  }
  else
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    LOG_MSG_ERROR( "QCMAP AP Handle %d Error updating modem about NAT type"
                   "while disabling STA mode - qmi err %d Resp err %d",
                   qcmap_cm_handle, qmi_error, resp_msg.resp.error);
    return QCMAP_CM_ERROR;
  }

  /* Send QMI msg to modem with 15 sec timeout */
  qmi_error = qmi_client_send_msg_sync(
                qcmap_cm_cb.qmi_qcmap_handle,
                QMI_QCMAP_STATION_MODE_DISABLE_REQ_V01,
                &req_msg,
                sizeof(qcmap_station_mode_disable_req_msg_v01),
                &resp_msg,
                sizeof(qcmap_station_mode_disable_resp_msg_v01),
                QCMAP_CM_QMI_TIMEOUT_VALUE);
  if ((qmi_error == QMI_NO_ERR) &&
      (resp_msg.resp.result == QMI_RESULT_SUCCESS_V01) &&
      (resp_msg.resp.error == QMI_ERR_NONE_V01))
  {
    //Success
    *qcmap_cm_errno = QCMAP_CM_ENOERROR;
    ret = QCMAP_CM_SUCCESS;
    //Do we need update state?
    //qcmap_cm_cb.state = QCMAP_CM_WAN_DISCONNECTING;

    /* Set the various config parameters while moving from AP+STA to AP mode */
    qcmap_cm_config_init(qcmap_cm_handle, qcmap_cm_errno);
  }
  else
  {
    // Error: STA MODE not changed.
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    //qcmap_cm_cb.state = QCMAP_CM_WAN_DISCONNECTING;
    LOG_MSG_ERROR( "QCMAP AP Handle %d STA disable qmi err %d Resp error %d",
                   qcmap_cm_handle, qmi_error, resp_msg.resp.error);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_EXTD_FIREWALL_ENTRY_OP_ADD()

DESCRIPTION

  Adds an extended firewall entry to the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the firewall entry could be added, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The entry could not be added.


SIDE EFFECTS

===========================================================================*/
static int qcmap_cm_extd_firewall_entry_op_add
(
  qcmap_cm_extd_firewall_entry_conf_t *firewall_entry, /* Firewall Entry          */
  int    *qcmap_cm_errno                            /* error condition value */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_add_extd_firewall_config_req_msg_v01 add_extd_firewall_config_req_msg_v01;
  qcmap_add_extd_firewall_config_resp_msg_v01 add_extd_firewall_config_resp_msg_v01;
  int next_hdr_prot;
  int inc;

  ds_assert(qcmap_cm_errno != NULL);
  ds_assert(firewall_entry != NULL);

  memset(&add_extd_firewall_config_req_msg_v01, 0,
         sizeof(qcmap_add_extd_firewall_config_req_msg_v01));

  if(firewall_entry->filter_spec.ip_vsn == IP_V4)
  {
    if (qcmap_cm_cb.mobile_ap_handle != QCMAP_WAN_INVALID_QCMAP_HANDLE )
    {
      add_extd_firewall_config_req_msg_v01.mobile_ap_handle =
                                                    qcmap_cm_cb.mobile_ap_handle;
    }
    else
    {
      LOG_MSG_ERROR( "Cannot add firewall rule for IP family type %d",
                     firewall_entry->filter_spec.ip_vsn, 0, 0);
      return QCMAP_CM_ERROR;

    }
  }
  else if(firewall_entry->filter_spec.ip_vsn == IP_V6)
  {
    if(qcmap_cm_cb.ipv6_mobile_ap_handle != QCMAP_WAN_INVALID_QCMAP_HANDLE)
    {
      add_extd_firewall_config_req_msg_v01.mobile_ap_handle =
                                               qcmap_cm_cb.ipv6_mobile_ap_handle;

    }
    else
    {
      LOG_MSG_ERROR( "Cannot add firewall rule for IP family type  %d",
                     firewall_entry->filter_spec.ip_vsn, 0, 0);
      return QCMAP_CM_ERROR;
    }
  }


  LOG_MSG_INFO1("QCMAP CM mobile AP handle %d ",
                add_extd_firewall_config_req_msg_v01.mobile_ap_handle, 0, 0);

  LOG_MSG_INFO1("IP family type %d ",
                firewall_entry->filter_spec.ip_vsn, 0, 0);

  switch(firewall_entry->filter_spec.ip_vsn)
  {

    case IP_V4:

      next_hdr_prot = firewall_entry->filter_spec.ip_hdr.v4.next_hdr_prot;

      if(firewall_entry->filter_spec.ip_hdr.v4.field_mask &
                                                       IPFLTR_MASK_IP4_SRC_ADDR)
      {
        add_extd_firewall_config_req_msg_v01.ip4_src_addr_valid = TRUE;
        add_extd_firewall_config_req_msg_v01.ip4_src_addr.addr =
                         firewall_entry->filter_spec.ip_hdr.v4.src.addr.ps_s_addr;
        add_extd_firewall_config_req_msg_v01.ip4_src_addr.subnet_mask =
                  firewall_entry->filter_spec.ip_hdr.v4.src.subnet_mask.ps_s_addr;
        LOG_MSG_INFO1("IP4 src addr is:", 0, 0, 0);
        IPV4_ADDR_MSG(add_extd_firewall_config_req_msg_v01.ip4_src_addr.addr);
        LOG_MSG_INFO1("IP4 src subnet mask is:", 0, 0, 0);
        IPV4_ADDR_MSG(add_extd_firewall_config_req_msg_v01.ip4_src_addr.subnet_mask);

      }

      if(firewall_entry->filter_spec.ip_hdr.v4.field_mask &
                                                      IPFLTR_MASK_IP4_DST_ADDR)
      {
        add_extd_firewall_config_req_msg_v01.ip4_dst_addr_valid = TRUE;
        add_extd_firewall_config_req_msg_v01.ip4_dst_addr.addr =
                      firewall_entry->filter_spec.ip_hdr.v4.dst.addr.ps_s_addr;
        add_extd_firewall_config_req_msg_v01.ip4_dst_addr.subnet_mask =
                firewall_entry->filter_spec.ip_hdr.v4.dst.subnet_mask.ps_s_addr;
        LOG_MSG_INFO1("IP4 dst addr is:", 0, 0, 0);
        IPV4_ADDR_MSG(add_extd_firewall_config_req_msg_v01.ip4_dst_addr.addr);
        LOG_MSG_INFO1("IP4 dst subnet mask is:", 0, 0, 0);
        IPV4_ADDR_MSG(add_extd_firewall_config_req_msg_v01.ip4_dst_addr.subnet_mask);
      }

      if(firewall_entry->filter_spec.ip_hdr.v4.field_mask & IPFLTR_MASK_IP4_TOS)
      {
        add_extd_firewall_config_req_msg_v01.ip4_tos_valid = TRUE;
        add_extd_firewall_config_req_msg_v01.ip4_tos.value =
                                  firewall_entry->filter_spec.ip_hdr.v4.tos.val;
        add_extd_firewall_config_req_msg_v01.ip4_tos.mask =
                                 firewall_entry->filter_spec.ip_hdr.v4.tos.mask;
        LOG_MSG_INFO1("IP4  TOS value %d  mask %d ",
                      add_extd_firewall_config_req_msg_v01.ip4_tos.value,
                      add_extd_firewall_config_req_msg_v01.ip4_tos.mask, 0);
      }
      break;

   case IP_V6:
      next_hdr_prot = firewall_entry->filter_spec.ip_hdr.v6.next_hdr_prot;


      if(firewall_entry->filter_spec.ip_hdr.v6.field_mask &
                                                       IPFLTR_MASK_IP6_SRC_ADDR)
      {
        add_extd_firewall_config_req_msg_v01.ip6_src_addr_valid = TRUE;

        memcpy(add_extd_firewall_config_req_msg_v01.ip6_src_addr.addr,
               firewall_entry->filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr8,
               QCMAP_IPV6_ADDR_LEN_V01*sizeof(uint8));

        add_extd_firewall_config_req_msg_v01.ip6_src_addr.prefix_len =
                               firewall_entry->filter_spec.ip_hdr.v6.src.prefix_len;

        LOG_MSG_INFO1("IP6 src addr is", 0, 0, 0);
        IPV6_ADDR_MSG(firewall_entry->filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr64);

        LOG_MSG_INFO1("IPV6 src prefix length %d ",
                      add_extd_firewall_config_req_msg_v01.ip6_src_addr.prefix_len, 0, 0);
      }

      if(firewall_entry->filter_spec.ip_hdr.v6.field_mask & IPFLTR_MASK_IP6_DST_ADDR)
      {
        add_extd_firewall_config_req_msg_v01.ip6_dst_addr_valid = TRUE;

        memcpy(add_extd_firewall_config_req_msg_v01.ip6_dst_addr.addr,
               firewall_entry->filter_spec.ip_hdr.v6.dst.addr.in6_u.u6_addr8,
               QCMAP_IPV6_ADDR_LEN_V01*sizeof(uint8));

        add_extd_firewall_config_req_msg_v01.ip6_dst_addr.prefix_len =
                                firewall_entry->filter_spec.ip_hdr.v6.dst.prefix_len;

        LOG_MSG_INFO1("IP6 dst addr is", 0, 0, 0);
        IPV6_ADDR_MSG(firewall_entry->filter_spec.ip_hdr.v6.dst.addr.in6_u.u6_addr64);

        LOG_MSG_INFO1("IPV6 dst prefix length %d ",
                      add_extd_firewall_config_req_msg_v01.ip6_dst_addr.prefix_len, 0, 0);

      }

      if(firewall_entry->filter_spec.ip_hdr.v6.field_mask & IPFLTR_MASK_IP6_TRAFFIC_CLASS)
      {
        add_extd_firewall_config_req_msg_v01.ip6_trf_cls_valid = TRUE;
        add_extd_firewall_config_req_msg_v01.ip6_trf_cls.value =
                                        firewall_entry->filter_spec.ip_hdr.v6.trf_cls.val;
        add_extd_firewall_config_req_msg_v01.ip6_trf_cls.mask =
                                       firewall_entry->filter_spec.ip_hdr.v6.trf_cls.mask;
        LOG_MSG_INFO1("IPV6 traffic class value %d mask %d",
                      add_extd_firewall_config_req_msg_v01.ip6_trf_cls.value ,
                      add_extd_firewall_config_req_msg_v01.ip6_trf_cls.mask, 0);

      }

      break;

    default:
      LOG_MSG_INFO1("Unsupported protocol %d ",
                    firewall_entry->filter_spec.ip_vsn, 0, 0);
      return QCMAP_CM_ERROR;

  }

  LOG_MSG_INFO1("Next header protocol is %d ",
                next_hdr_prot, 0, 0);

  switch(next_hdr_prot)
  {
    case PS_IPPROTO_TCP:
      add_extd_firewall_config_req_msg_v01.next_hdr_prot =
                                             QCMAP_EXTD_FIREWALL_PROTO_TCP_V01;
      LOG_MSG_INFO1("TCP protocol %d ",
                add_extd_firewall_config_req_msg_v01.next_hdr_prot, 0, 0);
      if(firewall_entry->filter_spec.next_prot_hdr.tcp.field_mask &
                                                     IPFLTR_MASK_TCP_SRC_PORT )
      {
        add_extd_firewall_config_req_msg_v01.tcp_udp_src_valid = TRUE;
        add_extd_firewall_config_req_msg_v01.tcp_udp_src.port =
                         firewall_entry->filter_spec.next_prot_hdr.tcp.src.port;
        add_extd_firewall_config_req_msg_v01.tcp_udp_src.range =
                        firewall_entry->filter_spec.next_prot_hdr.tcp.src.range;
        LOG_MSG_INFO1("TCP protocol src port %d src range %d",
                        add_extd_firewall_config_req_msg_v01.tcp_udp_src.port,
                        add_extd_firewall_config_req_msg_v01.tcp_udp_src.range,
                        0);
      }

      if(firewall_entry->filter_spec.next_prot_hdr.tcp.field_mask &
                                                      IPFLTR_MASK_TCP_DST_PORT )
      {
        add_extd_firewall_config_req_msg_v01.tcp_udp_dst_valid = TRUE;
        add_extd_firewall_config_req_msg_v01.tcp_udp_dst.port =
                         firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port;
        add_extd_firewall_config_req_msg_v01.tcp_udp_dst.range =
                        firewall_entry->filter_spec.next_prot_hdr.tcp.dst.range;
        LOG_MSG_INFO1("TCP protocol dst port %d dst range %d",
                      add_extd_firewall_config_req_msg_v01.tcp_udp_dst.port,
                      add_extd_firewall_config_req_msg_v01.tcp_udp_dst.range,
                      0);
      }
      break;

    case PS_IPPROTO_UDP:
      add_extd_firewall_config_req_msg_v01.next_hdr_prot =
                                            QCMAP_EXTD_FIREWALL_PROTO_UDP_V01;
      LOG_MSG_INFO1("UDP protocol %d ",
                      add_extd_firewall_config_req_msg_v01.next_hdr_prot, 0, 0);
      if(firewall_entry->filter_spec.next_prot_hdr.udp.field_mask &
                                                     IPFLTR_MASK_UDP_SRC_PORT )
      {
        add_extd_firewall_config_req_msg_v01.tcp_udp_src_valid = TRUE;
        add_extd_firewall_config_req_msg_v01.tcp_udp_src.port =
                        firewall_entry->filter_spec.next_prot_hdr.udp.src.port;
        add_extd_firewall_config_req_msg_v01.tcp_udp_src.range =
                       firewall_entry->filter_spec.next_prot_hdr.udp.src.range;
        LOG_MSG_INFO1("UDP protocol src port %d src range %d",
                      add_extd_firewall_config_req_msg_v01.tcp_udp_src.port,
                      add_extd_firewall_config_req_msg_v01.tcp_udp_src.range, 0);
      }

      if(firewall_entry->filter_spec.next_prot_hdr.udp.field_mask &
                                                      IPFLTR_MASK_UDP_DST_PORT )
      {
        add_extd_firewall_config_req_msg_v01.tcp_udp_dst_valid = TRUE;
        add_extd_firewall_config_req_msg_v01.tcp_udp_dst.port =
                         firewall_entry->filter_spec.next_prot_hdr.udp.dst.port;
        add_extd_firewall_config_req_msg_v01.tcp_udp_dst.range =
                        firewall_entry->filter_spec.next_prot_hdr.udp.dst.range;
        LOG_MSG_INFO1("UDP protocol dst port %d dst range %d",
                      add_extd_firewall_config_req_msg_v01.tcp_udp_dst.port,
                      add_extd_firewall_config_req_msg_v01.tcp_udp_dst.range, 0);
      }
      break;

    case PS_IPPROTO_ICMP:
    case PS_IPPROTO_ICMP6:
      if(next_hdr_prot == PS_IPPROTO_ICMP)
      {
        add_extd_firewall_config_req_msg_v01.next_hdr_prot =
                                              QCMAP_EXTD_FIREWALL_PROTO_ICMP_V01;
      }
      else
      {
        add_extd_firewall_config_req_msg_v01.next_hdr_prot=
                                               QCMAP_EXTD_FIREWALL_PROTO_ICMP6_V01;
      }
      LOG_MSG_INFO1("ICMP protocol %d ",
                add_extd_firewall_config_req_msg_v01.next_hdr_prot, 0, 0);
      if(firewall_entry->filter_spec.next_prot_hdr.icmp.field_mask &
                                                      IPFLTR_MASK_ICMP_MSG_CODE )
      {
        add_extd_firewall_config_req_msg_v01.icmp_code_valid = TRUE;
        add_extd_firewall_config_req_msg_v01.icmp_code =
                             firewall_entry->filter_spec.next_prot_hdr.icmp.code;
        LOG_MSG_INFO1("ICMP protocol code %d",
                add_extd_firewall_config_req_msg_v01.icmp_code, 0, 0);
      }

      if(firewall_entry->filter_spec.next_prot_hdr.icmp.field_mask &
                                                      IPFLTR_MASK_ICMP_MSG_TYPE )
      {
        add_extd_firewall_config_req_msg_v01.icmp_type_valid = TRUE;
        add_extd_firewall_config_req_msg_v01.icmp_type =
                             firewall_entry->filter_spec.next_prot_hdr.icmp.type;
        LOG_MSG_INFO1("ICMP protocol type %d",
                add_extd_firewall_config_req_msg_v01.icmp_type, 0, 0);
      }
      break;

    case PS_IPPROTO_ESP:
      add_extd_firewall_config_req_msg_v01.next_hdr_prot =
                                               QCMAP_EXTD_FIREWALL_PROTO_ESP_V01;
      LOG_MSG_INFO1("ESP protocol %d ",
                add_extd_firewall_config_req_msg_v01.next_hdr_prot, 0, 0);
      if(firewall_entry->filter_spec.next_prot_hdr.esp.field_mask & IPFLTR_MASK_ESP_SPI )
      {
        add_extd_firewall_config_req_msg_v01.esp_spi_valid = TRUE;
        add_extd_firewall_config_req_msg_v01.esp_spi =
                               firewall_entry->filter_spec.next_prot_hdr.esp.spi;
        LOG_MSG_INFO1("ESP protocol spi %d",
                add_extd_firewall_config_req_msg_v01.esp_spi, 0, 0);
      }
      break;

    case PS_IPPROTO_TCP_UDP:
      add_extd_firewall_config_req_msg_v01.next_hdr_prot =
                                           QCMAP_EXTD_FIREWALL_PROTO_TCP_UDP_V01;
      LOG_MSG_INFO1("TCP_UDP protocol %d ",
                add_extd_firewall_config_req_msg_v01.next_hdr_prot, 0, 0);
      if(firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask &
                                                     IPFLTR_MASK_TCP_UDP_SRC_PORT )
      {
        add_extd_firewall_config_req_msg_v01.tcp_udp_src_valid = TRUE;
        add_extd_firewall_config_req_msg_v01.tcp_udp_src.port =
             firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.src.port;
        add_extd_firewall_config_req_msg_v01.tcp_udp_src.range =
            firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.src.range;
        LOG_MSG_INFO1("TCP_UDP protocol src port %d src range",
                      add_extd_firewall_config_req_msg_v01.tcp_udp_src.port,
                      add_extd_firewall_config_req_msg_v01.tcp_udp_src.range, 0);
      }

      if(firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask &
                                                     IPFLTR_MASK_TCP_UDP_DST_PORT )
      {
        add_extd_firewall_config_req_msg_v01.tcp_udp_dst_valid = TRUE;
        add_extd_firewall_config_req_msg_v01.tcp_udp_dst.port =
             firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.dst.port;
        add_extd_firewall_config_req_msg_v01.tcp_udp_dst.range =
            firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.dst.range;
        LOG_MSG_INFO1("TCP_UDP protocol dst port %d dst range",
                      add_extd_firewall_config_req_msg_v01.tcp_udp_src.port,
                      add_extd_firewall_config_req_msg_v01.tcp_udp_src.range, 0);
      }
      break;

    default:
      LOG_MSG_ERROR("Unsupported protocol %d ",next_hdr_prot, 0, 0);
      return QCMAP_CM_ERROR;

  }


  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_ADD_EXTD_FIREWALL_CONFIG_REQ_V01,
                                       &add_extd_firewall_config_req_msg_v01,
                                       sizeof(add_extd_firewall_config_req_msg_v01),
                                       &add_extd_firewall_config_resp_msg_v01,
                                       sizeof(add_extd_firewall_config_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
  if ((qmi_error == QMI_NO_ERR) &&
      (add_extd_firewall_config_resp_msg_v01.resp.result == QMI_NO_ERR) &&
      (add_extd_firewall_config_resp_msg_v01.firewall_handle_valid == TRUE))
  {
    firewall_entry->firewall_handle =
                              add_extd_firewall_config_resp_msg_v01.firewall_handle;
  }
  else
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Add firewall config failed %d %d", qmi_error,
                   add_extd_firewall_config_resp_msg_v01.resp.error, 0);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_EXTD_FIREWALL_OP_GET_HANDLE_LIST()

DESCRIPTION

  Gets all the  firewall handles from the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the firewall entry could be retrieved, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.


SIDE EFFECTS

===========================================================================*/
static int qcmap_cm_extd_firewall_op_get_handle_list
(
  qcmap_cm_get_extd_firewall_handle_list_conf_t *extd_firewall_handle_list, /*Handle list configuration*/
  int    *qcmap_cm_errno                                                    /*error value              */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  int handle_list_len =0;
  int index =0;

  ds_assert(qcmap_cm_errno != NULL);
  ds_assert(extd_firewall_handle_list != NULL);

  qcmap_get_firewall_config_handle_list_req_msg_v01 get_firewall_config_handle_list_req_msg_v01;
  qcmap_get_firewall_config_handle_list_resp_msg_v01 get_firewall_config_handle_list_resp_msg_v01;

  qcmap_get_extd_firewall_config_req_msg_v01 get_extd_firewall_config_req_msg_v01;
  qcmap_get_extd_firewall_config_resp_msg_v01 get_extd_firewall_config_resp_msg_v01;

  if(extd_firewall_handle_list->ip_family == QCMAP_IP_V4_V01)
  {
    if(qcmap_cm_cb.mobile_ap_handle != QCMAP_WAN_INVALID_QCMAP_HANDLE )
    {

      LOG_MSG_INFO1("Getting handle list  for IPv4 ",
                  0, 0, 0);

      get_firewall_config_handle_list_req_msg_v01.mobile_ap_handle =
                                                       qcmap_cm_cb.mobile_ap_handle;

      qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                           QMI_QCMAP_GET_FIREWALL_CONFIG_HANDLE_LIST_REQ_V01,
                                           &get_firewall_config_handle_list_req_msg_v01,
                                           sizeof(get_firewall_config_handle_list_req_msg_v01),
                                           &get_firewall_config_handle_list_resp_msg_v01,
                                           sizeof(get_firewall_config_handle_list_resp_msg_v01),
                                           QCMAP_CM_QMI_TIMEOUT_VALUE);
     }
    else
    {
      LOG_MSG_ERROR("Mobile AP V4 handle not valid", 0, 0, 0);
      return QCMAP_CM_ERROR;
    }
  }
  else if(extd_firewall_handle_list->ip_family == QCMAP_IP_V6_V01)
  {
    if(qcmap_cm_cb.ipv6_mobile_ap_handle != QCMAP_WAN_INVALID_QCMAP_HANDLE )
    {

      LOG_MSG_INFO1("Getting handle list for IPv6 ",0, 0, 0);

      get_firewall_config_handle_list_req_msg_v01.mobile_ap_handle =
                                                       qcmap_cm_cb.ipv6_mobile_ap_handle;

      qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                           QMI_QCMAP_GET_FIREWALL_CONFIG_HANDLE_LIST_REQ_V01,
                                           &get_firewall_config_handle_list_req_msg_v01,
                                           sizeof(get_firewall_config_handle_list_req_msg_v01),
                                           &get_firewall_config_handle_list_resp_msg_v01,
                                           sizeof(get_firewall_config_handle_list_resp_msg_v01),
                                           QCMAP_CM_QMI_TIMEOUT_VALUE);
    }
    else
    {
      LOG_MSG_ERROR("Mobile AP V6 handle not valid", 0, 0, 0);
      return QCMAP_CM_ERROR;
    }
  }
  else
  {
    LOG_MSG_ERROR("Invalid IP family type", 0, 0, 0);
    return QCMAP_CM_ERROR;
  }

 if((qmi_error == QMI_NO_ERR) &&
    (get_firewall_config_handle_list_resp_msg_v01.resp.result == QMI_NO_ERR) &&
    (get_firewall_config_handle_list_resp_msg_v01.firewall_handle_list_valid == TRUE))
 {
   if (get_firewall_config_handle_list_resp_msg_v01.firewall_handle_list_len > 0)
   {
     LOG_MSG_INFO1("handle list len %d ",
                   get_firewall_config_handle_list_resp_msg_v01.firewall_handle_list_len , 0, 0);

      handle_list_len =
        get_firewall_config_handle_list_resp_msg_v01.firewall_handle_list_len;

      while (index < handle_list_len)
      {
         extd_firewall_handle_list->handle_list[index] =
         get_firewall_config_handle_list_resp_msg_v01.firewall_handle_list[index];
         LOG_MSG_INFO1("Firewall handle %d ",
                          extd_firewall_handle_list->handle_list[index], 0, 0);
         index++;
      }
    }
    else
    {
       LOG_MSG_INFO1("No handles received ",0, 0, 0);
    }
  }
  else
  {
    LOG_MSG_INFO1("Handle list not valid", 0, 0, 0);
    ret = QCMAP_CM_ERROR;
  }

  if(ret == QCMAP_CM_SUCCESS)
  {
    extd_firewall_handle_list->num_of_entries = handle_list_len;
    LOG_MSG_INFO1("Number of firewall handles is %d",extd_firewall_handle_list->num_of_entries,0,0);
    return ret;
  }

}

/*===========================================================================

FUNCTION QCMAP_CM_EXTD_FIREWALL_ENTRY_OP_GET()

DESCRIPTION

  Gets a extended firewall entry from the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the firewall entry could be retrieved, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

SIDE EFFECTS

===========================================================================*/
static int qcmap_cm_extd_firewall_entry_op_get
(
  qcmap_cm_extd_firewall_entry_conf_t *firewall_entry, /* Firewall Entry   */
  int    *qcmap_cm_errno                               /* error value      */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_get_firewall_config_handle_list_req_msg_v01 get_firewall_config_handle_list_req_msg_v01;
  qcmap_get_firewall_config_handle_list_resp_msg_v01 get_firewall_config_handle_list_resp_msg_v01;
  qcmap_get_extd_firewall_config_req_msg_v01 get_extd_firewall_config_req_msg_v01;
  qcmap_get_extd_firewall_config_resp_msg_v01 get_extd_firewall_config_resp_msg_v01;
  uint32 current_handle;
  uint8 next_hdr_prot;

  ds_assert(qcmap_cm_errno != NULL);
  ds_assert(firewall_entry != NULL);

  if(firewall_entry->filter_spec.ip_vsn == QCMAP_IP_V4_V01)
  {
    if(qcmap_cm_cb.mobile_ap_handle != QCMAP_WAN_INVALID_QCMAP_HANDLE)
    {
      get_extd_firewall_config_req_msg_v01.mobile_ap_handle =
                                                  qcmap_cm_cb.mobile_ap_handle;
      get_extd_firewall_config_req_msg_v01.firewall_handle =
                                              firewall_entry->firewall_handle;
      LOG_MSG_INFO1("Getting IP4 firewall rule for handle %d ",
                        get_extd_firewall_config_req_msg_v01.firewall_handle, 0, 0);

      qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                           QMI_QCMAP_GET_EXTD_FIREWALL_CONFIG_REQ_V01,
                                           &get_extd_firewall_config_req_msg_v01,
                                           sizeof(get_extd_firewall_config_req_msg_v01),
                                           &get_extd_firewall_config_resp_msg_v01,
                                           sizeof(get_extd_firewall_config_resp_msg_v01),
                                           QCMAP_CM_QMI_TIMEOUT_VALUE);
    }
    else
    {
      LOG_MSG_ERROR("Mobile AP handle not valid", 0, 0, 0);
      return QCMAP_CM_ERROR;
    }

  }
  else if(firewall_entry->filter_spec.ip_vsn == QCMAP_IP_V6_V01)
  {
    if(qcmap_cm_cb.ipv6_mobile_ap_handle != QCMAP_WAN_INVALID_QCMAP_HANDLE)
    {
      get_extd_firewall_config_req_msg_v01.mobile_ap_handle =
                                                  qcmap_cm_cb.ipv6_mobile_ap_handle;
      get_extd_firewall_config_req_msg_v01.firewall_handle =
                                              firewall_entry->firewall_handle;
      LOG_MSG_INFO1("Getting IP6 firewall rule for handle %d ",
                        get_extd_firewall_config_req_msg_v01.firewall_handle, 0, 0);

      qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                           QMI_QCMAP_GET_EXTD_FIREWALL_CONFIG_REQ_V01,
                                           &get_extd_firewall_config_req_msg_v01,
                                           sizeof(get_extd_firewall_config_req_msg_v01),
                                           &get_extd_firewall_config_resp_msg_v01,
                                           sizeof(get_extd_firewall_config_resp_msg_v01),
                                           QCMAP_CM_QMI_TIMEOUT_VALUE);
    }
    else
    {
      LOG_MSG_ERROR("Mobile AP handle not valid", 0, 0, 0);
      return QCMAP_CM_ERROR;
    }

  }

  if ((qmi_error == QMI_NO_ERR) &&
      (get_extd_firewall_config_resp_msg_v01.resp.result == QMI_NO_ERR))
  {
    LOG_MSG_INFO1("IP family type = %d", firewall_entry->filter_spec.ip_vsn, 0, 0);
    switch(firewall_entry->filter_spec.ip_vsn)
    {
      case QCMAP_IP_V4_V01:
        if(get_extd_firewall_config_resp_msg_v01.ip4_src_addr_valid)
        {
           firewall_entry->filter_spec.ip_hdr.v4.field_mask |=
                                               IPFLTR_MASK_IP4_SRC_ADDR;
           firewall_entry->filter_spec.ip_hdr.v4.src.addr.ps_s_addr =
                    get_extd_firewall_config_resp_msg_v01.ip4_src_addr.addr;
           firewall_entry->filter_spec.ip_hdr.v4.src.subnet_mask.ps_s_addr =
                    get_extd_firewall_config_resp_msg_v01.ip4_src_addr.subnet_mask;

           LOG_MSG_INFO1("IPV4 src addr is:", 0, 0, 0);
           IPV4_ADDR_MSG(firewall_entry->filter_spec.ip_hdr.v4.src.addr.ps_s_addr);

           LOG_MSG_INFO1("IPV4 src subnet mask", 0, 0, 0);
           IPV4_ADDR_MSG(firewall_entry->filter_spec.ip_hdr.v4.src.subnet_mask.ps_s_addr);
        }

        if(get_extd_firewall_config_resp_msg_v01.ip4_dst_addr_valid)
        {
           firewall_entry->filter_spec.ip_hdr.v4.field_mask |=
                                                    IPFLTR_MASK_IP4_DST_ADDR;
           firewall_entry->filter_spec.ip_hdr.v4.dst.addr.ps_s_addr =
                        get_extd_firewall_config_resp_msg_v01.ip4_dst_addr.addr;
           firewall_entry->filter_spec.ip_hdr.v4.dst.subnet_mask.ps_s_addr =
                  get_extd_firewall_config_resp_msg_v01.ip4_dst_addr.subnet_mask;

            LOG_MSG_INFO1("IPV4 dst addr is:", 0, 0, 0);
            IPV4_ADDR_MSG(firewall_entry->filter_spec.ip_hdr.v4.dst.addr.ps_s_addr);

            LOG_MSG_INFO1("IPV4 dst subnet mask", 0, 0, 0);
            IPV4_ADDR_MSG(firewall_entry->filter_spec.ip_hdr.v4.dst.subnet_mask.ps_s_addr);
         }

         if(get_extd_firewall_config_resp_msg_v01.ip4_tos_valid)
         {
             firewall_entry->filter_spec.ip_hdr.v4.field_mask |=
                                                        IPFLTR_MASK_IP4_TOS;
             firewall_entry->filter_spec.ip_hdr.v4.tos.val =
                             get_extd_firewall_config_resp_msg_v01.ip4_tos.value;
             firewall_entry->filter_spec.ip_hdr.v4.tos.mask =
                               get_extd_firewall_config_resp_msg_v01.ip4_tos.mask;
             LOG_MSG_INFO1("tos val %d mask %d",
                           firewall_entry->filter_spec.ip_hdr.v4.tos.val,
                           firewall_entry->filter_spec.ip_hdr.v4.tos.mask, 0);
          }
          break;

      case QCMAP_IP_V6_V01:

        if(get_extd_firewall_config_resp_msg_v01.ip6_src_addr_valid)
        {
          firewall_entry->filter_spec.ip_hdr.v6.field_mask |= IPFLTR_MASK_IP6_SRC_ADDR;
          memcpy(firewall_entry->filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr8,
                 get_extd_firewall_config_resp_msg_v01.ip6_src_addr.addr,
                 QCMAP_IPV6_ADDR_LEN_V01*sizeof(uint8));

          firewall_entry->filter_spec.ip_hdr.v6.src.prefix_len =
            get_extd_firewall_config_resp_msg_v01.ip6_src_addr.prefix_len;
          LOG_MSG_INFO1("IPV6 src addr is:", 0, 0, 0);
          IPV6_ADDR_MSG(firewall_entry->filter_spec.ip_hdr.v6.src.addr.in6_u.u6_addr64);
          LOG_MSG_INFO1("IPV6 src prefix length %d ",
                firewall_entry->filter_spec.ip_hdr.v6.src.prefix_len, 0, 0);
        }

        if(get_extd_firewall_config_resp_msg_v01.ip6_dst_addr_valid)
        {
          firewall_entry->filter_spec.ip_hdr.v6.field_mask |= IPFLTR_MASK_IP6_DST_ADDR;

          memcpy(firewall_entry->filter_spec.ip_hdr.v6.dst.addr.in6_u.u6_addr8,
                 get_extd_firewall_config_resp_msg_v01.ip6_dst_addr.addr,
                 QCMAP_IPV6_ADDR_LEN_V01*sizeof(uint8));

          firewall_entry->filter_spec.ip_hdr.v6.dst.prefix_len = get_extd_firewall_config_resp_msg_v01.ip6_dst_addr.prefix_len;

          LOG_MSG_INFO1("IPV6 dst addr is:", 0, 0, 0);
          IPV6_ADDR_MSG(firewall_entry->filter_spec.ip_hdr.v6.dst.addr.in6_u.u6_addr64);

          LOG_MSG_INFO1("IPV6 src prefix length %d ",
                firewall_entry->filter_spec.ip_hdr.v6.dst.prefix_len, 0, 0);
        }

        if(get_extd_firewall_config_resp_msg_v01.ip6_trf_cls_valid)
        {
          firewall_entry->filter_spec.ip_hdr.v6.field_mask |= IPFLTR_MASK_IP6_TRAFFIC_CLASS;
          firewall_entry->filter_spec.ip_hdr.v6.trf_cls.val = get_extd_firewall_config_resp_msg_v01.ip6_trf_cls.value;
          firewall_entry->filter_spec.ip_hdr.v6.trf_cls.mask = get_extd_firewall_config_resp_msg_v01.ip6_trf_cls.mask;
          LOG_MSG_INFO1("ip6 trf class value %d mask %d",
                firewall_entry->filter_spec.ip_hdr.v6.trf_cls.val, firewall_entry->filter_spec.ip_hdr.v6.trf_cls.mask, 0);
        }
        break;

      default:
        LOG_MSG_ERROR("BAD IP protocol", 0, 0, 0);
        return QCMAP_CM_ERROR;
    }

    if(get_extd_firewall_config_resp_msg_v01.next_hdr_prot_valid)
    {
      LOG_MSG_INFO1("Next header protocol is %d ",
                    get_extd_firewall_config_resp_msg_v01.next_hdr_prot, 0, 0);

      switch(get_extd_firewall_config_resp_msg_v01.next_hdr_prot)
      {
         case QCMAP_EXTD_FIREWALL_PROTO_TCP_V01:
           next_hdr_prot = PS_IPPROTO_TCP;
           if(get_extd_firewall_config_resp_msg_v01.tcp_udp_src_valid)
           {
              firewall_entry->filter_spec.next_prot_hdr.tcp.field_mask |=
                                                   IPFLTR_MASK_TCP_SRC_PORT;
              firewall_entry->filter_spec.next_prot_hdr.tcp.src.port =
                        get_extd_firewall_config_resp_msg_v01.tcp_udp_src.port;
              firewall_entry->filter_spec.next_prot_hdr.tcp.src.range =
                        get_extd_firewall_config_resp_msg_v01.tcp_udp_src.range;
              LOG_MSG_INFO1("TCP src port %d range %d",
                            firewall_entry->filter_spec.next_prot_hdr.tcp.src.port,
                            firewall_entry->filter_spec.next_prot_hdr.tcp.src.range,
                            0);
           }

           if(get_extd_firewall_config_resp_msg_v01.tcp_udp_dst_valid)
           {
              firewall_entry->filter_spec.next_prot_hdr.tcp.field_mask |=
                                                    IPFLTR_MASK_TCP_DST_PORT;
              firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port =
                       get_extd_firewall_config_resp_msg_v01.tcp_udp_dst.port;
              firewall_entry->filter_spec.next_prot_hdr.tcp.dst.range =
                      get_extd_firewall_config_resp_msg_v01.tcp_udp_dst.range;
              LOG_MSG_INFO1("TCP dst port %d range %d",
                            firewall_entry->filter_spec.next_prot_hdr.tcp.dst.port,
                            firewall_entry->filter_spec.next_prot_hdr.tcp.dst.range,
                            0);
            }
            break;

         case QCMAP_EXTD_FIREWALL_PROTO_UDP_V01:
           next_hdr_prot = PS_IPPROTO_UDP ;
           if(get_extd_firewall_config_resp_msg_v01.tcp_udp_src_valid)
           {
              firewall_entry->filter_spec.next_prot_hdr.udp.field_mask |=
                                                    IPFLTR_MASK_UDP_SRC_PORT;
              firewall_entry->filter_spec.next_prot_hdr.udp.src.port =
                       get_extd_firewall_config_resp_msg_v01.tcp_udp_src.port;
              firewall_entry->filter_spec.next_prot_hdr.udp.src.range =
                         get_extd_firewall_config_resp_msg_v01.tcp_udp_src.range;
              LOG_MSG_INFO1("UDP src port %d range %d",
                            firewall_entry->filter_spec.next_prot_hdr.udp.src.port,
                            firewall_entry->filter_spec.next_prot_hdr.udp.src.range,
                            0);
           }

           if(get_extd_firewall_config_resp_msg_v01.tcp_udp_dst_valid)
           {
               firewall_entry->filter_spec.next_prot_hdr.udp.field_mask |=
                                                      IPFLTR_MASK_UDP_DST_PORT;
               firewall_entry->filter_spec.next_prot_hdr.udp.dst.port =
                            get_extd_firewall_config_resp_msg_v01.tcp_udp_dst.port;
               firewall_entry->filter_spec.next_prot_hdr.udp.dst.range =
                           get_extd_firewall_config_resp_msg_v01.tcp_udp_dst.range;
               LOG_MSG_INFO1("UDP dst port %d range %d",
                              firewall_entry->filter_spec.next_prot_hdr.udp.dst.port,
                              firewall_entry->filter_spec.next_prot_hdr.udp.dst.range,
                              0);
           }
           break;

         case QCMAP_EXTD_FIREWALL_PROTO_ICMP_V01:
         case QCMAP_EXTD_FIREWALL_PROTO_ICMP6_V01:
           if(get_extd_firewall_config_resp_msg_v01.next_hdr_prot ==
                                   QCMAP_EXTD_FIREWALL_PROTO_ICMP_V01 )
           {
             next_hdr_prot = PS_IPPROTO_ICMP ;
           }
           else
           {
             next_hdr_prot = PS_IPPROTO_ICMP6;
           }
         
           if(get_extd_firewall_config_resp_msg_v01.icmp_code_valid)
           {
               firewall_entry->filter_spec.next_prot_hdr.icmp.field_mask |=
                                                  IPFLTR_MASK_ICMP_MSG_CODE;
               firewall_entry->filter_spec.next_prot_hdr.icmp.code =
                              get_extd_firewall_config_resp_msg_v01.icmp_code;
               LOG_MSG_INFO1("ICMP code %d",
                             firewall_entry->filter_spec.next_prot_hdr.icmp.code, 0, 0);
           }

           if(get_extd_firewall_config_resp_msg_v01.icmp_type_valid)
           {
               firewall_entry->filter_spec.next_prot_hdr.icmp.field_mask |=
                                                     IPFLTR_MASK_ICMP_MSG_TYPE;
               firewall_entry->filter_spec.next_prot_hdr.icmp.type =
                               get_extd_firewall_config_resp_msg_v01.icmp_type;
               LOG_MSG_INFO1("ICMP type %d",
                             firewall_entry->filter_spec.next_prot_hdr.icmp.type, 0, 0);
           }
           break;

         case QCMAP_EXTD_FIREWALL_PROTO_ESP_V01:
           next_hdr_prot = PS_IPPROTO_ESP;
           if(get_extd_firewall_config_resp_msg_v01.esp_spi_valid)
           {
               firewall_entry->filter_spec.next_prot_hdr.esp.field_mask |=
                                                       IPFLTR_MASK_ESP_SPI;
               firewall_entry->filter_spec.next_prot_hdr.esp.spi =
                                get_extd_firewall_config_resp_msg_v01.esp_spi;
               LOG_MSG_INFO1("ESP SPI %d",
                             firewall_entry->filter_spec.next_prot_hdr.esp.spi, 0, 0);
           }
           break;

         case QCMAP_EXTD_FIREWALL_PROTO_TCP_UDP_V01:
           next_hdr_prot = PS_IPPROTO_TCP_UDP;
           if(get_extd_firewall_config_resp_msg_v01.tcp_udp_src_valid)
           {
                firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask |=
                                                           IPFLTR_MASK_TCP_UDP_SRC_PORT;
                firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.src.port =
                                   get_extd_firewall_config_resp_msg_v01.tcp_udp_src.port;
                firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.src.range =
                                  get_extd_firewall_config_resp_msg_v01.tcp_udp_src.range;
                LOG_MSG_INFO1("TCP_UDP src port %d range %d",
                              firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.src.port,
                              firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.src.range,
                              0);
           }

           if(get_extd_firewall_config_resp_msg_v01.tcp_udp_dst_valid)
           {
                firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.field_mask |=
                                                               IPFLTR_MASK_TCP_UDP_DST_PORT;
                firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.dst.port =
                                          get_extd_firewall_config_resp_msg_v01.tcp_udp_dst.port;
                firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.dst.range =
                                     get_extd_firewall_config_resp_msg_v01.tcp_udp_dst.range;
                LOG_MSG_INFO1("TCP_UDP dst port %d range %d",
                              firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.dst.port,
                              firewall_entry->filter_spec.next_prot_hdr.tcp_udp_port_range.dst.range,
                              0);
           }
           break;

           default:
             LOG_MSG_INFO1("Bad next header protocol %d",
                           get_extd_firewall_config_resp_msg_v01.next_hdr_prot , 0, 0);
             next_hdr_prot = PS_NO_NEXT_HDR;
        }
 
        LOG_MSG_INFO1("Next header protocol is %d ",
                next_hdr_prot, 0, 0);

        if(firewall_entry->filter_spec.ip_vsn == QCMAP_IP_V4_V01)
        {
            firewall_entry->filter_spec.ip_hdr.v4.field_mask |= IPFLTR_MASK_IP4_NEXT_HDR_PROT;
            firewall_entry->filter_spec.ip_hdr.v4.next_hdr_prot = next_hdr_prot;
        }
        else if(firewall_entry->filter_spec.ip_vsn = QCMAP_IP_V6_V01)
        {
            firewall_entry->filter_spec.ip_hdr.v6.field_mask |= IPFLTR_MASK_IP6_NEXT_HDR_PROT;
            firewall_entry->filter_spec.ip_hdr.v6.next_hdr_prot = next_hdr_prot;
        }
       }
     }
     else
     {
        *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
         LOG_MSG_ERROR( "Get extd firewall config failed %d %d for handle %d",
                        qmi_error, get_extd_firewall_config_resp_msg_v01.resp.error,
                        get_extd_firewall_config_req_msg_v01.firewall_handle);
      }

     return ret;

}

/*===========================================================================

FUNCTION QCMAP_CM_EXTD_FIREWALL_ENTRY_OP_DELETE()

DESCRIPTION

  Deletes an extended firewall entry from the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the firewall entry could be retrieved, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

SIDE EFFECTS

=============================================================================*/
static int qcmap_cm_extd_firewall_entry_op_delete
(
  qcmap_cm_extd_firewall_handle_conf_t *extd_firewall_handle,/* Firewall Entry*/
  int    *qcmap_cm_errno                                     /* error value   */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_delete_firewall_config_req_msg_v01 delete_firewall_config_req_msg_v01;
  qcmap_delete_firewall_config_resp_msg_v01 delete_firewall_config_resp_msg_v01;

  ds_assert(qcmap_cm_errno != NULL);
  ds_assert(extd_firewall_handle !=NULL);

  if(extd_firewall_handle->ip_family == QCMAP_IP_V4_V01)
  {
    if(qcmap_cm_cb.mobile_ap_handle != QCMAP_WAN_INVALID_QCMAP_HANDLE )
    {
      delete_firewall_config_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;
      delete_firewall_config_req_msg_v01.firewall_handle = extd_firewall_handle->handle;

      qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_DELETE_FIREWALL_CONFIG_REQ_V01,
                                       &delete_firewall_config_req_msg_v01,
                                       sizeof(delete_firewall_config_req_msg_v01),
                                       &delete_firewall_config_resp_msg_v01,
                                       sizeof(delete_firewall_config_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
    }
    else
    {
      LOG_MSG_ERROR("Invalid mobile AP handle for IP family %d",
                    extd_firewall_handle->ip_family, 0, 0);
      return QCMAP_CM_ERROR;
    }
  }
  else if(extd_firewall_handle->ip_family == QCMAP_IP_V6_V01)
  {
    if(qcmap_cm_cb.ipv6_mobile_ap_handle != QCMAP_WAN_INVALID_QCMAP_HANDLE )
    {
      delete_firewall_config_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.ipv6_mobile_ap_handle;
      delete_firewall_config_req_msg_v01.firewall_handle = extd_firewall_handle->handle;

      qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_DELETE_FIREWALL_CONFIG_REQ_V01,
                                       &delete_firewall_config_req_msg_v01,
                                       sizeof(delete_firewall_config_req_msg_v01),
                                       &delete_firewall_config_resp_msg_v01,
                                       sizeof(delete_firewall_config_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);
    }
    else
    {
      LOG_MSG_ERROR("Invalid mobile AP handle for IP family %d",
                    extd_firewall_handle->ip_family, 0, 0);
      return QCMAP_CM_ERROR;
    }
  }
  else
  {
    LOG_MSG_ERROR("Invalid IP family type %d",
                  extd_firewall_handle->ip_family, 0, 0);
    return QCMAP_CM_ERROR;
  }
  if ((qmi_error != QMI_NO_ERR) ||
      (delete_firewall_config_resp_msg_v01.resp.result != QMI_NO_ERR))
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Delete firewall rule failed %d %d",
                   qmi_error, delete_firewall_config_resp_msg_v01.resp.error, 0);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_FIREWALL_ENTRY_OP_DELETE_ALL()

DESCRIPTION

  Deletes a firewall entry in the QCMAP CM configuration.

DEPENDENCIES
  None.

RETURN VALUE
  If the firewall entry could be deleted, QCMAP_CM_SUCCESS,
  else, QCMAP_CM_ERROR.

  qcmap_cm_errno Values
  ----------------
  QCMAP_CM_EOPNOTSUPP        The entry could not be deleted.


SIDE EFFECTS

===========================================================================*/
static int
qcmap_cm_firewall_entry_op_delete_all
(
  int                            *qcmap_cm_errno        /* Error condition */
)
{
  int ret = QCMAP_CM_SUCCESS, i;
  qcmap_cm_extd_firewall_handle_conf_t extd_firewall_handle;

  for (i = 0; i< qcmap_cm_cb.cfg->nat_config.num_extd_firewall_entries; i++)
  {
    memset(&extd_firewall_handle, 0, sizeof(qcmap_cm_extd_firewall_handle_conf_t));
    if(qcmap_cm_cb.cfg->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_vsn == IP_V4)
    {
      extd_firewall_handle.handle = qcmap_cm_cb.cfg->nat_config.extd_firewall_entries[i].extd_firewall_entry.firewall_handle;
      extd_firewall_handle.ip_family = IP_V4;
    }
    else if(qcmap_cm_cb.cfg->nat_config.extd_firewall_entries[i].extd_firewall_entry.filter_spec.ip_vsn == IP_V6)
    {
      extd_firewall_handle.handle = qcmap_cm_cb.cfg->nat_config.extd_firewall_entries[i].extd_firewall_entry.firewall_handle;
      extd_firewall_handle.ip_family = IP_V6;
    }

    ret = qcmap_cm_extd_firewall_entry_op_delete(&extd_firewall_handle, qcmap_cm_errno);
  }

  return ret;
}

/*===========================================================================


FUNCTION QCMAP_CM_EXTD_FIREWALL_OP()

DESCRIPTION
  This Function  performs add/get/delete extended firewall entry and
  get firewall handle list

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

=============================================================================*/
int qcmap_cm_extd_firewall_op
(
  int    qcmap_cm_handle,                           /*Mobile AP CM Handle    */
  qcmap_cm_firewall_entry_req_e firewall_op_type,   /*Extd firewall operation*/
  qcmap_cm_extd_firewall_conf_t *extd_firewall_conf,/*Extended Firewall conf */
  int    *qcmap_cm_errno                            /* error condition value */
)
{
  int ret = QCMAP_CM_SUCCESS;

  ds_assert(qcmap_cm_errno != NULL);
  ds_assert(extd_firewall_conf != NULL);

  LOG_MSG_INFO1("qcmap_cm_extd_firewall_op: enter", 0, 0, 0);
  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state == QCMAP_CM_DISABLE)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "QCMAP CM disable state", 0, 0, 0);
    return ret;
  }

  qcmap_cm_unlock();

  switch (firewall_op_type)
  {
    case QCMAP_CM_ADD_EXTD_FIREWALL_ENTRY_RULE:
      ret = qcmap_cm_extd_firewall_entry_op_add(&(extd_firewall_conf->extd_firewall_entry), qcmap_cm_errno);
      break;
    case QCMAP_CM_GET_EXTD_FIREWALL_ENTRY_RULE:
      ret = qcmap_cm_extd_firewall_entry_op_get(&(extd_firewall_conf->extd_firewall_entry), qcmap_cm_errno);
      break;
    case QCMAP_CM_GET_FIREWALL_HANDLE_LIST:
      ret = qcmap_cm_extd_firewall_op_get_handle_list(&(extd_firewall_conf->extd_firewall_handle_list), qcmap_cm_errno);
      break;
    case QCMAP_CM_DELETE_EXTD_FIREWALL_ENTRY_RULE:
      ret = qcmap_cm_extd_firewall_entry_op_delete(&(extd_firewall_conf->extd_firewall_handle), qcmap_cm_errno);
      break;
    default:
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      LOG_MSG_ERROR( "Invalid extended firewall entry op %d", firewall_op_type, 0, 0);
      break;
  }

  return ret;
}

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
)
{
  int file_handle, ret, bytes_read = 0;
  struct stat stat_buf;

  ds_assert(file_name != NULL);
  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_eri_read_config: enter", 0, 0, 0);
  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state == QCMAP_CM_DISABLE)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "QCMAP CM disable state", 0, 0, 0);
    return ret;
  }

  qcmap_cm_unlock();

  file_handle = open(file_name, O_RDONLY);

  if (file_handle < 0)
  {
    *qcmap_cm_errno = errno;
    LOG_MSG_ERROR( "Couldn't open config file %d", errno, 0, 0);
    return QCMAP_CM_ERROR;
  }

  if (fstat(file_handle, &stat_buf) < 0)
  {
    *qcmap_cm_errno = errno;
    LOG_MSG_ERROR( "Couldn't stat config file %d", errno, 0, 0);
    close(file_handle);
    return QCMAP_CM_ERROR;
  }

  if (stat_buf.st_size > QCMAP_WAN_MAX_ERI_DATA_SIZE)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    LOG_MSG_ERROR( "Config file too big. size %d", stat_buf.st_size, 0, 0);
    close(file_handle);
    return QCMAP_CM_ERROR;
  }

  do
  {
    /*This fix is to resolve a KW error*/
    if(bytes_read >=0 && bytes_read < QCMAP_WAN_MAX_ERI_DATA_SIZE)
    ret = read(file_handle, &qcmap_cm_cb.eri_roam_data[bytes_read], (QCMAP_WAN_MAX_ERI_DATA_SIZE - bytes_read));
  } while (ret > 0 && (bytes_read += ret) < QCMAP_WAN_MAX_ERI_DATA_SIZE);

  close(file_handle);

  if (stat_buf.st_size != bytes_read)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    LOG_MSG_ERROR( "eri_roam_data_len %d != bytes_read %d", qcmap_cm_cb.eri_roam_data_len, bytes_read, 0);
    return QCMAP_CM_ERROR;
  }

  qcmap_cm_cb.eri_roam_data_len = (uint16)stat_buf.st_size;

  return QCMAP_CM_SUCCESS;
}

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
)
{
  int                 rval;
  int                 ret = QCMAP_CM_SUCCESS;

  dsi_hndl_t dsi_handle;
  dsi_data_pkt_stats stats_info;

  ds_assert(wwan_stats != NULL);
  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_get_wwan_statistics: enter", 0, 0, 0); 

  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  /* If WWAN is not connected return error */
  if (qcmap_cm_cb.state != QCMAP_CM_WAN_CONNECTED)
  {
    *qcmap_cm_errno = QCMAP_CM_ENOWWAN;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "QCMAP CM WWAN not in connected state", 0, 0, 0);
    return ret;
  }

  if (ip_family == QCMAP_IP_V4_V01)
  {
    dsi_handle = qcmap_cm_cb.dsi_net_hndl.handle;
  }
  else if (ip_family == QCMAP_IP_V6_V01)
  {
    dsi_handle = qcmap_cm_cb.ipv6_dsi_net_hndl.handle;
  }
  else
  {
    /* *qmi_err_num = QMI_ERR_INVALID_IP_FAMILY_PREF_V01; */
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "QCMAP CM Invalid family %d", ip_family, 0, 0);
    return ret;
  }

  memset(&stats_info, 0, sizeof(stats_info));

  rval = dsi_get_pkt_stats(dsi_handle, &stats_info);

  if (rval != DSI_SUCCESS)
  {
    /**qmi_err_num = QMI_ERR_INTERNAL_V01;*/
    LOG_MSG_ERROR("dsi_get_pkt_stats Failed error",
                   0,0,0);
    ret = QCMAP_CM_ERROR;
    return ret;
  }

  wwan_stats->bytes_rx = stats_info.bytes_rx;
  wwan_stats->bytes_tx = stats_info.bytes_tx;
  wwan_stats->pkts_rx  = stats_info.pkts_rx;
  wwan_stats->pkts_tx  = stats_info.pkts_tx;
  wwan_stats->pkts_dropped_rx = stats_info.pkts_dropped_rx;
  wwan_stats->pkts_dropped_tx = stats_info.pkts_dropped_tx;

  qcmap_cm_unlock();

  return ret;
} /* qcmap_cm_get_wwan_statistics() */

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
)
{
  int                 rval;
  int                 ret = QCMAP_CM_SUCCESS;

  dsi_hndl_t dsi_handle;
  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_reset_wwan_statistics: enter", 0, 0, 0); 

  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  /* If WWAN is not connected return error */
  if (qcmap_cm_cb.state != QCMAP_CM_WAN_CONNECTED)
  {
    *qcmap_cm_errno = QCMAP_CM_ENOWWAN;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "QCMAP CM WWAN not in connected state", 0, 0, 0);
    return ret;
  }

  /*
     TODO:
  */
  if (ip_family == QCMAP_IP_V4_V01)
  {
    dsi_handle = qcmap_cm_cb.dsi_net_hndl.handle;
  }
  else if (ip_family == QCMAP_IP_V6_V01)
  {
    dsi_handle = qcmap_cm_cb.ipv6_dsi_net_hndl.handle;
  }
  else
  {
    qcmap_cm_unlock();
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "QCMAP CM Invalid family %d", ip_family, 0, 0);
    return ret;
  }

  rval = dsi_reset_pkt_stats(dsi_handle);

  if (rval != DSI_SUCCESS)
  {
    qcmap_cm_unlock();
    /**qmi_err_num = QMI_ERR_INTERNAL_V01;*/
    LOG_MSG_ERROR("dsi_reset_pkt_stats Failed error",
                   0,0,0);
    ret = QCMAP_CM_ERROR;
    return ret;
  }


  qcmap_cm_unlock();

  return ret;
} /* qcmap_cm_reset_wwan_statistics() */


/*===========================================================================

FUNCTION COEX_GET_WWAN_STATUS()

DESCRIPTION
  This Function retrieves the LTE frequency used by WiFi-LTE coex feature.
   
DEPENDENCIES
  None.

RETURN VALUE
  On success, returns true.
  On error, return false

SIDE EFFECTS
===========================================================================*/
boolean coex_get_wwan_status(int * lte_frequency)
{
  int rc, retry_count = 0;  
  qmi_client_type clnt, notifier;  
  qmi_cci_os_signal_type os_params;  
  unsigned int num_services = 0, num_entries = 0;  
  qmi_service_info info[10]; 
  boolean ret;
  coex_get_wwan_state_resp_msg_v01 coex_resp;

  ds_assert(lte_frequency!= NULL);

  LOG_MSG_INFO1("Getting lte frequency from coex service",0,0,0);
  // Get the service object for coex_manager API  
  qmi_idl_service_object_type coex_service_object = coex_get_service_object_v01();
  if(!coex_service_object)
  {
      LOG_MSG_ERROR("coex_get_service_object failed",0,0,0);
      return FALSE;
  }
  LOG_MSG_INFO1("notifying qmi that our client is up",0,0,0);
  rc = qmi_client_notifier_init(coex_service_object, &os_params, &notifier);
  LOG_MSG_INFO1("get_coex_wwan_status:qmi_client_notifier_init %d", rc, 0, 0);

  if(rc!= QMI_NO_ERR)
  {
      LOG_MSG_ERROR("Unable to notify qmi_client",0,0,0);
      return FALSE;
  }
  // Check if the service is up, if not wait on a signal  
  while (retry_count < QCMAP_COEX_MAX_RETRY )
  {
    rc = qmi_client_get_service_list(coex_service_object, NULL, NULL, &num_services);
    LOG_MSG_INFO1("get_coex_wwan_status:qmi_client_get_service_list: %d", rc, 0, 0); 
    if (rc == QMI_NO_ERR)
    {  
      break;  
    }
    QMI_CCI_OS_SIGNAL_WAIT(&os_params, QCMAP_COEX_TIMEOUT_VALUE);
    QMI_CCI_OS_SIGNAL_CLEAR(&os_params);
    LOG_MSG_INFO1("Returned from os signal wait",0,0,0);
    printf("waiting for co-ex service");
    retry_count++;
  }

  LOG_MSG_INFO1("get_coex_wwan_status: qmi_client_get_service_list: num_e %d num_s %d",
                num_entries, num_services, 0);

  if(retry_count == QCMAP_COEX_MAX_RETRY )
  {
    LOG_MSG_ERROR("Reached maximum retry attempts %d", retry_count, 0, 0);
    qmi_client_release(notifier); 
    return FALSE; 
  }

  // the server has gone up, so we store the information  
  num_entries = num_services;
  rc = qmi_client_get_service_list(coex_service_object, info, &num_entries, &num_services);

  LOG_MSG_INFO1("get_coex_wwan_status: qmi_client_get_service_list: num_e %d num_s %d", 
                num_entries, num_services, 0);

  if(rc!= QMI_NO_ERR)
  {
    LOG_MSG_ERROR("qmi_client_get_service_list failed rc %d", rc, 0, 0);
    qmi_client_release(notifier); 
    return FALSE; 
  }
  
  rc = qmi_client_init(&info[0], coex_service_object, NULL, NULL, NULL, &clnt);  

  LOG_MSG_INFO1("get_coex_wwan_status:qmi_client_init %d", rc, 0, 0);

  if(rc!= QMI_NO_ERR)
  {
    LOG_MSG_ERROR("qmi_client_init failed rc %d",rc,0,0);
    qmi_client_release(notifier); 
    return FALSE;
  }

  memset(&coex_resp, 0, sizeof(coex_get_wwan_state_resp_msg_v01));  

  rc = qmi_client_send_msg_sync(clnt, QMI_COEX_GET_WWAN_STATE_REQ_V01, NULL, 0, &coex_resp, sizeof(coex_resp), 5000);  
  
  LOG_MSG_INFO1("get_coex_wwan_status:qmi_client_send_msg_sync %d resp_result %d resp error %d", 
                rc, coex_resp.resp.result, coex_resp.resp.error);

  if (rc == QMI_NO_ERR && coex_resp.resp.result == QMI_NO_ERR && coex_resp.lte_band_info_valid) 
  {
    *lte_frequency = coex_resp.lte_band_info.ul_band.freq;
    ret= TRUE;
    LOG_MSG_INFO1("Obtained lte frequency %d through coex service",*lte_frequency, 0, 0);
  }
  else
  {  
    LOG_MSG_ERROR("Failed to get lte frequency from coex service",0,0,0);
    ret=FALSE;
  }

  rc = qmi_client_release(clnt);  
  rc = qmi_client_release(notifier);  
  return ret; 
} 

/*===========================================================================
  FUNCTION  qcmap_cm_config_init
===========================================================================*/
void qcmap_cm_config_init
(
  int    qcmap_cm_handle,                           /* Handle for MobileAP CM  */
  int    *qcmap_cm_errno                            /* Error condition value  */
)
{
  int i=0;
  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_config_init: enter", 0, 0, 0);

  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return;
  }

  if (qcmap_cm_cb.state == QCMAP_CM_DISABLE)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "QCMAP CM disable state", 0, 0, 0);
    return;
  }

  qcmap_cm_unlock();

  /* Config SNAT */
  for (i = 0; i < qcmap_cm_cb.cfg->nat_config.num_port_fwding_entries; i++)
  {
    qcmap_cm_static_nat_op(qcmap_cm_cb.handle, QCMAP_CM_ADD_SNAT,
                           &qcmap_cm_cb.cfg->nat_config.port_fwding_entries[i],
                           qcmap_cm_errno);
  }

  /* Config Firewall */
  if (qcmap_cm_cb.cfg->nat_config.firewall_enabled)
  {
    qcmap_cm_firewall_op(qcmap_cm_cb.handle,
                         QCMAP_CM_ENABLE_FIREWALL,
                         &qcmap_cm_cb.cfg->nat_config.firewall_pkts_allowed,
                         qcmap_cm_errno);
  }
  else
  {
    qcmap_cm_firewall_op(qcmap_cm_cb.handle,
                         QCMAP_CM_DISABLE_FIREWALL,
                         NULL,
                         qcmap_cm_errno);
  }

  /* Config Extd Firewall Rules */
  for (i = 0; i < qcmap_cm_cb.cfg->nat_config.num_extd_firewall_entries; i++)
  {
    qcmap_cm_extd_firewall_op(qcmap_cm_cb.handle,
                              QCMAP_CM_ADD_EXTD_FIREWALL_ENTRY_RULE,
                              &qcmap_cm_cb.cfg->nat_config.extd_firewall_entries[i],
                              qcmap_cm_errno);
  }

  /* Config DMZ */
  if (qcmap_cm_cb.cfg->nat_config.dmz_ip != 0)
  {
    qcmap_cm_dmz_op(qcmap_cm_cb.handle,
                    QCMAP_CM_ADD_DMZ,
                    &qcmap_cm_cb.cfg->nat_config.dmz_ip,
                    qcmap_cm_errno);
  }

  /* Config NAT timeout */
  if (qcmap_cm_cb.cfg->nat_config.nat_entry_timeout != 0)
  {
    qcmap_cm_nat_timeout_op(qcmap_cm_cb.handle,
                            QCMAP_CM_SET_NAT_ENTRY_TIMEOUT,
                            &qcmap_cm_cb.cfg->nat_config.nat_entry_timeout,
                            qcmap_cm_errno);
  }

  /* Config IPSEC VPN passthrough */
  if (qcmap_cm_cb.cfg->nat_config.enable_ipsec_vpn_pass_through)
  {
    qcmap_cm_vpn_op(qcmap_cm_cb.handle,
                    QCMAP_CM_SET_IPSEC,
                    &qcmap_cm_cb.cfg->nat_config.enable_ipsec_vpn_pass_through,
                    qcmap_cm_errno);
  }

  /* Config PPTP VPN passthrough */
  if (qcmap_cm_cb.cfg->nat_config.enable_pptp_vpn_pass_through)
  {
    qcmap_cm_vpn_op(qcmap_cm_cb.handle, QCMAP_CM_SET_PPTP,
                    &qcmap_cm_cb.cfg->nat_config.enable_pptp_vpn_pass_through,
                    qcmap_cm_errno);
  }

  /* Config L2TP VPN passthrough */
  if (qcmap_cm_cb.cfg->nat_config.enable_l2tp_vpn_pass_through)
  {
    qcmap_cm_vpn_op(qcmap_cm_cb.handle, QCMAP_CM_SET_L2TP,
                    &qcmap_cm_cb.cfg->nat_config.enable_l2tp_vpn_pass_through,
                    qcmap_cm_errno);
  }
  return;
}
int qcmap_cm_nat_type_set
(
  int                qcmap_cm_handle,    /* Handler for SoftAP CM */
  qcmap_cm_nat_type  nat_type,           /* NAT Type specified    */
  int                *qcmap_cm_errno     /* error condition value */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_change_nat_type_req_msg_v01 req_msg;
  qcmap_change_nat_type_resp_msg_v01 resp_msg;
/*-------------------------------------------------------------------------*/

  /* Validate args */
  ds_assert(qcmap_cm_errno != NULL);

  /* Set request msg parameters */
  req_msg.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;
  req_msg.nat_type_option = nat_type;
  req_msg.nat_type_option_valid = TRUE;

  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  qcmap_cm_unlock();

  /* Send QMI msg to modem with 15 sec timeout */
  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                    QMI_QCMAP_CHANGE_NAT_TYPE_REQ_V01,
                                    &req_msg,
                                    sizeof(qcmap_change_nat_type_req_msg_v01),
                                    &resp_msg,
                                    sizeof(qcmap_change_nat_type_resp_msg_v01),
                                    QCMAP_CM_QMI_TIMEOUT_VALUE);
  if ((qmi_error == QMI_NO_ERR) &&
      (resp_msg.resp.result == QMI_RESULT_SUCCESS_V01) &&
      (resp_msg.resp.error == QMI_ERR_NONE_V01))
  {
    //Success
    *qcmap_cm_errno = QCMAP_CM_ENOERROR;
    ret = QCMAP_CM_SUCCESS;
    /* Set the various config parameters while switching NAT types */
    qcmap_cm_config_init(qcmap_cm_handle, qcmap_cm_errno);
  }
  else
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "QCMAP AP Handle %d NAT type changing qmi err %d Resp err %d",
                   qcmap_cm_handle, qmi_error, resp_msg.resp.error);
  }

  return ret;
}

int qcmap_cm_nat_type_get
(
  int                      qcmap_cm_handle,      /* Handle for MobileAP CM  */
  qcmap_cm_nat_type        *cur_nat_type,        /* Current NAT type */
  int                      *qcmap_cm_errno       /* Error condition  */
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  qcmap_get_nat_type_req_msg_v01 get_nat_type_req_msg_v01;
  qcmap_get_nat_type_resp_msg_v01 get_nat_type_resp_msg_v01;

  qcmap_cm_lock();

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state == QCMAP_CM_DISABLE)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    qcmap_cm_unlock();
    LOG_MSG_ERROR("QCMAP CM disable state",
                  0, 0, 0);
    return ret;
  }

  qcmap_cm_unlock();

  get_nat_type_req_msg_v01.mobile_ap_handle = qcmap_cm_cb.mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qcmap_cm_cb.qmi_qcmap_handle,
                                       QMI_QCMAP_GET_NAT_TYPE_REQ_V01,
                                       &get_nat_type_req_msg_v01,
                                       sizeof(get_nat_type_req_msg_v01),
                                       &get_nat_type_resp_msg_v01,
                                       sizeof(get_nat_type_resp_msg_v01),
                                       QCMAP_CM_QMI_TIMEOUT_VALUE);


  if ((qmi_error != QMI_NO_ERR) ||
      (get_nat_type_resp_msg_v01.resp.result != QMI_NO_ERR) ||
      (get_nat_type_resp_msg_v01.nat_type_option_valid != TRUE))
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Get NAT Type failed %d %d",
                   qmi_error,
                   get_nat_type_resp_msg_v01.resp.error, 0);
  }

  *cur_nat_type = (qcmap_cm_nat_type)get_nat_type_resp_msg_v01.nat_type_option;
  return ret;
}
