/******************************************************************************

                        N E T M G R _ I W L A N _C L I E N T . CPP

******************************************************************************/

/******************************************************************************

  @file    netmgr_iwlan_client.cpp
  @brief   Network manager iWLAN client

  DESCRIPTION
  Implementation of iwlan client module

******************************************************************************/
/*===========================================================================

  Copyright (c) 2014-2015 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#ifdef FEATURE_DATA_IWLAN
extern "C" {
  #include <stdio.h>
  #include "qmi_client.h"
  #include "qmi_cci_target.h"
  #include "qmi_cci_common.h"
  #include "qmi_client_instance_defs.h"
  #include "qmi_platform.h"
  #include "ds_util.h"
  #include "data_system_determination_v01.h"
  #include "netmgr_cmdq.h"
  #include <arpa/inet.h>
}

#include "netmgr_iwlan_client.h"
#include "CneFactory.h"
#include "ICneNetworkObserver.h"
#include "ICneNetworkObserverNotifier.h"
#include "ICneFeatureObserver.h"
#include "ICneFeatureObserverNotifier.h"

#include <pthread.h>

#define NETMGR_SUCCESS (0)
#define NETMGR_FAILURE (-1)

#define QMI_DSD_SYNC_MSG_TIMEOUT        (10000)
#define NETMGR_CLIENT_DEFAULT_DEV_ID    "rmnet0"
#define NETMGR_CLIENT_MDMUSB_DEV_ID     "rmnet_usb0"
#define NETMGR_CLIENT_MSM_DEV_ID        "rmnet_data0"
#define NETMGR_CLIENT_MDMMHI_DEV_ID     NETMGR_CLIENT_MSM_DEV_ID

#ifdef __cplusplus
class NetmgrIwlanClient : public ICneNetworkObserverNotifier,
    public ICneFeatureObserverNotifier
{
public:
  static NetmgrIwlanClient *getInstance();
  bool                     regSignals(void);
  bool                     stopSignals(void);
  virtual void       onNetConfigChange (const CnoNetConfigType &netConfig);
  int initQmiClient(const char* devName);
  int releaseQmiClient();

  /* Callbacks registered with QMI FW to receive and process
   * SSR notifications */
  static void errorCallback
      (
         qmi_client_type          user_handle,
         qmi_client_error_type    error,
         void                     *err_cb_data
      );

  static void notifierCallback
      (
         qmi_client_type                user_handle,
         qmi_idl_service_object_type    service_obj,
         qmi_client_notify_event_type   service_event,
         void                           *notify_cb_data
      );

  int processOutOfServiceEvent();
  int processInServiceEvent();

protected:
   NetmgrIwlanClient();

private:
   void getQcciConnId(qmi_connection_id_type qmiConnId);

   bool createObservers();

   static NetmgrIwlanClient *instancePtr;

   bool                           qmiInitRetry;
   qmi_client_type                qmi_notifier;
   qmi_client_type                qmi_clnt_hndl;
   qmi_client_qmux_instance_type  qcci_conn_id;
   CnoNetType                     lastNetType;
   ICneNetworkObserver            *cno;
   ICneFeatureObserver            *cfo;
};
#else
  typedef struct NetmgrIwlanClient NetmgrIwlanClient;
#endif /* __cplusplus*/

using namespace std;

#define NETMGR_MAX_LOG_MSG_SIZE      512

#ifdef __cplusplus
  /* C++ treats anything between "" as string literals which is interpreted as const char*.
   * The current logging macros do not have support for handling const char* so we created
   * a new function to handle this. If required we can move it into ds_util module */

/*---------------------------------------------------------------------------
   Function for formating log messages
---------------------------------------------------------------------------*/
  LOCAL void
  netmgr_format_msg (char* buf_ptr, const char* fmt,...)
  {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf_ptr, NETMGR_MAX_LOG_MSG_SIZE, fmt, ap);
    va_end(ap);
  }

  #define NETMGR_LOG_ERR(...)                                         \
  {                                                                   \
    char buf[NETMGR_MAX_LOG_MSG_SIZE];                                \
    netmgr_format_msg(buf, __VA_ARGS__);                              \
    MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_ERROR, "%s", buf);  \
  }

  #define NETMGR_LOG_LOW(...)                                         \
  {                                                                   \
    char buf[NETMGR_MAX_LOG_MSG_SIZE];                                \
    netmgr_format_msg(buf, __VA_ARGS__);                              \
    MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_LOW, "%s", buf);    \
  }

  #define NETMGR_LOG_MED(...)                                         \
  {                                                                   \
    char buf[NETMGR_MAX_LOG_MSG_SIZE];                                \
    netmgr_format_msg(buf, __VA_ARGS__);                              \
    MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_MED, "%s", buf);    \
  }

  #define NETMGR_LOG_HIGH(...)                                        \
  {                                                                   \
    char buf[NETMGR_MAX_LOG_MSG_SIZE];                                \
    netmgr_format_msg(buf, __VA_ARGS__);                              \
    MSG_SPRINTF_1(MSG_SSID_LINUX_DATA, MSG_LEGACY_HIGH, "%s", buf);   \
  }

#else
  #define NETMGR_LOG_ERR  netmgr_log_err
  #define NETMGR_LOG_LOW  netmgr_log_low
  #define NETMGR_LOG_MED  netmgr_log_med
  #define NETMGR_LOG_HIGH netmgr_log_high
#endif

static qmi_cci_os_signal_type iwlan_os_params;
static qmi_client_type        iwlan_notifier;

/*===========================================================================
                            PUBLIC DEFINITIONS
===========================================================================*/
static pthread_t iwlanClientThread;
static pthread_mutex_t iwlanClientMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  iwlanClientStopCond = PTHREAD_COND_INITIALIZER;

static pthread_mutex_t iwlanClientRecMutex;

NetmgrIwlanClient* NetmgrIwlanClient::instancePtr = NULL;

/*===========================================================================
  FUNCTION  getInstance
===========================================================================*/
/*!
@brief
  Returns an instance of NetmgrIwlanClient. This is a singleton class.

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
NetmgrIwlanClient* NetmgrIwlanClient::getInstance(void)
{
  if (NULL == instancePtr)
  {
    instancePtr = new NetmgrIwlanClient();
  }

  return instancePtr;
}

/*===========================================================================
  FUNCTION  getQcciConnId
===========================================================================*/
/*!
@brief
  Returns the correct QCCI conn id based on the qmi connection id

@param
  qmiConnId QMI connection id

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void NetmgrIwlanClient::getQcciConnId(qmi_connection_id_type qmiConnId)
{
  /* TODO: Add new qmi client connection for 8994 */
  switch (qmiConnId)
  {
  case QMI_CONN_ID_RMNET_0:
    qcci_conn_id = QMI_CLIENT_QMUX_RMNET_INSTANCE_0;
    break;
  case QMI_CONN_ID_RMNET_MDM_0:
    qcci_conn_id = QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0;
    break;
  case QMI_CONN_ID_RMNET_MHI_0:
    qcci_conn_id = QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0;
    break;

  case QMI_CONN_ID_RMNET_MHI_1:
    qcci_conn_id = QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_1;
    break;

  default:
    NETMGR_LOG_MED("Defaulting to rmnet0");
    qcci_conn_id = QMI_CLIENT_QMUX_RMNET_INSTANCE_0;
    break;
  }
}

/*===========================================================================
  FUNCTION  NetmgrIwlanClient
===========================================================================*/
/*!
@brief
  Constructor

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
NetmgrIwlanClient::NetmgrIwlanClient()
{
  /* Initialize member variables */
  cno = NULL;
  cfo = NULL;
  lastNetType = NET_UNKNOWN;
  qmiInitRetry = true;
  qmi_clnt_hndl = NULL;
  qmi_notifier = NULL;
}

/*===========================================================================
  FUNCTION  createObservers
===========================================================================*/
/*!
@brief
  Create observer objects

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
bool NetmgrIwlanClient::createObservers()
{
  if (NULL == cno)
  {
    cno = CneFactory::CreateCneNetworkObserver(this);

    if (NULL == cno)
    {
      NETMGR_LOG_ERR("NetmgrIwlanClient::createObservers: Failed to create CNE Network observer!");
      return false;
    }
  }

  if (NULL == cfo)
  {
    cfo = CneFactory::CreateCneFeatureObserver(this);

    if (NULL == cfo)
    {
      NETMGR_LOG_ERR("NetmgrIwlanClient::createObservers: Failed to create CNE feature observer!");
      return false;
    }
  }

  return true;
}

/*===========================================================================
  FUNCTION  regSignals
===========================================================================*/
/*!
@brief
  Starts observers and registers for appropriate signals

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
bool NetmgrIwlanClient::regSignals()
{
  CnoNetConfigType netConfig;

  if (!createObservers())
  {
    NETMGR_LOG_ERR("regSignals(): No signals registered!");
    return false;
  }

  if (CO_RET_SUCCESS != cno->startSignal(SIGNAL_NET_CONFIG_CHANGE))
  {
    NETMGR_LOG_ERR("regSignals(): Failed to start SIGNAL_NET_CONFIG_CHANGE!");
    return false;
  }

  if (CO_RET_SUCCESS != cfo->startSignal(SIGNAL_FEATURE_ALL)) {
    NETMGR_LOG_ERR("regSignals(): Failed to start SIGNAL_FEATURE_ALL!");
    return false;
  }

  /* Before calling onNetConfigChange from this thread if wifi were
   * to get disconnected then CNE might call onNetConfigChange from
   * a different thread. In order to avoid loss of information we
   * will lock a recursive mutex from netmgr to serialize this
   * operation in the event that CNE and netmgr are having
   * contention */
  pthread_mutex_lock(&iwlanClientRecMutex);

  /* If wifi was connected before netmgr client registered with cne then
   * we need to call getNetConfig to get the current wifi status
   * If wifi was not turned on then getNetConfig will fail */
  if (CO_RET_SUCCESS != cno->getNetConfig(netConfig))
  {
    /* Registration did not fail, only getNetConfig has failed
     * This is not a hard failre so don't return false */
    NETMGR_LOG_ERR("regSignals(): Failed on getNetConfig!");
  }
  else
  {
    /* Call onNetConfigChange to send the current WLAN status to modem */
    onNetConfigChange(netConfig);
  }

  /* Unlock mutex */
  pthread_mutex_unlock(&iwlanClientRecMutex);

  return true;
}

/*===========================================================================
  FUNCTION  stopSignals
===========================================================================*/
/*!
@brief
  Stops CNE observers

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
bool NetmgrIwlanClient::stopSignals()
{
  if (CO_RET_SUCCESS != cno->stopSignal(SIGNAL_NET_CONFIG_CHANGE))
  {
    NETMGR_LOG_ERR("NetmgrIwlanClient::stopSignals: "
                   "Failed to stop SIGNAL_NET_CONFIG_CHANGE");
    return false;
  }

  if (CO_RET_SUCCESS != cfo->stopSignal(SIGNAL_FEATURE_ALL))
  {
    NETMGR_LOG_ERR("NetmgrIwlanClient::stopSignals: "
                   "Failed to stop SIGNAL_FEATURE_ALL");
    return false;
  }

  return true;
}

/*===========================================================================
  FUNCTION  onNetConfigChange
===========================================================================*/
/*!
@brief
  Callback registered with CNE framework to receive WLAN and WQE change
  notifications

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
void NetmgrIwlanClient::onNetConfigChange(const CnoNetConfigType &netConfig)
{
  int rc = QMI_NO_ERR;
  CfoFeatureStatusType wqeStatus;
  dsd_wqe_status_enum_v01 qmiWQEStatus = DSD_WQE_INACTIVE_V01;
  bool wqeAvail = false;

  /* There is a possibility for both CNE and netmgr calling this function
   * at the same time. Use mutex locking to prevent contention */
  pthread_mutex_lock(&iwlanClientRecMutex);

  do
  {
    if (cfo && CO_RET_SUCCESS == cfo->getFeatureStatus(CFO_FEATURE_WQE, wqeStatus))
    {
      qmiWQEStatus = ((wqeStatus == CFO_STATUS_ACTIVE) ? DSD_WQE_ACTIVE_V01 : DSD_WQE_INACTIVE_V01);
      wqeAvail = true;
      NETMGR_LOG_MED("OnNetConfigChange: qmiWQEStatus=%d", qmiWQEStatus);
    }
    else
    {
      NETMGR_LOG_ERR("OnNetConfigChange: failed to obtain WQE status");
    }

    /* WLAN connected */
    if (NET_WLAN == netConfig.net.type)
    {
      CnoWlanSuppNetConfigType wnc;
      static const in6_addr null_ip6_addr = {{{0}}};

      NETMGR_LOG_MED("OnNetConfigChange: Propagating WLAN connect event");

      if (CO_RET_SUCCESS != cno->getWlanSuppNetConfig(wnc))
      {
        NETMGR_LOG_ERR("OnNetConfigChange: Failed to get WLAN supp netconfig");
      }

      /* Send WLAN connected DSD indication */
      dsd_wlan_available_req_msg_v01   req_msg;
      dsd_wlan_available_resp_msg_v01  resp_msg;

      memset(&req_msg, 0, sizeof(req_msg));
      memset(&resp_msg, 0, sizeof(resp_msg));

      memcpy(req_msg.wlan_ap_mac_address,
             wnc.bssid,
             QMI_DSD_MAC_ADDR_LEN_V01);

      if (0 != netConfig.ip4.s_addr)
      {
        req_msg.wlan_ipv4_address_valid = TRUE;
        req_msg.wlan_ipv4_address = ntohl(netConfig.ip4.s_addr);
      }

      if (0 != wnc.ip4dns)
      {
        NETMGR_LOG_ERR("OnNetConfigChange: Copying ipv4 dns addresses ");

        req_msg.dns_ipv4_address_1_valid = TRUE;
        req_msg.dns_ipv4_address_1 = ntohl(wnc.ip4dns[0].s_addr);

        req_msg.dns_ipv4_address_2_valid = TRUE;
        req_msg.dns_ipv4_address_2 = ntohl(wnc.ip4dns[1].s_addr);
      }

      if (NULL != wnc.ssid)
      {
        NETMGR_LOG_ERR("OnNetConfigChange: Copying ssid");

        req_msg.ssid_len = (uint32_t)sizeof(wnc.ssid);
        req_msg.ssid_valid = TRUE;
        if (NULL != memmove(req_msg.ssid,
                            wnc.ssid,
                            QMI_DSD_MAX_SSID_LEN_V01))
        {
          req_msg.ssid_len = (uint32_t)sizeof(req_msg.ssid);
          req_msg.ssid_valid = TRUE;

          NETMGR_LOG_ERR("OnNetConfigChange: Copied ssid %s of len %d",
                         req_msg.ssid, req_msg.ssid_len);
        }
        else
        {
          NETMGR_LOG_ERR("OnNetConfigChange: Failed to copy ssid");
        }
      }

      if (0 != memcmp(netConfig.ip6.s6_addr,
                      null_ip6_addr.s6_addr,
                      sizeof(netConfig.ip6.s6_addr)))
      {
        req_msg.wlan_ipv6_address_valid = TRUE;
        memcpy(req_msg.wlan_ipv6_address.ipv6_address,
               netConfig.ip6.s6_addr,
               QMI_DSD_IPV6_ADDR_LEN_V01);

        //FIXME: Get correct prefix length
        req_msg.wlan_ipv6_address.prefix_len = 64;

      }

      if (true == wqeAvail)
      {
        req_msg.wqe_status_valid = TRUE;
        req_msg.wqe_status = qmiWQEStatus;
      }

      /* Send message */
      rc = qmi_client_send_msg_sync(qmi_clnt_hndl,
                                    QMI_DSD_WLAN_AVAILABLE_REQ_V01,
                                    &req_msg,
                                    sizeof(req_msg),
                                    &resp_msg,
                                    sizeof(resp_msg),
                                    QMI_DSD_SYNC_MSG_TIMEOUT);

      if (QMI_NO_ERR != rc)
      {
        NETMGR_LOG_ERR("OnNetConfigChange: Failed to send qmi_dsd_wlan_available_req, err=%d",
                       rc);
      }
      else if (QMI_NO_ERR != resp_msg.resp.result)
      {
        NETMGR_LOG_ERR("OnNetConfigChange: Failed to send qmi_dsd_wlan_available_req, err=%d",
                       resp_msg.resp.result);
      }

    }
    /* If WLAN was last connected but not anymore, send a DISCONNECT */
    else if (NET_WLAN == lastNetType)
    {
      NETMGR_LOG_MED("OnNetConfigChange: Propagating WLAN disconnect event");

      // Send the WLAN disconnect DSD indication
      dsd_wlan_not_available_req_msg_v01   req_msg;
      dsd_wlan_not_available_resp_msg_v01  resp_msg;

      memset(&req_msg, 0, sizeof(req_msg));
      memset(&resp_msg, 0, sizeof(resp_msg));

      if (true == wqeAvail)
      {
        req_msg.wqe_status_valid = TRUE;
        req_msg.wqe_status = qmiWQEStatus;
      }

      /* Send message */
      rc = qmi_client_send_msg_sync(qmi_clnt_hndl,
                                    QMI_DSD_WLAN_NOT_AVAILABLE_REQ_V01,
                                    &req_msg,
                                    sizeof(req_msg),
                                    &resp_msg,
                                    sizeof(resp_msg),
                                    QMI_DSD_SYNC_MSG_TIMEOUT);

      if (QMI_NO_ERR != rc)
      {
        NETMGR_LOG_ERR("OnNetConfigChange: failed to send qmi_dsd_wlan_not_available_req, err=%d",
                      rc);
      }
      else if (QMI_NO_ERR != resp_msg.resp.result)
      {
        NETMGR_LOG_ERR("OnNetConfigChange: failed to send qmi_dsd_wlan_not_available_req, err=%d",
                      resp_msg.resp.error);
      }
    }

    lastNetType = netConfig.net.type;

  } while (0);

  /* Unlock mutex */
  pthread_mutex_unlock(&iwlanClientRecMutex);
}

/*===========================================================================
  FUNCTION  initQmiClient
===========================================================================*/
/*!
@brief
  Initializes QMI DSD service client

@return
  NETMGR_SUCCESS on success
  NETMGR_FAILURE on error

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int NetmgrIwlanClient::initQmiClient(const char* devName)
{
  int rc = NETMGR_FAILURE;
  int ret = QMI_NO_ERR;
  int i = 0;
  int muxId = -1;
  int epType = -1;
  int epId = -1;
  qmi_service_info qmi_svc_info;
  qmi_idl_service_object_type dsd_srvc_obj = dsd_get_service_object_v01();

  do {
      if (NULL == dsd_srvc_obj)
      {
        NETMGR_LOG_ERR("Unable to get QMI-DSD object");
        return NETMGR_FAILURE;
      }

      if (NULL == qmi_clnt_hndl)
      {
        memset(&qmi_svc_info, 0, sizeof(qmi_service_info));
        memset(&iwlan_os_params, 0, sizeof(qmi_cci_os_signal_type));

        /* Get the qcci conn id */
        getQcciConnId(QMI_PLATFORM_DEV_NAME_TO_CONN_ID_EX(devName,
                                                          &muxId,
                                                          &epType,
                                                          &epId));

        /* Register with xport */
        qmi_cci_qmux_xport_register(qcci_conn_id);

        /* Initialize notifier client handle */
        ret = qmi_client_notifier_init(dsd_srvc_obj, &iwlan_os_params, &qmi_notifier);
        if (QMI_NO_ERR != ret)
        {
          NETMGR_LOG_ERR("failed to initialize client notifier");
          rc = NETMGR_FAILURE;
          break;
        }

        /* Wait till the service comes up */
        while (1)
        {
          /* Check if QMI-DSD service is up */
          ret = qmi_client_get_service_instance(dsd_srvc_obj, qcci_conn_id, &qmi_svc_info);
          NETMGR_LOG_LOW("qmi_client_get_service_instance ret [%d],",
                         ret);

          if (QMI_NO_ERR == ret)
          {
            break;
          }

          /* If there was an error in getting the instance, probably due to service not
           * being available wait for signal from service */
          QMI_CCI_OS_SIGNAL_WAIT(&iwlan_os_params, 500);
        }

        /* Initialize the client */
        ret = qmi_client_init(&qmi_svc_info,
                              dsd_srvc_obj,
                              NULL,
                              NULL,
                              NULL,
                              &qmi_clnt_hndl);

        if (QMI_NO_ERR != ret)
        {
          NETMGR_LOG_ERR("Failed to initialize QMI-DSD client");
          break;
        }

        /* Release the notifier client */
        ret = qmi_client_release(qmi_notifier);
        if (QMI_NO_ERR != ret)
        {
          NETMGR_LOG_ERR("failed to release notifier");
        }

        /* Register for error callback */
        (void) qmi_client_register_error_cb(qmi_clnt_hndl,
                                            NetmgrIwlanClient::errorCallback,
                                            NULL);
      }

      rc = NETMGR_SUCCESS;
  } while (0);

  return rc;
}

/*===========================================================================
  FUNCTION  releaseQmiClient
===========================================================================*/
/*!
@brief
  Releases QMI DSD service client

@return
  NETMGR_SUCCESS on success
  NETMGR_FAILURE on error

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int NetmgrIwlanClient::releaseQmiClient()
{
  int rc = QMI_NO_ERR;

  CneFactory::DestroyCneNetworkObserver(cno);
  CneFactory::DestroyCneFeatureObserver(cfo);

  cno = NULL;
  cfo = NULL;

  qmi_cci_qmux_xport_unregister(qcci_conn_id);

  rc = qmi_client_release(qmi_clnt_hndl);

  if (QMI_NO_ERR != rc)
  {
    NETMGR_LOG_ERR("Error releasing QMI-DSD client");
    return NETMGR_FAILURE;
  }

  qmi_clnt_hndl = NULL;
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  processOutOfServiceEvent
===========================================================================*/
/*!
@brief
  Handler for processing modem out of service event during SSR

@return
  NETMGR_SUCCESS on success
  NETMGR_FAILURE on error

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int NetmgrIwlanClient::processOutOfServiceEvent()
{
  qmi_idl_service_object_type dsd_srvc_obj = dsd_get_service_object_v01();

  if (NULL == dsd_srvc_obj) {
    NETMGR_LOG_ERR("processOutOfServiceEvent(): Failed to get DSD object!");
    return NETMGR_FAILURE;
  }

  NETMGR_LOG_LOW("processOutOfServiceEvent(): Releasing DSD client");
  /* Stop CNE signals */
  if (!stopSignals())
  {
    NETMGR_LOG_ERR("processOutOfServiceEvent(): Failed to stop signals");
    return NETMGR_FAILURE;
  }

  (void) qmi_client_release(qmi_clnt_hndl);
  qmi_clnt_hndl = NULL;

  (void) qmi_client_notifier_init(dsd_srvc_obj, &iwlan_os_params, &iwlan_notifier);
  (void) qmi_client_register_notify_cb(iwlan_notifier, NetmgrIwlanClient::notifierCallback, NULL);

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  processInServiceEvent
===========================================================================*/
/*!
@brief
  Handler for processing modem in service event during SSR

@return
  NETMGR_SUCCESS on success
  NETMGR_FAILURE on error

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int NetmgrIwlanClient::processInServiceEvent()
{
  int rc;
  const char *dev_id;
  ds_target_t target = ds_get_target();
  CnoNetConfigType netConfig;

  /* Determine the correct device id based on target */
  switch (target)
  {
  case DS_TARGET_FUSION4_5_PCIE:
    dev_id = NETMGR_CLIENT_MDMMHI_DEV_ID;
    break;
  case DS_TARGET_MDM:
    dev_id = NETMGR_CLIENT_MDMUSB_DEV_ID;
    break;
  case DS_TARGET_MSM8994:
    dev_id = NETMGR_CLIENT_MSM_DEV_ID;
    break;
  case DS_TARGET_MSM:
  default:
    dev_id = NETMGR_CLIENT_DEFAULT_DEV_ID;
    break;
  }

  NETMGR_LOG_LOW("processInServiceEvent(): Recovered from SSR, reinitializing DSD client");
  (void) qmi_client_release(iwlan_notifier);

  /* Re-initialize client */
  rc = initQmiClient(dev_id);

  if (NETMGR_SUCCESS != rc)
  {
    NETMGR_LOG_ERR("processInServiceEvent(): Failed to create DSD client");
    return NETMGR_FAILURE;
  }

  /* Re-start CNE signals */
  if (!regSignals())
  {
    NETMGR_LOG_ERR("processInServiceEvent(): Failed to start signals!");
    return NETMGR_FAILURE;
  }

  /* Call onNetConfigChange to send the current WLAN status */
  if (CO_RET_SUCCESS != cno->getNetConfig(netConfig))
  {
    NETMGR_LOG_ERR("processInServiceEvent(): Failed to get netConfig!");
  }

  onNetConfigChange(netConfig);

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  errorCallback
===========================================================================*/
/*!
@brief
  Callback to receive error notifications from QMI FW

@return
  None
*/
/*=========================================================================*/
void NetmgrIwlanClient::errorCallback
(
  qmi_client_type          user_handle,
  qmi_client_error_type    error,
  void                     *err_cb_data
)
{
  (void) user_handle;
  (void) error;
  (void) err_cb_data;

  netmgr_cmdq_cb_cmd_t* cmd_buf = NULL;
  NETMGR_LOG_MED("errorCallback(): MODEM is OUT of SERVICE");

  cmd_buf = netmgr_cmdq_get_cmd();

  if (NULL == cmd_buf) {
    NETMGR_LOG_ERR("errorCallback(): Failed to allocate command buffer!");
    return;
  }

  cmd_buf->cmd_data.cmd_type                        = NETMGR_CB_CMD_TYPE_IWLAN;
  cmd_buf->cmd_data.netmgr_union.iwlan_client_event = NETMGR_IWLAN_CLIENT_OOS_EV;

  if ( NETMGR_SUCCESS != netmgr_cmdq_put_cmd(cmd_buf) ) {
    NETMGR_LOG_ERR("errorCallback(): Failed to put command!");
    netmgr_cmdq_release_cmd(cmd_buf);
  }
}

/*===========================================================================
  FUNCTION  notifierCallback
===========================================================================*/
/*!
@brief
  Callback to receive notifier events from QMI FW

@return
  None
*/
/*=========================================================================*/
void NetmgrIwlanClient::notifierCallback
(
  qmi_client_type                user_handle,
  qmi_idl_service_object_type    service_obj,
  qmi_client_notify_event_type   service_event,
  void                           *notify_cb_data
)
{
  (void) user_handle;
  (void) service_obj;
  (void) notify_cb_data;

  netmgr_cmdq_cb_cmd_t* cmd_buf = NULL;
  NETMGR_LOG_MED("notifierCallback(): MODEM is IN SERVICE");

  cmd_buf = netmgr_cmdq_get_cmd();

  if (NULL == cmd_buf) {
    NETMGR_LOG_ERR("notifierCallback(): Failed to allocate command buffer!");
    return;
  }

  switch (service_event) {
  case QMI_CLIENT_SERVICE_COUNT_INC:
    /* Service is back up, post IN_SERVICE to command thread */
    NETMGR_LOG_MED("notifierCallback(): DSD service is up!");
    cmd_buf->cmd_data.cmd_type                        = NETMGR_CB_CMD_TYPE_IWLAN;
    cmd_buf->cmd_data.netmgr_union.iwlan_client_event = NETMGR_IWLAN_CLIENT_IS_EV;

    if ( NETMGR_SUCCESS != netmgr_cmdq_put_cmd(cmd_buf) ) {
      NETMGR_LOG_ERR("notifierCallback(): Failed to put command!");
      netmgr_cmdq_release_cmd(cmd_buf);
    }

    break;

  case QMI_CLIENT_SERVICE_COUNT_DEC:
    NETMGR_LOG_MED("notifierCallback(): DSD service is down!");
    break;
  default:
    NETMGR_LOG_ERR("notifierCallback(): Unsupported service event!");
    break;
  }
}

/*===========================================================================
  FUNCTION  iwlanClientLoop
===========================================================================*/
/*!
@brief
  Instantiates the iwlan client and then waits on a condition variable
  till netmgr is active

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static void*
iwlanClientLoop(void* args)
{
  (void) args;

  /* Get an instance of NetmgrIwlanClient */
  NetmgrIwlanClient* iwlan_client = NetmgrIwlanClient::getInstance();
  int ret = 0;

  if (NULL == iwlan_client) {
    NETMGR_LOG_ERR("Failed to allocate iwlan client!");
    return (void*) NULL;
  }

  pthread_mutex_lock(&iwlanClientMutex);

  /* Create observers and register for signals */
  iwlan_client->regSignals();

  while (!ret) {
    ret = pthread_cond_wait(&iwlanClientStopCond, &iwlanClientMutex);
  }

  NETMGR_LOG_ERR("iwlan client thread terminating...");
  pthread_mutex_unlock(&iwlanClientMutex);

  return NULL;
}

/*===========================================================================
                     GLOBAL FUNCTIONS AND DEFINITIONS
===========================================================================*/

extern "C" {
/*===========================================================================
  FUNCTION  netmgrIwlanClientInit
===========================================================================*/
/*!
@brief
  Initializes the netmgr iwlan client state

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgrIwlanClientInit(void)
{
  const char *dev_id;
  int rc = NETMGR_FAILURE;
  ds_target_t target = ds_get_target();
  pthread_mutexattr_t attr;

  /* Get an instance of NetmgrIwlanClient */
  NetmgrIwlanClient* iwlan_client = NetmgrIwlanClient::getInstance();

  if (NULL == iwlan_client) {
    NETMGR_LOG_ERR("Failed to allocate iwlan client!");
    return NETMGR_FAILURE;
  }

  /* Determine the correct device id based on target */
  switch (target)
  {
  case DS_TARGET_FUSION4_5_PCIE:
    dev_id = NETMGR_CLIENT_MDMMHI_DEV_ID;
    break;
  case DS_TARGET_MDM:
    dev_id = NETMGR_CLIENT_MDMUSB_DEV_ID;
    break;
  case DS_TARGET_MSM8994:
    dev_id = NETMGR_CLIENT_MSM_DEV_ID;
    break;
  case DS_TARGET_MSM:
  default:
    dev_id = NETMGR_CLIENT_DEFAULT_DEV_ID;
    break;
  }

  /* Initialize QMI-DSD client */
  rc = iwlan_client->initQmiClient(dev_id);
  if (NETMGR_SUCCESS != rc)
  {
    NETMGR_LOG_ERR("Failed to initialize QMI-DSD client for netmgr iWLAN client");
    return NETMGR_FAILURE;
  }

  /* Initialize recursive mutex */
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&iwlanClientRecMutex, &attr);
  pthread_mutexattr_destroy(&attr);

  /* Start a thread */
  if (pthread_create(&iwlanClientThread, NULL, iwlanClientLoop,
                     (void*)NULL) < 0) {
    NETMGR_LOG_ERR("Could not create iwlan client thread!");
    return NETMGR_FAILURE;
  }

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgrIwlanClientRelease
===========================================================================*/
/*!
@brief
  Cleans up the netmgr iwlan client state

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgrIwlanClientRelease(void)
{
  int rc = NETMGR_SUCCESS;

   /* Get an instance of NetmgrIwlanClient */
  NetmgrIwlanClient* iwlan_client = NetmgrIwlanClient::getInstance();

  rc = iwlan_client->releaseQmiClient();

  /* Stop the thread */
  pthread_mutex_lock(&iwlanClientMutex);
  pthread_cond_signal(&iwlanClientStopCond);
  pthread_mutex_unlock(&iwlanClientMutex);
  pthread_mutex_destroy(&iwlanClientRecMutex);

  return rc;
}

/*===========================================================================
  FUNCTION  netmgrIwlanClientProcessEvent
===========================================================================*/
/*!
@brief
  Cleans up the netmgr iwlan client state

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgrIwlanClientProcessEvent(netmgr_client_event_t evt)
{
  int rc = NETMGR_SUCCESS;

  /* Get an instance of NetmgrIwlanClient */
  NetmgrIwlanClient* iwlan_client = NetmgrIwlanClient::getInstance();

  switch (evt)
  {
  case NETMGR_IWLAN_CLIENT_OOS_EV:
    NETMGR_LOG_MED("netmgrIwlanClientProcessEvent(): Modem is out of service");
    rc = iwlan_client->processOutOfServiceEvent();
    break;

  case NETMGR_IWLAN_CLIENT_IS_EV:
    NETMGR_LOG_MED("netmgrIwlanClientProcessEvent(): Modem is in service");
    rc = iwlan_client->processInServiceEvent();
    break;

  default:
    NETMGR_LOG_ERR("netmgrIwlanClientProcessEvent(): Unsupported operation");
    rc = NETMGR_FAILURE;
  }

  return rc;
}

} /* extern "C" */
#endif /* FEATURE_DATA_IWLAN */

