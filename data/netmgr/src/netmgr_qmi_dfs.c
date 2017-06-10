/******************************************************************************

                       N E T M G R _ Q M I _D F S . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_qmi_dfs.c
  @brief   Network Manager QMI data filter service helper

  DESCRIPTION
  Network Manager QMI data filter service helper

******************************************************************************/
/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include "qmi_client.h"
#include "qmi_cci_target.h"
#include "qmi_idl_lib.h"
#include "qmi_cci_common.h"
#include "netmgr_qmi_dfs.h"
#include "netmgr_exec.h"
#include "netmgr_qmi_wda.h"
#include "netmgr_kif.h"
#include "netmgr_main.h"
#include "qmi_client_instance_defs.h"
#include "qmi_platform.h"

#include "netmgr_util.h"

#ifndef NULL
#define NULL 0
#endif

#define NETMGR_QMI_MIN_HANDLE 0
#define NETMGR_QMI_NO_HANDLE  -1

/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

qmi_client_type  qmi_dfs_clnt_hndl, qmi_dfs_notifier;
qmi_cci_os_signal_type netmgr_qmi_dfs_os_params;

/*===========================================================================
  FUNCTION  netmgr_qmi_dfs_callback
===========================================================================*/
/*!
@brief
  Call back function registered for QMI DFS indications
*/
/*=========================================================================*/
static
void netmgr_qmi_dfs_callback
(
  qmi_client_type    user_handle,
  unsigned int       msg_id,
  void               *ind_buf,
  unsigned int       ind_buf_len,
  void               *ind_cb_data
)
{
  qmi_client_error_type   qmi_err;
  dfs_low_latency_traffic_ind_msg_v01 filterModeStatus;

  (void) ind_cb_data;

  if (ind_buf == NULL)
  {
    netmgr_log_err("%s(): Called with null pointers!\n", __func__);
    return;
  }

  switch (msg_id)
  {
    case QMI_DFS_LOW_LATENCY_TRAFFIC_IND_V01:
      qmi_err = qmi_client_message_decode(user_handle,
                                          QMI_IDL_INDICATION,
                                          msg_id,
                                          ind_buf,
                                          ind_buf_len,
                                          &filterModeStatus,
                                          sizeof(filterModeStatus));
      if(QMI_NO_ERR != qmi_err)
      {
        netmgr_log_err("%s(): Invalid filter mode ind msg error %d", __func__, qmi_err);
        return;
      }

      netmgr_kif_ifioctl_change_sleep_state((const char *)netmgr_main_get_phys_net_dev(),
                                            !filterModeStatus.traffic_start);
      netmgr_log_med("%s(): Got QMI_DFS_LOW_LATENCY_TRAFFIC_IND_V01 traffic_state: %d\n",
                     __func__,
                     filterModeStatus.traffic_start);

      break;

    default:
      netmgr_log_med("%s(): No handler for DFS IND id %d", __func__, msg_id);
  }
  return;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_dfs_init_client
===========================================================================*/
/*!
@brief
  Initialize QMI handle

@return
  int - NETMGR_QMI_DFS_SUCCESS on successful operation,
        NETMGR_QMI_DFS_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
static int netmgr_qmi_dfs_init_client
(
  qmi_client_qmux_instance_type qcci_conn_id,
  qmi_client_type  *qmi_clnt_hndl,
  qmi_client_type  *notifier
)
{
  int rc;
  qmi_service_info info= {{0}};
  qmi_idl_service_object_type dfs_svc_obj = dfs_get_service_object_v01();

  if (!dfs_svc_obj)
  {
    netmgr_log_err("%s(): Failed to get DFS service object\n", __func__);
    return NETMGR_QMI_DFS_FAILURE;
  }

  memset(&netmgr_qmi_dfs_os_params, 0, sizeof(qmi_cci_os_signal_type));

  rc = qmi_client_notifier_init(dfs_svc_obj, &netmgr_qmi_dfs_os_params, notifier);
  if (rc != QMI_NO_ERR)
  {
    netmgr_log_err("%s(): Failed to do client notifier init\n", __func__);
    return NETMGR_QMI_DFS_FAILURE;
  }
  while(1)
  {
    rc = qmi_client_get_service_instance(dfs_svc_obj, qcci_conn_id, &info);
    netmgr_log_low("%s(): qmi_client_get_service_list() returned %d \n",
                   __func__,
                   rc);
    if(rc == QMI_NO_ERR)
      break;

    /* wait for server to come up */
    QMI_CCI_OS_SIGNAL_WAIT(&netmgr_qmi_dfs_os_params, 500);
  }

  /* The server has come up, store the information in info variable */
  rc = qmi_client_get_service_instance(dfs_svc_obj, qcci_conn_id, &info);
  if ( rc != QMI_NO_ERR )
  {
    netmgr_log_err("%s(): Failed to get service instance\n", __func__);
    return NETMGR_QMI_DFS_FAILURE;
  }
  netmgr_log_low("%s(): qmi_client_get_service_list() returned %d",
                  __func__, rc);

  rc = qmi_client_init(&info,
                       dfs_svc_obj,
                       &netmgr_qmi_dfs_callback,
                       NULL,
                       NULL,
                       qmi_clnt_hndl);

  netmgr_log_low("%s(): qmi_client_init returned %d\n", __func__, rc);
  return rc;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_dfs_ind_cb
===========================================================================*/
/*!
@brief
 Main callback function regisgterd during client init. It posts a command to do the
 required processing in the Command Thread context.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static
void netmgr_qmi_dfs_ind_cb
(
  qmi_client_type                user_handle,
  unsigned int                   msg_id,
  void                           *ind_buf,
  unsigned int                   ind_buf_len,
  void                           *ind_cb_data
)
{
  int link;
  netmgr_exec_cmd_t * cmd = NULL;
  qmi_client_error_type   qmi_err;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT(ind_buf != NULL);

  /* Get link id from user data ptr */
  if (NULL == ind_cb_data)
  {
    netmgr_log_err("%s(): Received invalid input!\n",__func__);
    goto bail;
  }

  /* While initialinz the client and registering for callback we were passing the link argument as a void*
   * In order to read it back we need to tpercast it to int* and dereference */
  link = (int)((uintptr_t) ind_cb_data);

  /* Verify link id */
  NETMGR_ASSERT(netmgr_kif_verify_link(link) == NETMGR_SUCCESS);

  /* Allocate a command object */
  cmd = netmgr_exec_get_cmd();
  NETMGR_ASSERT(cmd);

  netmgr_log_med("%s(): received indication=%d on link=%d\n", __func__, msg_id, link);

  switch (msg_id)
  {
#ifdef FEATURE_DATA_IWLAN
    case QMI_DFS_REVERSE_IP_TRANSPORT_FILTERS_UPDATED_IND_V01:
      qmi_err = qmi_client_message_decode(user_handle,
                                          QMI_IDL_INDICATION,
                                          msg_id,
                                          ind_buf,
                                          ind_buf_len,
                                          &cmd->data.info.qmi_msg.data.dfs_ind.info.filter_ind,
                                          sizeof(cmd->data.info.qmi_msg.data.dfs_ind.info.filter_ind));

      if (QMI_NO_ERR != qmi_err)
      {
        goto bail;
      }
      break;
#endif /* FEATURE_DATA_IWLAN */

    default:
      netmgr_log_err("received unsupported indication type %d\n", msg_id);
      goto bail;
  }

  cmd->data.type                     = NETMGR_QMI_MSG_CMD;
  cmd->data.link                     = link;
  cmd->data.info.qmi_msg.type        = NETMGR_QMI_DFS_IND_CMD;
  cmd->data.info.qmi_msg.data.dfs_ind.link   = link;
  cmd->data.info.qmi_msg.data.dfs_ind.ind_id = msg_id;

  /* Post command for processing in the command thread context */
  if( NETMGR_SUCCESS != netmgr_exec_put_cmd( cmd ) ) {
    NETMGR_ABORT("%s(): failed to put commmand\n", __func__);
    goto bail;
  }

  return;

bail:
  if (NULL != cmd)
  {
    netmgr_exec_release_cmd( cmd );
  }
  NETMGR_LOG_FUNC_EXIT;
}

#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION  netmgr_qmi_dfs_process_rev_ip_filter_rules
===========================================================================*/
/*!
@brief
 Performs processing of an incoming DFS Indication message. This function
 is executed in the Command Thread context.

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
netmgr_qmi_dfs_process_rev_ip_filter_rules
(
  int                                               link,
  dfs_reverse_ip_transport_filters_action_enum_v01  action,
  dfs_filter_rule_type_v01                          *rules,
  unsigned int                                      num_rules
)
{
  unsigned int i;

  if (!rules)
  {
    netmgr_log_err("%s(): invalid input!\n", __func__);
    return;
  }

  if (!NETMGR_KIF_IS_REV_RMNET_LINK(link))
  {
    netmgr_log_high("Ignoring QMI DFS rev ip filter update ind on forward link=%d\n", link);
    return;
  }

  for (i = 0; i < num_rules; ++i)
  {
    if ((DFS_PROTO_ESP_V01 == rules[i].xport_info.xport_protocol) &&
        (QMI_DFS_IPSEC_FILTER_MASK_SPI_V01 & rules[i].xport_info.esp_info.valid_params))
    {
      int ip_family = (DFS_IP_FAMILY_IPV4_V01 == rules[i].ip_info.ip_version) ? AF_INET : AF_INET6;

      if (DFS_REVERSE_IP_TRANSPORT_FILTERS_ADDED_V01 == action)
      {
        netmgr_log_med("%s(): installing ip_family=%d SPI filter=0x%x on link=%d!\n",
                       __func__,
                       rules[i].ip_info.ip_version,
                       htonl(rules[i].xport_info.esp_info.spi),
                       link);

        if (NETMGR_SUCCESS != netmgr_kif_install_spi_filter_rule(ip_family,
                                                                 htonl(rules[i].xport_info.esp_info.spi)))
        {
          netmgr_log_err("%s(): failed to install SPI filter ip_family=%d SPI=0x%x on link=%d!\n",
                         __func__,
                         rules[i].ip_info.ip_version,
                         htonl(rules[i].xport_info.esp_info.spi),
                         link);
        }
      }
      else if (DFS_REVERSE_IP_TRANSPORT_FILTERS_DELETED_V01 == action)
      {
        netmgr_log_med("%s(): removing ip_family=%d SPI filter=0x%x on link=%d!\n",
                       __func__,
                       rules[i].ip_info.ip_version,
                       htonl(rules[i].xport_info.esp_info.spi),
                       link);

        if (NETMGR_SUCCESS != netmgr_kif_remove_spi_filter_rule(ip_family,
                                                                htonl(rules[i].xport_info.esp_info.spi)))
        {
          netmgr_log_err("%s(): failed to remove SPI filter ip_family=%d SPI=0x%x on link=%d!\n",
                         __func__,
                         rules[i].ip_info.ip_version,
                         htonl(rules[i].xport_info.esp_info.spi),
                         link);
        }
      }
      else
      {
        netmgr_log_err("%s(): unknown action=%d on link=%d!\n", __func__, action, link);
      }
    }
  }
}
#endif /* FEATURE_DATA_IWLAN */

/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  rmnet_qmi_dfs_release_qmi_client
===========================================================================*/
/*!
@brief
  Release QMI handle

@arg *dev_id Name of device to release the QMI DFS client

@return
  void

*/
/*=========================================================================*/
void rmnet_qmi_dfs_release_qmi_client
(
  const char *dev_id
)
{
  int rc;
  qmi_client_qmux_instance_type qcci_conn_id = 0;
  if (!dev_id)
  {
    netmgr_log_err("%s(): Called with null pointers!\n", __func__);
    return;
  }

  if (NETMGR_SUCCESS !=(netmgr_qmi_map_dev_conn_instance(dev_id,
                                                         &qcci_conn_id)))
  {
    netmgr_log_err("%s(): bad device ID received\n", __func__);
    return;
  }
  qmi_cci_qmux_xport_unregister(qcci_conn_id);

  rc = qmi_client_release(qmi_dfs_clnt_hndl);
  netmgr_log_low("%s(): qmi_client_release of qmi_clnt_hndl returned %d\n", __func__, rc);

  rc = qmi_client_release(qmi_dfs_notifier);
  netmgr_log_low("%s(): qmi_client_release of notifier returned %d\n", __func__, rc);
}

/*===========================================================================
  FUNCTION  netmgr_qmi_dfs_enable_low_latency_filters
===========================================================================*/
/*!
@brief
  Enable the low latency filter indications

@arg *dev_id Name of device to enable the low latency filter indications on

@return
  int - NETMGR_QMI_DFS_SUCCESS on successful operation,
        NETMGR_QMI_DFS_BAD_ARGUMENTS if called with null/invalid arguments
        NETMGR_QMI_DFS_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_dfs_enable_low_latency_filters
(
  const char *dev_id
)
{
  int rc;
  qmi_client_qmux_instance_type qcci_conn_id = 0;
  dfs_indication_register_req_msg_v01 request;
  dfs_indication_register_resp_msg_v01 response;

  if (!dev_id)
  {
    netmgr_log_err("%s(): Called with null pointers!\n", __func__);
    return NETMGR_QMI_DFS_BAD_ARGUMENTS;
  }

  if (NETMGR_SUCCESS !=(netmgr_qmi_map_dev_conn_instance(dev_id,
                                                         &qcci_conn_id)))
  {
    netmgr_log_err("%s(): bad device ID received\n", __func__);
    return NETMGR_QMI_DFS_BAD_ARGUMENTS;
  }

  qmi_cci_qmux_xport_register(qcci_conn_id);

  netmgr_qmi_dfs_init_client(qcci_conn_id, &qmi_dfs_clnt_hndl, &qmi_dfs_notifier);
  if (qmi_dfs_clnt_hndl == NULL)
  {
      rc = NETMGR_QMI_DFS_FAILURE;
      goto bail;
  }

  request.report_low_latency_traffic_valid = TRUE;
  request.report_low_latency_traffic = TRUE;

  rc = qmi_client_send_msg_sync(qmi_dfs_clnt_hndl,
                                QMI_DFS_INDICATION_REGISTER_REQ_V01,
                                &request,
                                sizeof(dfs_indication_register_req_msg_v01),
                                &response,
                                sizeof(dfs_indication_register_resp_msg_v01),
                                NETMGR_QMI_TIMEOUT);
  if (QMI_NO_ERR != rc)
  {
    netmgr_log_err("%s(): Error sending QMI_DFS_INDICATION_REGISTER_REQ_V01 message: %d",
                   __func__,
                   rc);
    rc = NETMGR_QMI_DFS_FAILURE;
    goto bail;
  }

  netmgr_log_med("%s(): Successfully registered for QMI_DFS_INDICATION_REGISTER_REQ_V01: %d",
                 __func__,
                 rc);

  return NETMGR_QMI_DFS_SUCCESS;

bail:
  rmnet_qmi_dfs_release_qmi_client(dev_id);
  return rc;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_dfs_init_qmi_client
===========================================================================*/
/*!
@brief
  Initialize QMI handle

@return
  int - NETMGR_QMI_DFS_SUCCESS on successful operation,
        NETMGR_QMI_DFS_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_dfs_init_qmi_client
(
  int                      link,
  const char               *dev_str,
  qmi_ip_family_pref_type  ip_family,
  qmi_client_type          *user_handle
)
{
  qmi_connection_id_type conn_id;
  qmi_client_error_type rc = QMI_INTERNAL_ERR;
  dfs_bind_client_req_msg_v01 req;
  dfs_bind_client_resp_msg_v01 resp;
  qmi_client_qmux_instance_type qcci_conn_id;
  unsigned int sio_port = 0;
  int epid = -1;
  int ep_type = -1;
  int mux_id = -1;
  /* On 64-bit compilers we cannot typecast int to void* directly */

  memset(&netmgr_qmi_dfs_os_params, 0, sizeof(qmi_cci_os_signal_type));

  *user_handle = NULL;

  if (QMI_CONN_ID_INVALID ==
      (conn_id = QMI_PLATFORM_DEV_NAME_TO_CONN_ID_EX(dev_str,
                                                     &ep_type,
                                                     &epid,
                                                     &mux_id)))
  {
    netmgr_log_err("%s(): failed to find mux_id for dev_str=%s!\n", __func__, dev_str);
    goto bail;
  }

  /* Create client using QCCI libary - this is a blocking call.
     DFS service should be available over IPC Router */
  rc = qmi_client_init_instance(dfs_get_service_object_v01(),
                                QMI_CLIENT_INSTANCE_ANY,
                                netmgr_qmi_dfs_ind_cb,
                                (void *)(uintptr_t)link,
                                &netmgr_qmi_dfs_os_params,
                                NETMGR_QMI_TIMEOUT,
                                user_handle);

  if (QMI_NO_ERR != rc)
  {
    netmgr_log_err("%s(): failed on qmi_client_init_instance with rc=%d!\n", __func__, rc);
    goto bail;
  }

  /* Bind to epid/mux-id */
  if (mux_id > 0)
  {
    memset (&req, 0, sizeof(req));
    memset (&resp, 0, sizeof(resp));

    req.ep_id_valid = (epid != -1);
    req.ep_id.ep_type = (data_ep_type_enum_v01) ep_type;
    req.ep_id.iface_id = (uint32_t) epid;

    req.mux_id_valid = (mux_id != -1);
    req.mux_id = (uint8_t) mux_id;

    req.ip_preference_valid = 1;
    req.ip_preference = ip_family;

    /* Send bind_req */
    rc = qmi_client_send_msg_sync(*user_handle,
                                  QMI_DFS_BIND_CLIENT_REQ_V01,
                                  (void *)&req,
                                  sizeof(req),
                                  (void*)&resp,
                                  sizeof(resp),
                                  NETMGR_QMI_TIMEOUT);
  }

bail:
  if (rc != QMI_NO_ERR || QMI_RESULT_SUCCESS_V01 != resp.resp.result)
  {
    if (NULL != *user_handle)
    {
      qmi_client_release(*user_handle);
    }
    return NETMGR_FAILURE;
  }
  else
  {
    return NETMGR_SUCCESS;
  }
}

/*===========================================================================
  FUNCTION  netmgr_qmi_dfs_process_ind
===========================================================================*/
/*!
@brief
 Performs processing of an incoming DFS Indication message. This function
 is executed in the Command Thread context.

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
netmgr_qmi_dfs_process_ind
(
  int           link,
  unsigned int  ind_id,
  void          *ind_data
)
{
  NETMGR_LOG_FUNC_ENTRY;

  /* Verify link id is valid before proceeding */
  NETMGR_ASSERT(netmgr_kif_verify_link(link) == NETMGR_SUCCESS);

  NETMGR_ASSERT(ind_data);

  /* Process based on indication type */
  switch (ind_id)
  {
#ifdef FEATURE_DATA_IWLAN
    case QMI_DFS_REVERSE_IP_TRANSPORT_FILTERS_UPDATED_IND_V01:
    {
      dfs_reverse_ip_transport_filters_updated_ind_msg_v01  *filter_ind;

      filter_ind = (dfs_reverse_ip_transport_filters_updated_ind_msg_v01 *) ind_data;
      if (filter_ind->filter_rules_valid)
      {
        netmgr_qmi_dfs_process_rev_ip_filter_rules(link,
                                                   filter_ind->filter_action,
                                                   filter_ind->filter_rules,
                                                   filter_ind->filter_rules_len);
      }
      break;
    }
#endif /* FEATURE_DATA_IWLAN */

    default:
      /* Ignore all other indications */
      netmgr_log_high("Ignoring QMI DFS IND of type 0x%x\n", ind_id);
      break;
  }

  NETMGR_LOG_FUNC_EXIT;
  return;
}

#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION  netmgr_qmi_dfs_query_and_process_rev_ip_filters
===========================================================================*/
/*!
@brief
  Query and process reverse IP transport filters

@return
  int - NETMGR_QMI_DFS_SUCCESS on successful operation,
        NETMGR_QMI_DFS_FAILURE if there was an error sending QMI message

*/
/*=========================================================================*/
int netmgr_qmi_dfs_query_and_process_rev_ip_filters
(
  int                link,
  qmi_client_type    user_handle
)
{
  qmi_client_error_type rc = QMI_NO_ERR;
  int ret = NETMGR_SUCCESS;
  dfs_get_reverse_ip_transport_filters_req_msg_v01   req;
  dfs_get_reverse_ip_transport_filters_resp_msg_v01  resp;

  memset (&req, 0, sizeof(req));
  memset (&resp, 0, sizeof(resp));

  /* Send bind_req */
  rc = qmi_client_send_msg_sync(user_handle,
                                QMI_DFS_GET_REVERSE_IP_TRANSPORT_FILTERS_REQ_V01,
                                (void *)&req,
                                sizeof(req),
                                (void*)&resp,
                                sizeof(resp),
                                NETMGR_QMI_TIMEOUT);

  if (QMI_NO_ERR != rc || QMI_RESULT_SUCCESS_V01 != resp.resp.result)
  {
    netmgr_log_err("%s(): rev ip filter query failed rc=%d on link=%d\n", __func__, rc, link);
    ret = NETMGR_FAILURE;
    goto bail;
  }

  if (resp.filter_rules_valid)
  {
    netmgr_qmi_dfs_process_rev_ip_filter_rules(link,
                                               DFS_REVERSE_IP_TRANSPORT_FILTERS_ADDED_V01,
                                               resp.filter_rules,
                                               resp.filter_rules_len);
  }

bail:
  return ret;
}

#endif /* FEATURE_DATA_IWLAN */


