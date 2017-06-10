/******************************************************************************

                        QTI_RMNET_DPM.C

******************************************************************************/

/******************************************************************************

  @file    qti_rmnet_dpm.c
  @brief   Tethering Interface module for Data Port Mapper interaction.


  DESCRIPTION
  This file has functions which interact with DPM QMI service for
  RMNET tethering.

  ---------------------------------------------------------------------------
  Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------


******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        -------------------------------------------------------
12/9/13    sb         Add port mapper functionality. Modem interface file interaction

******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <fcntl.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#include "qti.h"
#include "qti_cmdq.h"
#include "data_common_v01.h"
#include "data_port_mapper_v01.h"
#include "qmi_client_instance_defs.h"


static  qti_rmnet_param        * rmnet_state_config;
static  qti_dpl_param          * dpl_state_config;
qmi_client_os_params           qti_dpm_os_params;
/*===========================================================================
                               FUNCTION DEFINITIONS
=========================================================================*/

/*===========================================================================

FUNCTION QTI_DPM_INIT()

DESCRIPTION
  Initializes a DPM QMI client

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/

int qti_dpm_init
(
   qti_rmnet_param       * rmnet_state,
   qti_dpl_param         * dpl_state
)
{

  qmi_idl_service_object_type                            qti_dpm_service_object;
  qmi_client_error_type                                  qmi_error, qmi_err_code = QMI_NO_ERR;
  int ret;
/*---------------------------------------------------------------------------*/

  if(rmnet_state == NULL || dpl_state == NULL)
  {
    LOG_MSG_ERROR("RmNet/DPL state not valid", 0, 0, 0);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1("qti_dpm_init()", 0, 0, 0);

/*-----------------------------------------------------------------------------
  Deregister QMUXD ports since we want the client init request to go over IPC router
------------------------------------------------------------------------------*/
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_USB_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_SMUX_INSTANCE_0);
  qmi_cci_qmux_xport_unregister(QMI_CLIENT_QMUX_RMNET_MHI_INSTANCE_0);

/*-----------------------------------------------------------------------------
  Static pointer to QTI RmNet config and DPL config
------------------------------------------------------------------------------*/
  rmnet_state_config = rmnet_state;
  dpl_state_config = dpl_state;

/*-----------------------------------------------------------------------------
  Obtain a DPM client for QTI
  - get the service object
  - obtain the client
------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
  Get the service object
------------------------------------------------------------------------------*/
  qti_dpm_service_object = dpm_get_service_object_v01();
  if (qti_dpm_service_object == NULL)
  {
    LOG_MSG_ERROR("QTI DPM service object not available",
                   0, 0, 0);
    return QTI_FAILURE;
  }

  memset(&qti_dpm_os_params, 0, sizeof(qmi_client_os_params));
/*-----------------------------------------------------------------------------
  Client init
------------------------------------------------------------------------------*/

  qmi_error = qmi_client_init_instance(qti_dpm_service_object,
                                       QMI_CLIENT_INSTANCE_ANY,
                                       NULL,
                                       NULL,
                                       &qti_dpm_os_params,
                                       QTI_QMI_MSG_TIMEOUT_VALUE,
                                       &(rmnet_state_config->qti_dpm_handle));

  if (qmi_error != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Can not init DPM client %d", qmi_error, 0, 0);
    return QTI_FAILURE;
  }
  dpl_state_config->qti_dpm_handle = rmnet_state_config->qti_dpm_handle;

#ifndef FEATURE_MDM_LE
  if( (DS_TARGET_MSM8994 == rmnet_state_config->target) ||
      (DS_TARGET_JOLOKIA == rmnet_state_config->target) )
  {
/*-----------------------------------------------------------------------------
  Register for err callback
------------------------------------------------------------------------------*/
    (void) qmi_client_register_error_cb(rmnet_state_config->qti_dpm_handle,
                                        dpm_erro_cb,
                                        NULL);
  }
#endif

  return QTI_SUCCESS;
}


/*===========================================================================

FUNCTION QTI_RMNET_DPM_PORT_OPEN()

DESCRIPTION
  Opens the USB tethered SMD port using DPM port open request

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/

int qti_rmnet_dpm_port_open()
{
  dpm_open_port_req_msg_v01    qti_dpm_port_open_req_msg;
  dpm_open_port_resp_msg_v01   qti_dpm_port_open_resp_msg;
  qmi_client_error_type        qmi_error;
  int                          ret_val;

/*--------------------------------------------------------------------------*/

  if(rmnet_state_config == NULL)
  {
    LOG_MSG_ERROR("RmNet state not valid", 0, 0, 0);
    return QTI_FAILURE;
  }

  memset(&qti_dpm_port_open_req_msg,
         0,
         sizeof(dpm_open_port_req_msg_v01));

  memset(&qti_dpm_port_open_resp_msg,
         0,
         sizeof(dpm_open_port_resp_msg_v01));

  ret_val = qti_rmnet_ph_get_ep_info();
  if (ret_val != QTI_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to get EP info", 0, 0, 0);
    return QTI_FAILURE;
  }

  qti_dpm_port_open_req_msg.control_port_list_valid = TRUE;
  qti_dpm_port_open_req_msg.control_port_list_len = 1;
  strlcpy((char *)qti_dpm_port_open_req_msg.control_port_list[0].port_name,
          USB_TETHERED_SMD_CH,
          strlen(USB_TETHERED_SMD_CH)+1);
  qti_dpm_port_open_req_msg.control_port_list[0].default_ep_id.ep_type =
       rmnet_state_config->ep_info.ph_ep_info.ep_type;
  qti_dpm_port_open_req_msg.control_port_list[0].default_ep_id.iface_id =
       rmnet_state_config->ep_info.ph_ep_info.peripheral_iface_id;

  qti_dpm_port_open_req_msg.hardware_data_port_list_valid = TRUE;
  qti_dpm_port_open_req_msg.hardware_data_port_list_len = 1;
  qti_dpm_port_open_req_msg.hardware_data_port_list[0].ep_id.ep_type =
                  rmnet_state_config->ep_info.ph_ep_info.ep_type;
  qti_dpm_port_open_req_msg.hardware_data_port_list[0].ep_id.iface_id =
    rmnet_state_config->ep_info.ph_ep_info.peripheral_iface_id;

  qti_dpm_port_open_req_msg.hardware_data_port_list[0].hardware_ep_pair.consumer_pipe_num =
    rmnet_state_config->ep_info.ipa_ep_pair.consumer_pipe_num;

  qti_dpm_port_open_req_msg.hardware_data_port_list[0].hardware_ep_pair.producer_pipe_num =
    rmnet_state_config->ep_info.ipa_ep_pair.producer_pipe_num;

  LOG_MSG_INFO1_6("Sending to modem  SMD channel %s, Peripheral type %d, peripheral id %d, ipa consumer EP %d, ipa producer EP %d",
                 qti_dpm_port_open_req_msg.control_port_list[0].port_name,
                 rmnet_state_config->ep_info.ph_ep_info.ep_type,
                 rmnet_state_config->ep_info.ph_ep_info.peripheral_iface_id,
                 rmnet_state_config->ep_info.ipa_ep_pair.consumer_pipe_num,
                 rmnet_state_config->ep_info.ipa_ep_pair.producer_pipe_num, 0);

  qmi_error = qmi_client_send_msg_sync(rmnet_state_config->qti_dpm_handle,
                                       QMI_DPM_OPEN_PORT_REQ_V01,
                                       &qti_dpm_port_open_req_msg,
                                       sizeof(dpm_open_port_req_msg_v01),
                                       &qti_dpm_port_open_resp_msg,
                                       sizeof(dpm_open_port_resp_msg_v01),
                                       QTI_QMI_MSG_TIMEOUT_VALUE);

  LOG_MSG_INFO1("qmi_client_send_msg_sync(dpm_port_open): error %d result %d",
                qmi_error,
                qti_dpm_port_open_resp_msg.resp.result,
                0);

  if ((qmi_error != QMI_NO_ERR) ||
      (qti_dpm_port_open_resp_msg.resp.result != QMI_NO_ERR))
  {
    LOG_MSG_ERROR( "Cannot open port %d : %d",
        qmi_error, qti_dpm_port_open_resp_msg.resp.error,0);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1( "DPM port opened", 0, 0, 0);
  ds_system_call("echo QTI:USB tethered modem SMD port opened > /dev/kmsg",
                 strlen("echo QTI:USB tethered modem SMD port opened > /dev/kmsg"));
  return QTI_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_RMNET_DPM_PORT_CLOSE()

DESCRIPTION
  Closes the USB tethered SMD port using DPM port close request

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/


int qti_rmnet_dpm_port_close()
{
  dpm_close_port_req_msg_v01    qti_dpm_port_close_req_msg;
  dpm_close_port_resp_msg_v01   qti_dpm_port_close_resp_msg;
  qmi_client_error_type        qmi_error;

/*--------------------------------------------------------------------------*/

  if(rmnet_state_config == NULL)
  {
    LOG_MSG_ERROR("RmNet state not valid", 0, 0, 0);
    return QTI_FAILURE;
  }

  memset(&qti_dpm_port_close_req_msg,
         0,
         sizeof(dpm_close_port_req_msg_v01));

  memset(&qti_dpm_port_close_resp_msg,
         0,
         sizeof(dpm_close_port_resp_msg_v01));


  qti_dpm_port_close_req_msg.control_port_list_valid = TRUE;
  qti_dpm_port_close_req_msg.control_port_list_len = 1;
  strlcpy(qti_dpm_port_close_req_msg.control_port_list[0].port_name,
          USB_TETHERED_SMD_CH,
          strlen(USB_TETHERED_SMD_CH)+1);


  qti_dpm_port_close_req_msg.data_port_list_valid = TRUE;
  qti_dpm_port_close_req_msg.data_port_list_len = 1;
  qti_dpm_port_close_req_msg.data_port_list[0].ep_type =
      rmnet_state_config->ep_info.ph_ep_info.ep_type;
  qti_dpm_port_close_req_msg.data_port_list[0].iface_id =
    rmnet_state_config->ep_info.ph_ep_info.peripheral_iface_id;

  qmi_error = qmi_client_send_msg_sync(rmnet_state_config->qti_dpm_handle,
                                       QMI_DPM_CLOSE_PORT_REQ_V01,
                                       &qti_dpm_port_close_req_msg,
                                       sizeof(dpm_close_port_req_msg_v01),
                                       &qti_dpm_port_close_resp_msg,
                                       sizeof(dpm_close_port_resp_msg_v01),
                                       QTI_QMI_MSG_TIMEOUT_VALUE);

  LOG_MSG_INFO1("qmi_client_send_msg_sync(dpm_port_close): error %d result %d",
                qmi_error,
                qti_dpm_port_close_resp_msg.resp.result,
                0);

  if ((qmi_error != QMI_NO_ERR) ||
      (qti_dpm_port_close_resp_msg.resp.result != QMI_NO_ERR))
  {
    LOG_MSG_ERROR( "Cannot close port %d : %d",
        qmi_error, qti_dpm_port_close_resp_msg.resp.error,0);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1( "DPM port closed", 0, 0, 0);

  return QTI_SUCCESS;
}


/*===========================================================================

FUNCTION QTI_RMNET_DPM_RELEASE()

DESCRIPTION
  Releases a DPM QMI client

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/

int qti_rmnet_dpm_release()
{
  qmi_client_error_type                 qmi_error;

/*------------------------------------------------------------------------*/

  qmi_error = qmi_client_release(rmnet_state_config->qti_dpm_handle);
  if (qmi_error != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Can not release DFS client %d", qmi_error, 0, 0);
    return QTI_FAILURE;
  }
  LOG_MSG_INFO1("Successfully deregistered DFS client", 0, 0, 0);
  return QTI_SUCCESS;
}

/*===========================================================================

FUNCTION DPM_ERROR_CB()

DESCRIPTION
  This function is invoked by QCCI framework when DPM service is no longer
  available on modem which in turn is used by QTI to identify that modem is not
  in service

DEPENDENCIES
  None.

RETURN VALUE


SIDE EFFECTS
  None

=========================================================================*/
void dpm_erro_cb()
{
  int numBytes=0;
  int len;
  qti_cmdq_cmd_t * cmd_buf = NULL;

  LOG_MSG_INFO1("qti_dpm_erro_cb",0,0,0);

  cmd_buf = qti_cmdq_get_cmd();

  if(cmd_buf == NULL)
  {
    LOG_MSG_ERROR("qti_cmdq: failed to allocate memeory for cmd", 0, 0, 0);
    return;
  }
  cmd_buf->data.event = QTI_RMNET_DPM_MODEM_NOT_IN_SERVICE_EVENT;

  if( QTI_CMDQ_SUCCESS != qti_cmdq_put_cmd( cmd_buf ) )
  {
    qti_cmdq_release_cmd(cmd_buf);
    LOG_MSG_ERROR("qti_cmdq: failed to put commmand",0,0,0);
  }
}

/*===========================================================================

FUNCTION DPM_NOTIFY_CB()

DESCRIPTION
  This function is invoked by QCCI framework when DPM service is available on
  modem which in turn is used by QTI to identify that modem is in service.

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

=========================================================================*/
void dpm_notify_cb()
{
  int numBytes=0;
  int len;
  qti_cmdq_cmd_t * cmd_buf = NULL;

  LOG_MSG_INFO1("qti_dpm_notify_cb",0,0,0);
  /*--------------------------------------------------------------------------
    Post modem
    ---------------------------------------------------------------------------*/
  cmd_buf = qti_cmdq_get_cmd();

  if(cmd_buf == NULL)
  {
    LOG_MSG_ERROR("qti_cmdq: failed to allocate memeory for cmd", 0, 0, 0);
    return;
  }
  cmd_buf->data.event = QTI_RMNET_DPM_MODEM_IN_SERVICE_EVENT;

  if( QTI_CMDQ_SUCCESS != qti_cmdq_put_cmd( cmd_buf ) )
  {
    qti_cmdq_release_cmd(cmd_buf);
    LOG_MSG_ERROR("qti_cmdq: failed to put commmand",0,0,0);
  }
}

/*===========================================================================

FUNCTION QTI_DPL_DPM_PORT_OPEN()

DESCRIPTION
  Opens the USB tethered DPL port using DPM port open request

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/

int qti_dpl_dpm_port_open()
{
  dpm_open_port_req_msg_v01    qti_dpm_port_open_req_msg;
  dpm_open_port_resp_msg_v01   qti_dpm_port_open_resp_msg;
  qmi_client_error_type        qmi_error;
  int                          ret_val;

/*--------------------------------------------------------------------------*/

  if(dpl_state_config == NULL)
  {
    LOG_MSG_ERROR("DPL state not valid", 0, 0, 0);
    return QTI_FAILURE;
  }

  memset(&qti_dpm_port_open_req_msg,
         0,
         sizeof(dpm_open_port_req_msg_v01));

  memset(&qti_dpm_port_open_resp_msg,
         0,
         sizeof(dpm_open_port_resp_msg_v01));
/*-----------------------------------------------------------------------------
  Get the DPL USB/IPA EP info
------------------------------------------------------------------------------*/
  ret_val = qti_dpl_ph_get_ep_info();
  if (ret_val != QTI_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to get EP info", 0, 0, 0);
    return QTI_FAILURE;
  }

/*-----------------------------------------------------------------------------
  Populate the DPM port open message
------------------------------------------------------------------------------*/
  qti_dpm_port_open_req_msg.hardware_data_port_list_valid = TRUE;
  qti_dpm_port_open_req_msg.hardware_data_port_list_len = 1;
  qti_dpm_port_open_req_msg.hardware_data_port_list[0].ep_id.ep_type =
                  dpl_state_config->ep_info.ph_ep_info.ep_type;
  qti_dpm_port_open_req_msg.hardware_data_port_list[0].ep_id.iface_id =
    dpl_state_config->ep_info.ph_ep_info.peripheral_iface_id;

  qti_dpm_port_open_req_msg.hardware_data_port_list[0].hardware_ep_pair.consumer_pipe_num =
    dpl_state_config->ep_info.ipa_ep_pair.consumer_pipe_num;

  qti_dpm_port_open_req_msg.hardware_data_port_list[0].hardware_ep_pair.producer_pipe_num =
    dpl_state_config->ep_info.ipa_ep_pair.producer_pipe_num;

  LOG_MSG_INFO1_6("Sending to modem  Peripheral type %d, peripheral id %d, ipa consumer EP %d, ipa producer EP %d",
                 dpl_state_config->ep_info.ph_ep_info.ep_type,
                 dpl_state_config->ep_info.ph_ep_info.peripheral_iface_id,
                 dpl_state_config->ep_info.ipa_ep_pair.consumer_pipe_num,
                 dpl_state_config->ep_info.ipa_ep_pair.producer_pipe_num, 0, 0);
/*-----------------------------------------------------------------------------
  Send the DPM port open QMI message
------------------------------------------------------------------------------*/
  qmi_error = qmi_client_send_msg_sync(dpl_state_config->qti_dpm_handle,
                                       QMI_DPM_OPEN_PORT_REQ_V01,
                                       &qti_dpm_port_open_req_msg,
                                       sizeof(dpm_open_port_req_msg_v01),
                                       &qti_dpm_port_open_resp_msg,
                                       sizeof(dpm_open_port_resp_msg_v01),
                                       QTI_QMI_MSG_TIMEOUT_VALUE);

  LOG_MSG_INFO1("qmi_client_send_msg_sync(dpm_port_open): error %d result %d",
                qmi_error,
                qti_dpm_port_open_resp_msg.resp.result,
                0);

  if ((qmi_error != QMI_NO_ERR) ||
      (qti_dpm_port_open_resp_msg.resp.result != QMI_NO_ERR))
  {
    LOG_MSG_ERROR( "Cannot open DPL port %d : %d",
        qmi_error, qti_dpm_port_open_resp_msg.resp.error,0);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1( "DPL port opened", 0, 0, 0);

  return QTI_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_DPL_DPM_PORT_CLOSE()

DESCRIPTION
  Closes the USB tethered SMD port using DPM port close request

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/


int qti_dpl_dpm_port_close()
{
  dpm_close_port_req_msg_v01    qti_dpm_port_close_req_msg;
  dpm_close_port_resp_msg_v01   qti_dpm_port_close_resp_msg;
  qmi_client_error_type         qmi_error;

/*--------------------------------------------------------------------------*/

  if(dpl_state_config == NULL)
  {
    LOG_MSG_ERROR("RmNet state not valid", 0, 0, 0);
    return QTI_FAILURE;
  }

  memset(&qti_dpm_port_close_req_msg,
         0,
         sizeof(dpm_close_port_req_msg_v01));

  memset(&qti_dpm_port_close_resp_msg,
         0,
         sizeof(dpm_close_port_resp_msg_v01));

/*-----------------------------------------------------------------------------
  Populate the DPM port close QMI message
------------------------------------------------------------------------------*/
  qti_dpm_port_close_req_msg.data_port_list_valid = TRUE;
  qti_dpm_port_close_req_msg.data_port_list_len = 1;
  qti_dpm_port_close_req_msg.data_port_list[0].ep_type =
      dpl_state_config->ep_info.ph_ep_info.ep_type;
  qti_dpm_port_close_req_msg.data_port_list[0].iface_id =
    dpl_state_config->ep_info.ph_ep_info.peripheral_iface_id;
/*-----------------------------------------------------------------------------
  Send the QMI message
------------------------------------------------------------------------------*/
  qmi_error = qmi_client_send_msg_sync(dpl_state_config->qti_dpm_handle,
                                       QMI_DPM_CLOSE_PORT_REQ_V01,
                                       &qti_dpm_port_close_req_msg,
                                       sizeof(dpm_close_port_req_msg_v01),
                                       &qti_dpm_port_close_resp_msg,
                                       sizeof(dpm_close_port_resp_msg_v01),
                                       QTI_QMI_MSG_TIMEOUT_VALUE);

  LOG_MSG_INFO1("qmi_client_send_msg_sync(dpm_port_close): error %d result %d",
                qmi_error,
                qti_dpm_port_close_resp_msg.resp.result,
                0);

  if ((qmi_error != QMI_NO_ERR) ||
      (qti_dpm_port_close_resp_msg.resp.result != QMI_NO_ERR))
  {
    LOG_MSG_ERROR( "Cannot close port %d : %d",
        qmi_error, qti_dpm_port_close_resp_msg.resp.error,0);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1( "DPL port closed", 0, 0, 0);

  return QTI_SUCCESS;
}


