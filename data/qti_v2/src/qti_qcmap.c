/*=============================================================================

FILE:  qti_qcmap.c

SERVICES: Implementation of Tethering Interface module interface with QCMAP
===============================================================================

Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

=============================================================================*/

/*=============================================================================
  EDIT HISTORY FOR MODULE

  Please notice that the changes are listed in reverse chronological order.

  when       who        what, where, why
  --------   ---        ------------------------------------------------------
  11/13/12   sb         Created module.

=============================================================================*/


/*=============================================================================
                               INCLUDE FILES
==============================================================================*/
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
#include "qualcomm_mobile_access_point_msgr_v01.h"
#include "qti_cmdq.h"
#include "qmi_client_instance_defs.h"

/*=============================================================================
                                MACRO DEFINITIONS
==============================================================================*/


/*=============================================================================
                                VARIABLE DEFINITIONS
==============================================================================*/
static qti_conf_t * qti_qcmap_conf;

/*=============================================================================
                                FUNCTION FORWARD DECLARATION
==============================================================================*/
void qti_qcmap_msgr_ind_cb
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

/*=============================================================================
  FUNCTION QTI_QCMAP_INIT()

  DESCRIPTION

  This function will initiate for Qcmap client connection.

  DEPENDENCIES
  None.

  RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

  SIDE EFFECTS
  None
==============================================================================*/
int qti_qcmap_init(qti_conf_t * qti_conf )
{
  qti_cmdq_cmd_t            * cmd_buf = NULL;

  ds_assert(qti_conf != NULL);

  LOG_MSG_INFO1("qti_qcmap_init()", 0, 0, 0);

  qti_qcmap_conf = qti_conf;

  cmd_buf = qti_cmdq_get_cmd();
  if(cmd_buf == NULL)
  {
    LOG_MSG_ERROR("qti_cmdq: failed to allocate memeory for cmd", 0, 0, 0);
    return QTI_FAILURE;
  }

/*-----------------------------------------------------------------------------
  Post a command queue message to initialize QCMAP client connection
------------------------------------------------------------------------------*/
  cmd_buf->data.event = QTI_QCMAP_INIT_EVENT;
  if( QTI_CMDQ_SUCCESS != qti_cmdq_put_cmd( cmd_buf ) )
  {
    qti_cmdq_release_cmd(cmd_buf);
    LOG_MSG_ERROR("qti_cmdq: failed to put commmand",0,0,0);
    return QTI_FAILURE;
  }

  return QTI_SUCCESS;
}

/*=============================================================================
  FUNCTION QTI_QCMAP_CONNECT

  DESCRIPTION

  This function initializes QTI interface to QCMAP

  DEPENDENCIES
  None.

  RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

  SIDE EFFECTS
  None
==============================================================================*/
int qti_qcmap_connect(void)
{

  qmi_idl_service_object_type                            qti_qcmap_msgr_service_object;
  qmi_client_error_type                                  qmi_error, qmi_err_code = QMI_NO_ERR;
  qcmap_msgr_mobile_ap_status_ind_register_req_msg_v01   qcmap_mobile_ap_status_ind_reg;
  qcmap_msgr_mobile_ap_status_ind_register_resp_msg_v01  qcmap_mobile_ap_status_ind_rsp;
  qcmap_msgr_wwan_status_ind_register_req_msg_v01        wwan_status_ind_reg;
  qcmap_msgr_wwan_status_ind_register_resp_msg_v01       wwan_status_ind_rsp;
  qmi_client_os_params                                   qti_qcmap_msgr_os_params;
/*---------------------------------------------------------------------------*/

  LOG_MSG_INFO1("qti_qcmap_connect()", 0, 0, 0);
/*-----------------------------------------------------------------------------
  Deregister QMUXD ports since we want the client init request to go over IPC
  router. The way QCCI works is that it tries to find the required service over
  both QMUXD and IPC router. But then QMUXD waits for modem to come up thus
  delaying the overall client init request. Since we know that QCMAP_MSGR is
  over IPC router and we want to avoid the delay so we deregister here.
------------------------------------------------------------------------------*/
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0);
/*-----------------------------------------------------------------------------
  Obtain a QCMAP messenger service client for QTI
  - get the service object
  - obtain the client
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Get the service object
------------------------------------------------------------------------------*/
  qti_qcmap_msgr_service_object = qcmap_msgr_get_service_object_v01();
  if (qti_qcmap_msgr_service_object == NULL)
  {
    LOG_MSG_ERROR("QTI QCMAP messenger service object not available",
                   0, 0, 0);
    return QTI_FAILURE;
  }

  memset(&qti_qcmap_msgr_os_params, 0, sizeof(qmi_client_os_params));
/*-----------------------------------------------------------------------------
  Client init
------------------------------------------------------------------------------*/

  qmi_error = qmi_client_init_instance(qti_qcmap_msgr_service_object,
                                       QMI_CLIENT_INSTANCE_ANY,
                                       qti_qcmap_msgr_ind_cb,
                                       NULL,
                                       &qti_qcmap_msgr_os_params,
                                       QTI_QMI_MSG_TIMEOUT_VALUE,
                                       &(qti_qcmap_conf->qti_qcmap_msgr_handle));

  if (qmi_error != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Can not init QCMAP MSGR client %d", qmi_error, 0, 0);
    return QTI_FAILURE;
  }

/*-----------------------------------------------------------------------------
  Register for WWAN indications from QCMAP
-----------------------------------------------------------------------------*/
  memset(&wwan_status_ind_reg,0,
         sizeof(qcmap_msgr_wwan_status_ind_register_req_msg_v01));

  memset(&wwan_status_ind_rsp,0,
         sizeof(qcmap_msgr_wwan_status_ind_register_resp_msg_v01));

  wwan_status_ind_reg.register_indication = 1;
  qmi_error = qmi_client_send_msg_sync(qti_qcmap_conf->qti_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_WWAN_STATUS_IND_REG_REQ_V01,
                                       (void*)&wwan_status_ind_reg,
                                       sizeof(qcmap_msgr_wwan_status_ind_register_req_msg_v01),
                                       (void*)&wwan_status_ind_rsp,
                                       sizeof(qcmap_msgr_wwan_status_ind_register_resp_msg_v01),
                                       QTI_QMI_MSG_TIMEOUT_VALUE);

  LOG_MSG_INFO1("qmi_client_send_msg_sync(enable): error %d result %d",
      qmi_error, wwan_status_ind_rsp.resp.result,0);

  if ((qmi_error != QMI_NO_ERR) ||
      (wwan_status_ind_rsp.resp.result != QMI_NO_ERR))
  {
    LOG_MSG_ERROR("Can not register for wwan status %d : %d",
        qmi_error, wwan_status_ind_rsp.resp.error,0);
    return QTI_FAILURE;
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
  qmi_error = qmi_client_send_msg_sync(qti_qcmap_conf->qti_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_MOBILE_AP_STATUS_IND_REG_REQ_V01,
                                       (void*)&qcmap_mobile_ap_status_ind_reg,
                                       sizeof(qcmap_msgr_mobile_ap_status_ind_register_req_msg_v01),
                                       (void*)&qcmap_mobile_ap_status_ind_rsp,
                                       sizeof(qcmap_msgr_mobile_ap_status_ind_register_resp_msg_v01),
                                       QTI_QMI_MSG_TIMEOUT_VALUE);

  LOG_MSG_INFO1("qmi_client_send_msg_sync(enable): error %d result %d",
      qmi_error, qcmap_mobile_ap_status_ind_rsp.resp.result, 0);

  if ((qmi_error != QMI_NO_ERR) ||
      (qcmap_mobile_ap_status_ind_rsp.resp.result != QMI_NO_ERR))
  {
    LOG_MSG_ERROR("Can not register for mobile ap status %d : %d",
                  qmi_error,
                  qcmap_mobile_ap_status_ind_rsp.resp.error, 0);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1("Done registering for mobile ap status",0,0,0);

  return QTI_SUCCESS;
}

/*=============================================================================
  FUNCTION QTI_QCMAP_EXIT()

  DESCRIPTION

  This function releases QCMAP client

  DEPENDENCIES
  None.

  RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

  SIDE EFFECTS
  None
==============================================================================*/
 int qti_qcmap_exit()
{
  qmi_client_error_type     qmi_error;

/*----------------------------------------------------------------------------*/

  qmi_error = qmi_client_release(qti_qcmap_conf->qti_qcmap_msgr_handle);
  qti_qcmap_conf->qti_qcmap_msgr_handle = NULL;

  if (qmi_error != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Can not release client qcmap client handle %d",
                  qmi_error,0,0);
    return QTI_FAILURE;
  }

  return QTI_SUCCESS;
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
void qti_qcmap_msgr_ind_cb
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
        if (ind_data.mobile_ap_handle == qti_qcmap_conf->qti_mobile_ap_handle)
        {
          LOG_MSG_INFO1("qcmap_msgr_qmi_qcmap_ind: WWAN Connected",0,0,0);
          return;
        }
      }
      else if (ind_data.conn_status == QCMAP_MSGR_WWAN_STATUS_CONNECTING_FAIL_V01)
      {
        if (ind_data.mobile_ap_handle == qti_qcmap_conf->qti_mobile_ap_handle)
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
        if (ind_data.mobile_ap_handle == qti_qcmap_conf->qti_mobile_ap_handle)
        {
          LOG_MSG_INFO1("qcmap_msgr_qmi_qcmap_ind: WWAN Disconnected",0,0,0);
          return;
        }
      }
      else if (ind_data.conn_status == QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_FAIL_V01)
      {
        if (ind_data.mobile_ap_handle == qti_qcmap_conf->qti_mobile_ap_handle)
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



/*===========================================================================

FUNCTION enable_mobile_ap()

DESCRIPTION

  This function enables QC Mobile AP
  QTI uses the services of QC Mobile AP

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/

static int qti_enable_mobile_ap()
{
  qmi_client_error_type                        qmi_error;
  qmi_client_error_type                        qmi_err_code = QMI_NO_ERR;
  qcmap_msgr_mobile_ap_enable_resp_msg_v01     qcmap_enable_resp_msg_v01;
  qmi_cci_os_signal_type                       qti_qcmap_msgr_os_params;
  int                                          retry_count=0;

/*---------------------------------------------------------------------------
  we will retry 10 times in a span of 10 seconds incase mobileap failes
----------------------------------------------------------------------------*/
  while( retry_count < QTI_MAX_MOBILEAP_RETRY )
  {
    memset(&qcmap_enable_resp_msg_v01, 0,sizeof(qcmap_msgr_mobile_ap_enable_resp_msg_v01));

/*---------------------------------------------------------------------------
  Enable Mobile AP
----------------------------------------------------------------------------*/
    qmi_error = qmi_client_send_msg_sync( qti_qcmap_conf->qti_qcmap_msgr_handle,
                                          QMI_QCMAP_MSGR_MOBILE_AP_ENABLE_REQ_V01,
                                          NULL,
                                          0,
                                          (void*)&qcmap_enable_resp_msg_v01,
                                          sizeof(qcmap_msgr_mobile_ap_enable_resp_msg_v01),
                                          QTI_QMI_MSG_TIMEOUT_VALUE );
    LOG_MSG_INFO1("qmi_client_send_msg_sync(enable): error %d result %d valid %d",
                   qmi_error,
                   qcmap_enable_resp_msg_v01.resp.result,
                   qcmap_enable_resp_msg_v01.mobile_ap_handle_valid );

    if ((qmi_error != QMI_NO_ERR) ||
        (qcmap_enable_resp_msg_v01.resp.result != QMI_NO_ERR) ||
        (qcmap_enable_resp_msg_v01.mobile_ap_handle_valid != TRUE))
    {
        LOG_MSG_ERROR("Can not enable qcmap %d : %d",
                       qmi_error, qcmap_enable_resp_msg_v01.resp.error,0);
        sleep(ONE_SEC);
        retry_count++;

/*---------------------------------------------------------------------------
  If there is no messages in the command queue only then we try for mobileap enabe,
  this because we need to honour messages from netlink
----------------------------------------------------------------------------*/
        if( qti_get_cmdq_length() == 0)
        {
          LOG_MSG_INFO1("Mobile ap retry in progress", 0, 0, 0);
          continue;
        }
    }
    break;
  }

  if( retry_count == QTI_MAX_MOBILEAP_RETRY)
  {
    LOG_MSG_ERROR("Enable Mobileap retry failed",0,0,0);
    return QTI_FAILURE;
  }

/*---------------------------------------------------------------------------
  Assign the Mobile AP handle which is used by QTI
----------------------------------------------------------------------------*/
  if (qcmap_enable_resp_msg_v01.mobile_ap_handle > 0)
  {
    qti_qcmap_conf->qti_mobile_ap_handle = qcmap_enable_resp_msg_v01.mobile_ap_handle;
    LOG_MSG_INFO1(" QTI QCMAP Enabled",0,0,0);
    return QTI_SUCCESS;
  }
  else
  {
    LOG_MSG_INFO1("QCMAP Enable Failure",0,0,0);
    return QTI_FAILURE;
  }
}

/*===========================================================================

FUNCTION disable_mobile_ap()

DESCRIPTION

  This function disables QC Mobile AP

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/

static int qti_disable_mobile_ap()
{
  qcmap_msgr_mobile_ap_disable_req_msg_v01    qcmap_disable_req_msg_v01;
  qcmap_msgr_mobile_ap_disable_resp_msg_v01   qcmap_disable_resp_msg_v01;
  qmi_client_error_type                       qmi_error;
  int                                         retry_count=0;

/*---------------------------------------------------------------------------
  we will retry 10 times in a span of 10 seconds incase mobileap disable failes
----------------------------------------------------------------------------*/
  while( retry_count < QTI_MAX_MOBILEAP_RETRY )
  {
    memset(&qcmap_disable_req_msg_v01,
           0,
           sizeof(qcmap_msgr_mobile_ap_disable_req_msg_v01));

    memset(&qcmap_disable_resp_msg_v01,
           0,
          sizeof(qcmap_msgr_mobile_ap_disable_resp_msg_v01));

    // TO DO : check the status of mobile ap
/*--------------------------------------------------------------------------
  Disable mobile AP
---------------------------------------------------------------------------*/
    qcmap_disable_req_msg_v01.mobile_ap_handle = qti_qcmap_conf->qti_mobile_ap_handle;

    qmi_error = qmi_client_send_msg_sync(qti_qcmap_conf->qti_qcmap_msgr_handle,
                                         QMI_QCMAP_MSGR_MOBILE_AP_DISABLE_REQ_V01,
                                         &qcmap_disable_req_msg_v01,
                                         sizeof(qcmap_msgr_mobile_ap_disable_req_msg_v01),
                                         &qcmap_disable_resp_msg_v01,
                                         sizeof(qcmap_msgr_mobile_ap_disable_resp_msg_v01),
                                         QTI_QMI_MSG_TIMEOUT_VALUE);

    LOG_MSG_INFO1("qmi_client_send_msg_sync(disable): error %d result %d",
                  qmi_error,
                  qcmap_disable_resp_msg_v01.resp.result,
                  0);


    if ((qmi_error != QMI_NO_ERR) ||
        (qcmap_disable_resp_msg_v01.resp.result != QMI_NO_ERR))
    {
      LOG_MSG_ERROR( "Can not disable qcmap %d : %d",
                     qmi_error, qcmap_disable_resp_msg_v01.resp.error, 0 );
      retry_count++;
      sleep(ONE_SEC);
      continue;
    }
    break;
  }

  if( retry_count == QTI_MAX_MOBILEAP_RETRY )
  {
    LOG_MSG_ERROR("Disable Mobileap retry failed", 0, 0, 0);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1( "QCMAP disabled", 0, 0, 0);


  return QTI_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_USB_LINK_UP()

DESCRIPTION

  This function sends a message to QCMAP setup the USB link for RNDIS/ECM
  tethering

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
static int qti_usb_link_up
(
  qti_interface_e interface
)
{
  qcmap_msgr_usb_link_up_req_msg_v01   qcmap_qti_usb_link_up_req_msg;
  qcmap_msgr_usb_link_up_resp_msg_v01  qcmap_qti_usb_link_up_resp_msg;
  qmi_client_error_type                qmi_error;
/*--------------------------------------------------------------------------*/

  LOG_MSG_INFO1("Setup USB link start", 0, 0, 0);

  memset(&qcmap_qti_usb_link_up_req_msg,
         0,
         sizeof(qcmap_msgr_usb_link_up_req_msg_v01));

  memset(&qcmap_qti_usb_link_up_resp_msg,
         0,
         sizeof(qcmap_msgr_usb_link_up_resp_msg_v01));

/*-------------------------------------------------------------------------
  Setup USB link - RNDIS/ECM
---------------------------------------------------------------------------*/
  qcmap_qti_usb_link_up_req_msg.mobile_ap_handle =
                                         qti_qcmap_conf->qti_mobile_ap_handle;


  qcmap_qti_usb_link_up_req_msg.usb_link = interface;

  qmi_error = qmi_client_send_msg_sync(qti_qcmap_conf->qti_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_USB_LINK_UP_REQ_V01,
                                       &qcmap_qti_usb_link_up_req_msg,
                                       sizeof(qcmap_msgr_usb_link_up_req_msg_v01),
                                       &qcmap_qti_usb_link_up_resp_msg,
                                       sizeof(qcmap_msgr_usb_link_up_resp_msg_v01),
                                       QTI_QMI_MSG_TIMEOUT_VALUE);

  LOG_MSG_INFO1("Setup USB link. Error values: %d , %d",
        qmi_error, qcmap_qti_usb_link_up_resp_msg.resp.error, 0);

  if (( qmi_error != QMI_TIMEOUT_ERR ) && ((qmi_error != QMI_NO_ERR) ||
        (qcmap_qti_usb_link_up_resp_msg.resp.result != QMI_NO_ERR)))
  {
    LOG_MSG_ERROR("Setup USB link failed. Error values: %d , %d",
        qmi_error, qcmap_qti_usb_link_up_resp_msg.resp.error,0);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1("Setup USB link succeeded", 0, 0, 0);

  return QTI_SUCCESS;
}


/*===========================================================================

FUNCTION QTI_USB_LINK_DOWN()

DESCRIPTION

  This function sends a message to QCMAP to bring down the RNDIS/ECM USB link

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

==========================================================================*/
static int qti_usb_link_down
(
  qti_interface_e interface
)
{
  qcmap_msgr_usb_link_down_req_msg_v01   qcmap_qti_usb_link_down_req_msg;
  qcmap_msgr_usb_link_down_resp_msg_v01  qcmap_qti_usb_link_down_resp_msg;
  qmi_client_error_type                  qmi_error;
/*--------------------------------------------------------------------------*/

  LOG_MSG_INFO1("Tear down USB link start", 0, 0, 0);

  memset(&qcmap_qti_usb_link_down_req_msg,
         0,
         sizeof(qcmap_msgr_usb_link_down_req_msg_v01));

  memset(&qcmap_qti_usb_link_down_resp_msg,
         0,
         sizeof(qcmap_msgr_usb_link_down_resp_msg_v01));

/*-------------------------------------------------------------------------
  Setup USB link - RNDIS/ECM
---------------------------------------------------------------------------*/
  qcmap_qti_usb_link_down_req_msg.mobile_ap_handle =
                                         qti_qcmap_conf->qti_mobile_ap_handle;

  qcmap_qti_usb_link_down_req_msg.usb_link = interface;

  qmi_error = qmi_client_send_msg_sync(qti_qcmap_conf->qti_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_USB_LINK_DOWN_REQ_V01,
                                       &qcmap_qti_usb_link_down_req_msg,
                                       sizeof(qcmap_msgr_usb_link_down_req_msg_v01),
                                       &qcmap_qti_usb_link_down_resp_msg,
                                       sizeof(qcmap_msgr_usb_link_down_resp_msg_v01),
                                       QTI_QMI_MSG_TIMEOUT_VALUE);

  if (( qmi_error != QMI_TIMEOUT_ERR ) && ((qmi_error != QMI_NO_ERR) ||
        (qcmap_qti_usb_link_down_resp_msg.resp.result != QMI_NO_ERR)))
  {
    LOG_MSG_ERROR("Bring down USB link failed. Error values: %d , %d",
        qmi_error, qcmap_qti_usb_link_down_resp_msg.resp.error,0);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1("Bring down USB link succeeded", 0, 0, 0);

  return QTI_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_QMI_CMD_EXEC()

DESCRIPTION

  This function performs the execution of commands present in command queue.
  It mainly is involved in sending required QCMAP messages to QCMAP daemon to
  perform QCMAP specific operations

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/

int qti_qcmap_cmd_exec
(
  qti_event_e    event,
  qti_interface_e      interface
)
{
  int                ret_val;
/*------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
  Handle different events and perform QCMAP specific operations
--------------------------------------------------------------------------*/
  switch(event)
  {
/*-------------------------------------------------------------------------
  This event is processed when we get netlink up event upon USB cable
  plug in.
  - Here we enable QC Mobile AP
  - Enable RNDIS/ECM tethering
  - Connect backhaul
---------------------------------------------------------------------------*/
    case QTI_LINK_UP_EVENT:
    {
/*--------------------------------------------------------------------------
  Avoid link up down twice
---------------------------------------------------------------------------*/
      if(qti_qcmap_conf->state == QTI_LINK_DOWN_WAIT)
      {
        LOG_MSG_INFO1("Received duplicate LINK_UP event.Ignoring", 0, 0, 0);
        break;
      }

      LOG_MSG_INFO1("Got USB link up event", 0, 0, 0);
/*--------------------------------------------------------------------------
  Enable QC Mobile AP
---------------------------------------------------------------------------*/
      ds_system_call("echo QTI:Enable mobileap > /dev/kmsg",\
                     strlen("echo QTI:Enable mobileap > /dev/kmsg"));

      ret_val = qti_enable_mobile_ap();
      if (ret_val==QTI_SUCCESS)
      {
        LOG_MSG_INFO1(" Mobile AP enable: successful", 0, 0, 0);
      }
      else
      {
        LOG_MSG_ERROR(" Mobile AP enable: unsuccessful. Aborting", 0, 0, 0);
        return QTI_FAILURE;
      }

/*--------------------------------------------------------------------------
  Setup RNDIS/ECM tethering
---------------------------------------------------------------------------*/
      ret_val = qti_usb_link_up(interface);
      if (ret_val==QTI_SUCCESS)
      {
        LOG_MSG_INFO1(" Setup USB tethering: successful. Interface = %d",
                            interface, 0, 0);
      }
      else
      {
        LOG_MSG_ERROR(" Setup USB tethering: unsuccessful. Interface = %d",
                            interface, 0, 0);
        return QTI_FAILURE;
      }

      qti_qcmap_conf->state=QTI_LINK_DOWN_WAIT;

/*--------------------------------------------------------------------------
  Write to dmesg log. It will help in debugging customer issues quickly.
  But we need to make sure we dont write too many messages to dmesg.
---------------------------------------------------------------------------*/
      ds_system_call("echo QTI:LINK_DOWN_WAIT state > /dev/kmsg",
                     strlen("echo QTI:LINK_DOWN_WAIT state > /dev/kmsg"));
      break;
    }
/*-------------------------------------------------------------------------
  Processes netlink down event which happens upon USB cable plug out
  - Disable RNDIS/ECM tethering
  - Here we disconnect backhaul
  - Disable QC Mobile AP
--------------------------------------------------------------------------*/
    case QTI_LINK_DOWN_EVENT:
    {
/*--------------------------------------------------------------------------
  Avoid link bring down twice
---------------------------------------------------------------------------*/
      if(qti_qcmap_conf->state == QTI_LINK_UP_WAIT)
      {
        LOG_MSG_INFO1("Received duplicate LINK_DOWN event.Ignoring", 0, 0, 0);
        break;
      }

      LOG_MSG_INFO1(" Got USB link down event", 0, 0, 0);
/*--------------------------------------------------------------------------
  Bring down RNDIS/ECM tethering
---------------------------------------------------------------------------*/
      ret_val = qti_usb_link_down(interface);
      if (ret_val==QTI_SUCCESS)
      {
        LOG_MSG_INFO1(" Bring down USB tethering: successful. Interface = %d",
                            interface, 0, 0);
      }
      else
      {
        LOG_MSG_ERROR(" Bring down USB tethering: unsuccessful. Interface = %d",
                            interface, 0, 0);
      }

/*--------------------------------------------------------------------------
  Disable QC Mobile AP
---------------------------------------------------------------------------*/
      ret_val = qti_disable_mobile_ap();
      if (ret_val==QTI_SUCCESS)
      {
        LOG_MSG_INFO1(" Mobile AP disable: successful", 0, 0, 0);
      }
      else
      {
        LOG_MSG_ERROR(" Mobile AP disable: unsuccessful. Aborting", 0, 0, 0);
        return QTI_FAILURE;
      }

      qti_qcmap_conf->state = QTI_LINK_UP_WAIT;

/*--------------------------------------------------------------------------
  Write to dmesg log. It will help in debugging customer issues quickly.
  But we need to make sure we dont write too many messages to dmesg.
---------------------------------------------------------------------------*/
      ds_system_call("echo QTI:LINK_UP_WAIT state > /dev/kmsg",
                     strlen("echo QTI:LINK_UP_WAIT state > /dev/kmsg"));
      break;
    }

    case QTI_QCMAP_INIT_EVENT:
    {
      ret_val = qti_qcmap_connect();
      if(ret_val != QTI_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to initialize QTI as a QCMAP client",
            0, 0, 0);
        return QTI_FAILURE;
      }
/*-------------------------------------------------------------------------
  Connection to QCMAP needs to be completed before calling netlink query,
  since QCMAP needs to handles the usb link up and down messages generated.
  Now, Query the kernel about the current links by sending RTM_GETLINK.
  This is useful to get RTM_NEWLINK for the interfaces if the USB is
  plugged in and then powered up.
--------------------------------------------------------------------------*/
      ret_val = qti_nl_query_if();
      if(ret_val != QTI_SUCCESS)
      {
        LOG_MSG_ERROR("Failed sending RTM_GETLINK to kernel",0,0,0);
      }
      break;
    }

    default:
      LOG_MSG_INFO1("Ignoring event %d received",event,0,0);
      break;
  }

  LOG_MSG_INFO1("Succeed handle QCMAP event",0,0,0);
  return QTI_SUCCESS;
}
