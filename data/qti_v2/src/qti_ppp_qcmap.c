/*=============================================================================

FILE:  qti_ppp_qcmap.c

SERVICES: Implementation of QTI interface with QCMAP for PPP
===============================================================================

Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

=============================================================================*/

/*=============================================================================
  EDIT HISTORY FOR MODULE

  Please notice that the changes are listed in reverse chronological order.

  when       who        what, where, why
  --------   ---        ------------------------------------------------------
  02/19/14   cp         Initial version

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
#include <sys/un.h>
#include "ds_util.h"
#include "ds_string.h"
#include "qualcomm_mobile_access_point_msgr_v01.h"
#include "qti_ppp.h"
#include "stringl.h"

/*=============================================================================
                                MACRO DEFINITIONS
==============================================================================*/


/*=============================================================================
                                VARIABLE DEFINITIONS
==============================================================================*/
static qti_ppp_conf_t * qti_ppp_qcmap_conf;
/*=============================================================================
                                FUNCTION FORWARD DECLARATION
==============================================================================*/
void qti_ppp_qcmap_msgr_ind_cb
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
  FUNCTION QTI_PPP_QCMAP_INIT()

  DESCRIPTION

  This function initializes QTI interface to QCMAP for PPP.

  DEPENDENCIES
  None.

  RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure

  SIDE EFFECTS
  None
==============================================================================*/
int qti_ppp_qcmap_init(qti_ppp_conf_t * qti_ppp_conf)
{
  qmi_idl_service_object_type                            qti_qcmap_msgr_service_object;
  qmi_client_error_type                                  qmi_error, qmi_err_code = QMI_NO_ERR;
  qcmap_msgr_mobile_ap_status_ind_register_req_msg_v01   qcmap_mobile_ap_status_ind_reg;
  qcmap_msgr_mobile_ap_status_ind_register_resp_msg_v01  qcmap_mobile_ap_status_ind_rsp;
  qcmap_msgr_wwan_status_ind_register_req_msg_v01        wwan_status_ind_reg;
  qcmap_msgr_wwan_status_ind_register_resp_msg_v01       wwan_status_ind_rsp;
  qmi_cci_os_signal_type                                 qti_qcmap_msgr_os_params;
  int                                                    ret_val = QTI_PPP_SUCCESS;
/*---------------------------------------------------------------------------*/


  ds_assert(qti_ppp_conf != NULL);

  LOG_MSG_INFO1("qti_ppp_qcmap_init()", 0, 0, 0);

/*-----------------------------------------------------------------------------
  Static pointer to QTI configuration variable
------------------------------------------------------------------------------*/
  qti_ppp_qcmap_conf = qti_ppp_conf;

/*-----------------------------------------------------------------------------
  Obtain a QCMAP messenger service client for QTI
  - get the service object
  - notify the client
  - get service list
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
    return QTI_PPP_FAILURE;
  }

/*----------------------------------------------------------------------------
  Obtain a QCMAP messenger client for QTI
------------------------------------------------------------------------------*/
  qmi_error = qmi_client_init_instance(qti_qcmap_msgr_service_object,
                              QMI_CLIENT_INSTANCE_ANY,
                              qti_ppp_qcmap_msgr_ind_cb,
                              NULL,
                              &qti_qcmap_msgr_os_params,
                              QTI_PPP_QMI_MAX_TIMEOUT_MS,
                              &(qti_ppp_qcmap_conf->qti_ppp_qcmap_msgr_handle));

  LOG_MSG_INFO1("qmi_client_init_instance: %d", qmi_error, 0, 0);


  if (qmi_error != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Can not init qcmap_msgr client %d", qmi_error, 0, 0);
    return QTI_PPP_FAILURE;
  }

/*-----------------------------------------------------------------------------
  Register for WWAN indications from QCMAP
-----------------------------------------------------------------------------*/
  memset(&wwan_status_ind_reg,0,
         sizeof(qcmap_msgr_wwan_status_ind_register_req_msg_v01));

  memset(&wwan_status_ind_rsp,0,
         sizeof(qcmap_msgr_wwan_status_ind_register_resp_msg_v01));

  wwan_status_ind_reg.register_indication = 1;
  qmi_error = qmi_client_send_msg_sync(qti_ppp_qcmap_conf->qti_ppp_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_WWAN_STATUS_IND_REG_REQ_V01,
                                       (void*)&wwan_status_ind_reg,
                                       sizeof(qcmap_msgr_wwan_status_ind_register_req_msg_v01),
                                       (void*)&wwan_status_ind_rsp,
                                       sizeof(qcmap_msgr_wwan_status_ind_register_resp_msg_v01),
                                       QTI_PPP_QMI_TIMEOUT_VALUE);

  LOG_MSG_INFO1("qmi_client_send_msg_sync(wwan_status_reg): error %d result %d",
      qmi_error, wwan_status_ind_rsp.resp.result,0);

  if ((qmi_error != QMI_NO_ERR) ||
      (wwan_status_ind_rsp.resp.result != QMI_NO_ERR))
  {
    LOG_MSG_ERROR("Can not register for wwan status %d : %d",
        qmi_error, wwan_status_ind_rsp.resp.error,0);
    return QTI_PPP_FAILURE;
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
  qmi_error = qmi_client_send_msg_sync(qti_ppp_qcmap_conf->qti_ppp_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_MOBILE_AP_STATUS_IND_REG_REQ_V01,
                                       (void*)&qcmap_mobile_ap_status_ind_reg,
                                       sizeof(qcmap_msgr_mobile_ap_status_ind_register_req_msg_v01),
                                       (void*)&qcmap_mobile_ap_status_ind_rsp,
                                       sizeof(qcmap_msgr_mobile_ap_status_ind_register_resp_msg_v01),
                                       QTI_PPP_QMI_TIMEOUT_VALUE);

  LOG_MSG_INFO1("qmi_client_send_msg_sync(mobileap_status): error %d result %d",
      qmi_error, qcmap_mobile_ap_status_ind_rsp.resp.result, 0);

  if ((qmi_error != QMI_NO_ERR) ||
      (qcmap_mobile_ap_status_ind_rsp.resp.result != QMI_NO_ERR))
  {
    LOG_MSG_ERROR("Can not register for mobile ap status %d : %d",
                  qmi_error,
                  qcmap_mobile_ap_status_ind_rsp.resp.error, 0);
    return QTI_PPP_FAILURE;
  }

  LOG_MSG_INFO1("Done registering for mobile ap status",0,0,0);

  return QTI_PPP_SUCCESS;
}

/*=============================================================================
  FUNCTION QTI_QCMAP_EXIT()

  DESCRIPTION

  This function releases QCMAP client

  DEPENDENCIES
  None.

  RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure

  SIDE EFFECTS
  None
==============================================================================*/
 int qti_ppp_qcmap_exit()
{
  qmi_client_error_type     qmi_error;

/*----------------------------------------------------------------------------*/

  qmi_error = qmi_client_release(qti_ppp_qcmap_conf->qti_ppp_qcmap_msgr_handle);
  qti_ppp_qcmap_conf->qti_ppp_qcmap_msgr_handle = NULL;

  if (qmi_error != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Can not release client qcmap client handle %d",
                  qmi_error,0,0);
    return QTI_PPP_FAILURE;
  }

  return QTI_PPP_SUCCESS;
}

/*===========================================================================
  FUNCTION  qti_ppp_qcmap_msgr_ind_cb
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
void qti_ppp_qcmap_msgr_ind_cb
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
        if (ind_data.mobile_ap_handle == qti_ppp_qcmap_conf->qti_ppp_mobile_ap_handle)
        {
          LOG_MSG_INFO1("qcmap_msgr_qmi_qcmap_ind: WWAN Connected",0,0,0);
          return;
        }
      }
      else if (ind_data.conn_status == QCMAP_MSGR_WWAN_STATUS_CONNECTING_FAIL_V01)
      {
        if (ind_data.mobile_ap_handle == qti_ppp_qcmap_conf->qti_ppp_mobile_ap_handle)
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
        if (ind_data.mobile_ap_handle == qti_ppp_qcmap_conf->qti_ppp_mobile_ap_handle)
        {
          LOG_MSG_INFO1("qcmap_msgr_qmi_qcmap_ind: WWAN Disconnected",0,0,0);
          return;
        }
      }
      else if (ind_data.conn_status == QCMAP_MSGR_WWAN_STATUS_DISCONNECTING_FAIL_V01)
      {
        if (ind_data.mobile_ap_handle == qti_ppp_qcmap_conf->qti_ppp_mobile_ap_handle)
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
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

/*==========================================================================*/

int qti_ppp_enable_mobile_ap()
{
  qmi_client_error_type                        qmi_error;
  qmi_client_error_type                        qmi_err_code = QMI_NO_ERR;
  qcmap_msgr_mobile_ap_enable_resp_msg_v01     qcmap_enable_resp_msg_v01;

/*--------------------------------------------------------------------------*/

  memset(&qcmap_enable_resp_msg_v01,
         0,
         sizeof(qcmap_msgr_mobile_ap_enable_resp_msg_v01));

/*---------------------------------------------------------------------------
  Enable Mobile AP
----------------------------------------------------------------------------*/
  qmi_error = qmi_client_send_msg_sync(qti_ppp_qcmap_conf->qti_ppp_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_MOBILE_AP_ENABLE_REQ_V01,
                                       NULL,
                                       0,
                                       (void*)&qcmap_enable_resp_msg_v01,
                                       sizeof(qcmap_msgr_mobile_ap_enable_resp_msg_v01),
                                       QTI_PPP_QMI_TIMEOUT_VALUE);

  LOG_MSG_INFO1("qmi_client_send_msg_sync(enable): error %d result %d valid %d",
                qmi_error,
                qcmap_enable_resp_msg_v01.resp.result,
                qcmap_enable_resp_msg_v01.mobile_ap_handle_valid);

  if ((qmi_error != QMI_NO_ERR) ||
      (qcmap_enable_resp_msg_v01.resp.result != QMI_RESULT_SUCCESS_V01 &&
      qcmap_enable_resp_msg_v01.resp.error != QMI_ERR_NO_EFFECT_V01))
  {
    LOG_MSG_ERROR("Can not enable qcmap %d : %d",
        qmi_error, qcmap_enable_resp_msg_v01.resp.error,0);
    return QTI_PPP_FAILURE;
  }

/*---------------------------------------------------------------------------
  Assign the Mobile AP handle which is used by QTI
----------------------------------------------------------------------------*/
  if (qcmap_enable_resp_msg_v01.mobile_ap_handle > 0)
  {
    qti_ppp_qcmap_conf->qti_ppp_mobile_ap_handle = qcmap_enable_resp_msg_v01.mobile_ap_handle;
    LOG_MSG_INFO1(" QTI QCMAP Enabled",0,0,0);
    return QTI_PPP_SUCCESS;
  }
  else if ( qti_ppp_qcmap_conf->qti_ppp_mobile_ap_handle < 0 )
  {
    LOG_MSG_INFO1("QCMAP Enable Failure",0,0,0);
	return QTI_PPP_FAILURE;
  }
}

/*===========================================================================

FUNCTION disable_mobile_ap()

DESCRIPTION

  This function disables QC Mobile AP

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

/*==========================================================================*/

int qti_ppp_disable_mobile_ap()
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
  qcmap_disable_req_msg_v01.mobile_ap_handle = qti_ppp_qcmap_conf->qti_ppp_mobile_ap_handle;

  qmi_error = qmi_client_send_msg_sync(qti_ppp_qcmap_conf->qti_ppp_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_MOBILE_AP_DISABLE_REQ_V01,
                                       &qcmap_disable_req_msg_v01,
                                       sizeof(qcmap_msgr_mobile_ap_disable_req_msg_v01),
                                       &qcmap_disable_resp_msg_v01,
                                       sizeof(qcmap_msgr_mobile_ap_disable_resp_msg_v01),
                                       QTI_PPP_QMI_TIMEOUT_VALUE);

  LOG_MSG_INFO1("qmi_client_send_msg_sync(disable): error %d result %d",
                qmi_error,
                qcmap_disable_resp_msg_v01.resp.result,
                0);


  if ((qmi_error != QMI_NO_ERR) ||
      (qcmap_disable_resp_msg_v01.resp.result != QMI_NO_ERR))
  {
    LOG_MSG_ERROR( "Can not disable qcmap %d : %d",
        qmi_error, qcmap_disable_resp_msg_v01.resp.error,0);
    return QTI_PPP_FAILURE;
  }

  LOG_MSG_INFO1( "QCMAP disabled", 0, 0, 0);


  return QTI_PPP_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_PPP_USB_LINK_UP()

DESCRIPTION

  This function sends a message to QCMAP setup the USB link for DUN
  tethering

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

/*==========================================================================*/
int qti_ppp_usb_link_up
(
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
                                         qti_ppp_qcmap_conf->qti_ppp_mobile_ap_handle;


  qcmap_qti_usb_link_up_req_msg.usb_link = QCMAP_MSGR_USB_LINK_PPP_V01;

  qmi_error = qmi_client_send_msg_sync(qti_ppp_qcmap_conf->qti_ppp_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_USB_LINK_UP_REQ_V01,
                                       &qcmap_qti_usb_link_up_req_msg,
                                       sizeof(qcmap_msgr_usb_link_up_req_msg_v01),
                                       &qcmap_qti_usb_link_up_resp_msg,
                                       sizeof(qcmap_msgr_usb_link_up_resp_msg_v01),
                                       QTI_PPP_QMI_TIMEOUT_VALUE);

  LOG_MSG_INFO1("Setup USB link. Error values: %d , %d",
        qmi_error, qcmap_qti_usb_link_up_resp_msg.resp.error, 0);

  if (( qmi_error != QMI_TIMEOUT_ERR ) && ((qmi_error != QMI_NO_ERR) ||
        (qcmap_qti_usb_link_up_resp_msg.resp.result != QMI_NO_ERR)))
  {
    LOG_MSG_ERROR("Setup USB link failed. Error values: %d , %d",
        qmi_error, qcmap_qti_usb_link_up_resp_msg.resp.error,0);
    return QTI_PPP_FAILURE;
  }

  LOG_MSG_INFO1("Setup USB link succeeded", 0, 0, 0);

  return QTI_PPP_SUCCESS;
}


/*===========================================================================

FUNCTION QTI_USB_PPP_LINK_DOWN()

DESCRIPTION

  This function sends a message to QCMAP to bring down the PPP link

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

/*==========================================================================*/
int qti_ppp_usb_link_down
(
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
                                         qti_ppp_qcmap_conf->qti_ppp_mobile_ap_handle;

  qcmap_qti_usb_link_down_req_msg.usb_link = QCMAP_MSGR_USB_LINK_PPP_V01;

  qmi_error = qmi_client_send_msg_sync(qti_ppp_qcmap_conf->qti_ppp_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_USB_LINK_DOWN_REQ_V01,
                                       &qcmap_qti_usb_link_down_req_msg,
                                       sizeof(qcmap_msgr_usb_link_down_req_msg_v01),
                                       &qcmap_qti_usb_link_down_resp_msg,
                                       sizeof(qcmap_msgr_usb_link_down_resp_msg_v01),
                                       QTI_PPP_QMI_TIMEOUT_VALUE);

  if (( qmi_error != QMI_TIMEOUT_ERR ) && ((qmi_error != QMI_NO_ERR) ||
        (qcmap_qti_usb_link_down_resp_msg.resp.result != QMI_NO_ERR)))
  {
    LOG_MSG_ERROR("Bring down USB link failed. Error values: %d , %d",
        qmi_error, qcmap_qti_usb_link_down_resp_msg.resp.error,0);
    return QTI_PPP_FAILURE;
  }

  LOG_MSG_INFO1("Bring down USB link succeeded", 0, 0, 0);

  return QTI_PPP_SUCCESS;
}
#ifdef FEATURE_DUN_PROFILE_VALIDATION
/*===========================================================================

FUNCTION QTI_PPP_VALIDATE_DUN_PROFILE()

DESCRIPTION

  This function sends a message to QCMAP to validate the DUN profile for SoftAP call.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

/*==========================================================================*/
int qti_ppp_validate_dun_profile
(
  uint8_t dun_profile_id,
  qti_ppp_tech_pref_mask_v01 tech_pref
)
{
  qcmap_msgr_validate_dun_profile_req_msg_v01 validate_dun_profile_req_msg;
  qcmap_msgr_validate_dun_profile_resp_msg_v01 validate_dun_profile_resp_msg;
  qmi_client_error_type                  qmi_error;
/*--------------------------------------------------------------------------*/

  LOG_MSG_INFO1("Validate DUN profile", 0, 0, 0);

  memset(&validate_dun_profile_req_msg,
         0,
         sizeof(qcmap_msgr_validate_dun_profile_req_msg_v01));

  memset(&validate_dun_profile_resp_msg,
         0,
         sizeof(qcmap_msgr_validate_dun_profile_resp_msg_v01));

/*-------------------------------------------------------------------------
  Setup USB link - RNDIS/ECM
---------------------------------------------------------------------------*/
  validate_dun_profile_req_msg.mobile_ap_handle =
                                         qti_ppp_qcmap_conf->qti_ppp_mobile_ap_handle;

  validate_dun_profile_req_msg.dun_profile_id = dun_profile_id;
  validate_dun_profile_req_msg.tech_pref = tech_pref;

  qmi_error = qmi_client_send_msg_sync(qti_ppp_qcmap_conf->qti_ppp_qcmap_msgr_handle,
                                       QMI_QCMAP_MSGR_VALIDATE_DUN_PROFILE_REQ_V01,
                                       &validate_dun_profile_req_msg,
                                       sizeof(qcmap_msgr_validate_dun_profile_req_msg_v01),
                                       &validate_dun_profile_resp_msg,
                                       sizeof(qcmap_msgr_validate_dun_profile_resp_msg_v01),
                                       QTI_PPP_QMI_TIMEOUT_VALUE);

  if (( qmi_error != QMI_TIMEOUT_ERR ) && ((qmi_error != QMI_NO_ERR) ||
        (validate_dun_profile_resp_msg.resp.result != QMI_NO_ERR)))
  {
    LOG_MSG_ERROR("Validate DUN profile failed. Error values: %d , %d",
        qmi_error, validate_dun_profile_resp_msg.resp.error,0);
    return QTI_PPP_FAILURE;
  }

  LOG_MSG_INFO1("Validate DUN profile succeeded", 0, 0, 0);

  return QTI_PPP_SUCCESS;
}
#endif /* FEATURE_DUN_PROFILE_VALIDATION. */
