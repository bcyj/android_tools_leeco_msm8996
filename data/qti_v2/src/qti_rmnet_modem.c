/******************************************************************************

                        QTI_RMNET_MODEM.C

******************************************************************************/

/******************************************************************************

  @file    qti_rmnet_modem.c
  @brief   Tethering Interface module for RMNET tethering. This file contains
           QTI interaction with modem interconnect for RMNET tethering

  DESCRIPTION
  Implementation file for QTI interaction with modem interconnect for RmNet
  tethering.

  ---------------------------------------------------------------------------
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------


******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        -------------------------------------------------------
01/22/14   sb        Initial version for modem interface for Fusion, LE and MSM.

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
#include <stdint.h>
#include <linux/msm_rmnet.h>

#include "qti.h"
#include "data_common_v01.h"
#include "data_port_mapper_v01.h"
#include "qmi_client_instance_defs.h"



static qti_rmnet_param               * rmnet_state_config;
qmi_client_type                      dpm_notifier;
extern qmi_client_os_params          qti_dpm_os_params;
static  qti_dpl_param                * dpl_state_config;


/*===========================================================================
                          FUNCTION DEFINITIONS

============================================================================*/

/*===========================================================================

FUNCTION QTI_RMNET_MODEM_EVENT_CB()

DESCRIPTION

  This function
  - handles SSR events.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/
static void qti_rmnet_modem_event_cb
(
  qmi_sys_event_type              event_id,
  const qmi_sys_event_info_type   *event_info,
  void                            *user_data
)
{
#ifndef FEATURE_MDM_LE
  int                             ret_val;
  int                             line_state;
/*----------------------------------------------------------------------*/

  LOG_MSG_INFO1("Received QMUXD cb event id %d", event_id, 0, 0);
  if( DS_TARGET_FUSION4_5_PCIE == rmnet_state_config->target )
  {
    if(event_id == QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND)
    {
      LOG_MSG_INFO1("Received QMUXD cb event on conn id %d",
                    event_info->qmi_modem_service_ind.conn_id, 0, 0);

      if(event_info->qmi_modem_service_ind.conn_id ==
         rmnet_state_config->qmux_conn_id)
      {
        ret_val = qti_rmnet_ph_set_modem_state(MODEM_OUT_OF_SERVICE);
        if (ret_val != QTI_SUCCESS)
        {
          LOG_MSG_ERROR("Failed to set modem state on peripheral driver file", 0, 0, 0);
        }

        ret_val = qti_rmnet_data_teardown_bridge();
        if (ret_val != QTI_SUCCESS)
        {
          LOG_MSG_ERROR("Failed to teardown data bridge driver", 0, 0, 0);
        }

        ret_val = qti_rmnet_dfs_release();
        if (ret_val != QTI_SUCCESS)
        {
          LOG_MSG_ERROR("Failed to release DFS client", 0, 0, 0);
        }

        LOG_MSG_INFO1("Completed processing modem out of service ind", 0, 0, 0);
      }
    }
    else if(event_id == QMI_SYS_EVENT_MODEM_IN_SERVICE_IND)
    {
      if(event_info->qmi_modem_service_ind.conn_id ==
         rmnet_state_config->qmux_conn_id)
      {
        ret_val = qti_rmnet_ph_set_modem_state(MODEM_IN_SERVICE);
        if (ret_val != QTI_SUCCESS)
        {
          LOG_MSG_ERROR("Failed to set modem state on peripheral driver file", 0, 0, 0);
        }

        ret_val = ioctl(rmnet_state_config->ph_iface_fd, FRMNET_CTRL_GET_LINE_STATE, &line_state);
        if (ret_val)
        {
          LOG_MSG_INFO1("Couldn't get FRMNET LINE STATE from driver",
                        0, 0, 0);
          return;
        }
        else if (line_state == 0)
        {
          LOG_MSG_INFO1("USB cable not connected. So dont initialize modem components",
                        0, 0, 0);
          return;
        }

        ret_val = qti_rmnet_data_init_bridge();
        if (ret_val != QTI_SUCCESS)
        {
          LOG_MSG_ERROR("Failed to init data bridge driver", 0, 0, 0);
        }

        ret_val = qti_rmnet_dfs_init();
        if (ret_val != QTI_SUCCESS)
        {
          LOG_MSG_ERROR("Failed to initialize DFS client", 0, 0, 0);
        }

        LOG_MSG_INFO1("Completed processing modem in service ind", 0, 0, 0);
      }
    }
    else
    {
      LOG_MSG_INFO1("Ignoring event %d", event_id, 0, 0);
    }
  }
#endif //FEATURE_MDM_LE
}

/*===========================================================================

FUNCTION QTI_RMNET_MODEM_INIT()

DESCRIPTION

  This function
  - initializes RMNET modem state parameters.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/

int qti_rmnet_modem_init
(
  qti_rmnet_param  * rmnet_state,
  qti_sock_thrd_fd_read_f read_f,
  qti_dpl_param    * dpl_state
)
{
  int qmi_err = QMI_NO_ERR;
/*-------------------------------------------------------------------------*/

  if (rmnet_state == NULL)
  {
    LOG_MSG_ERROR("Invalid rmnet state data received", 0, 0, 0);
    return QTI_FAILURE;
  }

  rmnet_state_config = rmnet_state;
  dpl_state_config = dpl_state;
/*------------------------------------------------------------------------
  Initialize a QMUX client and obtain a handle
-------------------------------------------------------------------------*/
  qmi_err = qmi_qmux_if_pwr_up_init_ex(read_f,
                                       qti_rmnet_modem_event_cb,
                                       NULL,
                                       &rmnet_state_config->qti_qmux_qmi_handle,
                                       QMI_QMUX_IF_CLNT_MODE_RAW);

  if (qmi_err != QMI_NO_ERR)
  {
    LOG_MSG_ERROR("Failed to get a QMUX handle for QTI", 0, 0, 0);
    return QTI_FAILURE;
  }
  else
  {
    LOG_MSG_INFO1("Succeeded to get a QMUX handle for QTI", 0, 0, 0);
  }

  return QTI_SUCCESS;
}


/*===========================================================================

FUNCTION QTI_RMNET_GET_QMUX_CONN_ID()

DESCRIPTION

  This function
  - maps to the right QMUXD connection id corresponding to the USB tethered
  data call

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

/*=========================================================================*/
static int qti_rmnet_get_qmux_conn_id
(
  qmi_connection_id_type *qmi_conn_id
)
{

/*-------------------------------------------------------------------------*/
  if(qmi_conn_id == NULL)
  {
    LOG_MSG_ERROR("Invalid pointer for connection id", 0, 0, 0);
    return QTI_FAILURE;
  }

  *qmi_conn_id = rmnet_state_config->qmux_conn_id;
  return QTI_SUCCESS;
}


/*===========================================================================

FUNCTION QTI_RMNET_QMUX_BUF_ALLOC()

DESCRIPTION

  This function
  - allocates buffer to store the QMI QMUX packet

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

/*=========================================================================*/

static int qti_rmnet_qmux_buf_alloc
(
  qti_qmux_buf_s *buf,
  uint32_t        size
)
{
  void *data_ptr;
/*-------------------------------------------------------------------------*/
  if(buf == NULL)
  {
    LOG_MSG_ERROR("Invalid pointer for buf passed", 0, 0, 0);
    return QTI_FAILURE;
  }

/*--------------------------------------------------------------------------
  Protect against memory leaks via successive calls to buf_alloc
-------------------------------------------------------------------------- */
  if (buf->data != NULL)
  {
    LOG_MSG_INFO1("Attempted double-alloc of buffer! Old size %d new size %d",
                  buf->size, size, 0);
    free(buf->data);
    buf->data = NULL;
    buf->size = 0;
  }

/*--------------------------------------------------------------------------
  Allocate data for storing QMI packet
--------------------------------------------------------------------------*/
  data_ptr = malloc(size);
  if (data_ptr == NULL)
  {
    LOG_MSG_ERROR("Could not allocate data_ptr ", 0, 0, 0);
    buf->size = 0;
    return QTI_FAILURE;
  }
  else
  {
    memset(data_ptr, 0, size);
    buf->data = data_ptr;
    buf->size = size;
  }

  return QTI_SUCCESS;
}


/*===========================================================================

FUNCTION QTI_RMNET_MODEM_SEND_MSG()

DESCRIPTION

  This function
  - send QMI message to USB

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/

void qti_rmnet_modem_send_msg
(
   void      *data,
   uint32_t   len
)
{
  int                          ret;
  qti_qmux_buf_s               qmi_qmux_if_buf;
  qmi_connection_id_type       qmi_conn_id;
  qti_qmux_msg_s               *qmux_msg;
  boolean                      ret_val = QTI_FAILURE;
  int                          rc;
  int                          actual_data_size;
/*-----------------------------------------------------------------------*/

  if (data == NULL)
  {
    LOG_MSG_ERROR("Invalid data received", 0, 0, 0);
    return;
  }

  if( (DS_TARGET_FUSION4_5_PCIE == rmnet_state_config->target) ||
      (DS_TARGET_JOLOKIA == rmnet_state_config->target) )
  {
    ret = qti_rmnet_process_qmi_tx_to_modem(data, len );
    if(ret != QTI_SUCCESS)
    {
      LOG_MSG_ERROR("QMI message not parsed correctly", 0, 0, 0);
      return;
    }
  }
  LOG_MSG_INFO1("Will send QMI message of size %d to modem", len, 0 , 0);

  memset(&qmi_qmux_if_buf, 0 , sizeof(qti_qmux_buf_s));

  if (rmnet_state_config->qti_qmux_qmi_handle == 0)
  {
    LOG_MSG_ERROR("Tried to send QMUX message to modem, but qmux handle is invalid",
                   0, 0, 0);
  }
  else if (len < QTI_QMUX_MIN_MSG_LEN_BYTES)
  {
    LOG_MSG_ERROR("Not sending short QMUX message to the modem! Got %d bytes, min %d",
                  len, QTI_QMUX_MIN_MSG_LEN_BYTES, 0);
  }
  else if (qti_rmnet_get_qmux_conn_id(&qmi_conn_id)!= QTI_SUCCESS)
  {
    LOG_MSG_ERROR("Couldn't find connection ID", 0, 0, 0);
  }
  else if (qti_rmnet_qmux_buf_alloc(&qmi_qmux_if_buf,
                                    ((len - QTI_QMUX_HDR_LEN_BYTES) +
                                    QMI_QMUX_IF_MSG_HDR_SIZE)) != QTI_SUCCESS)
  {
    LOG_MSG_ERROR("Couldn't allocate buffer of size %d for qmi_qmux_if",
                  ((len - QTI_QMUX_HDR_LEN_BYTES) + QMI_QMUX_IF_MSG_HDR_SIZE), 0, 0);
  }
  else
  {
    qmux_msg = (qti_qmux_msg_s *)data;
    actual_data_size = len - QTI_QMUX_HDR_LEN_BYTES;
    if (qmux_msg->qmux_hdr.svc_type == QTI_QMUX_SVC_TYPE_QMI_CTL)
    {
/*----------------------------------------------------------------------------
  The qmi_qmux_if APIs expect to be passed a buffer that has
  QMI_QMUX_IF_MSG_HDR_SIZE bytes of free space available in front of the
  buffer containing the SDU.
-----------------------------------------------------------------------------*/
      memcpy(((uint8_t *) qmi_qmux_if_buf.data + QMI_QMUX_IF_MSG_HDR_SIZE),
             (uint8_t *)(data + sizeof(qti_qmux_if_type_t) + sizeof(qti_qmux_hdr_s)),
             actual_data_size);

      rc = qmi_qmux_if_send_raw_qmi_cntl_msg(
                rmnet_state_config->qti_qmux_qmi_handle,
                qmi_conn_id,
                ((unsigned char *) qmi_qmux_if_buf.data + QMI_QMUX_IF_MSG_HDR_SIZE),
                actual_data_size);
    }
/*----------------------------------------------------------------------------
 Send QMI_SDU message
-----------------------------------------------------------------------------*/
    else
    {

/*----------------------------------------------------------------------------
  The qmi_qmux_if APIs expect to be passed a buffer that has
  QMI_QMUX_IF_MSG_HDR_SIZE bytes of free space available in front of the
  buffer containing the SDU.
-----------------------------------------------------------------------------*/
       memcpy(((uint8_t *) qmi_qmux_if_buf.data + QMI_QMUX_IF_MSG_HDR_SIZE),
              (uint8_t *)(data + sizeof(qti_qmux_if_type_t) + sizeof(qti_qmux_hdr_s)),
               actual_data_size);

       rc = qmi_qmux_if_send_qmi_msg(
               rmnet_state_config->qti_qmux_qmi_handle,
               qmi_conn_id,
               (qmi_service_id_type) qmux_msg->qmux_hdr.svc_type,
               (qmi_client_id_type) qmux_msg->qmux_hdr.client_id,
               ((unsigned char *) qmi_qmux_if_buf.data + QMI_QMUX_IF_MSG_HDR_SIZE),
               actual_data_size);
    }

    if (rc != QMI_NO_ERR)
    {
      LOG_MSG_ERROR("Error %d when sending QMI message for svc %d",
                     rc, qmux_msg->qmux_hdr.svc_type, 0);
    }
    else
    {
      LOG_MSG_INFO1("Successfully sent QMI message to modem", 0, 0, 0);
      ret_val = QTI_SUCCESS;
    }

    free(qmi_qmux_if_buf.data);
    qmi_qmux_if_buf.data = NULL;
    qmi_qmux_if_buf.size = 0;
  }

  return;

}

/*===========================================================================

FUNCTION QTI_RMNET_MODEM_RECV_MSG()

DESCRIPTION

  This function
  - receives QMI messages from modem

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/


void qti_rmnet_modem_recv_msg
(
  qmi_connection_id_type conn_id,
  qmi_service_id_type    service_id,
  qmi_client_id_type     client_id,
  unsigned char          control_flags,
  unsigned char         *rx_msg,
  int                    rx_msg_len
)
{
  int ret;
  int                    ret_val;
  qti_qmux_buf_s         buf;
  qti_qmux_msg_s         *qmux_msg;


 /*-------------------------------------------------------------------------*/

  if (rx_msg == NULL)
  {
    LOG_MSG_ERROR("Invalid RX data received", 0, 0, 0);
    return;
  }

  LOG_MSG_INFO1("Received QMUX data buffer with size %d on connection ID %d",
                 rx_msg_len, conn_id, 0);

  if (service_id == QMI_CTL_SERVICE &&
      rx_msg_len < sizeof(qti_qmux_qmi_ctl_sdu_s))
  {
    LOG_MSG_ERROR("Received short QMI payload: %d bytes (minimum %d)",
                   rx_msg_len, sizeof(qti_qmux_qmi_ctl_sdu_s), 0);
    return;
  }
  else if (service_id != QMI_CTL_SERVICE &&
           rx_msg_len < sizeof(qti_qmux_sdu_s))
  {
    LOG_MSG_ERROR("Received short regular QMUX payload: %d bytes (minimum %d)",
                   rx_msg_len, sizeof(qti_qmux_sdu_s), 0);
    return;
  }
  else if ( conn_id != rmnet_state_config->qmux_conn_id)
  {
    LOG_MSG_ERROR("Received message on invalid connection id %d",
                   conn_id, 0, 0);
    return;
  }
  else
  {
    memset(&buf, 0 , sizeof(qti_qmux_buf_s));
    if (qti_rmnet_qmux_buf_alloc(&buf, (rx_msg_len + QTI_QMUX_HDR_LEN_BYTES)) != QTI_SUCCESS)
    {
      LOG_MSG_ERROR("Couldn't allocate buffer of size %d bytes for QMUX response!",
                    (rx_msg_len + QTI_QMUX_HDR_LEN_BYTES), 0, 0);
    }
    else
    {

/*----------------------------------------------------------------------------
  Add the QMUX header
---------------------------------------------------------------------------- */
      qmux_msg = (qti_qmux_msg_s *) buf.data;
      qmux_msg->if_type = QTI_QMUX_IF_TYPE_QMUX;
      qmux_msg->qmux_hdr.length    = (rx_msg_len + sizeof(qti_qmux_hdr_s));
      qmux_msg->qmux_hdr.ctl_flags = (uint8) control_flags;
      qmux_msg->qmux_hdr.svc_type  = (uint8) service_id;
      qmux_msg->qmux_hdr.client_id = (uint8) client_id;
      memcpy(&(qmux_msg->sdu), rx_msg, rx_msg_len);

      if((DS_TARGET_FUSION4_5_PCIE == rmnet_state_config->target ) ||
         (DS_TARGET_JOLOKIA == rmnet_state_config->target ))
      {
        ret_val = qti_rmnet_process_qmi_rx_from_modem(
                           (qti_qmux_msg_s *)qmux_msg,
                           (rx_msg_len + QTI_QMUX_HDR_LEN_BYTES));
        if(ret_val != QTI_SUCCESS)
        {
          LOG_MSG_ERROR("QMI message not parsed correctly", 0, 0, 0);
        }
      }
/*----------------------------------------------------------------------------
  Write QMI message into peripheral device file
---------------------------------------------------------------------------- */
      qti_rmnet_ph_send_msg((qti_qmux_msg_s *)qmux_msg,
                            (rx_msg_len + QTI_QMUX_HDR_LEN_BYTES));

      free(buf.data);
      buf.data = NULL;
      buf.size = 0;
    }

    return;
  }
}




/*===========================================================================

FUNCTION QTI_RMNET_MODEM_SET_DTR()

DESCRIPTION

  This function
  - toggles DTR.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/
int qti_rmnet_modem_set_dtr(uint8_t set)
{
  int             fd = 0;
  int             dtr_sig;
  int             ret = QTI_FAILURE;
/*------------------------------------------------------------------------*/


  fd = open(rmnet_state_config->modem_iface_device_file, O_RDWR);

  if (fd < 0)
  {
    LOG_MSG_ERROR("Opening the device file failed. errno %d", errno, 0, 0);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1("Request to set DTR %d", set, 0, 0);

/*-------------------------------------------------------------------------
  Set DTR high
-------------------------------------------------------------------------*/
  if (set == SET_DTR_HIGH)
  {
    dtr_sig = 0;
    dtr_sig |= ioctl(fd, TIOCMGET, &dtr_sig);
    if( dtr_sig >= 0  && !(dtr_sig & TIOCM_DTR))
    {
      LOG_MSG_INFO1("DTR bit not set dtr_sig:%d ",
                    dtr_sig, 0, 0);

      dtr_sig |= TIOCM_DTR;

      LOG_MSG_INFO1("DTR bit will be set to dtr_sig:%d ",
                    dtr_sig, 0, 0);

      if((ioctl(fd, TIOCMSET, (void *)&dtr_sig)) == -1)
      {
        LOG_MSG_ERROR("Ioctl call to set DTR bit failed. DTR %d",
                      dtr_sig, 0, 0);
      }
      else
      {
        dtr_sig = 0;
        dtr_sig |= ioctl(fd, TIOCMGET, &dtr_sig);
        if( dtr_sig >= 0  && (dtr_sig & TIOCM_DTR))
        {
          LOG_MSG_INFO1("DTR bit set:%d", dtr_sig, 0, 0);
          ret=QTI_SUCCESS;
        }
        else
        {
          LOG_MSG_ERROR("Unable to get DTR bit after set %d", dtr_sig, 0, 0);
        }
      }
    }
    else if (dtr_sig >= 0  && (dtr_sig & TIOCM_DTR))
    {
      LOG_MSG_INFO1("DTR high already set, dtr_sig %d", dtr_sig, 0, 0);
      ret=QTI_SUCCESS;
    }
    else if (dtr_sig == -1)
    {
      LOG_MSG_ERROR("Failed to get DTR bits,exiting. DTR %d",
                     dtr_sig, 0, 0);
    }
  }
/*-------------------------------------------------------------------------
  Set DTR low
-------------------------------------------------------------------------*/
  else if (set == SET_DTR_LOW)
  {
    dtr_sig = 0;
    dtr_sig |=  ioctl(fd, TIOCMGET, &dtr_sig);
    if( dtr_sig >= 0 && (dtr_sig & TIOCM_DTR))
    {
      LOG_MSG_INFO1("Got DTR bits. DTR %d", dtr_sig, 0, 0);

      dtr_sig &= (~TIOCM_DTR);

      LOG_MSG_INFO1("Will clear DTR bit. DTR %d", dtr_sig, 0, 0);

      if(ioctl(fd, TIOCMSET, (void *)&dtr_sig) == -1)
      {
        LOG_MSG_ERROR("Reset DTR bit failed. DTR %d",
                      dtr_sig, 0, 0);
      }
      else
      {
        dtr_sig = 0;
        dtr_sig |= ioctl(fd, TIOCMGET, &dtr_sig);
        if( dtr_sig >= 0  && !(dtr_sig & TIOCM_DTR))
        {
          LOG_MSG_INFO1("Successfully reset DTR bit. DTR %d", dtr_sig, 0, 0);
          ret = QTI_SUCCESS;
        }
        else
        {
          LOG_MSG_ERROR("Unable to Clear DTR bit. DTR %d", dtr_sig, 0, 0);
        }
      }
    }
    else if (dtr_sig >= 0 && !(dtr_sig & TIOCM_DTR))
    {
      LOG_MSG_INFO1("DTR bit already reset. DTR %d", dtr_sig, 0, 0);
      ret = QTI_SUCCESS;
    }
    else if( dtr_sig == -1)
    {
      LOG_MSG_ERROR("Failed to get DTR bits, exiting. DTR %d",
                     dtr_sig, 0, 0);
    }
  }

  if(fd > 0)
  {
    close(fd);
    fd = 0;
  }

  return ret;
}


/*===========================================================================

FUNCTION QTI_RMNET_MODEM_GET_EP_ID()

DESCRIPTION
  - obtains PCIe EP ID

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

SIDE EFFECTS
  None

==========================================================================*/

int qti_rmnet_modem_get_ep_id
(
  uint32_t *ep_id
)
{
  int fd;
  int rval = QTI_FAILURE;
  struct ifreq ifr;
  struct rmnet_ioctl_extended_s *ext_ioctl_arg;

/*-----------------------------------------------------------------------*/
  if(ep_id == NULL)
  {
    LOG_MSG_ERROR("Got invalid EP ID", 0, 0, 0);
    return QTI_FAILURE;
  }

  /* Open a datagram socket to use for issuing the ioctl */
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    LOG_MSG_ERROR("get_epid: socket failed", 0, 0, 0);
    goto error;
  }

  /* Allocate the extended IOCTL argument */
  ext_ioctl_arg = (void *) malloc (sizeof(struct rmnet_ioctl_extended_s));
  if (NULL == ext_ioctl_arg)
  {
    LOG_MSG_ERROR("get_epid: malloc for ifru_data failed", 0, 0, 0);
    close(fd);
    goto error;
  }

  /* Initialize the ioctl req struct */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name,
              rmnet_state_config->modem_data_iface_name,
              strlen(MHI_DATA_INTERFACE)+1);
  ifr.ifr_ifru.ifru_data = ext_ioctl_arg;
  memset(ext_ioctl_arg, 0, sizeof(struct rmnet_ioctl_extended_s));
  ext_ioctl_arg->extended_ioctl = RMNET_IOCTL_GET_EPID;

  /* Get current if flags for the device */
  if (ioctl(fd, RMNET_IOCTL_EXTENDED, &ifr) < 0)
  {
    LOG_MSG_ERROR("get_epid: ioctl RMNET_IOCTL_GET_EPID failed err %d", errno, 0, 0);
    close(fd);
    free(ext_ioctl_arg);
    goto error;
  }

  *ep_id = ext_ioctl_arg->u.data;
  LOG_MSG_INFO1("get_epid: ioctl RMNET_IOCTL_GET_EPID as %d",
                ext_ioctl_arg->u.data, 0, 0);

  /* close temporary socket */
  close(fd);
  free(ext_ioctl_arg);
  rval = QTI_SUCCESS;

error:
  return rval;
}

/*===========================================================================

FUNCTION QTI_RMNET_MODEM_SET_MTU()

DESCRIPTION
- sets MTU on modem interface

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

SIDE EFFECTS
  None

==========================================================================*/
int qti_rmnet_modem_set_mtu
(
  uint32_t mtu
)
{
  int fd;
  int rval = QTI_FAILURE;
  struct ifreq ifr;
/*-----------------------------------------------------------------------*/
  /* Open a datagram socket to use for issuing the ioctl */
  if( (fd = socket( AF_INET, SOCK_DGRAM, 0 )) < 0 )
  {
    LOG_MSG_ERROR("ifioctl_set: socket failed", 0, 0, 0);
    goto error;
  }

  /* Initialize the ioctl req struct to null */
  memset(&ifr, 0, sizeof(ifr));

  /* Set device name in ioctl req struct */
  strlcpy(ifr.ifr_name,
              rmnet_state_config->modem_data_iface_name,
              strlen(MHI_DATA_INTERFACE)+1);

  ifr.ifr_data = (void*)(intptr_t)mtu;

  /* Set if MTU for the device */
  if( ioctl( fd, SIOCSIFMTU, &ifr ) < 0 )
  {
    LOG_MSG_ERROR("ifioctl_set: SIOCSIFMTU ioctl failed err %d", errno, 0, 0);
    close(fd);
    goto error;
  }

  /* Close temporary socket */
  close(fd);

  rval = QTI_SUCCESS;

  error:
  return rval;

}


/*===========================================================================

FUNCTION QTI_RMNET_MODEM_SET_MRU()

DESCRIPTION
  - sets MRU on modem interface

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

SIDE EFFECTS
  None

==========================================================================*/
int qti_rmnet_modem_set_mru
(
  uint32_t mru
)
{
  int fd;
  int rval = QTI_FAILURE;
  struct ifreq ifr;
  struct rmnet_ioctl_extended_s *ext_ioctl_arg;
/*-----------------------------------------------------------------------*/

  /* Open a datagram socket to use for issuing the ioctl */
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    LOG_MSG_ERROR("set_mtu: socket failed", 0, 0, 0);
    goto error;
  }

  /* Allocate the extended IOCTL argument */
  ext_ioctl_arg = (void *) malloc (sizeof(struct rmnet_ioctl_extended_s));
  if (NULL == ext_ioctl_arg)
  {
    LOG_MSG_ERROR("set_mru: malloc for ifru_data failed", 0, 0, 0);
    close(fd);
    goto error;
  }


  /* Initialize the ioctl req struct */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name,
              rmnet_state_config->modem_data_iface_name,
              strlen(MHI_DATA_INTERFACE)+1);
  ifr.ifr_ifru.ifru_data = ext_ioctl_arg;
  memset(ext_ioctl_arg, 0, sizeof(struct rmnet_ioctl_extended_s));
  ext_ioctl_arg->extended_ioctl = RMNET_IOCTL_SET_MRU;
  ext_ioctl_arg->u.data = mru;

  /* Get current if flags for the device */
  if (ioctl(fd, RMNET_IOCTL_EXTENDED, &ifr) < 0)
  {
    LOG_MSG_ERROR("set_mru: ioctl RMNET_IOCTL_SET_MRU failed err %d", errno, 0, 0);
    close(fd);
    free(ext_ioctl_arg);
    goto error;
  }

  LOG_MSG_INFO1("set_mru: ioctl RMNET_IOCTL_SET_MRU as %d",
                ext_ioctl_arg->u.data, 0, 0);

  /* close temporary socket */
  close(fd);
  free(ext_ioctl_arg);
  rval = QTI_SUCCESS;

error:
  return rval;
}

/*===========================================================================

FUNCTION QTI_RMNET_MODEM_CHANGE_SLEEP_STATE()

DESCRIPTION
  - sets/resets sleep state on MHI interface

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

SIDE EFFECTS
  None

==========================================================================*/
int qti_rmnet_modem_change_sleep_state
(
  boolean sleep_state
)
{
  int fd;
  int rval = QTI_FAILURE;
  struct ifreq ifr;
  struct rmnet_ioctl_extended_s *ext_ioctl_arg;
/*-----------------------------------------------------------------------*/

  /* Open a datagram socket to use for issuing the ioctl */
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    LOG_MSG_ERROR("change_sleep_state: socket failed", 0, 0, 0);
    goto error;
  }

  /* Allocate the extended IOCTL argument */
  ext_ioctl_arg = (void *) malloc (sizeof(struct rmnet_ioctl_extended_s));
  if (NULL == ext_ioctl_arg)
  {
    LOG_MSG_ERROR("change_sleep_state: malloc for ifru_data failed", 0, 0, 0);
    close(fd);
    goto error;
  }

  /* Initialize the ioctl req struct */
  memset(&ifr, 0, sizeof(ifr));
  strlcpy(ifr.ifr_name,
          rmnet_state_config->modem_data_iface_name,
          strlen(MHI_DATA_INTERFACE)+1);
  ifr.ifr_ifru.ifru_data = ext_ioctl_arg;
  memset(ext_ioctl_arg, 0, sizeof(struct rmnet_ioctl_extended_s));
  ext_ioctl_arg->extended_ioctl = RMNET_IOCTL_SET_SLEEP_STATE;
  ext_ioctl_arg->u.data = sleep_state;

  /* Get current if flags for the device */
  if (ioctl(fd, RMNET_IOCTL_EXTENDED, &ifr) < 0)
  {
    LOG_MSG_ERROR("change_sleep_state: ioctl for change sleep state failed err %d",
                  errno, 0, 0);
    close(fd);
    free(ext_ioctl_arg);
    goto error;
  }

  LOG_MSG_INFO1("change_sleep_state: ioctl set sleep state as %d",
                sleep_state, 0, 0);

  /* close temporary socket */
  close(fd);
  free(ext_ioctl_arg);
  rval = QTI_SUCCESS;

error:
  return rval;
}

/*===========================================================================

FUNCTION QTI_RMNET_MODEM_NOT_IN_SERVICE()

DESCRIPTION
  Function invoked during SSR functionality, when modem is not in service.

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

==========================================================================*/
void qti_rmnet_modem_not_in_service()
{
  int        ret_val;
  qmi_idl_service_object_type dpm_svc_obj;

#ifndef FEATURE_MDM_LE

  if( (DS_TARGET_MSM8994 == rmnet_state_config->target) ||
      (DS_TARGET_JOLOKIA == rmnet_state_config->target) )
  {

    dpm_svc_obj = dpm_get_service_object_v01();
    if( dpm_svc_obj == NULL )
    {
      LOG_MSG_ERROR("Failed to get dpm service object", 0, 0, 0);
      return;
    }

    ret_val = qti_rmnet_ph_set_modem_state(MODEM_OUT_OF_SERVICE);
    if (ret_val != QTI_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to set modem state on peripheral driver file", 0, 0, 0);
    }

    ret_val = qti_rmnet_dpm_release();
    if (ret_val != QTI_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to release DPM client", 0, 0, 0);
    }

/*----------------------------------------------------------------------------
    Register icallback to get notified when modem is in service
---------------------------------------------------------------------------- */
    (void) qmi_client_notifier_init(dpm_svc_obj, &qti_dpm_os_params, &dpm_notifier);
    (void) qmi_client_register_notify_cb(dpm_notifier,dpm_notify_cb , NULL);

    LOG_MSG_INFO1("Completed processing modem out of service ind", 0, 0, 0);
  }

#endif
}

/*===========================================================================

FUNCTION QTI_RMNET_MODEM_IN_SERVICE()

DESCRIPTION
  Function is used during SSR functionality, when modem is back in service.

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

==========================================================================*/

void qti_rmnet_modem_in_service()
{
  int                        ret_val;
  int                        line_state;
  qmi_client_error_type      qmi_error;

#ifndef FEATURE_MDM_LE

  if( (DS_TARGET_MSM8994 == rmnet_state_config->target) ||
      (DS_TARGET_JOLOKIA == rmnet_state_config->target) )
  {
    ret_val = qti_rmnet_ph_set_modem_state(MODEM_IN_SERVICE);
    if (ret_val != QTI_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to set modem state on peripheral driver file", 0, 0, 0);
    }

    qmi_error = qmi_client_release(dpm_notifier);
    if (qmi_error != QMI_NO_ERR)
    {
      LOG_MSG_ERROR("Can not release DFS notifier client %d", qmi_error, 0, 0);
      return;
    }

    ret_val =  qti_dpm_init(rmnet_state_config,dpl_state_config);
    if (ret_val != QTI_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to initialize DPM client", 0, 0, 0);
    }

    ret_val = ioctl(rmnet_state_config->ph_iface_fd, FRMNET_CTRL_GET_LINE_STATE, &line_state);
    if (ret_val)
    {
      LOG_MSG_INFO1("Couldn't get FRMNET LINE STATE from driver",
                     0, 0, 0);
      return;
    }
    else if (line_state == 0)
    {
      LOG_MSG_INFO1("USB cable not connected. So dont initialize modem components",
                    0, 0, 0);
      return;
    }
    else if (line_state == 1)
    {
      ret_val = qti_rmnet_dpm_port_open();
      if (ret_val != QTI_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to open SMD port through DPM", 0, 0, 0);
      }
    }

    ret_val = ioctl(dpl_state_config->dpl_iface_fd, FRMNET_CTRL_GET_LINE_STATE, &line_state);
    if (ret_val)
    {
      LOG_MSG_INFO1("Couldn't get LINE STATE from DPL driver",
                     0, 0, 0);
      return;
    }
    else if (line_state == 0)
    {
      LOG_MSG_INFO1("USB DPL cable not connected. So dont initialize DPL components",
                    0, 0, 0);
      return;
    }
    else if (line_state == 1)
    {
      ret_val = qti_dpl_dpm_port_open();
      if (ret_val != QTI_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to open DPL port through DPM", 0, 0, 0);
      }
    }

    LOG_MSG_INFO1("Completed processing modem in service ind", 0, 0, 0);
  }

#endif
}
