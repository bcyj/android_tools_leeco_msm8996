/******************************************************************************

                           QCMAP_CM.C

******************************************************************************/

/******************************************************************************

  @file    qcmap_cm.c
  @brief   Mobile AP Connection Manager Lib Implementation

  DESCRIPTION
  Mobile AP Connection Manager Lib Implementation.

  ---------------------------------------------------------------------------
  Copyright (c) 2011-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        --------------------------------------------------------
07/11/12   gk         9x25
10/26/12   cp         Added support for Dual AP and different types of NAT.
03/28/13   mp         Added support to get IPv6 WWAN configuration.
01/03/14   vm         Changes to support IoE on 9x25
01/16/14   cp         Added support for modem loopback call.
******************************************************************************/

/*===========================================================================

                          INCLUDE FILES FOR MODULE

===========================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
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
#include "comdef.h"
#include "msg.h"
#include "ds_util.h"
#include "qmi_client.h"
#include "qualcomm_mobile_access_point_msgr_v01.h"
#include "network_access_service_v01.h"
#include "wireless_data_service_v01.h"
#include "qmi_client_instance_defs.h"
#include "qcmap_cm_api.h"


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

extern unsigned int dsi_qcmap_sockfd;
extern unsigned int cmdq_qcmap_sockfd;
extern unsigned int nas_qcmap_sockfd;
extern unsigned int timer_qcmap_sockfd;

extern unsigned int modem_loopback_mode;

#define QCMAP_STA_COOKIE 0xDCDCDCDC



int
qcmap_cm_qmi_nas_ind
(
  qmi_client_type user_handle,                    /* QMI user handle       */
  unsigned int    msg_id,                         /* Indicator message ID  */
  void           *ind_buf,                        /* Raw indication data   */
  unsigned int    ind_buf_len,                    /* Raw data length       */
  void           *ind_cb_data                     /* User call back handle */
);

int
qcmap_cm_qmi_timer_ind
(
  qcmap_timer_enum_t  timer_id                         /* Indicator message ID  */
);
/*===========================================================================
                     FORWARD DECLARATIONS
===========================================================================*/

/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

/*---------------------------------------------------------------------------
   Type representing an QCMAP CM CB (QCMAP CM control block); this captures
   all state and other data related to the QCMAP CM
---------------------------------------------------------------------------*/
typedef struct
{
    qcmap_cm_state_e          state;
    int                       handle;
    qcmap_cm_v6_state_e       ipv6_state;

    /* MobileAP config read from XML file or default.*/
    qcmap_cm_conf_t          *cfg;

    /* libqcmap_cm library callback info */
    qcmap_cm_cb_fcn           qcmap_cm_callback;
    void                     *qcmap_cm_callback_user_data;
    pthread_mutex_t           qcmap_cm_mutex;

    /* dsi related info */
    dsi_call_info_t           dsi_net_hndl;
    char dsi_device_name[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 2];

    /* dsi ipv6 related info */
    dsi_call_info_t           ipv6_dsi_net_hndl;
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
    uint8                     eri_roam_data[QCMAP_WAN_MAX_ERI_DATA_SIZE];
    uint16                    eri_roam_data_len;

    /* STA Cookie Value. */
    uint32                    sta_cookie;
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
  .eri_roam_data_len           = 0,
  .disable_in_process          = FALSE,
  .sta_cookie                    = 0
};



void qcmap_cm_dsi_net_cb_fcn( dsi_hndl_t hndl,
                     void * user_data,
                     dsi_net_evt_t evt,
                     dsi_evt_payload_t *payload_ptr )
{
  qcmap_dsi_buffer_t qcmap_dsi_buffer;
//  unsigned int dsi_qcmap_sockfd;
  int numBytes=0, len;
  struct sockaddr_un dsi_qcmap;

  dsi_qcmap.sun_family = AF_UNIX;
  strcpy(dsi_qcmap.sun_path, QCMAP_DSI_UDS_FILE);
  len = strlen(dsi_qcmap.sun_path) + sizeof(dsi_qcmap.sun_family);

  qcmap_dsi_buffer.dsi_nethandle = hndl;
  /* Not used currently, but if used, make sure you allocate buffer and not use the buffer provided by dsi */
  qcmap_dsi_buffer.user_data = user_data;
  qcmap_dsi_buffer.evt = evt;
  /* Not used currently, but if used, make sure you allocate buffer and not use the buffer provided by dsi */
  qcmap_dsi_buffer.payload_ptr = payload_ptr;

  if ((numBytes = sendto(dsi_qcmap_sockfd, (void *)&qcmap_dsi_buffer, sizeof(qcmap_dsi_buffer_t), 0,
             (struct sockaddr *)&dsi_qcmap, len)) == -1)
  {
    LOG_MSG_ERROR("Send Failed from dsi callback context", 0, 0, 0);
    return;
  }
  LOG_MSG_INFO1("Send succeeded in dsi callback context", 0, 0, 0);
  return;
}

/*===========================================================================
  FUNCTION qcmap_cm_dsi_net_init_cb
===========================================================================*/
/*!
@brief
  This function is a call back function to dsi_init_ex. After the dsi module
  is initialized, this call back is called and it notifies QCMAP that dsi is
  initilialized.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void qcmap_cm_dsi_net_init_cb(void)
{
  qcmap_dsi_buffer_t qcmap_dsi_buffer;
  int numBytes = 0, len;
  struct sockaddr_un dsi_qcmap;
  uint32_t *user_data = NULL;

  dsi_qcmap.sun_family = AF_UNIX;
  strlcpy(dsi_qcmap.sun_path, QCMAP_DSI_UDS_FILE,sizeof(dsi_qcmap.sun_path));
  len = strlen(dsi_qcmap.sun_path) + sizeof(dsi_qcmap.sun_family);
  bzero(&qcmap_dsi_buffer, sizeof(qcmap_dsi_buffer_t));

  user_data = (uint32_t *)malloc(sizeof(uint32_t));
  if (user_data == NULL)
  {
    LOG_MSG_ERROR("Out of Memory in dsi_init_cb!!", 0,0,0);
    return;
  }
  /* DSI initialized */
  *user_data = DSI_INITED;
  qcmap_dsi_buffer.user_data = user_data;

  if ((numBytes = sendto(dsi_qcmap_sockfd, (void *)&qcmap_dsi_buffer, sizeof(qcmap_dsi_buffer_t), 0,
             (struct sockaddr *)&dsi_qcmap, len)) == -1)
  {
    LOG_MSG_ERROR("Send Failed from dsi callback context", 0, 0, 0);
    free(user_data);
    user_data = NULL;
    return;
  }
  LOG_MSG_INFO1("Send succeeded in dsi callback context", 0, 0, 0);
  return;
}

/*===========================================================================
  FUNCTION qcmap_timer_handler
===========================================================================*/
/*!
@brief
  Function used to handle various QCMAP timers

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void qcmap_timer_handler(int sig, siginfo_t *si, void *uc)
{
  LOG_MSG_INFO1("qcmap_timer_handler()",0,0,0);
  qcmap_timer_enum_t event;
  boolean send_event=true;

  switch (si->si_value.sival_int)
  {
    case AUTO_CONNECT_V4:
    {
      LOG_MSG_INFO1("IPv4 timer expired timer=%x",qcmap_cm_cb.cfg->wan_config.timerid_v4,0,0);
      qcmap_cm_cb.cfg->wan_config.auto_connect_timer_running_v4=false;
      event = AUTO_CONNECT_V4;
    }
    break;
    case AUTO_CONNECT_V6:
    {
      LOG_MSG_INFO1("IPv6 timer expired timer=%x",qcmap_cm_cb.cfg->wan_config.timerid_v6,0,0);
      event = AUTO_CONNECT_V6;
      qcmap_cm_cb.cfg->wan_config.auto_connect_timer_running_v6=false;
    }
    break;
    case STA_ASSOC_FAIL:
    {
      LOG_MSG_INFO1("WLAN Assoc Timer Expired",0,0,0);
      if (qcmap_cm_cb.cfg->lan_config.ap_sta_bridge_sta_assoc_running)
      {
        event = STA_ASSOC_FAIL;
        qcmap_cm_cb.cfg->lan_config.ap_sta_bridge_sta_assoc_running = false;
      }
      else
      {
        LOG_MSG_ERROR("WLAN Assoc Timer Expired: Event recieved even though timer is not running",0,0,0);
        send_event=false;
      }
    }
    break;
    case STA_DHCP_FAIL:
    {
      LOG_MSG_INFO1("WLAN DHCP Timer Expired",0,0,0);
      if (qcmap_cm_cb.cfg->lan_config.ap_sta_bridge_sta_dhcp_running)
      {
        event = STA_DHCP_FAIL;
        qcmap_cm_cb.cfg->lan_config.ap_sta_bridge_sta_dhcp_running = false;
      }
      else
      {
        LOG_MSG_ERROR("WLAN DHCP Timer Expired: Event recieved even though timer is not running",0,0,0);
        send_event=false;
      }
    }
    break;
    default:
      LOG_MSG_INFO1("Incorrect Signal Recieved in qcmap_timer_handler() val=%d",si->si_value.sival_int,0,0);
  }
  if (send_event)
    qcmap_cm_qmi_timer_ind(event);
}


/*===========================================================================
  FUNCTION start_auto_connect
===========================================================================*/
/*!
@brief
  This Function is used to send connect to backhaul request incase the
  previous connect to backhal request failed.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void start_auto_connect(ip_version_enum_type family)
{
  struct itimerspec its;
  its.it_value.tv_nsec = 0;
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec =0;

  if( family == QCMAP_MSGR_IP_FAMILY_V4_V01 )
  {
    LOG_MSG_INFO1("qcmap_cm_cb.state =%d ",qcmap_cm_cb.state,0,0);
    qcmap_cm_cb.cfg->wan_config.auto_connect_timer_running_v4=true;
    LOG_MSG_INFO1("V4 New Connect to backhaul request with start after %d sec ",qcmap_cm_cb.cfg->wan_config.auto_timer_value_v4,0,0);
    its.it_value.tv_sec = qcmap_cm_cb.cfg->wan_config.auto_timer_value_v4;
    timer_settime(qcmap_cm_cb.cfg->wan_config.timerid_v4, 0, &its, NULL);
    if(qcmap_cm_cb.cfg->wan_config.auto_timer_value_v4 < MAX_WAN_CON_TIMEOUT)
      qcmap_cm_cb.cfg->wan_config.auto_timer_value_v4 = qcmap_cm_cb.cfg->wan_config.auto_timer_value_v4 * 2;
  }
  else
  {
    LOG_MSG_INFO1("qcmap_cm_cb.state =%d ",qcmap_cm_cb.state,0,0);
    qcmap_cm_cb.cfg->wan_config.auto_connect_timer_running_v6=true;
    its.it_value.tv_sec = qcmap_cm_cb.cfg->wan_config.auto_timer_value_v6;
    LOG_MSG_INFO1("V6 New Connect to backhaul request with start after %d sec ",its.it_value.tv_sec,0,0);
    timer_settime(qcmap_cm_cb.cfg->wan_config.timerid_v6, 0, &its, NULL);
    if(qcmap_cm_cb.cfg->wan_config.auto_timer_value_v6 < MAX_WAN_CON_TIMEOUT)
      qcmap_cm_cb.cfg->wan_config.auto_timer_value_v6 = qcmap_cm_cb.cfg->wan_config.auto_timer_value_v6 * 2;
  }
}

/*===========================================================================
 FUNCTION start_wlan_sta_timer
===========================================================================*/
/*!
@brief
  Used to start the WLAN STA timer(s)

@return
  void

@note

  - Dependencies
  - None

  - Side Effects
  - None
*/
/*=========================================================================*/
int start_wlan_sta_timer(qcmap_timer_enum_t sta_event)
{
  LOG_MSG_INFO1("start_wlan_sta_timer() event=%d",sta_event,0,0);
  struct itimerspec its;
  its.it_value.tv_nsec = 0;
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec =0;

  if( (sta_event == STA_ASSOC_FAIL) && (qcmap_cm_cb.cfg->lan_config.ap_sta_bridge_sta_assoc_running == false))
  {
    qcmap_cm_cb.cfg->lan_config.ap_sta_bridge_sta_assoc_running=true;
    LOG_MSG_INFO1("WLAN STA Assoc Timer Started for timerid %d",qcmap_cm_cb.cfg->lan_config.timerid_sta_assoc,0,0);
    its.it_value.tv_sec = QCMAP_WLAN_STA_ASSOC_TIMEOUT;
    if (timer_settime(qcmap_cm_cb.cfg->lan_config.timerid_sta_assoc, 0, &its, NULL) == -1)
    {
       LOG_MSG_INFO1("start_wlan_sta_timer timer_settime failed error:%s",strerror(errno),0,0);
       return QCMAP_CM_ERROR;
    }
  }
  else if ( (sta_event == STA_DHCP_FAIL) && (qcmap_cm_cb.cfg->lan_config.ap_sta_bridge_sta_dhcp_running == false) )
  {
    qcmap_cm_cb.cfg->lan_config.ap_sta_bridge_sta_dhcp_running=true;
    its.it_value.tv_sec = QCMAP_WLAN_STA_DHCP_TIMEOUT;
    LOG_MSG_INFO1("WLAN STA DHCP IP Assignment Timer Started for timerid %d",qcmap_cm_cb.cfg->lan_config.timerid_sta_dhcp,0,0);
    if (timer_settime(qcmap_cm_cb.cfg->lan_config.timerid_sta_dhcp, 0, &its, NULL) == -1 )
    {
       LOG_MSG_INFO1("start_wlan_sta_timer timer_settime failed error:%s",strerror(errno),0,0);
       return QCMAP_CM_ERROR;
    }
  }
  else
  {
    LOG_MSG_INFO1("Incorrect Event passed in start_wlan_sta_timer() event =%d",sta_event,0,0);
    return QCMAP_CM_ERROR;
  }
  return QCMAP_CM_SUCCESS;
}




/*===========================================================================
  FUNCTION stop_auto_timer
===========================================================================*/
/*!
@brief
 This Fuction will stop the running timer and reset the timer vaules.
@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void stop_auto_timer(ip_version_enum_type family)
{
  struct itimerspec its;
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = 0;
  LOG_MSG_INFO1("stopping auto timer \n",0,0,0);
  if(family == QCMAP_MSGR_IP_FAMILY_V4_V01)
  {
    LOG_MSG_INFO1("stopping auto timer for v4 time 0x%lx \n",qcmap_cm_cb.cfg->wan_config.timerid_v4,0,0);
    timer_settime(qcmap_cm_cb.cfg->wan_config.timerid_v4, 0, &its, NULL);
    qcmap_cm_cb.cfg->wan_config.auto_timer_value_v4 = AUTO_CONNECT_TIMER;
  }
  else
  {
    LOG_MSG_INFO1("stopping auto timer for v6 time 0x%lx \n",qcmap_cm_cb.cfg->wan_config.timerid_v4,0,0);
    timer_settime(qcmap_cm_cb.cfg->wan_config.timerid_v6, 0, &its, NULL);
    qcmap_cm_cb.cfg->wan_config.auto_timer_value_v6 =  AUTO_CONNECT_TIMER;
  }
}

/*===========================================================================
 FUNCTION stop_wlan_sta_timer
===========================================================================*/
/*!
@brief
  Used to stop the WLAN STA timer(s)

@return
  void

@note

  - Dependencies
  - None

  - Side Effects
  - None
*/
/*=========================================================================*/
int stop_wlan_sta_timer(qcmap_timer_enum_t sta_event)
{
  struct itimerspec its;
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = 0;
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = 0;
  LOG_MSG_INFO1("stopping wlan_sta timer for event:%d\n",sta_event,0,0);
  if(sta_event == STA_ASSOC_FAIL && qcmap_cm_cb.cfg->lan_config.ap_sta_bridge_sta_assoc_running)
  {
    LOG_MSG_INFO1("stopping WLAN STA timer for timer %d \n",qcmap_cm_cb.cfg->lan_config.timerid_sta_assoc,0,0);
    if (timer_settime(qcmap_cm_cb.cfg->lan_config.timerid_sta_assoc, 0, &its, NULL) == -1 )
    {
       ds_log_med("stop_wlan_sta_timer timer_settime failed error:%s",strerror(errno));
       return QCMAP_CM_ERROR;
    }
    qcmap_cm_cb.cfg->lan_config.ap_sta_bridge_sta_assoc_running=false;
  }
  else if (sta_event == STA_DHCP_FAIL && qcmap_cm_cb.cfg->lan_config.ap_sta_bridge_sta_dhcp_running )
  {
    LOG_MSG_INFO1("stopping WLAN STA timer for timer %d \n",qcmap_cm_cb.cfg->lan_config.timerid_sta_dhcp,0,0);
    if (timer_settime(qcmap_cm_cb.cfg->lan_config.timerid_sta_dhcp, 0, &its, NULL) == -1 )
    {
       ds_log_med("stop_wlan_sta_timer timer_settime failed error:%s",strerror(errno));
       return QCMAP_CM_ERROR;
    }
    qcmap_cm_cb.cfg->lan_config.ap_sta_bridge_sta_dhcp_running=false;
  }
  else
  {
    LOG_MSG_ERROR("Incorrect Event passed in stop_wlan_sta_timer() event =%d, sta_assoc_running =%d, sta_dhcp_running=%d",
                    sta_event,qcmap_cm_cb.cfg->lan_config.ap_sta_bridge_sta_assoc_running,qcmap_cm_cb.cfg->lan_config.ap_sta_bridge_sta_dhcp_running);
    return QCMAP_CM_ERROR;
  }
  return QCMAP_CM_SUCCESS;
}

/*===========================================================================
  FUNCTION qcmap_cm_process_dsi_init_ind
===========================================================================*/
/*!
@brief
 This Fuction will trigger backhaul request, if auto connect is enabled.
 It is called as a part of handling dsi_net initialization.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void qcmap_cm_process_dsi_init_ind(void)
{
  int qcmap_cm_errno = 0;
  qmi_error_type_v01 qmi_err_num;

  if( qcmap_cm_cb.state == QCMAP_CM_ENABLE )
   {
      if( qcmap_cm_cb.cfg->wan_config.auto_connect &&
          !qcmap_cm_cb.disable_in_process )
      {
         LOG_MSG_INFO1("qcmap_cm_process_dsi_init_ind: V4 Auto connect start", 0, 0, 0);
         qcmap_cm_connect_backhaul( qcmap_cm_cb.handle,
                               QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01,
                               &qcmap_cm_errno,
                               &qmi_err_num);
      }
  }
  if( qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_ENABLE )
  {
    if( qcmap_cm_cb.cfg->wan_config.auto_connect &&
        !qcmap_cm_cb.disable_in_process )
    {
       LOG_MSG_INFO1("qcmap_cm_process_dsi_init_ind: V6 Auto connect start", 0, 0, 0);
       qcmap_cm_connect_backhaul( qcmap_cm_cb.handle,
                                  QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01,
                                  &qcmap_cm_errno,
                                  &qmi_err_num);
    }
  }
}

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
void qcmap_cm_process_dsi_net_evt( dsi_hndl_t hndl,
                     void * user_data,
                     dsi_net_evt_t evt,
                     dsi_evt_payload_t *payload_ptr )
{
  qcmap_cm_event_e  event;
  boolean           callback = FALSE;
  int               i;
  int               err;
  int              *qcmap_cm_errno = &err;
  qmi_client_error_type                qmi_error;
  nas_indication_register_req_msg_v01  qcmap_nas_indication_register_req_msg_v01;
  nas_indication_register_resp_msg_v01 qcmap_nas_indication_register_resp_msg_v01;
  dsi_ce_reason_t   dsicallend;
  qmi_error_type_v01 qmi_err_num;

  bzero(&dsicallend,sizeof(dsicallend));
  LOG_MSG_INFO1("qcmap_cm_dsi_net_cb_fcn: dsi_nethandle 0x%x evt %d qcmap_cm_cb.state (V4) %d ",\
                  hndl, evt, qcmap_cm_cb.state);

  LOG_MSG_INFO1("qcmap_cm_cb.ipv6_state (V6) %d",qcmap_cm_cb.ipv6_state,0,0);

  if(hndl == qcmap_cm_cb.ipv6_dsi_net_hndl.handle)
  {
    if (qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_ENABLE)
    {
      if (evt == DSI_EVT_NET_IS_CONN)
      {
	/* 9x25 changes */
        qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_WAN_CONNECTED;
        event = QCMAP_CM_EVENT_WAN_IPv6_CONNECTED;
        callback = TRUE;
        LOG_MSG_INFO1("Received DSI_EVT_NET_IS_CONN in state V6_ENABLE",0,0,0);
        LOG_MSG_INFO1("Transition from V6_ENABLE to V6_WAN_CONNECTED",0,0,0);
        /*deregister from serving system NAS indication to save power*/
        memset(&qcmap_nas_indication_register_req_msg_v01, 0, sizeof(nas_indication_register_req_msg_v01));

        qcmap_nas_indication_register_req_msg_v01.sys_info_valid = TRUE;
        qcmap_nas_indication_register_req_msg_v01.sys_info = 0x00;

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
      else if(evt == DSI_EVT_NET_NO_NET)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in state V6_ENABLE",0,0,0);
	/* 9x25 changes */
        qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_ENABLE;
        event = QCMAP_CM_EVENT_WAN_IPv6_CONNECTING_FAIL;
        callback = TRUE;
      }
    }
    else if(qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_WAN_CONNECTING)
    {
      if(evt == DSI_EVT_NET_IS_CONN)
      {
	  /* 9x25 changes */
          char device[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 2];
          int rval;

          memset(device, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);
          qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_WAN_CONNECTED;
          event = QCMAP_CM_EVENT_WAN_IPv6_CONNECTED;
          callback = TRUE;
          LOG_MSG_INFO1("Received DS_ENETISCONN in state V6_WAN_CONNECTING",0,0,0);
          LOG_MSG_INFO1("Transition from state V6_WAN_CONNECTING to V6_WAN_CONNECTED",0,0,0);
          stop_auto_timer( QCMAP_MSGR_IP_FAMILY_V6_V01 );
          rval = dsi_get_device_name(qcmap_cm_cb.ipv6_dsi_net_hndl.handle, device, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1);

          if(rval != DSI_SUCCESS)
          {
            LOG_MSG_ERROR ("Couldn't get ipv6 rmnet name. rval %d",
                             rval, 0, 0);
            strlcpy((char *)device, QCMAP_V6_DEFAULT_DEVICE_NAME, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1);
          }

          LOG_MSG_INFO1("device_name:%s",device, 0, 0);
          memcpy(&qcmap_cm_cb.ipv6_dsi_device_name, device, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);


          LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv6 WAN Connected",qcmap_cm_cb.handle, 0, 0);
          /*deregister from serving system NAS indication to save power*/
          memset(&qcmap_nas_indication_register_req_msg_v01, 0, sizeof(nas_indication_register_req_msg_v01));

          qcmap_nas_indication_register_req_msg_v01.sys_info_valid = TRUE;
          qcmap_nas_indication_register_req_msg_v01.sys_info = 0x00;

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
      else if(evt == DSI_EVT_NET_NO_NET)
      {
	/* 9x25 changes */
        qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_ENABLE;
        event = QCMAP_CM_EVENT_WAN_IPv6_CONNECTING_FAIL;
        callback = TRUE;
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in V6_WAN_CONNECTING",0,0,0);
        LOG_MSG_INFO1("Transition from V6_WAN_CONNECTING to V6_ENABLE",0,0,0);
	    if(dsi_get_call_end_reason(qcmap_cm_cb.ipv6_dsi_net_hndl.handle,&dsicallend,DSI_IP_FAMILY_V6) == DSI_SUCCESS)
        {
          LOG_MSG_ERROR("Call Disconnected reason type=%d reason code =%d ",dsicallend.reason_type,dsicallend.reason_code,0);
        }
        if(qcmap_cm_cb.disable_in_process)
        {
          qcmap_cm_disable(qcmap_cm_cb.handle, qcmap_cm_errno, &qmi_err_num);
        }
        else if (qcmap_cm_cb.cfg->wan_config.auto_connect)
        {
          /*register for serving system NAS indication*/
          memset(&qcmap_nas_indication_register_req_msg_v01, 0, sizeof(nas_indication_register_req_msg_v01));
          qcmap_nas_indication_register_req_msg_v01.sys_info_valid = TRUE;
          qcmap_nas_indication_register_req_msg_v01.sys_info = 0x01;

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
          start_auto_connect( QCMAP_MSGR_IP_FAMILY_V6_V01 );
        }
      }
      qcmap_cm_cb.qcmap_cm_callback(qcmap_cm_cb.handle, event,
                                      qcmap_cm_cb.qcmap_cm_callback_user_data,&dsicallend);
      return;
    }
    else if(qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_WAN_CONNECTED)
    {
      if(evt == DSI_EVT_NET_IS_CONN)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_IS_CONN in V6_WAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_IS_CONN in state V6_WAN_CONNECTED",0,0,0);
      }
      else if(evt == DSI_EVT_NET_NO_NET)
      {
	/* 9x25 changes */
        qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_ENABLE;
        event = QCMAP_CM_EVENT_WAN_IPv6_DISCONNECTED;
        callback = TRUE;
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in V6_WAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Transition from V6_WAN_CONNECTED to V6_ENABLE",0,0,0);
		if(dsi_get_call_end_reason(qcmap_cm_cb.ipv6_dsi_net_hndl.handle,&dsicallend,DSI_IP_FAMILY_V6) == DSI_SUCCESS)
        {
          LOG_MSG_ERROR("Call Disconnected reason type=%d reason code =%d ",dsicallend.reason_type,dsicallend.reason_code,0);
        }
        qcmap_cm_cb.qcmap_cm_callback(qcmap_cm_cb.handle, event,
                                      qcmap_cm_cb.qcmap_cm_callback_user_data,&dsicallend);

        if(qcmap_cm_cb.disable_in_process)
        {
          qcmap_cm_disable(qcmap_cm_cb.handle, qcmap_cm_errno, &qmi_err_num);
        }
        else if (qcmap_cm_cb.cfg->wan_config.auto_connect)
        {
          /*register for serving system NAS indication*/
          memset(&qcmap_nas_indication_register_req_msg_v01, 0, sizeof(nas_indication_register_req_msg_v01));
          qcmap_nas_indication_register_req_msg_v01.sys_info_valid = TRUE;
          qcmap_nas_indication_register_req_msg_v01.sys_info = 0x01;

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
          start_auto_connect( QCMAP_MSGR_IP_FAMILY_V6_V01 );
        }
	return;
      }
    }
    else if(qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_WAN_DISCONNECTING)
    {
      if(evt == DSI_EVT_NET_IS_CONN)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_IS_CONN in V6_WAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_IS_CONN in V6_WAN_DISCONNECTING",0,0,0);
      }
      else if(evt == DSI_EVT_NET_NO_NET)
      {
	/* 9x25 changes */
        qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_ENABLE;
        event = QCMAP_CM_EVENT_WAN_IPv6_DISCONNECTED;

        /* Resetting timer value after disconnect*/
        stop_auto_timer(QCMAP_MSGR_IP_FAMILY_V6_V01);

        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in V6_WAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Transition from V6_WAN_DISCONNECTING to V6_ENABLE",0,0,0);
        callback = TRUE;
        if(dsi_get_call_end_reason(qcmap_cm_cb.ipv6_dsi_net_hndl.handle,&dsicallend,DSI_IP_FAMILY_V6) == DSI_SUCCESS)
        {
          LOG_MSG_INFO1("Call Disconnected reason type=%d reason code =%d ",dsicallend.reason_type,dsicallend.reason_code,0);
        }
        qcmap_cm_cb.qcmap_cm_callback(qcmap_cm_cb.handle, event,
                                      qcmap_cm_cb.qcmap_cm_callback_user_data,&dsicallend);
       if(qcmap_cm_cb.disable_in_process)
        {
          qcmap_cm_disable(qcmap_cm_cb.handle, qcmap_cm_errno, &qmi_err_num);
        }
		return;
      }
    }
  }

  if (hndl == qcmap_cm_cb.dsi_net_hndl.handle)
  {
    if (qcmap_cm_cb.state == QCMAP_CM_ENABLE)
    {
      if (evt == DSI_EVT_NET_IS_CONN)
      {
	/* 9x25 */
        qcmap_cm_cb.state = QCMAP_CM_WAN_CONNECTED;
        event = QCMAP_CM_EVENT_WAN_CONNECTED;
        callback = TRUE;
        LOG_MSG_INFO1("Received event DSI_EVT_NET_IS_CONN in state ENABLE",0,0,0);
        /*deregister from serving system NAS indication to save power*/
        memset(&qcmap_nas_indication_register_req_msg_v01, 0, sizeof(nas_indication_register_req_msg_v01));

        qcmap_nas_indication_register_req_msg_v01.sys_info_valid = TRUE;
        qcmap_nas_indication_register_req_msg_v01.sys_info = 0x00;

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
      else if (evt == DSI_EVT_NET_NO_NET)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in state ENABLE",0,0,0);
	/* 9x25 */
        qcmap_cm_cb.state = QCMAP_CM_ENABLE;
        event = QCMAP_CM_EVENT_WAN_CONNECTING_FAIL;
        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv4 Backhaul Connecting Fail",
                      qcmap_cm_cb.handle, 0, 0);
        callback = TRUE;
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in WAN_CONNECTING",0,0,0);
      }
    }
    else if (qcmap_cm_cb.state == QCMAP_CM_WAN_CONNECTING)
    {
      if (evt == DSI_EVT_NET_IS_CONN)
      {
        char device[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 2];
        int rval;

        qcmap_cm_cb.state = QCMAP_CM_WAN_CONNECTED;
        event = QCMAP_CM_EVENT_WAN_CONNECTED;
        callback = TRUE;

        LOG_MSG_INFO1("Received DS_ENETISCONN in state WAN_CONNECTING",0,0,0);
        LOG_MSG_INFO1("Transition from state WAN_CONNECTING to WAN_CONNECTED",0,0,0);
        stop_auto_timer(QCMAP_MSGR_IP_FAMILY_V4_V01);
        memset(device, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);

        rval = dsi_get_device_name(qcmap_cm_cb.dsi_net_hndl.handle, device, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1);

        if(rval != DSI_SUCCESS)
        {
          LOG_MSG_ERROR ("Couldn't get ipv4 rmnet name. rval %d",
                         rval, 0, 0);
          strlcpy((char *)device, QCMAP_V4_DEFAULT_DEVICE_NAME, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1);
        }

        //change it later to LOG_MSG_INFO1
        LOG_MSG_INFO1("device_name:%s", device,0,0);
        memcpy(&qcmap_cm_cb.dsi_device_name, device, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);


        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv4 WAN Connected", qcmap_cm_cb.handle, 0, 0);
        /*deregister from serving system NAS indication to save power*/
        memset(&qcmap_nas_indication_register_req_msg_v01, 0, sizeof(nas_indication_register_req_msg_v01));

        qcmap_nas_indication_register_req_msg_v01.sys_info_valid = TRUE;
        qcmap_nas_indication_register_req_msg_v01.sys_info = 0x00;

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
      else if (evt == DSI_EVT_NET_NO_NET)
      {
	/* 9x25 */
        qcmap_cm_cb.state = QCMAP_CM_ENABLE;
        event = QCMAP_CM_EVENT_WAN_CONNECTING_FAIL;
        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv4 Backhaul Connecting Fail",
                      qcmap_cm_cb.handle, 0, 0);
        callback = TRUE;

        if(dsi_get_call_end_reason(qcmap_cm_cb.dsi_net_hndl.handle,&dsicallend,DSI_IP_FAMILY_V4) == DSI_SUCCESS)
        {
          LOG_MSG_INFO1("Call Disconnected reason type=%d reason code =%d ",dsicallend.reason_type,dsicallend.reason_code,0);
        }
        if (callback)
          qcmap_cm_cb.qcmap_cm_callback(qcmap_cm_cb.handle, event,
              qcmap_cm_cb.qcmap_cm_callback_user_data,&dsicallend);

        if(qcmap_cm_cb.disable_in_process)
        {
          qcmap_cm_disable(qcmap_cm_cb.handle, qcmap_cm_errno, &qmi_err_num);
        }
        else if (qcmap_cm_cb.cfg->wan_config.auto_connect)
        {
          /*register for serving system NAS indication*/
          memset(&qcmap_nas_indication_register_req_msg_v01, 0, sizeof(nas_indication_register_req_msg_v01));
          qcmap_nas_indication_register_req_msg_v01.sys_info_valid = TRUE;
          qcmap_nas_indication_register_req_msg_v01.sys_info = 0x01;

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
          start_auto_connect( QCMAP_MSGR_IP_FAMILY_V4_V01 );
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in WAN_CONNECTING",0,0,0);
      }
      return;
    }
   }
    else if (qcmap_cm_cb.state == QCMAP_CM_WAN_CONNECTED)
    {
      if(evt == DSI_EVT_NET_IS_CONN)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_IS_CONN in WAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_IS_CONN in state WAN_CONNECTED",0,0,0);
      }
      else if(evt == DSI_EVT_NET_NO_NET)
      {
	/* 9x25 */
        qcmap_cm_cb.state = QCMAP_CM_ENABLE;
        event = QCMAP_CM_EVENT_WAN_DISCONNECTED;
        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv4 Backhaul Disconnected",qcmap_cm_cb.handle,0,0);
        callback = TRUE;

        if(dsi_get_call_end_reason(qcmap_cm_cb.dsi_net_hndl.handle,&dsicallend,DSI_IP_FAMILY_V4) == DSI_SUCCESS)
        {
          LOG_MSG_INFO1("Call Disconnected reason type=%d reason code =%d ",dsicallend.reason_type,dsicallend.reason_code,0);
        }
        LOG_MSG_INFO1("Received event DSI_EVT_NET_NO_NET in WAN_CONNECTED",0,0,0);
        LOG_MSG_INFO1("Transition from WAN_CONNECTED to ENABLE",0,0,0);
        if (callback)
          qcmap_cm_cb.qcmap_cm_callback(qcmap_cm_cb.handle, event,
                                        qcmap_cm_cb.qcmap_cm_callback_user_data,&dsicallend);
        if(qcmap_cm_cb.disable_in_process)
        {
          qcmap_cm_disable(qcmap_cm_cb.handle, qcmap_cm_errno, &qmi_err_num);
        }
        else if (qcmap_cm_cb.cfg->wan_config.auto_connect)
        {
          /*register for serving system NAS indication*/
          memset(&qcmap_nas_indication_register_req_msg_v01, 0, sizeof(nas_indication_register_req_msg_v01));
          qcmap_nas_indication_register_req_msg_v01.sys_info_valid = TRUE;
          qcmap_nas_indication_register_req_msg_v01.sys_info = 0x01;

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
          start_auto_connect( QCMAP_MSGR_IP_FAMILY_V4_V01 );
        }
        return;
      }
    }
    else if (qcmap_cm_cb.state == QCMAP_CM_WAN_DISCONNECTING)
    {
      if(evt == DSI_EVT_NET_IS_CONN)
      {
        LOG_MSG_INFO1("Received event DSI_EVT_NET_IS_CONN in WAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event DSI_EVT_NET_IS_CONN in WAN_DISCONNECTING",0,0,0);
      }
      else if(evt == DSI_EVT_NET_NO_NET)
      {
        qcmap_cm_cb.state = QCMAP_CM_ENABLE;
        event = QCMAP_CM_EVENT_WAN_DISCONNECTED;
        /* Resetting timer value after disconnect*/
        stop_auto_timer(QCMAP_MSGR_IP_FAMILY_V4_V01);

        LOG_MSG_INFO1("QCMAP AP Handle 0x%04x IPv4 Backhaul Disconnected",
                      qcmap_cm_cb.handle, 0, 0);
        callback = TRUE;
        if(dsi_get_call_end_reason(qcmap_cm_cb.dsi_net_hndl.handle,&dsicallend,DSI_IP_FAMILY_V4) == DSI_SUCCESS)
        {
          LOG_MSG_INFO1("Call Disconnected reason type=%d reason code =%d ",dsicallend.reason_type,dsicallend.reason_code,0);
        }
        LOG_MSG_INFO1("Received event DS_ENETNONET in WAN_DISCONNECTING",0,0,0);
        LOG_MSG_INFO1("Ignore event DS_ENETNONET in WAN_DISCONNECTING",0,0,0);
        if (callback)
          qcmap_cm_cb.qcmap_cm_callback(qcmap_cm_cb.handle, event,
                                        qcmap_cm_cb.qcmap_cm_callback_user_data,&dsicallend);
        if(qcmap_cm_cb.disable_in_process)
        {
          qcmap_cm_disable(qcmap_cm_cb.handle, qcmap_cm_errno, &qmi_err_num);
        }
        return;
      }
    }
  }
  if (callback)
    qcmap_cm_cb.qcmap_cm_callback(qcmap_cm_cb.handle, event,
                              qcmap_cm_cb.qcmap_cm_callback_user_data,NULL);
  return;
}

/*===========================================================================
  FUNCTION  qcmap_cm_process_qmi_nas_ind
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
void
qcmap_cm_process_qmi_nas_ind
(
  qmi_client_type user_handle,                    /* QMI user handle       */
  unsigned int    msg_id,                         /* Indicator message ID  */
  void           *ind_buf,                        /* Raw indication data   */
  unsigned int    ind_buf_len,                    /* Raw data length       */
  void           *ind_cb_data                     /* User call back handle */
)
{
  qmi_client_error_type qmi_error = QMI_NO_ERR;
  nas_sys_info_ind_msg_v01 sys_info_ind;
  nas_service_domain_enum_type_v01 srv_domain         = NAS_SERVICE_DOMAIN_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_domain_enum_type_v01 srv_domain_lte     = NAS_SERVICE_DOMAIN_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_domain_enum_type_v01 srv_domain_hdr     = NAS_SERVICE_DOMAIN_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_domain_enum_type_v01 srv_domain_cdma    = NAS_SERVICE_DOMAIN_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_domain_enum_type_v01 srv_domain_wcdma   = NAS_SERVICE_DOMAIN_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_domain_enum_type_v01 srv_domain_gsm     = NAS_SERVICE_DOMAIN_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_domain_enum_type_v01 srv_domain_tdscdma = NAS_SERVICE_DOMAIN_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_status_enum_type_v01 srv_status         = NAS_SERVICE_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_status_enum_type_v01 srv_status_lte     = NAS_SERVICE_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_status_enum_type_v01 srv_status_hdr     = NAS_SERVICE_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_status_enum_type_v01 srv_status_cdma    = NAS_SERVICE_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_status_enum_type_v01 srv_status_wcdma   = NAS_SERVICE_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_status_enum_type_v01 srv_status_gsm     = NAS_SERVICE_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_status_enum_type_v01 srv_status_tdscdma = NAS_SERVICE_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_roam_status_enum_type_v01 roam_status = NAS_ROAM_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;
  int               err;
  int              *qcmap_cm_errno = &err;
  int ret_val;
  qmi_error_type_v01 qmi_err_num;
  LOG_MSG_INFO1("qcmap_cm_process_qmi_nas_ind: user_handle %d msg_id %d ind_buf_len %d.",
                user_handle,
                msg_id,
                ind_buf_len);

  memset(&sys_info_ind, 0, sizeof(nas_sys_info_ind_msg_v01));

  /* Process based on indication type */
  switch (msg_id)
  {
    case QMI_NAS_SYS_INFO_IND_MSG_V01:
      qmi_error = qmi_client_message_decode(user_handle,
                                            QMI_IDL_INDICATION,
                                            msg_id,
                                            ind_buf,
                                            ind_buf_len,
                                            &sys_info_ind,
                                            sizeof(nas_sys_info_ind_msg_v01));
      if (qmi_error == QMI_NO_ERR)
      {
        /* Local domain and status variables */
        srv_domain_lte     = sys_info_ind.lte_sys_info.common_sys_info.srv_domain;
        srv_domain_hdr     = sys_info_ind.hdr_sys_info.common_sys_info.srv_domain;
        srv_domain_cdma    = sys_info_ind.cdma_sys_info.common_sys_info.srv_domain;
        srv_domain_wcdma   = sys_info_ind.wcdma_sys_info.common_sys_info.srv_domain;
        srv_domain_gsm     = sys_info_ind.gsm_sys_info.common_sys_info.srv_domain;
        srv_domain_tdscdma = sys_info_ind.tdscdma_sys_info.common_sys_info.srv_domain;
        srv_status_lte     = sys_info_ind.lte_srv_status_info.srv_status;
        srv_status_hdr     = sys_info_ind.hdr_srv_status_info.srv_status;
        srv_status_cdma    = sys_info_ind.cdma_srv_status_info.srv_status;
        srv_status_wcdma   = sys_info_ind.wcdma_srv_status_info.srv_status;
        srv_status_gsm     = sys_info_ind.gsm_srv_status_info.srv_status;
        srv_status_tdscdma = sys_info_ind.tdscdma_srv_status_info.srv_status;

        /* First Get the Service Domain. */
        /* If the LTE System Info is valid, check the LTE Service Domain. */
        if (sys_info_ind.lte_sys_info_valid == TRUE &&
            sys_info_ind.lte_sys_info.common_sys_info.srv_domain_valid == TRUE &&
            srv_status_lte == NAS_SYS_SRV_STATUS_SRV_V01 &&
            (srv_domain_lte == SYS_SRV_DOMAIN_PS_ONLY_V01 || srv_domain_lte == SYS_SRV_DOMAIN_CS_PS_V01))
        {
          srv_domain = sys_info_ind.lte_sys_info.common_sys_info.srv_domain;
          srv_status = sys_info_ind.lte_srv_status_info.srv_status;
          if (sys_info_ind.lte_sys_info.common_sys_info.roam_status_valid == TRUE)
          {
            roam_status = sys_info_ind.lte_sys_info.common_sys_info.roam_status;
          }
          LOG_MSG_INFO1("qcmap_cm_connect_backhaul: lte Service Domain %d Status %d, roam status %d", srv_domain, srv_status, roam_status);
        }
        /* If the HDR System Info is valid, check the HDR Service Domain. */
        else if (sys_info_ind.hdr_sys_info_valid == TRUE &&
            sys_info_ind.hdr_sys_info.common_sys_info.srv_domain_valid == TRUE &&
            srv_status_hdr == NAS_SYS_SRV_STATUS_SRV_V01 &&
            (srv_domain_hdr == SYS_SRV_DOMAIN_PS_ONLY_V01 || srv_domain_hdr == SYS_SRV_DOMAIN_CS_PS_V01))
        {
          srv_domain = sys_info_ind.hdr_sys_info.common_sys_info.srv_domain;
          srv_status = sys_info_ind.hdr_srv_status_info.srv_status;
          if (sys_info_ind.hdr_sys_info.common_sys_info.roam_status_valid == TRUE)
          {
            roam_status = sys_info_ind.hdr_sys_info.common_sys_info.roam_status;
          }
          LOG_MSG_INFO1("qcmap_cm_connect_backhaul: HDR Service Domain %d Status %d, roam status %d", srv_domain, srv_status, roam_status);
        }
        /* If the CDMA System Info is valid, check the CDMA Service Domain. */
        else if (sys_info_ind.cdma_sys_info_valid == TRUE &&
            sys_info_ind.cdma_sys_info.common_sys_info.srv_domain_valid == TRUE &&
            srv_status_cdma == NAS_SYS_SRV_STATUS_SRV_V01 &&
            (srv_domain_cdma == SYS_SRV_DOMAIN_PS_ONLY_V01 || srv_domain_cdma == SYS_SRV_DOMAIN_CS_PS_V01))
        {
          srv_domain = sys_info_ind.cdma_sys_info.common_sys_info.srv_domain;
          srv_status = sys_info_ind.cdma_srv_status_info.srv_status;
          if (sys_info_ind.cdma_sys_info.common_sys_info.roam_status_valid == TRUE)
          {
            roam_status = sys_info_ind.cdma_sys_info.common_sys_info.roam_status;
          }
          LOG_MSG_INFO1("qcmap_cm_connect_backhaul: CDMA Service Domain %d Status %d, roam status %d", srv_domain, srv_status, roam_status);
        }
        /* If the WCDMA System Info is valid, check the WCDMA Service Domain. */
        else if (sys_info_ind.wcdma_sys_info_valid == TRUE &&
            sys_info_ind.wcdma_sys_info.common_sys_info.srv_domain_valid == TRUE &&
            srv_status_wcdma == NAS_SYS_SRV_STATUS_SRV_V01 &&
            (srv_domain_wcdma == SYS_SRV_DOMAIN_PS_ONLY_V01 || srv_domain_wcdma == SYS_SRV_DOMAIN_CS_PS_V01))
        {
          srv_domain = sys_info_ind.wcdma_sys_info.common_sys_info.srv_domain;
          srv_status = sys_info_ind.wcdma_srv_status_info.srv_status;
          if (sys_info_ind.wcdma_sys_info.common_sys_info.roam_status_valid == TRUE)
          {
            roam_status = sys_info_ind.wcdma_sys_info.common_sys_info.roam_status;
          }
          LOG_MSG_INFO1("qcmap_cm_connect_backhaul: WCDMA Service Domain %d Status %d, roam status %d", srv_domain, srv_status, roam_status);
        }
        /* If the GSM System Info is valid, check the GSM Service Domain. */
        else if (sys_info_ind.gsm_sys_info_valid == TRUE &&
            sys_info_ind.gsm_sys_info.common_sys_info.srv_domain_valid == TRUE &&
            srv_status_gsm == NAS_SYS_SRV_STATUS_SRV_V01 &&
            (srv_domain_gsm == SYS_SRV_DOMAIN_PS_ONLY_V01 || srv_domain_gsm == SYS_SRV_DOMAIN_CS_PS_V01))
        {
          srv_domain = sys_info_ind.gsm_sys_info.common_sys_info.srv_domain;
          srv_status = sys_info_ind.gsm_srv_status_info.srv_status;
          if (sys_info_ind.gsm_sys_info.common_sys_info.roam_status_valid == TRUE)
          {
            roam_status = sys_info_ind.gsm_sys_info.common_sys_info.roam_status;
          }
          LOG_MSG_INFO1("qcmap_cm_connect_backhaul: GSM Service Domain %d Status %d, roam status %d", srv_domain, srv_status, roam_status);
        }
        /* If the TDSCDMA System Info is valid, check the TDSCDMA Service Domain. */
        else if (sys_info_ind.tdscdma_sys_info_valid == TRUE &&
            sys_info_ind.tdscdma_sys_info.common_sys_info.srv_domain_valid == TRUE &&
            srv_status_tdscdma == NAS_SYS_SRV_STATUS_SRV_V01 &&
            (srv_domain_tdscdma == SYS_SRV_DOMAIN_PS_ONLY_V01 || srv_domain_tdscdma == SYS_SRV_DOMAIN_CS_PS_V01))
        {
          srv_domain = sys_info_ind.tdscdma_sys_info.common_sys_info.srv_domain;
          srv_status = sys_info_ind.tdscdma_srv_status_info.srv_status;
          if (sys_info_ind.tdscdma_sys_info.common_sys_info.roam_status_valid == TRUE)
          {
            roam_status = sys_info_ind.tdscdma_sys_info.common_sys_info.roam_status;
          }
          LOG_MSG_INFO1("qcmap_cm_connect_backhaul: TDSCDMA Service Domain %d Status %d, roam status %d", srv_domain, srv_status, roam_status);
        }

        if ((srv_status == NAS_SYS_SRV_STATUS_SRV_V01) &&
            (srv_domain == SYS_SRV_DOMAIN_PS_ONLY_V01 || srv_domain == SYS_SRV_DOMAIN_CS_PS_V01))
        {
          ds_system_call("echo QCMAP: Modem in service NAS indication received > /dev/kmsg",
                          strlen("echo QCMAP: Modem in service NAS indication received > /dev/kmsg"));
          if(roam_status != NAS_ROAM_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01)
          {
            qcmap_cm_cb.roaming_indicator = roam_status;
          } else
          {
            qcmap_cm_cb.roaming_indicator = 0;
          }
          if (qcmap_cm_cb.cfg->wan_config.auto_connect && qcmap_cm_cb.state == QCMAP_CM_ENABLE &&
              !qcmap_cm_cb.disable_in_process)
          {
            LOG_MSG_INFO1("qcmap_cm_process_qmi_nas_ind: Auto connect start", 0, 0, 0);
            qcmap_cm_cb.backhaul_service = TRUE;
            if( qcmap_cm_cb.cfg->wan_config.auto_connect_timer_running_v4 )
            {
              stop_auto_timer(QCMAP_MSGR_IP_FAMILY_V4_V01);
            }
            qcmap_cm_connect_backhaul(qcmap_cm_cb.handle, QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01, qcmap_cm_errno, &qmi_err_num);
          }
          if(qcmap_cm_cb.cfg->wan_config.auto_connect && qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_ENABLE &&
             !qcmap_cm_cb.disable_in_process)
          {
            LOG_MSG_INFO1("qcmap_cm_process_qmi_nas_ind: Auto connect start", 0, 0, 0);
            qcmap_cm_cb.backhaul_service = TRUE;
            if( qcmap_cm_cb.cfg->wan_config.auto_connect_timer_running_v6 )
            {
              stop_auto_timer(QCMAP_MSGR_IP_FAMILY_V6_V01);
            }
            qcmap_cm_connect_backhaul(qcmap_cm_cb.handle, QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01,  qcmap_cm_errno, &qmi_err_num);
          }
        }
      }
      else
      {
        LOG_MSG_INFO1("qcmap_cm_process_qmi_nas_ind: could not decode message %d", qmi_error, 0, 0);
      }
      break;
    default:
      /* Ignore all other indications */
      break;
   }

  return;
}
/*===========================================================================
  FUNCTION  qcmap_cm_process_timer_ind
===========================================================================*/
/*!
@brief
 Processes an incoming QMI timer Indication.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void
qcmap_cm_process_qmi_timer_ind
(
 unsigned int    msg_id                         /* Indicator message ID  */
 )
{
  int qcmap_cm_errno,ret_val;
  qmi_error_type_v01 qmi_err_num;
  qcmap_cm_event_e  qcmap_event;

  LOG_MSG_INFO1("qcmap_cm_process_qmi_timer_ind() msg=%d",msg_id,0,0);
  switch(msg_id)
  {
    case AUTO_CONNECT_V4:

      LOG_MSG_INFO1("Calling qcmap_cm_connect_backhaul has a part of \
          auto connect for IPv4 call",0, 0, 0);

      ret_val = qcmap_cm_connect_backhaul(qcmap_cm_cb.handle,
          QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01, &qcmap_cm_errno, &qmi_err_num);
      if (ret_val == QCMAP_CM_SUCCESS)
      {
        LOG_MSG_INFO1("Call to Connect backhaul Success \n",0,0,0);
      }

      if (ret_val == QCMAP_CM_ERROR && qcmap_cm_errno == QCMAP_CM_EWOULDBLOCK)
      {
        LOG_MSG_INFO1("QCMAP WAN Connecting Inprogress \n",0,0,0);
      }
      else if( ret_val == QCMAP_CM_ERROR )
      {
        LOG_MSG_INFO1( "QCMAP WAN Connecting Fail, ret_val %d qcmap_cm_errno %d\n",
            ret_val, qcmap_cm_errno,0 );
      }
      break;

    case AUTO_CONNECT_V6:

      LOG_MSG_INFO1("Calling qcmap_cm_connect_backhaul has a part of \
          auto connect for IPv6 call",0, 0, 0);

      ret_val = qcmap_cm_connect_backhaul(qcmap_cm_cb.handle,
          QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01, &qcmap_cm_errno, &qmi_err_num);
      if (ret_val == QCMAP_CM_SUCCESS)
      {
        LOG_MSG_INFO1("Call to Connect backhaul Success \n",0,0,0);
      }

      if (ret_val == QCMAP_CM_ERROR && qcmap_cm_errno == QCMAP_CM_EWOULDBLOCK)
      {
        LOG_MSG_INFO1("QCMAP IPV6 WAN Connecting Inprogress \n",0,0,0);
      }
      else if( ret_val == QCMAP_CM_ERROR )
      {
        LOG_MSG_INFO1( "QCMAP IPV6 WAN Connecting Fail, ret_val %d qcmap_cm_errno %d\n",
            ret_val, qcmap_cm_errno,0 );
      }
      break;

    case STA_ASSOC_FAIL:
      {
         qcmap_event = QCMAP_CM_EVENT_STA_ASSOCIATION_FAIL;
         qcmap_cm_cb.qcmap_cm_callback(qcmap_cm_cb.handle, qcmap_event,
                                qcmap_cm_cb.qcmap_cm_callback_user_data,NULL);
      }
      break;

    case STA_DHCP_FAIL:
      {
        qcmap_event = QCMAP_CM_EVENT_STA_DHCP_IP_ASSIGNMENT_FAIL;
        qcmap_cm_cb.qcmap_cm_callback(qcmap_cm_cb.handle, qcmap_event,
                                qcmap_cm_cb.qcmap_cm_callback_user_data,NULL);
      }
      break;

    default:
      LOG_MSG_INFO1("Unknown Message id \n",0,0,0);
  }

  return;
}

/*===========================================================================
  FUNCTION  qcmap_cm_process_sta_ind
===========================================================================*/
/*!
@brief
 Processes an incoming QMI STA Indication.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void
qcmap_cm_process_sta_ind
(
  uint32 sta_cookie,
  qcmap_sta_event_t event
)
{
  qcmap_cm_event_e  qcmap_event;

  LOG_MSG_INFO1("qcmap_cm_process_qmi_sta_ind: sta_cookie 0x%x event %d .",
                sta_cookie,
                event, 0);

  if ( sta_cookie != qcmap_cm_cb.sta_cookie )
  {
    LOG_MSG_ERROR("qcmap_cm_process_qmi_sta_ind: Cookie invalid. .", 0, 0, 0);
    return;
  }

  switch ( event )
  {
    case STA_CONNECTED:
      qcmap_event = QCMAP_CM_EVENT_STA_CONNECTED;
      break;
    case STA_DISCONNECTED:
      qcmap_event = QCMAP_CM_EVENT_STA_DISCONNECTED;
      break;
    default:
      LOG_MSG_ERROR("qcmap_cm_process_qmi_sta_ind: Unsupported STA Event : %d.", event, 0, 0);
      return;
  }

  qcmap_cm_cb.qcmap_cm_callback(qcmap_cm_cb.handle, qcmap_event,

                                qcmap_cm_cb.qcmap_cm_callback_user_data,NULL);


  return;
}

/*===========================================================================
  FUNCTION  qcmap_cm_process_cradle_ind
===========================================================================*/
/*!
@brief
 Processes an incoming QMI Cradle Indication.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void
qcmap_cm_process_cradle_ind
(
  qcmap_cradle_event_t event
)
{
  qcmap_cm_event_e  qcmap_event;

  switch ( event )
  {
    case CRADLE_CONNECTED:
      qcmap_event = QCMAP_CM_EVENT_CRADLE_CONNECTED;
      break;
    case CRADLE_DISCONNECTED:
      qcmap_event = QCMAP_CM_EVENT_CRADLE_DISCONNECTED;
      break;
    default:
      LOG_MSG_ERROR("qcmap_cm_process_qmi_cradle_ind: Unsupported Cradle Event : %d.", event, 0, 0);
      return;
  }

  qcmap_cm_cb.qcmap_cm_callback(qcmap_cm_cb.handle, qcmap_event,

                                qcmap_cm_cb.qcmap_cm_callback_user_data,NULL);


  return;
}

/*===========================================================================
  FUNCTION  qcmap_cm_qmi_nas_ind
===========================================================================*/
/*!
@brief
 Receives an incoming QMI NAS Indication.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
qcmap_cm_qmi_nas_ind
(
  qmi_client_type user_handle,                    /* QMI user handle       */
  unsigned int    msg_id,                         /* Indicator message ID  */
  void           *ind_buf,                        /* Raw indication data   */
  unsigned int    ind_buf_len,                    /* Raw data length       */
  void           *ind_cb_data                     /* User call back handle */
)
{
  qcmap_nas_buffer_t qcmap_nas_buffer;
  int numBytes=0, len;
  struct sockaddr_un nas_qcmap;
  void *buf;

  LOG_MSG_INFO1("qcmap_cm_qmi_nas_ind: user_handle %d msg_id %d ind_buf_len %d.",
                user_handle,
                msg_id,
                ind_buf_len);

  nas_qcmap.sun_family = AF_UNIX;
  strcpy(nas_qcmap.sun_path, QCMAP_NAS_UDS_FILE);
  len = strlen(nas_qcmap.sun_path) + sizeof(nas_qcmap.sun_family);

  buf = malloc(ind_buf_len);
  ds_assert(buf != NULL);

  qcmap_nas_buffer.user_handle = user_handle;
  qcmap_nas_buffer.msg_id = msg_id;
  memcpy(buf, ind_buf, ind_buf_len);
  qcmap_nas_buffer.ind_buf = buf;
  qcmap_nas_buffer.ind_buf_len = ind_buf_len;

  /* ind_cb_data is not used, if in use, make a copy here */
  qcmap_nas_buffer.ind_cb_data = ind_cb_data;


  if ((numBytes = sendto(nas_qcmap_sockfd, (void *)&qcmap_nas_buffer, sizeof(qcmap_nas_buffer_t), 0,
             (struct sockaddr *)&nas_qcmap, len)) == -1)
  {
    LOG_MSG_ERROR("Send Failed from nas_callback context", 0, 0, 0);
    return QCMAP_CM_ERROR;
  }

  return QCMAP_CM_ENOERROR;
}
/*===========================================================================
  FUNCTION  qcmap_cm_qmi_timer_ind
===========================================================================*/
/*!
@brief
 Receives an incoming QMI TIMER Indication.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int
qcmap_cm_qmi_timer_ind
(
  qcmap_timer_enum_t timer_id                          /* Indicator message ID  */
)
{
  qcmap_timer_buffer_t qcmap_timer_buffer;
  int numBytes=0, len;
  struct sockaddr_un timer_qcmap;
  void *buf;

  LOG_MSG_INFO1("qcmap_cm_qmi_timer_ind: msg_id %d",
      timer_id,
      0,0);

  timer_qcmap.sun_family = AF_UNIX;
  strcpy(timer_qcmap.sun_path, QCMAP_TIMER_UDS_FILE);
  len = strlen(timer_qcmap.sun_path) + sizeof(timer_qcmap.sun_family);

  qcmap_timer_buffer.msg_id = timer_id;

  if ((numBytes = sendto(timer_qcmap_sockfd, (void *)&qcmap_timer_buffer, sizeof(qcmap_timer_buffer_t), 0,
          (struct sockaddr *)&timer_qcmap, len)) == -1)
  {
    LOG_MSG_ERROR("Send Failed from nas_callback context", 0, 0, 0);
    return QCMAP_CM_ERROR;
  }

  return QCMAP_CM_ENOERROR;
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
  int             *qcmap_cm_errno,                /* Error condition value */
  qmi_error_type_v01 *qmi_err_num
)
{
  char c;
  int qcmap_cm_handle = 0;
  int qmi_err;
  int ret_val;
  pthread_mutexattr_t attr;
  qmi_idl_service_object_type nas_qmi_idl_service_object;
  nas_indication_register_req_msg_v01 qcmap_nas_indication_register_req_msg_v01;
  nas_indication_register_resp_msg_v01 qcmap_nas_indication_register_resp_msg_v01;
  uint32_t num_services = 0, num_entries = 0;
  qmi_service_info info[10];
  qmi_client_error_type qmi_error;

  LOG_MSG_INFO1("qcmap_cm_enable: enter",0,0,0);

  ds_assert(qcmap_cm_callback != NULL);
  ds_assert(qcmap_cm_cfg != NULL);
  ds_assert(qcmap_cm_errno != NULL);
  ds_assert(qmi_err_num != NULL);

  if (qcmap_cm_cb.init == FALSE)
  {
    LOG_MSG_INFO1("qcmap_cm_enable: init",0,0,0);

    /* If it is the first time call to QCMAP CM */
    pthread_mutexattr_init(&attr);

    /* Set mutex to be reentry */
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&qcmap_cm_cb.qcmap_cm_mutex, &attr);
    qcmap_cm_cb.init = TRUE;
  }

  qcmap_cm_cb.cfg = qcmap_cm_cfg;

  if (qcmap_cm_cb.disable_in_process)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    LOG_MSG_ERROR( "qcmap_cm_enable fail because disable still in progress", 0, 0, 0);
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    return QCMAP_CM_ERROR;
  }

  if ( ((qcmap_cm_cb.dsi_net_hndl.handle != NULL) &&
       (qcmap_cm_cb.state > QCMAP_CM_DISABLE)) ||
       ((qcmap_cm_cb.ipv6_dsi_net_hndl.handle != NULL) &&
       (qcmap_cm_cb.ipv6_state > QCMAP_CM_V6_DISABLE)))
  {
    *qcmap_cm_errno = QCMAP_CM_ENOERROR;
    LOG_MSG_INFO1( "qcmap_cm_enable : already enabled",0,0,0);
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    return qcmap_cm_cb.handle;
  }

  qcmap_cm_cb.qcmap_cm_callback = qcmap_cm_callback;
  qcmap_cm_cb.qcmap_cm_callback_user_data = qcmap_cm_callback_user_data;

  memset( &qcmap_cm_cb.dsi_net_hndl, 0x0, sizeof(qcmap_cm_cb.dsi_net_hndl) );
  memset( &qcmap_cm_cb.ipv6_dsi_net_hndl, 0x0, sizeof(qcmap_cm_cb.ipv6_dsi_net_hndl) );

  nas_qmi_idl_service_object = nas_get_service_object_v01();
  if (nas_qmi_idl_service_object == NULL)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    LOG_MSG_INFO1("qcpmap service object(nas) not available.",0,0,0);
    *qmi_err_num = QMI_ERR_INTERNAL_V01;
    return qcmap_cm_handle;
  }

  LOG_MSG_INFO1("qcmap_cm_enable: nas_get_service_object_v01",0,0,0);

  qmi_error = qmi_client_notifier_init(nas_qmi_idl_service_object,
                                       &qcmap_cm_cb.qmi_nas_os_params,
                                       &qcmap_cm_cb.qmi_nas_notifier);

  LOG_MSG_INFO1("qcmap_cm_enable: qmi_client_notifier_init(nas): %d",
                qmi_error,0,0);

  if (qmi_error < 0)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    LOG_MSG_INFO1("qmi_client_notifier_init(nas) returned %d",
                  qmi_error,0,0);
    *qmi_err_num = QMI_ERR_INTERNAL_V01;
    return qcmap_cm_handle;
  }

  /* Check if the service is up, if not wait on a signal */
  while(1)
  {
    qmi_error = qmi_client_get_service_list(nas_qmi_idl_service_object,
                                            NULL,
                                            NULL,
                                            &num_services);
    LOG_MSG_INFO1("qcmap_cm_enable: qmi_client_get_service_list: %d", qmi_error,0,0);
    if(qmi_error == QMI_NO_ERR)
      break;
    /* wait for server to come up */
    QMI_CCI_OS_SIGNAL_WAIT(&qcmap_cm_cb.qmi_nas_os_params, 0);
  }

  num_entries = num_services;

  LOG_MSG_INFO1("qcmap_cm_enable: qmi_client_get_service_list: num_e %d num_s %d",
                num_entries, num_services,0);

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
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    LOG_MSG_INFO1("Can not get nas service list %d", qmi_error,0,0);
    *qmi_err_num = QMI_ERR_INTERNAL_V01;
    return qcmap_cm_handle;
  }

  LOG_MSG_INFO1("qcmap_cm_enable: calling qmi_client_init",0,0,0);

  qmi_error = qmi_client_init(&info[0],
                              nas_qmi_idl_service_object,
                              qcmap_cm_qmi_nas_ind,
                              NULL,
                              NULL,
                              &qcmap_cm_cb.qmi_nas_handle);

  LOG_MSG_INFO1("qcmap_cm_enable: qmi_client_init: %d",
                qmi_error,0,0);

  if (qmi_error != QMI_NO_ERR)
  {
    qmi_client_release(qcmap_cm_cb.qmi_nas_notifier);
    qcmap_cm_cb.qmi_nas_notifier = NULL;
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    LOG_MSG_INFO1( "Can not init nas client %d",
                   qmi_error,0,0);
    *qmi_err_num = QMI_ERR_INTERNAL_V01;
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
  qcmap_nas_indication_register_req_msg_v01.req_serving_system_valid = TRUE;
  qcmap_nas_indication_register_req_msg_v01.req_serving_system = 0x00;

  /*register for serving system NAS indication if autoconnect is enabled*/
  if(qcmap_cm_cb.cfg->wan_config.auto_connect)
  {
    LOG_MSG_INFO1("Registering for NAS Sys Info indications",0,0,0);
    qcmap_nas_indication_register_req_msg_v01.sys_info_valid = TRUE;
    qcmap_nas_indication_register_req_msg_v01.sys_info = 0x01;
   }
   else
   {
     LOG_MSG_INFO1("Deregistering from NAS Sys info indications",0,0,0);
    qcmap_nas_indication_register_req_msg_v01.sys_info_valid = TRUE;
    qcmap_nas_indication_register_req_msg_v01.sys_info = 0x00;
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
     *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
     LOG_MSG_ERROR( "Can not perform NAS indication register %d",
                   qmi_error, 0, 0);
     *qmi_err_num = QMI_ERR_INTERNAL_V01;
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

  qcmap_cm_cb.state = QCMAP_CM_ENABLE;
  qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_ENABLE;

  LOG_MSG_INFO1("Enable: STATES V4 %d V6 %d", qcmap_cm_cb.state, qcmap_cm_cb.ipv6_state,0);
  qcmap_cm_handle = qcmap_cm_cb.handle;

  if( qcmap_cm_cb.cfg->wan_config.auto_connect )
  {
    if ( qcmap_cm_cb.cfg->lan_config.enable_ipv4 )
    {
      ret_val = qcmap_cm_connect_backhaul(qcmap_cm_handle,
                            QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01, qcmap_cm_errno, qmi_err_num);
      if (ret_val == QCMAP_CM_ERROR && *qcmap_cm_errno == QCMAP_CM_EWOULDBLOCK)
      {
        LOG_MSG_INFO1("QCMAP WAN Connecting Inprogress \n",0,0,0);
      }
      else
      {
        LOG_MSG_INFO1( "QCMAP WAN Connecting Fail, ret_val %d qcmap_cm_errno %d\n",
            ret_val, qcmap_cm_errno,0 );
      }
    }
    if ( qcmap_cm_cb.cfg->lan_config.enable_ipv6 )
    {
      ret_val = qcmap_cm_connect_backhaul(qcmap_cm_handle,
                            QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01, qcmap_cm_errno, qmi_err_num);


      if (ret_val == QCMAP_CM_ERROR && *qcmap_cm_errno == QCMAP_CM_EWOULDBLOCK)

      {
        LOG_MSG_INFO1("QCMAP IPV6 WAN Connecting Inprogress \n",0,0,0);
      }
      else
      {
        LOG_MSG_INFO1( "QCMAP IPV6 WAN Connecting Fail, ret_val %d qcmap_cm_errno %d\n",
            ret_val, qcmap_cm_errno,0 );
      }
    }
  }
  /* Initialize STA Cookie value. */
  qcmap_cm_cb.sta_cookie = QCMAP_STA_COOKIE;

  LOG_MSG_INFO1("QCMAP AP Handle 0x%04x Enabled", qcmap_cm_handle,0,0);
  qcmap_cm_cb.qcmap_cm_callback(qcmap_cm_cb.handle,
                                QCMAP_CM_EVENT_ENABLED,
                                qcmap_cm_cb.qcmap_cm_callback_user_data,NULL);


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
  int *qcmap_cm_errno,                           /* Error condition value  */
  qmi_error_type_v01 *qmi_err_num
)
{
  int ret = QCMAP_CM_SUCCESS;
  sint15 dss_errno;
  int qmi_err, i, qcmap_cm_err, ret_val;
  qmi_client_error_type qmi_error;

  LOG_MSG_INFO1("qcmap_cm_disable: enter", 0, 0, 0);
  ds_assert(qcmap_cm_errno != NULL);


  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    LOG_MSG_INFO1( "Wrong QCMAP CM Handle",
                   0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state == QCMAP_CM_DISABLE &&
      qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_DISABLE)
  {
    *qcmap_cm_errno = QCMAP_CM_EALDDISCONN;
    ret = QCMAP_CM_SUCCESS;
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    LOG_MSG_ERROR( "Already disconnected...",0,0,0);
    return ret;
  }

  if(qcmap_cm_cb.state > QCMAP_CM_ENABLE || qcmap_cm_cb.ipv6_state > QCMAP_CM_V6_ENABLE)
  {
    qcmap_cm_cb.disable_in_process = TRUE;
    if ( qcmap_cm_cb.state > QCMAP_CM_ENABLE )
    {
      ret_val = qcmap_cm_disconnect_backhaul(qcmap_cm_cb.handle, QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01, qcmap_cm_errno, qmi_err_num);
      if (ret_val == QCMAP_CM_ERROR && *qcmap_cm_errno != QCMAP_CM_EWOULDBLOCK)
      {
        /* IPV4 Backhaul disconnecting failed. */
        LOG_MSG_ERROR("IPV4 backhaul disconnecting failed: Error %x", *qmi_err_num, 0, 0);
        return ret_val;
      }
    }
    if ( qcmap_cm_cb.ipv6_state > QCMAP_CM_V6_ENABLE )
    {
      ret_val = qcmap_cm_disconnect_backhaul(qcmap_cm_cb.handle, QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01, qcmap_cm_errno, qmi_err_num);
      if (ret_val == QCMAP_CM_ERROR && *qcmap_cm_errno != QCMAP_CM_EWOULDBLOCK)
      {
        /* IPV6 Backhaul disconnecting failed. */
        LOG_MSG_ERROR("IPV6 backhaul disconnecting failed: Error %x", *qmi_err_num, 0, 0);
        return ret_val;
      }
    }
  }
  else if(qcmap_cm_cb.state <= QCMAP_CM_ENABLE && qcmap_cm_cb.ipv6_state <= QCMAP_CM_V6_ENABLE)
  {
    qcmap_cm_cb.state = QCMAP_CM_DISABLE;
    qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_DISABLE;
    LOG_MSG_INFO1("Disable: STATES V4 %d V6 %d", qcmap_cm_cb.state, qcmap_cm_cb.ipv6_state, 0);


    qmi_error = qmi_client_release(qcmap_cm_cb.qmi_nas_notifier);
    qcmap_cm_cb.qmi_nas_notifier = NULL;

    if (qmi_error != QMI_NO_ERR)
    {
      LOG_MSG_ERROR("Can not release client nas notifier %d",
                  qmi_error, 0, 0);
    }
    qmi_error = qmi_client_release(qcmap_cm_cb.qmi_nas_handle);
    qcmap_cm_cb.qmi_nas_handle = NULL;

    if (qmi_error != QMI_NO_ERR)
    {
      LOG_MSG_ERROR("Can not release client nas handle %d",
                  qmi_error, 0, 0);
    }

    if (qmi_error != QMI_NO_ERR)
    {
      LOG_MSG_ERROR("Can not release client wds handle %d",
                  qmi_error, 0, 0);
    }

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

    qcmap_cm_cb.disable_in_process = FALSE;

    qcmap_cm_cb.sta_cookie = 0;

    /* qmi_err_num is set to QCMAP_CM_DISCONNECTED to indicate clients that MobileAP is
    is disabled. This is being used for IoE 9x25. */
    *qcmap_cm_errno = QCMAP_CM_DISCONNECTED;

    qcmap_cm_cb.qcmap_cm_callback(qcmap_cm_cb.handle,
                                QCMAP_CM_EVENT_DISABLED,
                                qcmap_cm_cb.qcmap_cm_callback_user_data,NULL);

    LOG_MSG_INFO1("QCMAP AP Handle 0x%04x Disabled",
                qcmap_cm_handle, 0, 0);

  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_ENABLE_IPV6()

DESCRIPTION

  It will enable IPv6.

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
int qcmap_cm_enable_ipv6
(
  int      qcmap_cm_handle,                   /* Handle for MobileAP CM    */
  qmi_error_type_v01 *qmi_err_num             /* Error condition value     */
)
{
  int ret = QCMAP_CM_SUCCESS;
  int qcmap_cm_errno;
  ds_assert(qmi_err_num != NULL);

  LOG_MSG_INFO1("qcmap_cm_enable_ipv6: enter", 0, 0, 0);

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if ( qcmap_cm_cb.cfg->wan_config.auto_connect &&
       qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_ENABLE)
  {
    qcmap_cm_connect_backhaul(qcmap_cm_cb.handle,
                              QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01, &qcmap_cm_errno, qmi_err_num);
  }

  return QCMAP_CM_SUCCESS;

}

/*===========================================================================

FUNCTION QCMAP_CM_DISABLE_IPV6()

DESCRIPTION

  It will disable IPv6.

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
int qcmap_cm_disable_ipv6
(
  int      qcmap_cm_handle,                   /* Handle for MobileAP CM    */
  int     *qcmap_cm_errno,                    /* Error condition value     */
  qmi_error_type_v01 *qmi_err_num
)
{
  int ret = QCMAP_CM_SUCCESS;

  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_disable_ipv6: enter", 0, 0, 0);

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  ret = qcmap_cm_disconnect_backhaul(qcmap_cm_cb.handle,
                                     QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01,
                                     qcmap_cm_errno, qmi_err_num);

  return ret;

}

/*===========================================================================

FUNCTION QCMAP_CM_ENABLE_IPV4()

DESCRIPTION

  It will enable IPv4 backhaul functionlity.

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
int qcmap_cm_enable_ipv4
(
  int      qcmap_cm_handle,                   /* Handle for MobileAP CM    */
  qmi_error_type_v01 *qmi_err_num             /* Error condition value     */
)
{
  int ret = QCMAP_CM_SUCCESS;
  int qcmap_cm_errno;
  ds_assert(qmi_err_num != NULL);

  LOG_MSG_INFO1("qcmap_cm_enable_ipv4: enter", 0, 0, 0);

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if ( qcmap_cm_cb.cfg->wan_config.auto_connect &&
       qcmap_cm_cb.state == QCMAP_CM_ENABLE)
  {
    qcmap_cm_connect_backhaul(qcmap_cm_cb.handle,
                              QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01, &qcmap_cm_errno, qmi_err_num);
  }

  return QCMAP_CM_SUCCESS;

}

/*===========================================================================

FUNCTION QCMAP_CM_DISABLE_IPV4()

DESCRIPTION

  It will disable IPv4 backhaul functionality.

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
int qcmap_cm_disable_ipv4
(
  int      qcmap_cm_handle,                   /* Handle for MobileAP CM    */
  int     *qcmap_cm_errno,                    /* Error condition value     */
  qmi_error_type_v01 *qmi_err_num
)
{
  int ret = QCMAP_CM_SUCCESS;

  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_disable_ipv4: enter", 0, 0, 0);

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  ret = qcmap_cm_disconnect_backhaul(qcmap_cm_cb.handle,
                                     QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01,
                                     qcmap_cm_errno, qmi_err_num);

  return ret;

}

int get_nas_config
(
  int  qcmap_cm_handle,   /* Handle for MobileAP CM */
  int *qcmap_cm_errno,     /* Error condition value  */
  qmi_error_type_v01 *qmi_err_num
)
{
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  nas_get_sys_info_resp_msg_v01 get_sys_info_resp_msg;
  nas_roam_status_enum_type_v01 roam_status = NAS_ROAM_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_roam_status_enum_type_v01 roam_status_hdr  = NAS_ROAM_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_roam_status_enum_type_v01 roam_status_cdma = NAS_ROAM_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;
  // check whether had backhaul service

  /* Call QMI NAS service to get system info. */
  nas_service_domain_enum_type_v01 srv_domain         = NAS_SERVICE_DOMAIN_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_domain_enum_type_v01 srv_domain_lte     = NAS_SERVICE_DOMAIN_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_domain_enum_type_v01 srv_domain_hdr     = NAS_SERVICE_DOMAIN_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_domain_enum_type_v01 srv_domain_cdma    = NAS_SERVICE_DOMAIN_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_domain_enum_type_v01 srv_domain_wcdma   = NAS_SERVICE_DOMAIN_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_domain_enum_type_v01 srv_domain_gsm     = NAS_SERVICE_DOMAIN_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_domain_enum_type_v01 srv_domain_tdscdma = NAS_SERVICE_DOMAIN_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_status_enum_type_v01 srv_status         = NAS_SERVICE_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_status_enum_type_v01 srv_status_lte     = NAS_SERVICE_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_status_enum_type_v01 srv_status_hdr     = NAS_SERVICE_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_status_enum_type_v01 srv_status_cdma    = NAS_SERVICE_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_status_enum_type_v01 srv_status_wcdma   = NAS_SERVICE_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_status_enum_type_v01 srv_status_gsm     = NAS_SERVICE_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;
  nas_service_status_enum_type_v01 srv_status_tdscdma = NAS_SERVICE_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01;

  memset(&get_sys_info_resp_msg, 0 , sizeof(nas_get_sys_info_resp_msg_v01));
  qmi_error = qmi_client_send_msg_sync( qcmap_cm_cb.qmi_nas_handle,
                                        QMI_NAS_GET_SYS_INFO_REQ_MSG_V01,
                                        NULL,
                                        0,
                                        &get_sys_info_resp_msg,
                                        sizeof(get_sys_info_resp_msg),
                                        QCMAP_CM_QMI_TIMEOUT_VALUE);
  if (qmi_error != QMI_NO_ERR)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    *qmi_err_num = QMI_ERR_INTERNAL_V01;
    LOG_MSG_ERROR( "QCMAP CM nas get sys info req msg fail %d",
                    qmi_error,0,0);
    return ret;
  }
  else
  {
    /* Local domain and status variables */
    srv_domain_lte     = get_sys_info_resp_msg.lte_sys_info.common_sys_info.srv_domain;
    srv_domain_hdr     = get_sys_info_resp_msg.hdr_sys_info.common_sys_info.srv_domain;
    srv_domain_cdma    = get_sys_info_resp_msg.cdma_sys_info.common_sys_info.srv_domain;
    srv_domain_wcdma   = get_sys_info_resp_msg.wcdma_sys_info.common_sys_info.srv_domain;
    srv_domain_gsm     = get_sys_info_resp_msg.gsm_sys_info.common_sys_info.srv_domain;
    srv_domain_tdscdma = get_sys_info_resp_msg.tdscdma_sys_info.common_sys_info.srv_domain;
    srv_status_lte     = get_sys_info_resp_msg.lte_srv_status_info.srv_status;
    srv_status_hdr     = get_sys_info_resp_msg.hdr_srv_status_info.srv_status;
    srv_status_cdma    = get_sys_info_resp_msg.cdma_srv_status_info.srv_status;
    srv_status_wcdma   = get_sys_info_resp_msg.wcdma_srv_status_info.srv_status;
    srv_status_gsm     = get_sys_info_resp_msg.gsm_srv_status_info.srv_status;
    srv_status_tdscdma = get_sys_info_resp_msg.tdscdma_srv_status_info.srv_status;
    /* First Get the Service Domain. */
    /* If the LTE System Info is valid, check the LTE Service Domain. */
    if (get_sys_info_resp_msg.lte_sys_info_valid == TRUE &&
        get_sys_info_resp_msg.lte_sys_info.common_sys_info.srv_domain_valid == TRUE &&
        srv_status_lte == NAS_SYS_SRV_STATUS_SRV_V01 &&
        (srv_domain_lte == SYS_SRV_DOMAIN_PS_ONLY_V01 || srv_domain_lte == SYS_SRV_DOMAIN_CS_PS_V01))
    {
      srv_domain = get_sys_info_resp_msg.lte_sys_info.common_sys_info.srv_domain;
      srv_status = get_sys_info_resp_msg.lte_srv_status_info.srv_status;
      if (get_sys_info_resp_msg.lte_sys_info.common_sys_info.roam_status_valid == TRUE)
      {
        roam_status = get_sys_info_resp_msg.lte_sys_info.common_sys_info.roam_status;
      }
      LOG_MSG_INFO1("qcmap_cm_connect_backhaul: lte Service Domain %d Status %d, roam status %d", srv_domain, srv_status, roam_status);
    }
    /* If the HDR System Info is valid, check the HDR Service Domain. */
    else if (get_sys_info_resp_msg.hdr_sys_info_valid == TRUE &&
        get_sys_info_resp_msg.hdr_sys_info.common_sys_info.srv_domain_valid == TRUE &&
        srv_status_hdr == NAS_SYS_SRV_STATUS_SRV_V01 &&
        (srv_domain_hdr == SYS_SRV_DOMAIN_PS_ONLY_V01 || srv_domain_hdr == SYS_SRV_DOMAIN_CS_PS_V01))
    {
      srv_domain = get_sys_info_resp_msg.hdr_sys_info.common_sys_info.srv_domain;
      srv_status = get_sys_info_resp_msg.hdr_srv_status_info.srv_status;
      if (get_sys_info_resp_msg.hdr_sys_info.common_sys_info.roam_status_valid == TRUE)
      {
        roam_status = get_sys_info_resp_msg.hdr_sys_info.common_sys_info.roam_status;
        roam_status_hdr = get_sys_info_resp_msg.hdr_sys_info.common_sys_info.roam_status;
      }
      LOG_MSG_INFO1("qcmap_cm_connect_backhaul: HDR Service Domain %d Status %d, roam status %d", srv_domain, srv_status, roam_status);
    }
    /* If the CDMA System Info is valid, check the CDMA Service Domain. */
    else if (get_sys_info_resp_msg.cdma_sys_info_valid == TRUE &&
        get_sys_info_resp_msg.cdma_sys_info.common_sys_info.srv_domain_valid == TRUE &&
        srv_status_cdma == NAS_SYS_SRV_STATUS_SRV_V01 &&
        (srv_domain_cdma == SYS_SRV_DOMAIN_PS_ONLY_V01 || srv_domain_cdma == SYS_SRV_DOMAIN_CS_PS_V01))
    {
      srv_domain = get_sys_info_resp_msg.cdma_sys_info.common_sys_info.srv_domain;
      srv_status = get_sys_info_resp_msg.cdma_srv_status_info.srv_status;
      if (get_sys_info_resp_msg.cdma_sys_info.common_sys_info.roam_status_valid == TRUE)
      {
        roam_status = get_sys_info_resp_msg.cdma_sys_info.common_sys_info.roam_status;
        roam_status_cdma = get_sys_info_resp_msg.cdma_sys_info.common_sys_info.roam_status;
      }
      LOG_MSG_INFO1("qcmap_cm_connect_backhaul: CDMA Service Domain %d Status %d, roam status %d", srv_domain, srv_status, roam_status);
    }
    /* If the WCDMA System Info is valid, check the WCDMA Service Domain. */
    else if (get_sys_info_resp_msg.wcdma_sys_info_valid == TRUE &&
        get_sys_info_resp_msg.wcdma_sys_info.common_sys_info.srv_domain_valid == TRUE &&
        srv_status_wcdma == NAS_SYS_SRV_STATUS_SRV_V01 &&
        (srv_domain_wcdma == SYS_SRV_DOMAIN_PS_ONLY_V01 || srv_domain_wcdma == SYS_SRV_DOMAIN_CS_PS_V01))
    {
      srv_domain = get_sys_info_resp_msg.wcdma_sys_info.common_sys_info.srv_domain;
      srv_status = get_sys_info_resp_msg.wcdma_srv_status_info.srv_status;
      if (get_sys_info_resp_msg.wcdma_sys_info.common_sys_info.roam_status_valid == TRUE)
      {
        roam_status = get_sys_info_resp_msg.wcdma_sys_info.common_sys_info.roam_status;
      }
      LOG_MSG_INFO1("qcmap_cm_connect_backhaul: WCDMA Service Domain %d Status %d, roam status %d", srv_domain, srv_status, roam_status);
    }
    /* If the GSM System Info is valid, check the GSM Service Domain. */
    else if (get_sys_info_resp_msg.gsm_sys_info_valid == TRUE &&
        get_sys_info_resp_msg.gsm_sys_info.common_sys_info.srv_domain_valid == TRUE &&
        srv_status_gsm == NAS_SYS_SRV_STATUS_SRV_V01 &&
        (srv_domain_gsm == SYS_SRV_DOMAIN_PS_ONLY_V01 || srv_domain_gsm == SYS_SRV_DOMAIN_CS_PS_V01))
    {
      srv_domain = get_sys_info_resp_msg.gsm_sys_info.common_sys_info.srv_domain;
      srv_status = get_sys_info_resp_msg.gsm_srv_status_info.srv_status;
      if (get_sys_info_resp_msg.gsm_sys_info.common_sys_info.roam_status_valid == TRUE)
      {
        roam_status = get_sys_info_resp_msg.gsm_sys_info.common_sys_info.roam_status;
      }
      LOG_MSG_INFO1("qcmap_cm_connect_backhaul: GSM Service Domain %d Status %d, roam status %d", srv_domain, srv_status, roam_status);
    }
    /* If the TDSCDMA System Info is valid, check the TDSCDMA Service Domain. */
    else if (get_sys_info_resp_msg.tdscdma_sys_info_valid == TRUE &&
        get_sys_info_resp_msg.tdscdma_sys_info.common_sys_info.srv_domain_valid == TRUE &&
        srv_status_tdscdma == NAS_SYS_SRV_STATUS_SRV_V01 &&
        (srv_domain_tdscdma == SYS_SRV_DOMAIN_PS_ONLY_V01 || srv_domain_tdscdma == SYS_SRV_DOMAIN_CS_PS_V01))
    {
      srv_domain = get_sys_info_resp_msg.tdscdma_sys_info.common_sys_info.srv_domain;
      srv_status = get_sys_info_resp_msg.tdscdma_srv_status_info.srv_status;
      if (get_sys_info_resp_msg.tdscdma_sys_info.common_sys_info.roam_status_valid == TRUE)
      {
        roam_status = get_sys_info_resp_msg.tdscdma_sys_info.common_sys_info.roam_status;
      }
      LOG_MSG_INFO1("qcmap_cm_connect_backhaul: TDSCDMA Service Domain %d Status %d, roam status %d", srv_domain, srv_status, roam_status);
    }

    if ((srv_status != NAS_SYS_SRV_STATUS_SRV_V01) ||
        (srv_domain != SYS_SRV_DOMAIN_PS_ONLY_V01 && srv_domain != SYS_SRV_DOMAIN_CS_PS_V01))
    {
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      *qmi_err_num = QMI_ERR_NO_NETWORK_FOUND_V01;
      LOG_MSG_ERROR("QCMAP CM has no backhaul service",0,0,0);
      return ret;
    }

    /* QCMAP is on an active network. */
    qcmap_cm_cb.backhaul_service = TRUE;

    if(roam_status != NAS_ROAM_STATUS_ENUM_TYPE_MIN_ENUM_VAL_V01)
    {
      qcmap_cm_cb.roaming_indicator = roam_status;
    }
    else
    {
      qcmap_cm_cb.roaming_indicator = 0;
    }
  }

  if (qcmap_cm_cb.roaming_indicator >= NAS_SYS_ROAM_STATUS_ON_V01)
  {
      /* ...read the ERI config file for valid roam_status values. */
      if (qcmap_cm_eri_read_config(qcmap_cm_handle, qcmap_cm_cb.cfg->wan_config.eri_config_file, qcmap_cm_errno) != QCMAP_CM_SUCCESS)
      {
        LOG_MSG_INFO1("qcmap_cm_connect_backhaul: qcmap_cm_eri_read_config: qcmap_cm_errno %d", qcmap_cm_errno,0,0);
      }
  }

  /* Check to see whether we can connect backhaul during roam. */
  /* If we're connected to a network...*/
  if (qcmap_cm_cb.backhaul_service)
  {
    /* If the config indicates roaming data calls not allowed and we're
       connected to a roaming network...*/
    if (!qcmap_cm_cb.cfg->wan_config.roaming &&
        (qcmap_cm_cb.roaming_indicator >= NAS_SYS_ROAM_STATUS_ON_V01))
    {
      LOG_MSG_INFO1("qcmap_cm_connect_backhaul: qcmap_cm_cb.eri_roam_data_len %d", qcmap_cm_cb.eri_roam_data_len,0,0);
      ret = QCMAP_CM_ERROR;
      /* If ERI roam data, check if roaming status matches eri data. */
      if (qcmap_cm_cb.eri_roam_data_len > 0)
      {
        uint8 i = 0;

            /* Look to see if the current roaming status matched an eri data entry. */
            for (i = 0; i < qcmap_cm_cb.eri_roam_data_len; i++ )
            {
              if ( ( srv_status_hdr == NAS_SYS_SRV_STATUS_SRV_V01 &&
                     (srv_domain_hdr == SYS_SRV_DOMAIN_PS_ONLY_V01 ||
                     srv_domain_hdr == SYS_SRV_DOMAIN_CS_PS_V01) &&
                     roam_status_hdr == qcmap_cm_cb.eri_roam_data[i] ) ||
                   ( srv_status_cdma == NAS_SYS_SRV_STATUS_SRV_V01 &&
                     (srv_domain_cdma == SYS_SRV_DOMAIN_PS_ONLY_V01 ||
                     srv_domain_hdr == SYS_SRV_DOMAIN_CS_PS_V01) &&
                     roam_status_cdma == qcmap_cm_cb.eri_roam_data[i]) )
              {
                /* We have a match, this is a romaing exception. */
                ret = QCMAP_CM_SUCCESS;
                break;
              }
            }
          }
      /* If a roaming exception was not found, error out. */
      if (ret == QCMAP_CM_ERROR)
      {
        *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
        LOG_MSG_ERROR("QCMAP CM can not connect backhaul while it is roaming",0,0,0);
        *qmi_err_num = QMI_ERR_NO_NETWORK_FOUND_V01;
        return ret;
      }
    }
  }
  return QCMAP_CM_SUCCESS;
}

int bring_up_ipv4_wwan( int *qcmap_cm_errno, qmi_error_type_v01 *qmi_err_num )
{
  int ret = QCMAP_CM_SUCCESS, rval;
  qcmap_dsi_buffer_t qcmap_dsi_buffer;
  int numBytes=0, len;
  struct sockaddr_un cmdq_qcmap;
  dsi_call_param_value_t param_info, param_info_profile;

  if ( qcmap_cm_cb.cfg->lan_config.enable_ipv4 == TRUE )
  {
    if (qcmap_cm_cb.state == QCMAP_CM_WAN_CONNECTED)
    {
      ret = QCMAP_CM_SUCCESS;
      *qcmap_cm_errno = QCMAP_CM_EALDCONN;
      LOG_MSG_INFO1("QCMAP CM Backhaul already connected",0,0,0);
      *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
      return ret;
    }
    else if ((qcmap_cm_cb.state == QCMAP_CM_WAN_CONNECTING) ||
         (qcmap_cm_cb.state == QCMAP_CM_WAN_DISCONNECTING))
    {
      /* We have some outstanding WAN request */
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      LOG_MSG_INFO1("QCMAP CM has outstanding backhaul request",0,0,0);
      *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
      return ret;
    }

  /* If we already have an handle release it. */
  if ( qcmap_cm_cb.dsi_net_hndl.handle != NULL )
  {
    dsi_rel_data_srvc_hndl(qcmap_cm_cb.dsi_net_hndl.handle);
    qcmap_cm_cb.dsi_net_hndl.handle = NULL;
    memset(&qcmap_cm_cb.dsi_net_hndl, 0x0, sizeof(qcmap_cm_cb.dsi_net_hndl));
  }

  /* Acquire the handle. */
  qcmap_cm_cb.dsi_net_hndl.handle = dsi_get_data_srvc_hndl(qcmap_cm_dsi_net_cb_fcn, (void*) &qcmap_cm_cb.dsi_net_hndl);

  if (qcmap_cm_cb.dsi_net_hndl.handle == NULL)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_INFO1("bring_up_ipv4_wwan: Can not get dsi net handle",0,0,0);
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    return ret;
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

  LOG_MSG_INFO1("bring_up_ipv4_wwan: dsi_net_handle %X", qcmap_cm_cb.dsi_net_hndl.handle,0,0);

    /* Bring up the data call. */
    LOG_MSG_INFO1("Bring up IPV4 wwan",0,0,0);
    /* Connecting WWAN */

    /* KPI log message */
    ds_system_call("echo QCMAP:bringup v4  > /dev/kmsg", \
                    strlen("echo QCMAP:bringup v4  > /dev/kmsg"));

    rval = dsi_start_data_call(qcmap_cm_cb.dsi_net_hndl.handle);

    if (rval != DSI_SUCCESS)
    {
      *qcmap_cm_errno = QCMAP_CM_ENOWWAN;
      ret = QCMAP_CM_ERROR;
      LOG_MSG_ERROR("qcmap_cm_enable: dsi_start_data_call rval %d",
                  rval,0,0);

      *qmi_err_num = QMI_ERR_CALL_FAILED_V01;
      // Fake a NONET
      cmdq_qcmap.sun_family = AF_UNIX;
      strcpy(cmdq_qcmap.sun_path, QCMAP_CMDQ_UDS_FILE);
      len = strlen(cmdq_qcmap.sun_path) + sizeof(cmdq_qcmap.sun_family);

      qcmap_dsi_buffer.dsi_nethandle = qcmap_cm_cb.dsi_net_hndl.handle;
      qcmap_dsi_buffer.evt = DSI_EVT_NET_NO_NET;

      if ((numBytes = sendto(cmdq_qcmap_sockfd, (void *)&qcmap_dsi_buffer, sizeof(qcmap_dsi_buffer_t), 0,
                 (struct sockaddr *)&cmdq_qcmap, len)) == -1)
      {
        LOG_MSG_ERROR("Send Failed from bring_up_ipv4_wwan context", 0, 0, 0);
        return QCMAP_CM_ERROR;
      }
      LOG_MSG_INFO1("Send succeeded in bring_up_ipv4_wwan context", 0, 0, 0);
    }
    else
    {
      *qcmap_cm_errno = QCMAP_CM_EWOULDBLOCK;
      qcmap_cm_cb.state = QCMAP_CM_WAN_CONNECTING;
    }
  }
  else
  {
    ret = QCMAP_CM_ERROR;
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    LOG_MSG_ERROR("Cannot bring up IPV4 wwan:-  dsi handle 0x%x, IPv4 enabled:- %d",
                  qcmap_cm_cb.dsi_net_hndl.handle,
                  qcmap_cm_cb.cfg->lan_config.enable_ipv4,0);
  }
  return ret;
}

int bring_up_ipv6_wwan( int *qcmap_cm_errno, qmi_error_type_v01 *qmi_err_num )
{

  int ret = QCMAP_CM_SUCCESS, rval;
  qcmap_dsi_buffer_t qcmap_dsi_buffer;
  int numBytes=0, len;
  struct sockaddr_un cmdq_qcmap;
  dsi_call_param_value_t param_info, param_info_profile;

  if ( qcmap_cm_cb.cfg->lan_config.enable_ipv6 == TRUE )
  {

    if(qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_WAN_CONNECTED)
    {
      ret = QCMAP_CM_SUCCESS;
      *qcmap_cm_errno = QCMAP_CM_EALDCONN;
      LOG_MSG_INFO1("QCMAP CM Backhaul already connected",0,0,0);
      *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
      return ret;
    }
    else if (qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_WAN_CONNECTING)
    {
      /* We have some outstanding WAN request */
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      LOG_MSG_INFO1("QCMAP CM has outstanding backhaul request",0,0,0);
      *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
      return ret;
    }

    /* Release the previously acquired handle. */
    if (qcmap_cm_cb.ipv6_dsi_net_hndl.handle != NULL)
    {
      dsi_rel_data_srvc_hndl(qcmap_cm_cb.ipv6_dsi_net_hndl.handle);
      qcmap_cm_cb.ipv6_dsi_net_hndl.handle = NULL;
      memset(&qcmap_cm_cb.ipv6_dsi_net_hndl, 0x0, sizeof(qcmap_cm_cb.ipv6_dsi_net_hndl));
    }

    /* Acquire the handle. */
    qcmap_cm_cb.ipv6_dsi_net_hndl.handle = dsi_get_data_srvc_hndl(qcmap_cm_dsi_net_cb_fcn, (void*) &qcmap_cm_cb.ipv6_dsi_net_hndl);
    if (qcmap_cm_cb.ipv6_dsi_net_hndl.handle == NULL)
    {
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      ret = QCMAP_CM_ERROR;
      LOG_MSG_INFO1("bring_up_ipv6_wwan: Can not get ipv6 dsi net handle",0,0,0);
      *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
      return ret;
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

    LOG_MSG_INFO1("bring_up_ipv6_wwan: ipv6_dsi_net_handle %X", qcmap_cm_cb.ipv6_dsi_net_hndl.handle,0,0);

    LOG_MSG_INFO1("Bring up IPV6 wwan",0,0,0);

    /* KPI log message */
    ds_system_call("echo QCMAP:bringup v6  > /dev/kmsg", \
                    strlen("echo QCMAP:bringup v6  > /dev/kmsg"));

    /* Connecting IPv6 WAN */
    rval = dsi_start_data_call(qcmap_cm_cb.ipv6_dsi_net_hndl.handle);

    if (rval != DSI_SUCCESS )
    {
      *qcmap_cm_errno = QCMAP_CM_ENOWWAN;
      ret = QCMAP_CM_ERROR;
      LOG_MSG_ERROR("qcmap_cm_enable: dsi_start_data_call ipv6 rval %d",
        rval,0,0);
      *qmi_err_num = QMI_ERR_CALL_FAILED_V01;
      // Fake a NONET
      cmdq_qcmap.sun_family = AF_UNIX;
      strcpy(cmdq_qcmap.sun_path, QCMAP_CMDQ_UDS_FILE);
      len = strlen(cmdq_qcmap.sun_path) + sizeof(cmdq_qcmap.sun_family);

      qcmap_dsi_buffer.dsi_nethandle = qcmap_cm_cb.ipv6_dsi_net_hndl.handle;
      qcmap_dsi_buffer.evt = DSI_EVT_NET_NO_NET;
      if ((numBytes = sendto(cmdq_qcmap_sockfd, (void *)&qcmap_dsi_buffer, sizeof(qcmap_dsi_buffer_t), 0,
              (struct sockaddr *)&cmdq_qcmap, len)) == -1)
      {
        LOG_MSG_ERROR("Send Failed from bring_up_ipv6_wwan context", 0, 0, 0);
        return QCMAP_CM_ERROR;
      }
      LOG_MSG_INFO1("Send succeeded in bring_up_ipv6_wwan context", 0, 0, 0);
    }
    else
    {
      *qcmap_cm_errno = QCMAP_CM_EWOULDBLOCK;
      qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_WAN_CONNECTING;
    }
  }
  else
  {
    ret = QCMAP_CM_ERROR;
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    LOG_MSG_ERROR("Cannot bring up IPV6 wwan:-  dsi handle 0x%x, IPv6 enabled:- %d",
                 qcmap_cm_cb.ipv6_dsi_net_hndl.handle,
                 qcmap_cm_cb.cfg->lan_config.enable_ipv6,0);
  }
  return ret;
}

/*===========================================================================

FUNCTION BRING_DOWN_IPV4_WWAN()

DESCRIPTION

  It will teardown IPV4 WWAN.

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
int bring_down_ipv4_wwan
(
  int    qcmap_cm_handle,                      /* Handle for Mobile AP CM  */
  int    *qcmap_cm_errno,                      /* error condition value    */
  qmi_error_type_v01 *qmi_err_num
)
{
  int qmi_err;
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;

  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("\nbring_down_ipv4_wwan: enter",0,0,0);


  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    LOG_MSG_ERROR("Wrong QCMAP CM Handle",0,0,0);
    return ret;
  }

  if (qcmap_cm_cb.state <= QCMAP_CM_ENABLE)
  {
    /* WAN is not connected */
    *qcmap_cm_errno = QCMAP_CM_EALDDISCONN;
    ret = QCMAP_CM_SUCCESS;
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    LOG_MSG_ERROR("QCMAP CM IPV4 WAN is not connected",0,0,0);
    return ret;
  }
  /* Handle IPV4 disconnect backhaul request gracefully when
     mobile ap disable is processing */
  /* These changes are made to support consecutive usb plug unplug */

  if ((qcmap_cm_cb.state == QCMAP_CM_WAN_CONNECTING ||
       qcmap_cm_cb.state == QCMAP_CM_WAN_DISCONNECTING)&&
      (qcmap_cm_cb.disable_in_process == FALSE))
  {
    /* We can some outstanding WAN request */
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    LOG_MSG_ERROR("QCMAP CM has outstanding backhaul request",0,0,0);
    return ret;
  }
  /* Checks are added to make sure wan disconnected when mobile ap
     disable is triggered */
  if(qcmap_cm_cb.state == QCMAP_CM_WAN_CONNECTED ||
    ((qcmap_cm_cb.state == QCMAP_CM_WAN_CONNECTING)&&
     (qcmap_cm_cb.disable_in_process == TRUE)))
  {
    if (qcmap_cm_cb.dsi_net_hndl.handle != NULL)
    {
      LOG_MSG_INFO1("Bring down IPv4 WAN",0,0,0);
      qcmap_cm_cb.state = QCMAP_CM_WAN_DISCONNECTING;
      dsi_stop_data_call(qcmap_cm_cb.dsi_net_hndl.handle);
      LOG_MSG_INFO1("After dsi_stop_data_call",0,0,0);
      *qcmap_cm_errno = QCMAP_CM_EWOULDBLOCK;
      ret = QCMAP_CM_ERROR;
    }
    else
    {
      LOG_MSG_ERROR("Invalid Handle: Cannot bring down IPv4",0,0,0);
      *qcmap_cm_errno = QCMAP_CM_EINVAL;
      ret = QCMAP_CM_ERROR;
      *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    }
  }
  /* return QCMAP_CM_EWOULDBLOCK when disable in process to ensure
     mobile ap disable return success  */
  else if((qcmap_cm_cb.state == QCMAP_CM_WAN_DISCONNECTING)&&
          (qcmap_cm_cb.disable_in_process == TRUE))
  {
     LOG_MSG_INFO1("IPV4 call in disconnecting state: Disable process",0,0,0);
     *qcmap_cm_errno = QCMAP_CM_EWOULDBLOCK;
     ret = QCMAP_CM_ERROR;
  }
  else
  {
    *qcmap_cm_errno = QCMAP_CM_EINVAL;
    ret = QCMAP_CM_ERROR;
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    LOG_MSG_ERROR("Cannot bring down IPV4 wwan because of invalid state %d",
                  qcmap_cm_cb.state,0,0);
  }

  LOG_MSG_INFO1("QCMAP AP Handle %04x tear down backhaul.",
                qcmap_cm_handle,0,0);

  return ret;
}

/*===========================================================================

FUNCTION bring_down_ipv6_wwan()

DESCRIPTION

  It will teardown IPV6 WWAN.

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
int bring_down_ipv6_wwan
(
  int    qcmap_cm_handle,                      /* Handle for Mobile AP CM  */
  int    *qcmap_cm_errno,                      /* error condition value    */
  qmi_error_type_v01 *qmi_err_num
)
{
  int qmi_err;
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;

  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("\nbring_down_ipv6_wwan: enter",0,0,0);

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    LOG_MSG_ERROR("Wrong QCMAP CM Handle",0,0,0);
    return ret;
  }

  if (qcmap_cm_cb.ipv6_state <= QCMAP_CM_V6_ENABLE)
  {
    /* WAN is not connected */
    *qcmap_cm_errno = QCMAP_CM_EALDDISCONN;
    ret = QCMAP_CM_SUCCESS;
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    LOG_MSG_ERROR("QCMAP CM IPV6 WAN is not connected",0,0,0);
    return ret;
  }
  /* Handle IPV6 disconnect backhaul request gracefully when
     mobile ap disable is processing */
  /* These changes are made to support consecutive usb plug unplug */

  if ((qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_WAN_CONNECTING ||
       qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_WAN_DISCONNECTING)&&
      (qcmap_cm_cb.disable_in_process == FALSE))
  {
    /* We can some outstanding WAN request */
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    LOG_MSG_ERROR("QCMAP CM has outstanding backhaul request",0,0,0);
    return ret;
  }
  /* Checks are added to make sure wan disconnected when mobile ap
     disable is triggered */
  if((qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_WAN_CONNECTED) ||
    ((qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_WAN_CONNECTING)&&
     (qcmap_cm_cb.disable_in_process == TRUE)))
  {
    if (qcmap_cm_cb.ipv6_dsi_net_hndl.handle != NULL)
    {
      LOG_MSG_INFO1("Bring down IPv6 WAN",0,0,0);
      qcmap_cm_cb.ipv6_state = QCMAP_CM_V6_WAN_DISCONNECTING;
      dsi_stop_data_call(qcmap_cm_cb.ipv6_dsi_net_hndl.handle);
      LOG_MSG_INFO1("After dsi_stop_data_call",0,0,0);
      *qcmap_cm_errno = QCMAP_CM_EWOULDBLOCK;
      ret = QCMAP_CM_ERROR;
    }
    else
    {
      LOG_MSG_INFO1("Invalid Handle: Cannot bring down IPv6",0,0,0);
      *qcmap_cm_errno = QCMAP_CM_EINVAL;
      ret = QCMAP_CM_ERROR;
      *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    }
  }
  /* return QCMAP_CM_EWOULDBLOCK when disable in process to ensure
     mobile ap disable return success  */
  else if((qcmap_cm_cb.state == QCMAP_CM_V6_WAN_DISCONNECTING)&&
          (qcmap_cm_cb.disable_in_process == TRUE))
  {
     LOG_MSG_INFO1("IPV6 call in disconnecting state: Disable process",0,0,0);
     *qcmap_cm_errno = QCMAP_CM_EWOULDBLOCK;
     ret = QCMAP_CM_ERROR;
  }
  else
  {
    *qcmap_cm_errno = QCMAP_CM_EINVAL;
    ret = QCMAP_CM_ERROR;
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    LOG_MSG_ERROR("Cannot bring down IPV6 wwan because of invalid state %d",
                  qcmap_cm_cb.ipv6_state,0,0);
  }

  LOG_MSG_INFO1("QCMAP AP Handle %04x tear down backhaul.",
                qcmap_cm_handle,0,0);

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
  qcmap_msgr_wwan_call_type_v01    call_type, /* Call type to be brought UP. */
  int *qcmap_cm_errno,                            /* Error condition value  */
  qmi_error_type_v01 *qmi_err_num
)
{
  int ret = QCMAP_CM_SUCCESS, rval;
  qmi_client_error_type qmi_error;

  ds_assert(qcmap_cm_errno != NULL);
  ds_assert(qmi_err_num != NULL);
  LOG_MSG_INFO1("qcmap_cm_connect_backhaul: enter ",0,0,0);


  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    LOG_MSG_ERROR("Wrong QCMAP CM Handle",0,0,0);
    return ret;
  }

  /* Skip NAS check for modem loopback call. */
  if ( !modem_loopback_mode )
  {
    if (get_nas_config(qcmap_cm_handle, qcmap_cm_errno, qmi_err_num) != QCMAP_CM_SUCCESS)
    {
      ret = QCMAP_CM_ERROR;

      LOG_MSG_ERROR("get_nas_config() returned error!!",0,0,0);
      return ret;
    }
  }

  *qcmap_cm_errno = QCMAP_CM_EWOULDBLOCK;

  switch ( call_type )
  {
    case QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01:
    {
      if (bring_up_ipv4_wwan(qcmap_cm_errno, qmi_err_num) != QCMAP_CM_SUCCESS)
      {
        ret = QCMAP_CM_ERROR;
        LOG_MSG_ERROR("bring_up_ipv4_wwan() returned error!!",0,0,0);
      }
      break;
    }
    case QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01:
    {
      if (bring_up_ipv6_wwan(qcmap_cm_errno, qmi_err_num) != QCMAP_CM_SUCCESS)
      {
        ret = QCMAP_CM_ERROR;
        LOG_MSG_ERROR("bring_up_ipv6_wwan() returned error!!",0,0,0);
      }
      break;
    }
    default:
      *qcmap_cm_errno = QCMAP_CM_EINVAL;
      ret = QCMAP_CM_ERROR;
      *qmi_err_num = QMI_ERR_INVALID_IP_FAMILY_PREF_V01;
      LOG_MSG_ERROR("ConnectBackhaul() Invalid Call Type!!",0,0,0);
  }



  LOG_MSG_INFO1("QCMAP AP Handle 0x%04x Connecting Backhaul.",
                qcmap_cm_handle,0,0);

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
  qcmap_msgr_wwan_call_type_v01    call_type, /* Call type to be brought down. */
  int *qcmap_cm_errno,                           /* Error condition value  */
  qmi_error_type_v01 *qmi_err_num
)
{
  int qmi_err;
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;

  ds_assert(qcmap_cm_errno != NULL);
  ds_assert(qmi_err_num != NULL);

  LOG_MSG_INFO1("qcmap_cm_disconnect_backhaul: enter",0,0,0);


  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    LOG_MSG_ERROR("Wrong QCMAP CM Handle",0,0,0);
    return ret;
  }

  if (qcmap_cm_cb.state <= QCMAP_CM_ENABLE &&
      qcmap_cm_cb.ipv6_state <= QCMAP_CM_V6_ENABLE)
  {
    /* WAN is not connected */
    *qcmap_cm_errno = QCMAP_CM_EALDDISCONN;
    ret = QCMAP_CM_SUCCESS;
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    LOG_MSG_ERROR("QCMAP CM WAN is not connected",0,0,0);
    return ret;
  }

  switch ( call_type )
  {
    case QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01:
    {
      if (bring_down_ipv4_wwan(qcmap_cm_handle, qcmap_cm_errno, qmi_err_num)
                                   != QCMAP_CM_SUCCESS)
      {
        ret = QCMAP_CM_ERROR;
        LOG_MSG_ERROR("bring_down_ipv4_wwan() returned error!!",0,0,0);
      }
      break;
    }
    case QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01:
    {
      if (bring_down_ipv6_wwan(qcmap_cm_handle, qcmap_cm_errno, qmi_err_num)
                                   != QCMAP_CM_SUCCESS)
      {
        ret = QCMAP_CM_ERROR;
        LOG_MSG_ERROR("bring_down_ipv6_wwan() returned error!!",0,0,0);
      }
      break;
    }
    default:
      *qcmap_cm_errno = QCMAP_CM_EINVAL;
      ret = QCMAP_CM_ERROR;
      *qmi_err_num = QMI_ERR_INVALID_IP_FAMILY_PREF_V01;
      LOG_MSG_ERROR("DisconnectBackhaul() Invalid Call Type!!",0,0,0);
  }



  LOG_MSG_INFO1("QCMAP AP Handle %04x tear down backhaul.",
                qcmap_cm_handle,0,0);

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
  qmi_error_type_v01 *qmi_err_num
)
{
  return qcmap_cm_cb.state;
}

/*===========================================================================

FUNCTION QCMAP_CM_GET_IPV6_STATE()

DESCRIPTION

  It will get QCMAP CM current IPV6 state.

DEPENDENCIES
  None.

SIDE EFFECTS

===========================================================================*/

qcmap_cm_v6_state_e
qcmap_cm_get_ipv6_state
(
  qmi_error_type_v01 *qmi_err_num
)
{
  return qcmap_cm_cb.ipv6_state;
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
 boolean  auto_connect,                      /* Autoconnect Enable or Not */
 qmi_error_type_v01 *qmi_err_num
 )
{
  int ret = QCMAP_CM_SUCCESS;
  nas_indication_register_req_msg_v01 qcmap_nas_indication_register_req_msg_v01;
  nas_indication_register_resp_msg_v01 qcmap_nas_indication_register_resp_msg_v01;
  qmi_client_error_type qmi_error;

  ds_assert(qcmap_cm_errno != NULL);
  ds_assert(qmi_err_num != NULL);

  LOG_MSG_INFO1("qcmap_cm_set_auto_connect: enter", 0, 0, 0);


  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state == QCMAP_CM_DISABLE && qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_DISABLE)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    LOG_MSG_ERROR( "QCMAP CM disable state", 0, 0, 0);
    return ret;
  }

  qcmap_cm_cb.cfg->wan_config.auto_connect = auto_connect;


  /*if autoconnect is enabled register to serving system NAS indication and connect backhaul*/
  if(qcmap_cm_cb.cfg->wan_config.auto_connect)
  {
    memset(&qcmap_nas_indication_register_req_msg_v01, 0, sizeof(nas_indication_register_req_msg_v01));
    qcmap_nas_indication_register_req_msg_v01.sys_info_valid = TRUE;
    qcmap_nas_indication_register_req_msg_v01.sys_info = 0x01;
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

    if (qcmap_cm_cb.state == QCMAP_CM_ENABLE)
    {
      if( qcmap_cm_cb.cfg->wan_config.auto_connect_timer_running_v4 )
      {
        stop_auto_timer(QCMAP_MSGR_IP_FAMILY_V4_V01);
      }
      qcmap_cm_connect_backhaul(qcmap_cm_cb.handle, QCMAP_MSGR_WWAN_CALL_TYPE_V4_V01, qcmap_cm_errno, qmi_err_num);
    }
    if (qcmap_cm_cb.ipv6_state == QCMAP_CM_V6_ENABLE)
    {
      if( qcmap_cm_cb.cfg->wan_config.auto_connect_timer_running_v6 )
      {
        stop_auto_timer(QCMAP_MSGR_IP_FAMILY_V6_V01);
      }
      qcmap_cm_connect_backhaul(qcmap_cm_cb.handle, QCMAP_MSGR_WWAN_CALL_TYPE_V6_V01, qcmap_cm_errno, qmi_err_num);
    }
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
  qmi_error_type_v01 *qmi_err_num,               /* Error condition value  */
  boolean  roaming                               /* Roaming Enable or Not  */
)
{
  int ret = QCMAP_CM_SUCCESS;

  ds_assert(qmi_err_num != NULL);

  LOG_MSG_INFO1("qcmap_cm_set_roaming: enter", 0, 0, 0);

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state == QCMAP_CM_DISABLE)
  {
    *qmi_err_num = QMI_ERR_NO_EFFECT_V01;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "QCMAP CM disable state", 0, 0, 0);
    return ret;
  }

  qcmap_cm_cb.cfg->wan_config.roaming = roaming;
  return ret;
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
  in_addr_t *public_ip,                             /* Public IP for WWAN     */
  uint32 *pri_dns_addr,                         /* Primary DNS IP address  */
  in_addr_t *sec_dns_addr,                         /* Seconday DNS IP address */
  in_addr_t *default_gw_addr,                         /* Seconday DNS IP address */
  qmi_error_type_v01 *qmi_err_num
)
{
  int qmi_err;
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  dsi_addr_info_t addr_info;
  int rval, num_entries;
  char ip_str[20];


  ds_assert(public_ip != NULL);
  ds_assert(pri_dns_addr != NULL);
  ds_assert(sec_dns_addr != NULL);
  ds_assert(qmi_err_num != NULL);


  memset(ip_str, 0, 20);

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state != QCMAP_CM_WAN_CONNECTED)
  {
    *qmi_err_num = QMI_ERR_NO_NETWORK_FOUND_V01;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "QCMAP CM not in wan connected state", 0, 0, 0);
    return ret;
  }


  num_entries=1;
  memset(&addr_info, 0, sizeof(dsi_addr_info_t));
  rval = dsi_get_ip_addr(qcmap_cm_cb.dsi_net_hndl.handle, &addr_info, num_entries);

  LOG_MSG_INFO1("qcmap_cm_get_ipv4_net_conf: dsi_get_ip_addr, rval:%d", rval, 0, 0);

  if(rval != DSI_SUCCESS)
  {
    *qmi_err_num = QMI_ERR_INTERNAL_V01;
    LOG_MSG_ERROR("Couldn't get ipv4 ip address. rval %d",
                    rval,0,0);
    ret = QCMAP_CM_ERROR;
  }
  else
    LOG_MSG_INFO1("Got IP Address, rval:%d", rval, 0, 0);


  if (addr_info.iface_addr_s.valid_addr)
  {
    if (SASTORAGE_FAMILY(addr_info.iface_addr_s.addr) == AF_INET)
    {
      memset(ip_str, 0, 20);
      sprintf(ip_str, "%d.%d.%d.%d", SASTORAGE_DATA(addr_info.iface_addr_s.addr)[0], SASTORAGE_DATA(addr_info.iface_addr_s.addr)[1], SASTORAGE_DATA(addr_info.iface_addr_s.addr)[2], SASTORAGE_DATA(addr_info.iface_addr_s.addr)[3]);
      *public_ip = inet_addr(ip_str);
      LOG_MSG_INFO1("public_ip:0x%x", *public_ip, 0, 0);
    }
  }
  if (addr_info.dnsp_addr_s.valid_addr)
  {
      memset(ip_str, 0, 20);
      sprintf(ip_str, "%d.%d.%d.%d", SASTORAGE_DATA(addr_info.dnsp_addr_s.addr)[0], SASTORAGE_DATA(addr_info.dnsp_addr_s.addr)[1], SASTORAGE_DATA(addr_info.dnsp_addr_s.addr)[2], SASTORAGE_DATA(addr_info.dnsp_addr_s.addr)[3]);
    *pri_dns_addr = inet_addr(ip_str);
    LOG_MSG_INFO1("pri_dns_addr: 0x%x", *pri_dns_addr, 0, 0);
  }
  if (addr_info.dnss_addr_s.valid_addr)
  {
      memset(ip_str, 0, 20);
      sprintf(ip_str, "%d.%d.%d.%d", SASTORAGE_DATA(addr_info.dnss_addr_s.addr)[0], SASTORAGE_DATA(addr_info.dnss_addr_s.addr)[1], SASTORAGE_DATA(addr_info.dnss_addr_s.addr)[2], SASTORAGE_DATA(addr_info.dnss_addr_s.addr)[3]);
    *sec_dns_addr = inet_addr(ip_str);
    LOG_MSG_INFO1("sec_dns_addr: 0x%x", *sec_dns_addr, 0, 0);
  }
  if (addr_info.gtwy_addr_s.valid_addr)
  {
      memset(ip_str, 0, 20);
      sprintf(ip_str, "%d.%d.%d.%d", SASTORAGE_DATA(addr_info.gtwy_addr_s.addr)[0], SASTORAGE_DATA(addr_info.gtwy_addr_s.addr)[1], SASTORAGE_DATA(addr_info.gtwy_addr_s.addr)[2], SASTORAGE_DATA(addr_info.gtwy_addr_s.addr)[3]);
    *default_gw_addr = inet_addr(ip_str);
    LOG_MSG_INFO1("default_gw_addr: 0x%x", *default_gw_addr, 0, 0);
  }

  return ret;
}

/*===========================================================================

FUNCTION QCMAP_CM_GET_IPV6_NET_CONF()

DESCRIPTION
  This function gets the IPv6 WWAN network configuration.

DEPENDENCIES
  None.

RETURN VALUE
  On success, returns QCMAP_CM_SUCCESS.
  On error, return QCMAP_CM_ERROR and places the error condition value in
  *qcmap_cm_errno.

SIDE EFFECTS

===========================================================================*/
int qcmap_cm_get_ipv6_net_conf
(
  int                qcmap_cm_handle,   /* Handle for MobileAP CM */
  uint8_t            public_ip[],       /* Public IP for WWAN     */
  uint8_t            pri_dns_addr[],    /* Primary DNS IP address  */
  uint8_t            sec_dns_addr[],    /* Seconday DNS IP address */
  qmi_error_type_v01 *qmi_err_num
)
{
  int qmi_err;
  int ret = QCMAP_CM_SUCCESS;
  qmi_client_error_type qmi_error;
  dsi_addr_info_t addr_info;
  int rval, num_entries;
  char ip_str[INET6_ADDRSTRLEN];


  ds_assert(public_ip != NULL);
  ds_assert(pri_dns_addr != NULL);
  ds_assert(sec_dns_addr != NULL);
  ds_assert(qmi_err_num != NULL);

  memset(ip_str, 0, INET6_ADDRSTRLEN);

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

 if (qcmap_cm_cb.ipv6_state != QCMAP_CM_V6_WAN_CONNECTED)
  {
    *qmi_err_num = QMI_ERR_NO_NETWORK_FOUND_V01;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "QCMAP CM not in IPv6 WAN connected state", 0, 0, 0);
    return ret;
  }

  num_entries=1;
  memset(&addr_info, 0, sizeof(dsi_addr_info_t));
  rval = dsi_get_ip_addr(qcmap_cm_cb.ipv6_dsi_net_hndl.handle,
                           &addr_info, num_entries);

  LOG_MSG_INFO1("qcmap_cm_get_ipv6_net_conf: dsi_get_ip_addr, rval:%d",
                rval, 0, 0);

  if(rval != DSI_SUCCESS)
  {
    *qmi_err_num = QMI_ERR_INTERNAL_V01;
    QCMAP_CM_LOG("Couldn't get ipv6 ip address. rval %d",
                  rval);
    ret = QCMAP_CM_ERROR;
  }
  else
    LOG_MSG_INFO1("Got IPv6 Address, rval:%d", rval, 0, 0);


  if (addr_info.iface_addr_s.valid_addr)
  {
    if (SASTORAGE_FAMILY(addr_info.iface_addr_s.addr) == AF_INET6)
    {
      memcpy(public_ip, SASTORAGE_DATA(addr_info.iface_addr_s.addr),
             QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8));
    }
  }
  if (addr_info.dnsp_addr_s.valid_addr)
  {
    if (SASTORAGE_FAMILY(addr_info.dnsp_addr_s.addr) == AF_INET6)
    {
      memcpy(pri_dns_addr, SASTORAGE_DATA(addr_info.dnsp_addr_s.addr),
             QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8));
    }
  }
  if (addr_info.dnss_addr_s.valid_addr)
  {
    if (SASTORAGE_FAMILY(addr_info.dnss_addr_s.addr) == AF_INET6)
    {
      memcpy(sec_dns_addr, SASTORAGE_DATA(addr_info.dnss_addr_s.addr),
             QCMAP_MSGR_IPV6_ADDR_LEN_V01*sizeof(uint8));
    }
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
  qcmap_msgr_ip_family_enum_v01
          qcmap_dev_type,                      /* Dev type                 */
  char   *qcmap_dev_name,                      /* Device name              */
  int    *qcmap_cm_errno                       /* Error condition value    */
)
{
  char device[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 2];
  int rval;
  ds_assert(qcmap_dev_name != NULL);
  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_get_dev_name: enter",0,0,0);

  memset(device,0,sizeof(device));
  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle, qcmap_cm_handle:%d, qcmap_cm_cb.handle:%d",qcmap_cm_handle, qcmap_cm_cb.handle,0);
    return QCMAP_CM_ERROR;
  }
  if (qcmap_dev_type == QCMAP_MSGR_IP_FAMILY_V4_V01)
  {
    if (qcmap_cm_cb.state < QCMAP_CM_ENABLE)
    {
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;

      LOG_MSG_ERROR( "QCMAP CM V4 WAN not connected. %d", qcmap_cm_cb.state,0,0);
      return QCMAP_CM_ERROR;
    }
    else if (qcmap_cm_cb.dsi_net_hndl.handle != NULL)
    {
      rval = dsi_get_device_name(qcmap_cm_cb.dsi_net_hndl.handle, device, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1);
      if(rval != DSI_SUCCESS)
      {
        LOG_MSG_ERROR ("Couldn't get ipv4 rmnet name. rval %d",rval, 0, 0);
        *qcmap_cm_errno = rval;
      }
      LOG_MSG_INFO1("device_name:%s", device,0,0);
      memcpy(qcmap_dev_name, device, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);
    }else
    {
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      LOG_MSG_ERROR("Error during get ipv4 dev_name state =%d, handle=%d ",
                     qcmap_cm_cb.state,qcmap_cm_cb.dsi_net_hndl.handle,0);
      return QCMAP_CM_ERROR;
    }
  }
  else if (qcmap_dev_type == QCMAP_MSGR_IP_FAMILY_V6_V01)
  {
    if (qcmap_cm_cb.ipv6_state < QCMAP_CM_V6_ENABLE)
    {
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      LOG_MSG_ERROR( "QCMAP CM V6 WAN not connected. %d", qcmap_cm_cb.ipv6_state,0,0);
      return QCMAP_CM_ERROR;
    }
    else if ( qcmap_cm_cb.ipv6_dsi_net_hndl.handle != NULL)
    {
      rval = dsi_get_device_name(qcmap_cm_cb.ipv6_dsi_net_hndl.handle, device, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+1);
      if(rval != DSI_SUCCESS)
      {
        LOG_MSG_ERROR ("Couldn't get ipv6 rmnet name. rval %d",rval, 0, 0);
        *qcmap_cm_errno = rval;
      }

      LOG_MSG_INFO1("device_name:%s", device, 0, 0);
      memcpy(qcmap_dev_name, device, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2);
    }else
    {
      *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
      LOG_MSG_ERROR("Error during get ipv6 dev_name state =%d, handle=%d ",
                     qcmap_cm_cb.ipv6_state,qcmap_cm_cb.ipv6_dsi_net_hndl.handle,0);
      return QCMAP_CM_ERROR;
    }
  }
  LOG_MSG_INFO1( "qcmap_cm_get_dev_name:: %s", qcmap_dev_name,0,0);
  return QCMAP_CM_SUCCESS;
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
  int file_handle, ret;
  uint8 bytes_read = 0;
  struct stat stat_buf;

  ds_assert(file_name != NULL);
  ds_assert(qcmap_cm_errno != NULL);

  LOG_MSG_INFO1("qcmap_cm_eri_read_config: enter", 0, 0, 0);


  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qcmap_cm_errno = QCMAP_CM_EBADAPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if (qcmap_cm_cb.state == QCMAP_CM_DISABLE)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "QCMAP CM disable state", 0, 0, 0);
    return ret;
  }


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
    return QCMAP_CM_ERROR;
  }

  if (stat_buf.st_size > QCMAP_WAN_MAX_ERI_DATA_SIZE)
  {
    *qcmap_cm_errno = QCMAP_CM_EOPNOTSUPP;
    LOG_MSG_ERROR( "Config file too big. size %d", stat_buf.st_size, 0, 0);
    return QCMAP_CM_ERROR;
  }

  do
  {
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

extern int qcmap_cm_get_wwan_statistics
(
  int                             qcmap_cm_handle,/*Handle for Mobile AP CM*/
  qcmap_msgr_ip_family_enum_v01   ip_family,     /* V4 or V6 family        */
  qcmap_msgr_wwan_statistics_type_v01 *wwan_stats,    /* WWAN Statistics values */
  qmi_error_type_v01             *qmi_err_num
)
{
  int                 rval,i=0;
  int                 ret = QCMAP_CM_SUCCESS;
  int qcmap_cm_error = 0;
  unsigned char data[DATA_SIZE];
  dsi_data_pkt_stats stats_info;
  dsi_hndl_t dsi_handle;

  ds_assert(wwan_stats != NULL);

  LOG_MSG_INFO1("qcmap_cm_get_wwan_statistics: enter", 0, 0, 0);


  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if (ip_family == QCMAP_MSGR_IP_FAMILY_V4_V01)
  {
    dsi_handle = qcmap_cm_cb.dsi_net_hndl.handle;
  }
  else if (ip_family == QCMAP_MSGR_IP_FAMILY_V6_V01)
  {
    dsi_handle = qcmap_cm_cb.ipv6_dsi_net_hndl.handle;
  }
  else
  {
    *qmi_err_num = QMI_ERR_INVALID_IP_FAMILY_PREF_V01;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "QCMAP CM Invalid family %d", ip_family, 0, 0);
    return ret;
  }

  memset(&stats_info, 0, sizeof(stats_info));

  rval = dsi_get_pkt_stats(dsi_handle, &stats_info);

  if (rval != DSI_SUCCESS)
  {
    *qmi_err_num = QMI_ERR_INTERNAL_V01;
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
  qcmap_msgr_ip_family_enum_v01   ip_family,     /* V4 or V6 family        */
  qmi_error_type_v01             *qmi_err_num
)
{
  int                 rval;
  int                 ret = QCMAP_CM_SUCCESS;
  int                 qcmap_cm_error;
  char devname[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN+2];
  char command[MAX_COMMAND_STR_LEN];
  dsi_hndl_t dsi_handle;

  LOG_MSG_INFO1("qcmap_cm_reset_wwan_statistics: enter", 0, 0, 0);


  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if (ip_family == QCMAP_MSGR_IP_FAMILY_V4_V01)
  {
    dsi_handle = qcmap_cm_cb.dsi_net_hndl.handle;
  }
  else if (ip_family == QCMAP_MSGR_IP_FAMILY_V6_V01)
  {
    dsi_handle = qcmap_cm_cb.ipv6_dsi_net_hndl.handle;
  }
  else
  {
    *qmi_err_num = QMI_ERR_INVALID_IP_FAMILY_PREF_V01;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "QCMAP CM Invalid family %d", ip_family, 0, 0);
    return ret;
  }

  rval = dsi_reset_pkt_stats(dsi_handle);

  if (rval != DSI_SUCCESS)
  {
    *qmi_err_num = QMI_ERR_INTERNAL_V01;
    LOG_MSG_ERROR("dsi_reset_pkt_stats Failed error",
                   0,0,0);
    ret = QCMAP_CM_ERROR;
    return ret;
  }


  return ret;
} /* qcmap_cm_reset_wwan_statistics() */
/*===========================================================================

FUNCTION QCMAP_CM_SET_WWAN_CONFIG()

DESCRIPTION

  It will set WWAN  current 3gpp and 3gpp2 profile id of v4/46 state.

DEPENDENCIES
  None.

SIDE EFFECTS

===========================================================================*/
int qcmap_cm_set_wwan_policy
(
  int                             qcmap_cm_handle,/*Handle for Mobile AP CM*/
  qcmap_msgr_net_policy_info_v01 wwan_policy,   /* WWAN config values */
  qmi_error_type_v01             *qmi_err_num
)
{
  int ret = QCMAP_CM_SUCCESS;

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  LOG_MSG_INFO1("qcmap_cm_set_wwan_config: enter", 0, 0, 0);

  if( wwan_policy.tech_pref != QCMAP_WAN_TECH_ANY && wwan_policy.tech_pref != QCMAP_WAN_TECH_3GPP &&
      wwan_policy.tech_pref != QCMAP_WAN_TECH_3GPP2)
  {
    ret = QCMAP_CM_ERROR;
    *qmi_err_num = QMI_ERR_INVALID_TECH_PREF_V01;
    LOG_MSG_ERROR("Set wwan policy: invalid tech preference %d \n",
                  wwan_policy.tech_pref, 0, 0);
    return ret;
  }

  switch (wwan_policy.ip_family)
  {
    case QCMAP_MSGR_IP_FAMILY_V4_V01:
      LOG_MSG_INFO1("qcmap_cm_set_wwan_config v4: enter", 0, 0, 0);
      qcmap_cm_cb.cfg->wan_config.profile_id.v4.umts_profile_index = wwan_policy.v4_profile_id_3gpp;
      qcmap_cm_cb.cfg->wan_config.profile_id.v4.cdma_profile_index = wwan_policy.v4_profile_id_3gpp2;
      break;

    case QCMAP_MSGR_IP_FAMILY_V6_V01:
      LOG_MSG_INFO1("qcmap_cm_set_wwan_config v6: enter", 0, 0, 0);
      qcmap_cm_cb.cfg->wan_config.profile_id.v6.umts_profile_index = wwan_policy.v6_profile_id_3gpp;
      qcmap_cm_cb.cfg->wan_config.profile_id.v6.cdma_profile_index = wwan_policy.v6_profile_id_3gpp2;
      break;

    case QCMAP_MSGR_IP_FAMILY_V4V6_V01:
      LOG_MSG_INFO1("qcmap_cm_set_wwan_config v4v6: enter", 0, 0, 0);
      qcmap_cm_cb.cfg->wan_config.profile_id.v4.umts_profile_index = wwan_policy.v4_profile_id_3gpp;
      qcmap_cm_cb.cfg->wan_config.profile_id.v4.cdma_profile_index = wwan_policy.v4_profile_id_3gpp2;
      qcmap_cm_cb.cfg->wan_config.profile_id.v6.umts_profile_index = wwan_policy.v6_profile_id_3gpp;
      qcmap_cm_cb.cfg->wan_config.profile_id.v6.cdma_profile_index = wwan_policy.v6_profile_id_3gpp2;
      break;

     default:
      LOG_MSG_ERROR( "QCMAP CM Invalid family %d", wwan_policy.ip_family, 0, 0);
      *qmi_err_num = QMI_ERR_INVALID_IP_FAMILY_PREF_V01;
      ret = QCMAP_CM_ERROR;
      return ret;
  }

  qcmap_cm_cb.cfg->wan_config.tech = wwan_policy.tech_pref;

  return ret;
}
/*===========================================================================

FUNCTION QCMAP_CM_GET_DATA_BITRATE()

DESCRIPTION

  Returns the current data bitrates.

DEPENDENCIES
  None.

SIDE EFFECTS

===========================================================================*/
extern int qcmap_cm_get_data_bitrate
(
  int                             qcmap_cm_handle,/*Handle for Mobile AP CM*/
  qcmap_msgr_data_bitrate_v01    *data_rate,    /* Data rate values */
  qmi_error_type_v01             *qmi_err_num
)
{
  int                 ret = QCMAP_CM_SUCCESS;
  dsi_data_channel_rate_t stats_info;
  dsi_hndl_t dsi_handle;

  ds_assert(data_rate != NULL);

  LOG_MSG_INFO1("qcmap_cm_get_data_bitrate: enter", 0, 0, 0);

  memset(&stats_info, 0, sizeof(dsi_data_channel_rate_t));

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return ret;
  }

  if(qcmap_cm_cb.state == QCMAP_CM_WAN_CONNECTED)
  {
    dsi_handle = qcmap_cm_cb.dsi_net_hndl.handle;
  }
  else
  {
    *qmi_err_num = QMI_ERR_NO_NETWORK_FOUND_V01;
    ret = QCMAP_CM_ERROR;
    LOG_MSG_ERROR( "QCMAP CM WAN is down", 0, 0, 0);
    return ret;
  }

  if (dsi_get_current_data_channel_rate(dsi_handle, &stats_info) == DSI_ERROR)
  {
    *qmi_err_num = QMI_ERR_INTERNAL_V01;
    LOG_MSG_ERROR("dsi_get_current_data_channel Failed error",
                   0,0,0);
    ret = QCMAP_CM_ERROR;
    return ret;
  }

  data_rate->tx_rate = stats_info.current_tx_rate;
  data_rate->rx_rate = stats_info.current_rx_rate;
  data_rate->max_tx_rate = stats_info.max_tx_rate;
  data_rate->max_rx_rate = stats_info.max_rx_rate;

  return ret;
} /* qcmap_cm_get_data_bitrate() */

/*===========================================================================

FUNCTION QCMAP_CM_GET_IPV4_SIP_SERVER_INFO()

DESCRIPTION
 - Calls into dsi_netctrl IOCTL to get PCSCF address list and
   PCSCF FQDN list

DEPENDENCIES
  None.

SIDE EFFECTS

===========================================================================*/
int qcmap_cm_get_ipv4_sip_server_info
(
  int                             qcmap_cm_handle,/*Handle for Mobile AP CM*/
  dsi_pcscf_addr_info_t           * pcscf_addr_list,/*PCSCF address list*/
  dsi_pcscf_fqdn_list_t           * pcscf_fqdn_list,/*PCSCF FQDN list*/
  qmi_error_type_v01              *qmi_err_num /*QMI error number*/
)
{
  int dsi_err;
  int ret_val_fqdn;
  int ret_val_ip;

/*-------------------------------------------------------------------------*/

  ds_assert(pcscf_addr_list != NULL);
  ds_assert(pcscf_fqdn_list != NULL);
  ds_assert(qmi_err_num != NULL);

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return QCMAP_CM_ERROR;
  }

  if (qcmap_cm_cb.state != QCMAP_CM_WAN_CONNECTED)
  {
    *qmi_err_num = QMI_ERR_NO_NETWORK_FOUND_V01;
    LOG_MSG_ERROR( "QCMAP CM not in wan connected state", 0, 0, 0);
    return QCMAP_CM_ERROR;
  }

  ret_val_ip = dsi_iface_ioctl(qcmap_cm_cb.dsi_net_hndl.handle,
                  DSI_IFACE_IOCTL_GET_PCSCF_SERV_ADDRESS,
                  pcscf_addr_list,
                  &dsi_err);

  if(ret_val_ip == DSI_SUCCESS)
  {
    LOG_MSG_INFO1("Successfully got PCSCF server address", 0, 0, 0);
  }
  else
  {
    LOG_MSG_ERROR("Failure in getting PCSCF server address: %d", dsi_err, 0, 0);
  }

  ret_val_fqdn = dsi_iface_ioctl(qcmap_cm_cb.dsi_net_hndl.handle,
                  DSI_IFACE_IOCTL_GET_PCSCF_DOMAIN_NAME_LIST,
                  pcscf_fqdn_list,
                  &dsi_err);

  if(ret_val_fqdn == DSI_SUCCESS)
  {
    LOG_MSG_INFO1("Successfully got PCSCF domain name list", 0, 0, 0);
  }
  else
  {
    LOG_MSG_ERROR("Failure in getting PCSCF domain name list: %d", dsi_err, 0, 0);
  }

  if(ret_val_ip!= DSI_SUCCESS && ret_val_fqdn!= DSI_SUCCESS)
  {
    *qmi_err_num = QMI_ERR_INTERNAL_V01;
    return QCMAP_CM_ERROR;
  }
  else
  {
    *qmi_err_num = QMI_ERR_NONE_V01;
    return QCMAP_CM_SUCCESS;
  }
}

/*===========================================================================

FUNCTION QCMAP_CM_GET_IPV6_SIP_SERVER_INFO()

DESCRIPTION
 - Calls into dsi_netctrl IOCTL to get PCSCF address list and
   PCSCF FQDN list

DEPENDENCIES
  None.

SIDE EFFECTS

===========================================================================*/
int qcmap_cm_get_ipv6_sip_server_info
(
  int                             qcmap_cm_handle,/*Handle for Mobile AP CM*/
  dsi_pcscf_addr_info_t           *pcscf_addr_list,/*PCSCF address list*/
  dsi_pcscf_fqdn_list_t           *pcscf_fqdn_list,/*PCSCF FQDN list*/
  qmi_error_type_v01              *qmi_err_num /*QMI error number*/
)
{
  int dsi_err;
  int ret_val_fqdn;
  int ret_val_ip;

/*-------------------------------------------------------------------------*/

  ds_assert(pcscf_addr_list != NULL);
  ds_assert(pcscf_fqdn_list != NULL);
  ds_assert(qmi_err_num != NULL);

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return QCMAP_CM_ERROR;
  }

  if (qcmap_cm_cb.state != QCMAP_CM_V6_WAN_CONNECTED)
  {
    *qmi_err_num = QMI_ERR_NO_NETWORK_FOUND_V01;
    LOG_MSG_ERROR( "QCMAP CM not in v6 wan connected state", 0, 0, 0);
    return QCMAP_CM_ERROR;
  }

  ret_val_ip = dsi_iface_ioctl(qcmap_cm_cb.ipv6_dsi_net_hndl.handle,
                               DSI_IFACE_IOCTL_GET_PCSCF_SERV_ADDRESS,
                               pcscf_addr_list,
                               &dsi_err);

  if(ret_val_ip == DSI_SUCCESS)
  {
    LOG_MSG_INFO1("Successfully got IPV6 PCSCF server address", 0, 0, 0);
  }
  else
  {
    LOG_MSG_ERROR("Failure in getting IPV6 PCSCF server address: %d", dsi_err, 0, 0);
  }

  ret_val_fqdn = dsi_iface_ioctl(qcmap_cm_cb.ipv6_dsi_net_hndl.handle,
                                 DSI_IFACE_IOCTL_GET_PCSCF_DOMAIN_NAME_LIST,
                                 pcscf_fqdn_list,
                                 &dsi_err);

  if(ret_val_fqdn == DSI_SUCCESS)
  {
    LOG_MSG_INFO1("Successfully got IPV6 PCSCF domain name list", 0, 0, 0);
  }
  else
  {
    LOG_MSG_ERROR("Failure in getting IPV6 PCSCF domain name list: %d", dsi_err, 0, 0);
  }

  if(ret_val_ip != DSI_SUCCESS && ret_val_fqdn != DSI_SUCCESS)
  {
    *qmi_err_num = QMI_ERR_INTERNAL_V01;
    return QCMAP_CM_ERROR;
  }
  else
  {
    *qmi_err_num = QMI_ERR_NONE_V01;
    return QCMAP_CM_SUCCESS;
  }
}


/*===========================================================================

FUNCTION QCMAP_CM_DELETE_IPV6_DELEGATED_PREFIX()

DESCRIPTION
 - Removes a single prefix if prefix_valid is set, othwise removes all
 delegated prefix's

DEPENDENCIES
  None.

SIDE EFFECTS

===========================================================================*/
int qcmap_cm_delete_ipv6_delegated_prefix
(
  int                             qcmap_cm_handle,/*Handle for Mobile AP CM*/
  boolean                         prefix_valid,/*Boolean to flush single or all*/
  uint8_t                         *ipv6_addr,/*Prefix to delete*/
  qmi_error_type_v01              *qmi_err_num /*QMI error number*/
)
{
  int dsi_err;
  int ret_val_ip;
  dsi_delegated_ipv6_prefix_type prefix;
  struct in6_addr addr6;
  char straddr[INET6_ADDRSTRLEN];
/*-------------------------------------------------------------------------*/

  ds_assert(qmi_err_num != NULL);

  if (qcmap_cm_handle != qcmap_cm_cb.handle)
  {
    *qmi_err_num = QMI_ERR_INVALID_HANDLE_V01;
    LOG_MSG_ERROR( "Wrong QCMAP CM Handle", 0, 0, 0);
    return QCMAP_CM_ERROR;
  }

  if (qcmap_cm_cb.ipv6_state != QCMAP_CM_V6_WAN_CONNECTED)
  {
    *qmi_err_num = QMI_ERR_NO_NETWORK_FOUND_V01;
    LOG_MSG_ERROR( "QCMAP CM not in v6 wan connected state", 0, 0, 0);
    return QCMAP_CM_ERROR;
  }

  memset(&prefix, 0, sizeof(dsi_delegated_ipv6_prefix_type));
  if(prefix_valid)
  {
    memcpy(prefix.ipv6_addr, ipv6_addr, sizeof(prefix.ipv6_addr));
    prefix.prefix_len = 64;
    memcpy(addr6.s6_addr, prefix.ipv6_addr, sizeof(addr6.s6_addr));
    ds_log_med("Remove IPv6 address: %s", inet_ntop(AF_INET6, &addr6, straddr, INET6_ADDRSTRLEN));
  }

  ret_val_ip = dsi_iface_ioctl(qcmap_cm_cb.ipv6_dsi_net_hndl.handle,
                               DSI_IFACE_IOCTL_REMOVE_DELEGATED_IPV6_PREFIX,
                               &prefix,
                               &dsi_err);

  if(ret_val_ip != DSI_SUCCESS)
  {
    LOG_MSG_ERROR("Failure in flushing IPv6 Prefix: %d", dsi_err, 0, 0);
    *qmi_err_num = QMI_ERR_INTERNAL_V01;
    return QCMAP_CM_ERROR;
  }

  LOG_MSG_INFO1("Successfully flushed IPv6 Prefix", 0, 0, 0);
  *qmi_err_num = QMI_ERR_NONE_V01;
  return QCMAP_CM_SUCCESS;
}
