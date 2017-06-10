/******************************************************************************

                        QMI_IP_NETLINK.C

******************************************************************************/

/******************************************************************************

  @file    qmi_ip.c
  @brief   Qualcomm mapping interface over IP netlink handler

  DESCRIPTION


  ---------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------


******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
08/30/13   tw         Initial version

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#include "qmi_ip_netlink.h"
#include "qmi_ip_cmdq.h"

SSL* ssl;

/*=============================================================================
                                FUNCTION FORWARD DECLARATION
==============================================================================*/
void qmi_ip_qcmap_msgr_ind_cb
(
 qmi_client_type user_handle,                    /* QMI user handle       */
 unsigned int    msg_id,                         /* Indicator message ID  */
 void           *ind_buf,                        /* Raw indication data   */
 unsigned int    ind_buf_len,                    /* Raw data length       */
 void           *ind_cb_data                     /* User call back handle */
 );

/*=============================================================================
                                FUNCTION DEFINITIONS
==============================================================================*/

/*==========================================================================

FUNCTION QMI_IP_WDS_IND_CB()

DESCRIPTION

  Indication call back for WDS client: This function is registered during
  WDS client initialization and is invoked when any indication is sent from
  WDS service on modem to the client.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
void* qmi_ip_wds_ind_cb
(
  qmi_client_type user_handle,                    /* QMI user handle       */
  unsigned int                  msg_id,           /* Indicator message ID  */
  void                          *ind_buf,         /* Raw indication data   */
  unsigned int                  ind_buf_len,      /* Raw data length      */
  void                          *ind_cb_data      /* User call back handle */
)
{
  int ret;
  unsigned char *msg_ptr;
  unsigned char *tmp_msg_ptr;
  qmi_client_error_type qmi_error;

  if (ssl == NULL)
    return NULL;

  int msg_ptr_len = ind_buf_len + QMI_HEADER_SIZE;// ind_buf_len + header
  msg_ptr = malloc(msg_ptr_len);

  if (msg_ptr == NULL){
    LOG_MSG_ERROR("MALLOC failure",0,0,0);
    return NULL;
  }
  LOG_MSG_INFO1("qmi_ip_wds_ind_cb: msg_id received %d",msg_id,0,0);

  if(msg_id == QMI_WDS_PKT_SRVC_STATUS_IND_V01 &&
     (user_handle == qmi_ip_conf.qmi_ip_v4_wds_handle ))
  {
    wds_pkt_srvc_status_ind_msg_v01  ind_data;
    qmi_error = qmi_client_message_decode(user_handle,
                                          QMI_IDL_INDICATION,
                                          msg_id,
                                          ind_buf,
                                          ind_buf_len,
                                          &ind_data,
                                          sizeof(wds_pkt_srvc_status_ind_msg_v01));
    if (qmi_error != QMI_NO_ERR)
    {
      LOG_MSG_ERROR("qmi_ip_wds_ind_cb: qmi_client_message_decode error %d",
          qmi_error,0,0);
      return NULL;
    }
    LOG_MSG_INFO1("qmi_ip_wds_ind_cb:technology:%d",ind_data.tech_name,0,0);
    LOG_MSG_INFO1("qmi_ip_wds_ind_cb:link_status:%d",\
                    ind_data.status.connection_status,0,0);

    if (ind_data.status.connection_status == WDS_CONNECTION_STATUS_CONNECTED_V01 )
    {
      LOG_MSG_INFO1("qmi_ip_wds_ind_cb: eMBMS call connected",0,0,0);
      embms_call_state = UP;
    }
    else if (ind_data.status.connection_status == WDS_CONNECTION_STATUS_DISCONNECTED_V01)
    {
      LOG_MSG_INFO1("qmi_ip_wds_ind_cb: eMBMS call disconnected",0,0,0);
      embms_call_state = DOWN;
    }
  }
  if (user_handle == qmi_ip_conf.qmi_ip_v4_wds_handle )
  {
    tmp_msg_ptr = msg_ptr;

    WRITE_QMI_HEADER(tmp_msg_ptr, msg_ptr_len, QMI_WDS_SERVICE_TYPE, msg_id,
                     QMI_IND_CTRL_FLAGS, QMI_IND_TX_ID, msg_ptr_len - QMI_HEADER_SIZE);
    memcpy(tmp_msg_ptr, ind_buf, ind_buf_len);

    ret = SSL_write(ssl, msg_ptr, msg_ptr_len);
  }

  return;
}

/*==========================================================================

FUNCTION QMI_IP_WAIT_FOR_SRV_THEN_GET_INFO()

DESCRIPTION

  This function is used during QMI client initialization where we wait
  for the QMI service to come up and then we get the information about
  the service before we initialize a client for that service

DEPENDENCIES
  None.

RETURN VALUE
  TRUE on success
  FALSE on failure


SIDE EFFECTS
  None

==========================================================================*/

static boolean qmi_ip_wait_for_srv_then_get_info
(
  qmi_idl_service_object_type svc_obj,
  qmi_service_info *qmi_svc_info,
  qmi_client_qmux_instance_type rmnet_instance
)
{
  qmi_client_os_params os_params;
  qmi_client_type notifier_handle;
  qmi_client_error_type err;
  boolean success = FALSE;
  uint32_t num_services = 0, num_entries = 0;
  qmi_service_info            info[10];

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  -*/

  LOG_MSG_INFO1("Entering qti_qmi_wait_for_srv_then_get_info",0,0,0);
  err = qmi_cci_qmux_xport_register(rmnet_instance);

  /*-----------------------------------------------------------------------
  Initialize client notifier
  ------------------------------------------------------------------------*/
  err = qmi_client_notifier_init(svc_obj, &os_params, &notifier_handle);
  if (err != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Error %d while trying to initialize client notifier",
                  err,0,0);
  }
  else
  {
    err = qmi_client_get_service_instance(svc_obj,
                                          rmnet_instance,
                                          qmi_svc_info);

    if (err != QMI_NO_ERR)
    {
      LOG_MSG_ERROR("Error %d while trying to get service info", err,0,0);
    }
    else
    {
      LOG_MSG_INFO1("Got service instance success",0,0,0);
      success = TRUE;
    }

    /*----------------------------------------------------------------------
     We need to release the client notifier here. Client notifier is only
     used to notify the client when the QMI service comes up.However client
     notifier acts as an actual QMI service client even though it is not used
     as an actual client of the service. We need to release it here to avoid
     unnecessary overhead of maintaining another client.
    ------------------------------------------------------------------------*/
    err = qmi_client_release(notifier_handle);
    if (err != QMI_NO_ERR)
    {
      LOG_MSG_ERROR("Error %d while releasing client notifier handle", err,0,0);
    }
  }

  return success;
}

/*===========================================================================

FUNCTION QMI_IP_WDS_INIT()

DESCRIPTION

  This function initializes WDS client handle

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None
===========================================================================*/

int qmi_ip_wds_init(qmi_ip_conf_t * conf, SSL* s, qmi_client_qmux_instance_type rmnet_instance)
{
  qmi_service_info qmi_svc_info;
  int ret_val;
  qmi_client_error_type err;
  qmi_idl_service_object_type wds_service_object;
  wds_indication_register_req_msg_v01 ind_req;
  wds_indication_register_resp_msg_v01 ind_resp;
/*- - - - - - - - - --  - - - - - - -- - - - - - - - - - - - - - - - - - - - */
  if (s == NULL || conf == NULL)
    return -1;
  ssl = s;

  if ((conf->qmi_ip_v4_wds_handle != 0) && (conf->qmi_ip_v6_wds_handle != 0)){
    LOG_MSG_ERROR("Handles already populated",0,0,0);
    return QMI_IP_SUCCESS;
  }

  /*------------------------------------------------------------------
  Get the service object, wait for the service and get the information
  about the service and then get an handle for the service.
  -------------------------------------------------------------------*/
  wds_service_object = wds_get_service_object_v01();

  if(!wds_service_object)
  {
    LOG_MSG_ERROR("qmi_ip wds get service object failed",0,0,0);
    return QMI_IP_ERROR;
  }

  memset(&qmi_svc_info,0,sizeof(qmi_service_info ));

  if(!qmi_ip_wait_for_srv_then_get_info(wds_service_object,
                                      &qmi_svc_info,
                                        rmnet_instance))
  {
    LOG_MSG_ERROR("Error getting info for QMI_IP service",0,0,0);
    return QMI_IP_ERROR;
  }
  /*----------------------------------------------------------------
  Get handle for IPv4
  ------------------------------------------------------------------*/
  ret_val = qmi_client_init(&qmi_svc_info,
                            wds_service_object,
                            (void *) qmi_ip_wds_ind_cb,
                            NULL,
                            NULL,
                            &(conf->qmi_ip_v4_wds_handle));

  if(ret_val != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Error while trying to initialize client %d", ret_val,0,0);
    return QMI_IP_ERROR;
  }
  else
  {
    LOG_MSG_INFO1("Successfully allocated client init WDS service for v4, handle %d",
                  conf->qmi_ip_v4_wds_handle,0,0);
  }

  if (rmnet_instance != QMI_CLIENT_QMUX_RMNET_INSTANCE_8)
  {
    memset(&ind_req, 0, sizeof(wds_indication_register_req_msg_v01));
    memset(&ind_resp, 0, sizeof(wds_indication_register_resp_msg_v01));
    ind_req.report_embms_tmgi_list_valid = 1;
    ind_req.report_embms_tmgi_list = 1;
    ret_val = qmi_client_send_msg_sync(conf->qmi_ip_v4_wds_handle,
                                       QMI_WDS_INDICATION_REGISTER_REQ_V01,
                                       (void*)&ind_req,
                                       sizeof(wds_indication_register_req_msg_v01),
                                       (void*)&ind_resp,
                                       sizeof(wds_indication_register_resp_msg_v01),
                                       QMI_TIMEOUT);

    if((ret_val != QMI_NO_ERR) || (ind_resp.resp.result != QMI_NO_ERR))
    {
      LOG_MSG_ERROR("Error while trying send message: %d %d", ret_val,
                    ind_resp.resp.result,0);
      return QMI_IP_ERROR;
    }
  }

  /*----------------------------------------------------------------------
  Get handle for IPV6
  -----------------------------------------------------------------------*/
  ret_val = qmi_client_init(&qmi_svc_info,
                            wds_service_object,
                            (void *) qmi_ip_wds_ind_cb,
                            NULL,
                            NULL,
                            &(conf->qmi_ip_v6_wds_handle));
  if(ret_val != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Error while trying to initialize client ",0,0,0);
    return QMI_IP_ERROR;
  }
  else
  {
    LOG_MSG_INFO1("Successfully allocated client for v6, handle %d",
                  conf->qmi_ip_v6_wds_handle,0,0);
  }

  return QMI_IP_SUCCESS;
}

/*===========================================================================

FUNCTION QMI_IP_WDS_RELEASE()

DESCRIPTION

  This function releases the WDS client handles

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None
===========================================================================*/
void qmi_ip_wds_release(qmi_ip_conf_t * conf){
  int ret;

  if ((conf->qmi_ip_v4_wds_handle == 0) && (conf->qmi_ip_v6_wds_handle == 0)){
    LOG_MSG_INFO1("Handles not populated",0,0,0);
    return;
  }

  ret = qmi_client_release(conf->qmi_ip_v4_wds_handle);
  if (ret != QMI_NO_ERR)
  {
     LOG_MSG_ERROR("Error %d while releasing wds v4 client handle", ret,0,0);
  }
  else
    LOG_MSG_INFO1("Successfully released the wds v4 handle",0,0,0);

  ret = qmi_client_release(conf->qmi_ip_v6_wds_handle);
  if (ret != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Error %d while releasing wds v6 client handle", ret,0,0);
  }
  else
    LOG_MSG_INFO1("Successfully released the wds v6 handle",0,0,0);

  conf->qmi_ip_v4_wds_handle = (qmi_client_type) NULL;
  conf->qmi_ip_v6_wds_handle = (qmi_client_type) NULL;
}

/*=============================================================================
  FUNCTION qmi_ip_QCMAP_INIT()

  DESCRIPTION

  This function initializes qmi_ip interface to QCMAP

  DEPENDENCIES
  None.

  RETURN VALUE
  QMI_IP_SUCCESS on success
  QMI_IP_ERROR on failure

  SIDE EFFECTS
  None
==============================================================================*/
int qmi_ip_qcmap_init(qmi_ip_conf_t * conf)
{
  qmi_idl_service_object_type                            qmi_ip_qcmap_msgr_service_object;
  uint32_t                                               num_services = 0, num_entries = 0;
  qmi_service_info                                       info[10];
  qmi_client_error_type                                  qmi_error, qmi_err_code = QMI_NO_ERR;
  qcmap_msgr_mobile_ap_status_ind_register_req_msg_v01   qcmap_mobile_ap_status_ind_reg;
  qcmap_msgr_mobile_ap_status_ind_register_resp_msg_v01  qcmap_mobile_ap_status_ind_rsp;
  qcmap_msgr_wwan_status_ind_register_req_msg_v01        wwan_status_ind_reg;
  qcmap_msgr_wwan_status_ind_register_resp_msg_v01       wwan_status_ind_rsp;
  qmi_client_type                                        qmi_ip_qcmap_msgr_notifier;
  qmi_cci_os_signal_type                                 qmi_ip_qcmap_msgr_os_params;
  int                                                    retry_count = 0;
/*---------------------------------------------------------------------------*/

  if(conf == NULL)
  {
    LOG_MSG_ERROR("qmi_ip_qcmap_init() failed: conf null", 0, 0, 0);
    return QMI_IP_ERROR;
  }

  LOG_MSG_INFO1("qmi_ip_qcmap_init()", 0, 0, 0);

/*-----------------------------------------------------------------------------
  Obtain a QCMAP messenger service client for qmi_ip
  - get the service object
  - notify the client
  - get service list
  - obtain the client
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Get the service object
------------------------------------------------------------------------------*/
  qmi_ip_qcmap_msgr_service_object = qcmap_msgr_get_service_object_v01();
  if (qmi_ip_qcmap_msgr_service_object == NULL)
  {
    LOG_MSG_ERROR("qmi_ip QCMAP messenger service object not available",
                   0, 0, 0);
    return QMI_IP_ERROR;
  }

/*-----------------------------------------------------------------------------
  Notify the client
------------------------------------------------------------------------------*/

  qmi_error = qmi_client_notifier_init(qmi_ip_qcmap_msgr_service_object,
                                       &qmi_ip_qcmap_msgr_os_params,
                                       &qmi_ip_qcmap_msgr_notifier);
  if (qmi_error < 0)
  {
    LOG_MSG_ERROR("qmi_ip:qmi_client_notifier_init(qcmap_msgr) returned %d",
                   qmi_error, 0, 0);
    return QMI_IP_ERROR;
  }

/*----------------------------------------------------------------------------
  Check if the service is up, if not wait on a signal
-----------------------------------------------------------------------------*/
  while(retry_count < 10)//max retry
  {
    qmi_error = qmi_client_get_service_list(qmi_ip_qcmap_msgr_service_object,
                                            NULL,
                                            NULL,
                                            &num_services);
    LOG_MSG_INFO1(" qmi_ip: qmi_client_get_service_list: %d", qmi_error, 0, 0);

    if(qmi_error == QMI_NO_ERR)
      break;

/*----------------------------------------------------------------------------
     wait for server to come up
-----------------------------------------------------------------------------*/
    QMI_CCI_OS_SIGNAL_WAIT(&qmi_ip_qcmap_msgr_os_params, 500);//max timeout
    QMI_CCI_OS_SIGNAL_CLEAR(&qmi_ip_qcmap_msgr_os_params);
    LOG_MSG_INFO1("Returned from os signal wait", 0, 0, 0);
    retry_count++;
  }

  if(retry_count == 10 )//max retry
  {
    qmi_client_release(qmi_ip_qcmap_msgr_notifier);
    qmi_ip_qcmap_msgr_notifier = NULL;
    LOG_MSG_ERROR("Reached maximum retry attempts %d", retry_count, 0, 0);
    return QMI_IP_ERROR;
  }

  num_entries = num_services;

  LOG_MSG_INFO1(" qmi_ip: qmi_client_get_service_list: num_e %d num_s %d",
                num_entries, num_services, 0);

/*----------------------------------------------------------------------------
   The server has come up, store the information in info variable
------------------------------------------------------------------------------*/
  qmi_error = qmi_client_get_service_list(qmi_ip_qcmap_msgr_service_object,
                                          info,
                                          &num_entries,
                                          &num_services);

  LOG_MSG_INFO1("qmi_client_get_service_list: num_e %d num_s %d error %d",
                num_entries, num_services, qmi_error);

  if (qmi_error != QMI_NO_ERR)
  {
    qmi_client_release(qmi_ip_qcmap_msgr_notifier);
    qmi_ip_qcmap_msgr_notifier = NULL;
    LOG_MSG_ERROR("Can not get qcmap_msgr service list %d",
                  qmi_error, 0, 0);
    return QMI_IP_ERROR;
  }

/*----------------------------------------------------------------------------
  Obtain a QCMAP messenger client for qmi_ip
------------------------------------------------------------------------------*/
  qmi_error = qmi_client_init(&info[0],
                              qmi_ip_qcmap_msgr_service_object,
                              qmi_ip_qcmap_msgr_ind_cb,
                              NULL,
                              NULL,
                              &(conf->qmi_ip_qcmap_msgr_handle));

  LOG_MSG_INFO1("qmi_client_init: %d", qmi_error, 0, 0);


  if (qmi_error != QMI_NO_ERR)
  {
    qmi_client_release(qmi_ip_qcmap_msgr_notifier);
    qmi_ip_qcmap_msgr_notifier = NULL;
    LOG_MSG_ERROR("Can not init qcmap_msgr client %d", qmi_error, 0, 0);
    return QMI_IP_ERROR;
  }

/*----------------------------------------------------------------------------
  Release QCMAP notifier as it acts as a client also
------------------------------------------------------------------------------*/
  qmi_error = qmi_client_release(qmi_ip_qcmap_msgr_notifier);
  qmi_ip_qcmap_msgr_notifier = NULL;

  if (qmi_error != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Can not release client qcmap notifier %d",
                  qmi_error,0,0);
  }

/*-----------------------------------------------------------------------------
  Register for WWAN indications from QCMAP
-----------------------------------------------------------------------------*/
  memset(&wwan_status_ind_reg,0,
         sizeof(qcmap_msgr_wwan_status_ind_register_req_msg_v01));

  memset(&wwan_status_ind_rsp,0,
         sizeof(qcmap_msgr_wwan_status_ind_register_resp_msg_v01));

  wwan_status_ind_reg.register_indication = 1;
  qmi_error = qmi_client_send_msg_sync(conf->qmi_ip_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_WWAN_STATUS_IND_REG_REQ_V01,
                                       (void*)&wwan_status_ind_reg,
                                       sizeof(qcmap_msgr_wwan_status_ind_register_req_msg_v01),
                                       (void*)&wwan_status_ind_rsp,
                                       sizeof(qcmap_msgr_wwan_status_ind_register_resp_msg_v01),
                                       QMI_TIMEOUT);

  LOG_MSG_INFO1("qmi_client_send_msg_sync(enable): error %d result %d",
      qmi_error, wwan_status_ind_rsp.resp.result,0);

  if ((qmi_error != QMI_NO_ERR) ||
      (wwan_status_ind_rsp.resp.result != QMI_NO_ERR))
  {
    LOG_MSG_ERROR("Can not register for wwan status %d : %d",
        qmi_error, wwan_status_ind_rsp.resp.error,0);
    return QMI_IP_ERROR;
  }

  LOG_MSG_INFO1("Done registering for wwan status",0,0,0);



/*-----------------------------------------------------------------------------
  Register for MOBILE AP state indications from QCMAP
-----------------------------------------------------------------------------*/
  memset(&qcmap_mobile_ap_status_ind_reg, 0,
         sizeof(qcmap_msgr_mobile_ap_status_ind_register_req_msg_v01));
  memset(&qcmap_mobile_ap_status_ind_rsp, 0,
         sizeof(qcmap_msgr_mobile_ap_status_ind_register_resp_msg_v01));

  qcmap_mobile_ap_status_ind_reg.register_indication = 1;
  qmi_error = qmi_client_send_msg_sync(conf->qmi_ip_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_MOBILE_AP_STATUS_IND_REG_REQ_V01,
                                       (void*)&qcmap_mobile_ap_status_ind_reg,
                                       sizeof(qcmap_msgr_mobile_ap_status_ind_register_req_msg_v01),
                                       (void*)&qcmap_mobile_ap_status_ind_rsp,
                                       sizeof(qcmap_msgr_mobile_ap_status_ind_register_resp_msg_v01),
                                       9000);

  LOG_MSG_INFO1("qmi_client_send_msg_sync(enable): error %d result %d",
      qmi_error, qcmap_mobile_ap_status_ind_rsp.resp.result, 0);

  if ((qmi_error != QMI_NO_ERR) ||
      (qcmap_mobile_ap_status_ind_rsp.resp.result != QMI_NO_ERR))
  {
    LOG_MSG_ERROR("Can not register for mobile ap status %d : %d",
                  qmi_error,
                  qcmap_mobile_ap_status_ind_rsp.resp.error, 0);
    return QMI_IP_ERROR;
  }

  LOG_MSG_INFO1("Done registering for mobile ap status",0,0,0);

  return QMI_IP_SUCCESS;
}


int qmi_ip_qcmap_exit(void);

/*=============================================================================
  FUNCTION qmi_ip_QCMAP_EXIT()

  DESCRIPTION

  This function releases QCMAP client

  DEPENDENCIES
  None.

  RETURN VALUE
  QMI_IP_SUCCESS on success
  QMI_IP_ERROR on failure

  SIDE EFFECTS
  None
==============================================================================*/
int qmi_ip_qcmap_exit(void)
{
  qmi_client_error_type     qmi_error;

/*----------------------------------------------------------------------------*/

  qmi_error = qmi_client_release(qmi_ip_conf.qmi_ip_qcmap_msgr_handle);
  qmi_ip_conf.qmi_ip_qcmap_msgr_handle = NULL;

  if (qmi_error != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Can not release client qcmap client handle %d",
                  qmi_error,0,0);
    return QMI_IP_ERROR;
  }

  return QMI_IP_SUCCESS;
}


/*===========================================================================
  FUNCTION  qcmap_msgr_qmi_qcmap_ind
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
void qmi_ip_qcmap_msgr_ind_cb
(
 qmi_client_type user_handle,                    /* QMI user handle       */
 unsigned int    msg_id,                         /* Indicator message ID  */
 void           *ind_buf,                        /* Raw indication data   */
 unsigned int    ind_buf_len,                    /* Raw data length       */
 void           *ind_cb_data                     /* User call back handle */
 )
{
  qmi_client_error_type qmi_error;

  LOG_MSG_INFO1("qcmap_msgr_qmi_qcmap_ind: user_handle %X msg_id %d ind_buf_len %d.",
      user_handle, msg_id, ind_buf_len);

  switch (msg_id)
  {
    case QMI_QCMAP_MSGR_BRING_UP_WWAN_IND_V01:
    {
      qcmap_msgr_bring_up_wwan_ind_msg_v01 ind_data;

      qmi_error = qmi_client_message_decode(user_handle,
                                            QMI_IDL_INDICATION,
                                            msg_id,
                                            ind_buf,
                                            ind_buf_len,
                                            &ind_data,
                                            sizeof(qcmap_msgr_bring_up_wwan_ind_msg_v01));
      if (qmi_error != QMI_NO_ERR)
      {
        LOG_MSG_ERROR("qcmap_msgr_qmi_qcmap_ind: qmi_client_message_decode error %d",
            qmi_error,0,0);
        break;
      }


      /* Process packet service status indication for WWAN for QCMAP*/
      if (ind_data.conn_status == QCMAP_MSGR_WWAN_STATUS_CONNECTED_V01)
      {
        if (ind_data.mobile_ap_handle == qmi_ip_conf.qmi_ip_mobile_ap_handle)
        {
          LOG_MSG_INFO1("qcmap_msgr_qmi_qcmap_ind: WWAN Connected",0,0,0);
          return;
        }
      }
      else if (ind_data.conn_status == QCMAP_MSGR_WWAN_STATUS_CONNECTING_FAIL_V01)
      {
        if (ind_data.mobile_ap_handle == qmi_ip_conf.qmi_ip_mobile_ap_handle)
        {
          LOG_MSG_INFO1("qcmap_msgr_qmi_qcmap_ind: WWAN Connecting Failed...",0,0,0);
          return;
        }
      }

      break;
    }
    case QMI_QCMAP_MSGR_TEAR_DOWN_WWAN_IND_V01:
    {
      qcmap_msgr_tear_down_wwan_ind_msg_v01 ind_data;

      qmi_error = qmi_client_message_decode(user_handle,
                                            QMI_IDL_INDICATION,
                                            msg_id,
                                            ind_buf,
                                            ind_buf_len,
                                            &ind_data,
                                            sizeof(qcmap_msgr_tear_down_wwan_ind_msg_v01));
      if (qmi_error != QMI_NO_ERR)
      {
        LOG_MSG_ERROR("qcmap_msgr_qmi_qcmap_ind: qmi_client_message_decode error %d",
            qmi_error,0,0);
        break;
      }

      if (ind_data.conn_status == QCMAP_MSGR_WWAN_STATUS_DISCONNECTED_V01)
      {
        if (ind_data.mobile_ap_handle == qmi_ip_conf.qmi_ip_mobile_ap_handle)
        {
          LOG_MSG_INFO1("qcmap_msgr_qmi_qcmap_ind: WWAN Disconnected",0,0,0);
          return;
        }
      }
      else if (ind_data.conn_status == QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_FAIL_V01)
      {
        if (ind_data.mobile_ap_handle == qmi_ip_conf.qmi_ip_mobile_ap_handle)
        {
          LOG_MSG_INFO1("qcmap_msgr_qmi_qcmap_ind: WWAN Disconnecting Failed...",0,0,0);
          return;
        }
      }

      break;
    }
    case QMI_QCMAP_MSGR_WWAN_STATUS_IND_V01:
    {
      qcmap_msgr_wwan_status_ind_msg_v01 ind_data;

      qmi_error = qmi_client_message_decode(user_handle,
                                            QMI_IDL_INDICATION,
                                            msg_id,
                                            ind_buf,
                                            ind_buf_len,
                                            &ind_data,
                                            sizeof(qcmap_msgr_wwan_status_ind_msg_v01));
      if (qmi_error != QMI_NO_ERR)
      {
        LOG_MSG_ERROR("qcmap_msgr_qmi_qcmap_ind: qmi_client_message_decode error %d",
            qmi_error,0,0);
        break;
      }

      if (ind_data.wwan_status == QCMAP_MSGR_WWAN_STATUS_DISCONNECTED_V01)
      {
        LOG_MSG_INFO1("qcmap_msgr_qmi_qcmap_ind: WWAN Disconnected...",0,0,0);
        return;
      }
      else if (ind_data.wwan_status == QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_FAIL_V01)
      {
        LOG_MSG_INFO1("qcmap_msgr_qmi_qcmap_ind: WWAN Disconnecting Failed...",0,0,0);
        return;
      }
      else if (ind_data.wwan_status == QCMAP_MSGR_WWAN_STATUS_CONNECTED_V01)
      {
        LOG_MSG_INFO1("qcmap_msgr_qmi_qcmap_ind: WWAN Connected...",0,0,0);
        return;
      }
      else if (ind_data.wwan_status == QCMAP_MSGR_WWAN_STATUS_CONNECTING_FAIL_V01)
      {
        LOG_MSG_INFO1("qcmap_msgr_qmi_qcmap_ind: WWAN Connecting Failed...",0,0,0);
        return;
      }

      break;
    }
  case QMI_QCMAP_MSGR_MOBILE_AP_STATUS_IND_V01:
    {
      qcmap_msgr_mobile_ap_status_ind_msg_v01 ind_data;

      qmi_error = qmi_client_message_decode(user_handle,
                                            QMI_IDL_INDICATION,
                                            msg_id,
                                            ind_buf,
                                            ind_buf_len,
                                            &ind_data,
                                            sizeof(qcmap_msgr_mobile_ap_status_ind_msg_v01));
      if (qmi_error != QMI_NO_ERR)
      {
        LOG_MSG_ERROR("qcmap_msgr_qmi_qcmap_ind: qmi_client_message_decode error %d",
            qmi_error,0,0);
        break;
      }

      if (ind_data.mobile_ap_status == QCMAP_MSGR_MOBILE_AP_STATUS_CONNECTED_V01)
      {
        LOG_MSG_INFO1("qcmap_msgr_qmi_qcmap_ind: Mobile AP Connected...",0,0,0);
        return;
      }
      else if (ind_data.mobile_ap_status == QCMAP_MSGR_MOBILE_AP_STATUS_DISCONNECTED_V01)
      {
        LOG_MSG_INFO1("qcmap_msgr_qmi_qcmap_ind: Mobile AP Disconnected...",0,0,0);
        return;
      }
      break;
    }

  default:
    break;
}

  return;
}


static int qmi_ip_enable_mobile_ap(void);

/*===========================================================================

FUNCTION enable_mobile_ap()

DESCRIPTION

  This function enables QC Mobile AP
  QMI_IP uses the services of QC Mobile AP

DEPENDENCIES
  None.

RETURN VALUE
  QMI_IP_SUCCESS on success
  QMI_IP_ERROR on failure


SIDE EFFECTS
  None

*==========================================================================*/

static int qmi_ip_enable_mobile_ap(void)
{
  qmi_client_error_type                        qmi_error;
  qmi_client_error_type                        qmi_err_code = QMI_NO_ERR;
  qcmap_msgr_mobile_ap_enable_resp_msg_v01     qcmap_enable_resp_msg_v01;

/*--------------------------------------------------------------------------*/
  memset(&qcmap_enable_resp_msg_v01, 0, sizeof(qcmap_msgr_mobile_ap_enable_resp_msg_v01));
/*---------------------------------------------------------------------------
  Enable Mobile AP
----------------------------------------------------------------------------*/
  qmi_error = qmi_client_send_msg_sync(qmi_ip_conf.qmi_ip_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_MOBILE_AP_ENABLE_REQ_V01,
                                       NULL,
                                       0,
                                       (void*)&qcmap_enable_resp_msg_v01,
                                       sizeof(qcmap_msgr_mobile_ap_enable_resp_msg_v01),
                                       QMI_TIMEOUT);

  if ((qmi_error != QMI_NO_ERR) ||
      (qcmap_enable_resp_msg_v01.resp.result != QMI_NO_ERR) ||
      (qcmap_enable_resp_msg_v01.mobile_ap_handle_valid != TRUE))
  {
    LOG_MSG_ERROR("Can not enable qcmap %d : %d",
        qmi_error, qcmap_enable_resp_msg_v01.resp.error,0);
    return QMI_IP_ERROR;
  }

/*---------------------------------------------------------------------------
  Assign the Mobile AP handle which is used by QMI_IP
----------------------------------------------------------------------------*/
  if (qcmap_enable_resp_msg_v01.mobile_ap_handle > 0)
  {
    qmi_ip_conf.qmi_ip_mobile_ap_handle = qcmap_enable_resp_msg_v01.mobile_ap_handle;
    LOG_MSG_INFO1(" QMI_IP QCMAP Enabled",0,0,0);
    return QMI_IP_SUCCESS;
  }
  else
  {
    LOG_MSG_INFO1("QCMAP Enable Failure",0,0,0);
	return QMI_IP_ERROR;
  }
}

static int qmi_ip_disable_mobile_ap(void);

/*===========================================================================

FUNCTION disable_mobile_ap()

DESCRIPTION

  This function disables QC Mobile AP

DEPENDENCIES
  None.

RETURN VALUE
  QMI_IP_SUCCESS on success
  QMI_IP_ERROR on failure


SIDE EFFECTS
  None

*==========================================================================*/

static int qmi_ip_disable_mobile_ap(void)
{
  qcmap_msgr_mobile_ap_disable_req_msg_v01    qcmap_disable_req_msg_v01;
  qcmap_msgr_mobile_ap_disable_resp_msg_v01   qcmap_disable_resp_msg_v01;
  qmi_client_error_type                       qmi_error;

/*--------------------------------------------------------------------------*/

  memset(&qcmap_disable_req_msg_v01,
         0,
         sizeof(qcmap_msgr_mobile_ap_disable_req_msg_v01));

  memset(&qcmap_disable_resp_msg_v01,
         0,
         sizeof(qcmap_msgr_mobile_ap_disable_resp_msg_v01));

/*--------------------------------------------------------------------------
  Disable mobile AP
---------------------------------------------------------------------------*/
  qcmap_disable_req_msg_v01.mobile_ap_handle = qmi_ip_conf.qmi_ip_mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qmi_ip_conf.qmi_ip_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_MOBILE_AP_DISABLE_REQ_V01,
                                       &qcmap_disable_req_msg_v01,
                                       sizeof(qcmap_msgr_mobile_ap_disable_req_msg_v01),
                                       &qcmap_disable_resp_msg_v01,
                                       sizeof(qcmap_msgr_mobile_ap_disable_resp_msg_v01),
                                       5000);

  LOG_MSG_INFO1("qmi_client_send_msg_sync(disable): error %d result %d",
                qmi_error,
                qcmap_disable_resp_msg_v01.resp.result,
				0);


  if ((qmi_error != QMI_NO_ERR) ||
      (qcmap_disable_resp_msg_v01.resp.result != QMI_NO_ERR))
  {
    LOG_MSG_ERROR( "Can not disable qcmap %d : %d",
        qmi_error, qcmap_disable_resp_msg_v01.resp.error,0);
    return QMI_IP_ERROR;
  }

  LOG_MSG_INFO1( "QCMAP disabled", 0, 0, 0);


  return QMI_IP_SUCCESS;
}

/*===========================================================================

FUNCTION QMI_IP_QUEUE_EVENT()

DESCRIPTION

  This function queues the link event in the cmdq

DEPENDENCIES
  None.

RETURN VALUE
  QMI_IP_SUCCESS on success
  QMI_IP_ERROR on failure


SIDE EFFECTS
  None

/*=========================================================================*/
int qmi_ip_queue_event
(
  qmi_ip_event_e    event,
  int               mode
)
{
  int res;
  qmi_ip_cmdq_cmd_t *cmd_buf = NULL;

  cmd_buf = qmi_ip_cmdq_get_cmd();
  if (cmd_buf == NULL)
    return QMI_IP_ERROR;

  cmd_buf->data.event = event;
  cmd_buf->data.mode = mode;

  res = qmi_ip_cmdq_put_cmd (cmd_buf);

  return res;
}

/*===========================================================================

FUNCTION QMI_IP_DTR_HIGH()

DESCRIPTION

  This function queues the link event in the cmdq

DEPENDENCIES
  None.

RETURN VALUE
  QMI_IP_SUCCESS on success
  QMI_IP_ERROR on failure


SIDE EFFECTS
  None

/*=========================================================================*/
int qmi_ip_dtr_high(){
  int             fd = 0;
  int             dtr_sig;
  int             ret = QMI_IP_ERROR;
/*------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
  Open USB tethered SMD channel port
-------------------------------------------------------------------------*/
  fd = open(USB_TETHERED_SMD_CH, O_RDWR);
  if (fd < 0)
  {
    LOG_MSG_ERROR("QMI_IP :Opening the device file failed errno ",
                   0, 0, 0);
    return ret;
  }

  LOG_MSG_INFO1("QMI_IP :Request to set DTR HIGH", 0, 0, 0);

/*-------------------------------------------------------------------------
  Set DTR low
-------------------------------------------------------------------------*/
  dtr_sig = 0;
  dtr_sig |= ioctl(fd, TIOCMGET, &dtr_sig);
  if( dtr_sig >= 0  && !(dtr_sig & TIOCM_DTR))
  {
    LOG_MSG_INFO1("QMI_IP :Setting DTR bit...: ", 0, 0, 0);

    dtr_sig |= TIOCM_DTR;
    if((ioctl(fd, TIOCMSET, (void *)dtr_sig)) == -1)
    {
      LOG_MSG_ERROR("QMI_IP :Ioctl call to set DTR bit failed errno %d", errno, 0, 0);
    }
    else
    {
      dtr_sig = 0;
      dtr_sig |= ioctl(fd, TIOCMGET, &dtr_sig);
      if( dtr_sig >= 0  && (dtr_sig & TIOCM_DTR))
      {
        LOG_MSG_INFO1("QMI_IP :DTR bit set:", 0, 0, 0);
        ret = QMI_IP_SUCCESS;
      }
      else
      {
        LOG_MSG_ERROR("QMI_IP :Unable to set DTR bit", 0, 0, 0);
      }
    }
  }
  else if (dtr_sig == -1)
    LOG_MSG_ERROR("QMI_IP :Failed to set DTR bits...failed errno %d", errno, 0, 0);

  if(fd > 0)
  {
    close(fd);
    fd = 0;
  }

  return ret;
}

/*===========================================================================

FUNCTION QMI_IP_DTR_LOW()

DESCRIPTION

  This function queues the link event in the cmdq

DEPENDENCIES
  None.

RETURN VALUE
  QMI_IP_SUCCESS on success
  QMI_IP_ERROR on failure


SIDE EFFECTS
  None

/*=========================================================================*/
int qmi_ip_dtr_low(){
  int             fd = 0;
  int             dtr_sig;
  int             ret = QMI_IP_ERROR;
/*------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
  Open USB tethered SMD channel port
-------------------------------------------------------------------------*/
  fd = open(USB_TETHERED_SMD_CH, O_RDWR);
  if (fd < 0)
  {
    LOG_MSG_ERROR("QMI_IP :Opening the device file failed errno ",
                   0, 0, 0);
    return ret;
  }

  LOG_MSG_INFO1("QMI_IP :Request to set DTR LOW", 0, 0, 0);

/*-------------------------------------------------------------------------
  Set DTR low
-------------------------------------------------------------------------*/
  dtr_sig = 0;
  dtr_sig |= ioctl(fd, TIOCMGET, &dtr_sig);
  if( dtr_sig >= 0  && (dtr_sig & TIOCM_DTR))
  {
    LOG_MSG_INFO1("QMI_IP :Clearing DTR bit...: ", 0, 0, 0);

    dtr_sig &= (~TIOCM_DTR);
    if((ioctl(fd, TIOCMSET, (void *)dtr_sig)) == -1)
    {
      LOG_MSG_ERROR("QMI_IP :Ioctl call to clear DTR bit failed errno %d", errno, 0, 0);
    }
    else
    {
      dtr_sig = 0;
      dtr_sig |= ioctl(fd, TIOCMGET, &dtr_sig);
      if( dtr_sig >= 0  && !(dtr_sig & TIOCM_DTR))
      {
        LOG_MSG_INFO1("QMI_IP :DTR bit clear:", 0, 0, 0);
        ret = QMI_IP_SUCCESS;
      }
      else
      {
        LOG_MSG_ERROR("QMI_IP :Unable to clear DTR bit", 0, 0, 0);
      }
    }
  }
  else if (dtr_sig == -1)
    LOG_MSG_ERROR("QMI_IP :Failed to get DTR bits...failed errno %d", errno, 0, 0);

  if(fd > 0)
  {
    close(fd);
    fd = 0;
  }

  return ret;
}


/*===========================================================================

FUNCTION QMI_IP_PROCESS_LINK_EVENT()

DESCRIPTION

  This function performs the execution of commands present in command queue.
  It mainly is involved in sending required QCMAP messages to QCMAP daemon to
  perform QCMAP specific operations

DEPENDENCIES
  None.

RETURN VALUE
  QMI_IP_SUCCESS on success
  QMI_IP_ERROR on failure


SIDE EFFECTS
  None

/*=========================================================================*/

int qmi_ip_process_link_event
(
  qmi_ip_event_e    event,
  int               mode
)
{
  int                ret;
  int                qmi_err;
/*------------------------------------------------------------------------*/

  /*-------------------------------------------------------------------------
    Handle different events and perform QCMAP specific operations
  --------------------------------------------------------------------------*/
  LOG_MSG_INFO1("Process link event in mode %d.", device_mode, 0, 0);

  switch(event)
  {
  case QMI_IP_LINK_UP_EVENT:
    {
      LOG_MSG_INFO1("Received LINK_UP event.", 0, 0, 0);
      switch (mode) {
      case BRIDGE_MODE:
        {
          if (multicast_server_finished)
          {
            pthread_t           thread;
            pthread_create(&thread, NULL, (void (*)(void))multicast_listener, NULL);
          }

          ds_system_call("ifconfig odu0 promisc", strlen("ifconfig odu0 promisc"));

          if(qmi_ip_dtr_high() != QMI_IP_ERROR){
            LOG_MSG_INFO1("Successfully set DTR HIGH",0,0,0);
          } else
              LOG_MSG_ERROR("Unable to set DTR HIGH",0,0,0);
          break;
        }
      case ROUTER_MODE:
        {
        /*--------------------------------------------------------------------------
          Avoid link up down twice
        ---------------------------------------------------------------------------*/
          if(qmi_ip_conf.state == QMI_IP_LINK_DOWN_WAIT)
          {
            LOG_MSG_INFO1("Received duplicate LINK_UP event.Ignoring\n", 0, 0, 0);
            break;
          }

          /*--------------------------------------------------------------------------
            Enable QC Mobile AP
          ---------------------------------------------------------------------------*/
          ret = qmi_ip_enable_mobile_ap();
          if (ret == QMI_IP_SUCCESS)
          {
            LOG_MSG_INFO1("Mobile AP enable: successful\n", 0, 0, 0);
          }
          else
          {
            LOG_MSG_ERROR("Mobile AP enable: unsuccessful. Aborting\n", 0, 0, 0);
            return QMI_IP_ERROR;
          }

          qmi_ip_conf.state = QMI_IP_LINK_DOWN_WAIT;

          /*--------------------------------------------------------------------------
            Setup ODU link
          ---------------------------------------------------------------------------*/
          LOG_MSG_INFO1("DS System Calls\n", 0,0,0);
          //Assign a pre-defined IP address
          //GUEST_AP_IFACE_LL_ADDR
          ds_system_call( "ifconfig odu0 169.254.2.1 netmask 255.255.255.0", strlen("ifconfig odu0 169.254.2.1 netmask 255.255.255.0"));

          //Delete the subnet based route to USB interface
          ds_system_call("route del -net 169.254.3.0 netmask 255.255.255.0 dev odu0", strlen("route del -net 169.254.3.0 netmask 255.255.255.0 dev odu0"));

          /* First delete the link-local route. */
          ds_system_call("ip -6 route del fe80::/64 dev odu0", strlen("ip -6 route del fe80::/64 dev odu0"));

          ds_system_call("brctl addif bridge0 odu0", strlen("brctl addif bridge0 odu0"));

          /*------------------------------------------------------------------------
          Enable IP forwarding
          ------------------------------------------------------------------------*/
          ds_system_call("echo 1 > /proc/sys/net/ipv4/ip_forward",
                 strlen("echo 1 > /proc/sys/net/ipv4/ip_forward"));

          /*--------------------------------------------------------------------------
            Write to dmesg log. It will help in debugging customer issues quickly.
            But we need to make sure we dont write too many messages to dmesg.
          ---------------------------------------------------------------------------*/
          ds_system_call("echo QMI_IP:LINK_DOWN_WAIT state > /dev/kmsg",
                         strlen("echo QMI_IP:LINK_DOWN_WAIT state > /dev/kmsg"));

          break;
        }
      }
      break;
    }
    /*-------------------------------------------------------------------------
      Processes netlink down event which happens upon USB cable plug out
      - Here we disconnect backhaul
      - Disable QC Mobile AP
    --------------------------------------------------------------------------*/
    case QMI_IP_LINK_DOWN_EVENT:
    {
      LOG_MSG_INFO1("Received LINK_DOWN event.", 0, 0, 0);

      ssl_active = 0;

      switch (mode) {
      case BRIDGE_MODE:
        /* Set DTR to LOW */

        if(qmi_ip_dtr_low() != QMI_IP_ERROR){
            LOG_MSG_INFO1("Successfully set DTR LOW",0,0,0);
        } else
            LOG_MSG_ERROR("Unable to set DTR LOW",0,0,0);
        break;
      case ROUTER_MODE:
      /*--------------------------------------------------------------------------
        Avoid link bring down twice
      ---------------------------------------------------------------------------*/
        if(qmi_ip_conf.state == QMI_IP_LINK_UP_WAIT)
        {
          LOG_MSG_INFO1("Received duplicate LINK_DOWN event.Ignoring", 0, 0, 0);
          break;
        }

        /* Disable IP forwarding */
        ds_system_call("echo 0 > /proc/sys/net/ipv4/ip_forward",
                 strlen("echo 0 > /proc/sys/net/ipv4/ip_forward"));

        /* Delete bridge */
        ds_system_call("brctl delif bridge0 odu0", strlen("brctl delif bridge0 odu0"));

        /* Remove IP address from iface*/
        ds_system_call( "ip address flush dev odu0", strlen("ip address flush dev odu0"));

        /*--------------------------------------------------------------------------
          Disable QC Mobile AP
        ---------------------------------------------------------------------------*/
        ret = qmi_ip_disable_mobile_ap();
        if (ret == QMI_IP_SUCCESS)
        {
          LOG_MSG_INFO1(" Mobile AP disable: successful", 0, 0, 0);
        }
        else
        {
          LOG_MSG_ERROR(" Mobile AP disable: unsuccessful. Aborting", 0, 0, 0);
          return QMI_IP_ERROR;
        }

        qmi_ip_conf.state = QMI_IP_LINK_UP_WAIT;

        /*--------------------------------------------------------------------------
          Write to dmesg log. It will help in debugging customer issues quickly.
          But we need to make sure we dont write too many messages to dmesg.
        ---------------------------------------------------------------------------*/
        ds_system_call("echo QMI_IP:LINK_UP_WAIT state > /dev/kmsg",
                       strlen("echo QMI_IP:LINK_UP_WAIT state > /dev/kmsg"));
        break;
      }
      break;
    }

    default:
      LOG_MSG_INFO1("Ignoring event %d received",event,0,0);
      break;
  }

  return QMI_IP_SUCCESS;
}

