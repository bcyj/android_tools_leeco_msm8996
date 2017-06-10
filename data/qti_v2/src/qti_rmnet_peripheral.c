/******************************************************************************

                        QTI_RMNET_PERIPHERAL.C

******************************************************************************/

/******************************************************************************

  @file    qti_rmnet_peripheral.c
  @brief   Tethering Interface module for RMNET tethering. This file contains
           QTI interaction with perifpheral for RMNET tethering

  ---------------------------------------------------------------------------
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------


******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        -------------------------------------------------------
01/22/14   sb        Initial version for Fusion, LE and MSM.

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
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/msm_rmnet.h>

#include "qti.h"


static qti_rmnet_param        * rmnet_state_config;
static qti_dpl_param          * dpl_state_config;

/*===========================================================================
                          FUNCTION DEFINITIONS
============================================================================*/
/*===========================================================================

FUNCTION PRINT_BUFFER()

DESCRIPTION

  This function
  - prints QMI message.

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

=========================================================================*/
void print_buffer
(
  char    *buf,
  int      size
)
{
  int i;

  for(i=0; i < size; i++)
  {
    if(i%8 == 0)
      printf("\n%02X ", buf[i]);
    else
      printf("%02X ", buf[i]);
  }
  printf("\n");
}

#ifdef FEATURE_MDM_LE
/*===========================================================================

FUNCTION QTI_RMNET_SEND_LINE_STATE()

DESCRIPTION
  - send line state to qti_ppp

DEPENDENCIES
  None.

RETURN VALUE
  None

SIDE EFFECTS
  None

=========================================================================*/
static void qti_rmnet_send_line_state(int line_state)
{
  int sockfd, len, numBytes;
  struct sockaddr_un sta_qti_ppp;

  //create socket
  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
  {
    LOG_MSG_ERROR("Error creating socket, errno: %d", errno, 0, 0);
    return;
  }
  if (fcntl(sockfd, F_SETFD, FD_CLOEXEC) < 0)
    LOG_MSG_ERROR("Couldn't set Close on Exec, errno: %d", errno, 0, 0);

  /* Send the event to QMI_IP server socket. */
  sta_qti_ppp.sun_family = AF_UNIX;
  strlcpy(sta_qti_ppp.sun_path, QTI_PPP_LS_FILE, sizeof(QTI_PPP_LS_FILE));
  len = strlen(sta_qti_ppp.sun_path) + sizeof(sta_qti_ppp.sun_family);

  if ((numBytes = sendto(sockfd, &line_state, sizeof(line_state), 0,
                         (struct sockaddr *)&sta_qti_ppp, len)) == -1)
    LOG_MSG_ERROR("Send Failed from sta interface context", 0, 0, 0);

  close(sockfd);

}
#endif /* FEATURE_MDM_LE */
/*===========================================================================

FUNCTION QTI_RMNET_PROCESS_PH_RESET()

DESCRIPTION
  - processes peripheral reset
  - upon USB cable plug-in : setup RmNet data driver, DPM open,
                             init modem interface, set dtr high
  - upon USB cable plug-out : set dtr low, close modem interface,
                              teardown RmNet data driver, DPM close.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/
static int qti_rmnet_process_ph_reset()
{
  int                             ret_val;
  int                             ret;
  int                             line_state;

/*-------------------------------------------------------------------------*/

    ret = ioctl(rmnet_state_config->ph_iface_fd, FRMNET_CTRL_GET_LINE_STATE, &line_state);
    if (ret)
    {
      LOG_MSG_INFO1("Couldn't get FRMNET LINE STATE from driver",
                     0, 0, 0);
      line_state = -1;
      return QTI_FAILURE;
    }
    else
    {
#ifdef FEATURE_MDM_LE
      /* Send line state to qti_ppp */
      qti_rmnet_send_line_state(line_state);
#endif /* FEATURE_MDM_LE */
/*----------------------------------------------------------------------------
  peripheral cable plug in happened
----------------------------------------------------------------------------*/
      if(line_state == 1)
      {
        if(!rmnet_state_config->dtr_enabled)
        {

          if( DS_TARGET_FUSION4_5_PCIE == rmnet_state_config->target )
          {
            ret_val = qti_rmnet_data_init_bridge();
            if (ret_val != QTI_SUCCESS)
            {
              LOG_MSG_ERROR("Failed to initialize RmNet DATA", 0, 0, 0);
              return QTI_FAILURE;
            }

            ret_val = qti_rmnet_dfs_init();
            if (ret_val != QTI_SUCCESS)
            {
              LOG_MSG_ERROR("Failed to initialize DFS client", 0, 0, 0);
              ds_system_call("echo QTI:DFS ind registration failed > /dev/kmsg",
                             strlen("echo QTI:DFS ind registration failed > /dev/kmsg"));
            }
            else
            {
              ds_system_call("echo QTI:DFS ind registration succeeded > /dev/kmsg",
                           strlen("echo QTI:DFS ind registration succeeded > /dev/kmsg"));
            }
          }
          else
          {
            ret_val = qti_rmnet_dpm_port_open();
            if (ret_val != QTI_SUCCESS)
            {
              LOG_MSG_ERROR("Failed to open SMD port through DPM", 0, 0, 0);
              return QTI_FAILURE;
            }
          }

          if(qti_rmnet_modem_set_dtr(SET_DTR_HIGH)<0)
          {
            LOG_MSG_ERROR("QTI failed to signal DTR high ",
                           0, 0, 0);
          }
          else
          {
            LOG_MSG_INFO1("QTI succeeded to signal DTR high",
                           0, 0, 0);
          }

          rmnet_state_config->dtr_enabled = 1;
        }
      }
/*----------------------------------------------------------------------------
  ph cable plug out happened
----------------------------------------------------------------------------*/
      else if(line_state == 0)
      {
        if(rmnet_state_config->dtr_enabled)
        {

          if(qti_rmnet_modem_set_dtr(SET_DTR_LOW)<0)
          {
            LOG_MSG_ERROR("QTI failed to signal DTR low",
                           0, 0, 0);
          }
          else
          {
            LOG_MSG_INFO1("QTI succeeded to signal DTR low",
                          0, 0, 0);
          }

          if( DS_TARGET_FUSION4_5_PCIE == rmnet_state_config->target )
          {
            ret_val = qti_rmnet_data_teardown_bridge();
            if (ret_val != QTI_SUCCESS)
            {
              LOG_MSG_ERROR("Failed to reset RmNet data", 0, 0, 0);
            }

            ret_val = qti_rmnet_dfs_release();
            if (ret_val != QTI_SUCCESS)
            {
              LOG_MSG_ERROR("Failed to release DFS client", 0, 0, 0);
            }
          }
          else
          {
            ret_val = qti_rmnet_dpm_port_close();
            if (ret_val != QTI_SUCCESS)
            {
              LOG_MSG_ERROR("Failed to close SMD port through DPM", 0, 0, 0);
            }
          }

          rmnet_state_config->dtr_enabled = 0;
        }
      }
    }

    return QTI_SUCCESS;
}


/*===========================================================================

FUNCTION QTI_RMNET_PH_RECV_MSG()

DESCRIPTION

  This function
  - receives QMI messages from peripheral interface

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

=========================================================================*/


int qti_rmnet_ph_recv_msg
(
   int rmnet_ph_fd
)
{
  int        ret;
  int        ret_val;
  char       ph_rx_buf[RMNET_MAX_TRANSFER_SIZE];


 /*-------------------------------------------------------------------------*/

  memset(ph_rx_buf, 0, sizeof(ph_rx_buf));

  ret = read(rmnet_ph_fd, ph_rx_buf, RMNET_MAX_TRANSFER_SIZE);

  LOG_MSG_INFO1("Read %d data from peripheral device file", ret, 0, 0);

  if (ret < 0)
  {
    LOG_MSG_ERROR("Failed to read from the dev file. Errno %d",
                   errno, 0, 0);
    return QTI_FAILURE;
  }
  else if (ret > RMNET_MAX_TRANSFER_SIZE)
  {
    LOG_MSG_ERROR("Received data larger than dest buff %d",
                   errno, 0, 0);
    return QTI_FAILURE;
  }
/*----------------------------------------------------------------------------
  Peripheral cable plug out/ plug in happened
----------------------------------------------------------------------------*/
  else if (ret == 0)
  {
    qti_rmnet_process_ph_reset();
  }
  else
  {
/*----------------------------------------------------------------------------
  If ph fails to write zero length data upon ph connect, validation check
  to make sure ph reset is processed before sending any QMI message to
  modem.
----------------------------------------------------------------------------*/
    if(!rmnet_state_config->dtr_enabled)
    {
      ret_val = qti_rmnet_process_ph_reset();
      if (ret_val != QTI_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to process peripheral reset", 0, 0, 0);
        return QTI_FAILURE;
      }
    }

/*----------------------------------------------------------------------------
  Process and send QMI message to modem
----------------------------------------------------------------------------*/
    qti_rmnet_modem_send_msg(&ph_rx_buf, (uint32) ret);
  }

  return QTI_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_PH_FILE_OPEN()

DESCRIPTION

  This function
  - opens the device file which is used for interfacing with peripheral

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

=========================================================================*/


static int qti_ph_file_open
(
   char * rmnet_ph_dev_file,
   int  * ph_fd
)
{
  int qti_rmnet_fd;
/*--------------------------------------------------------------------------*/
  qti_rmnet_fd = open(rmnet_ph_dev_file, O_RDWR);
  if(qti_rmnet_fd == -1)
  {
    LOG_MSG_ERROR("Could not open device file. Errno %d",
                   errno, 0, 0);
    return QTI_FAILURE;
  }
  else
  {
    LOG_MSG_INFO1("Successfully opened peripheral device file. FD is %d",
                   qti_rmnet_fd, 0, 0);
    *ph_fd = qti_rmnet_fd;
    return QTI_SUCCESS;
  }
}

/*===========================================================================

FUNCTION QTI_MAP_FD_READ()

DESCRIPTION

  This function
  - adds the peripheral fd to the list of FD on which select call listens
  - maps the read function for the peripheral fd

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

=========================================================================*/

static int qti_map_fd_read
(
   qti_sk_fd_set_info_t    *sk_fd_set,
   int                     ph_fd,
   qti_sock_thrd_fd_read_f read_f
)
{
  if( sk_fd_set->num_fd < MAX_NUM_OF_FD )
  {
    FD_SET(ph_fd, &(sk_fd_set->fdset));
/*--------------------------------------------------------------------------
  Add fd to fdmap array and store read handler function ptr
-------------------------------------------------------------------------- */
    sk_fd_set->sk_fds[sk_fd_set->num_fd].sk_fd = ph_fd;
    sk_fd_set->sk_fds[sk_fd_set->num_fd].read_func = read_f;
    LOG_MSG_INFO1("Added read function for fd %d", ph_fd, 0, 0);

/*--------------------------------------------------------------------------
  Increment number of fds stored in fdmap
--------------------------------------------------------------------------*/
    sk_fd_set->num_fd++;
    if(sk_fd_set->max_fd < ph_fd)
    {
      LOG_MSG_INFO1("Updating ph max fd %d", ph_fd, 0, 0);
      sk_fd_set->max_fd = ph_fd;
    }
  }
  else
  {
    LOG_MSG_ERROR("Exceeds maximum num of FD", 0, 0, 0);
    return QTI_FAILURE;
  }

  return QTI_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_RMNET_PH_INIT()

DESCRIPTION

  This function
  - opens the ph device file
  - adds the ph fd to wait on select call

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/

int qti_rmnet_ph_init
(
  qti_rmnet_param  * rmnet_state,
  qti_sk_fd_set_info_t    * fd_set,
  qti_sock_thrd_fd_read_f read_f
)
{
  int ret_val;
/*-------------------------------------------------------------------------*/
  LOG_MSG_INFO1("Open peripheral file to receive QMI messages", 0, 0, 0);
  rmnet_state_config = rmnet_state;

  ret_val = qti_ph_file_open(rmnet_state->ph_iface_device_file,
                                    &(rmnet_state->ph_iface_fd));

  if(ret_val == QTI_FAILURE)
  {
    LOG_MSG_ERROR("Failed to open peripheral device file. Abort. Error %d",
                   errno, 0, 0);
    return QTI_FAILURE;
  }
  else
  {
    LOG_MSG_INFO1("Opened file's fd is %d", rmnet_state->ph_iface_fd, 0, 0);
    ret_val = qti_map_fd_read(fd_set,rmnet_state->ph_iface_fd, read_f);
    if(ret_val == QTI_FAILURE)
    {
      LOG_MSG_ERROR("Failed to map fd with the read function",
                     0, 0, 0);
      close(rmnet_state->ph_iface_fd);
      return QTI_FAILURE;
    }
  }

  return QTI_SUCCESS;
}


/*===========================================================================

FUNCTION QTI_RMNET_PH_SEND_MSG()

DESCRIPTION

  This function
  - send QMI message to peripheral

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/

void qti_rmnet_ph_send_msg
(
   void      *data,
   uint32_t   len
)
{
  int ret;
/*-----------------------------------------------------------------------*/

  ret = write(rmnet_state_config->ph_iface_fd, (char*)data, len);
  if (ret == -1)
  {
    LOG_MSG_ERROR("Couldn't send message to host. Errno %d", errno, 0, 0);
  }
  else if (ret != len)
  {
    LOG_MSG_ERROR_6("Unexpected return value when writing to device file: got %d,"
                    "expected %d",
                    ret,
                    len,
                    0, 0, 0, 0);
  }
  else
  {
    LOG_MSG_INFO1("Successfully sent message to host size %d on fd %d",
                  ret, rmnet_state_config->ph_iface_fd, 0);
  }
}

/*===========================================================================

FUNCTION QTI_RMNET_PH_GET_EP_INFO()

DESCRIPTION
  Gets the ph, IPA EP information from ph

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/

int qti_rmnet_ph_get_ep_info()
{

  int ret_val= QTI_SUCCESS ;
  int retry_count = 0;

/*-------------------------------------------------------------------------
  peripheral(USB) needs to wait till pipes(IPA-USB) get connected to get the required
  IPA pipe info. This can take sometime and if we call IOCTL before peripheral
  gets the IPA pipe info, the IOCTL can fail. So we retry for 5 sec to allow
  peripheral to get the IPA pipe info.
--------------------------------------------------------------------------*/
  while (retry_count < QTI_USB_RETRY)
  {
    ret_val = ioctl(rmnet_state_config->ph_iface_fd,
                    FRMNET_CTRL_EP_LOOKUP,
                    &(rmnet_state_config->ep_info));
    if(ret_val == QTI_SUCCESS)
      break;

    sleep(ONE_SEC);
    retry_count++;
    LOG_MSG_INFO1("Retrying to get EP info", 0, 0, 0);
  }

  if(retry_count == QTI_USB_RETRY)
  {
    LOG_MSG_ERROR("Unable to get EP info", 0, 0, 0);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1_6("Peripheral type %d, peripheral id %d, ipa consumer EP %d, ipa producer EP %d \n",
                  rmnet_state_config->ep_info.ph_ep_info.ep_type,
                  rmnet_state_config->ep_info.ph_ep_info.peripheral_iface_id,
                  rmnet_state_config->ep_info.ipa_ep_pair.consumer_pipe_num,
                  rmnet_state_config->ep_info.ipa_ep_pair.producer_pipe_num, 0, 0);

  return QTI_SUCCESS;
}


/*===========================================================================

FUNCTION QTI_RMNET_PH_SET_MTU()

DESCRIPTION
  - sets the MTU on the peripheral interface

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

SIDE EFFECTS
  None

==========================================================================*/
int qti_rmnet_ph_set_mtu
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
              rmnet_state_config->ph_data_iface_name,
              strlen(USB_DATA_INTERFACE)+1);

  ifr.ifr_data = (void*)(intptr_t)mtu;

  /* Set if MTU for the device */
  if( ioctl( fd, SIOCSIFMTU, &ifr ) < 0 )
  {
    LOG_MSG_ERROR("ifioctl_set: SIOCSIFMTU ioctl failed. errno %d", errno, 0, 0);
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

FUNCTION QTI_RMNET_PH_SET_MRU()

DESCRIPTION
  - sets the MRU on the peripheral interface

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

SIDE EFFECTS
  None

==========================================================================*/
int qti_rmnet_ph_set_mru
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
              rmnet_state_config->ph_data_iface_name,
              strlen(USB_DATA_INTERFACE)+1);
  ifr.ifr_ifru.ifru_data = ext_ioctl_arg;
  memset(ext_ioctl_arg, 0, sizeof(struct rmnet_ioctl_extended_s));
  ext_ioctl_arg->extended_ioctl = RMNET_IOCTL_SET_MRU;
  ext_ioctl_arg->u.data = mru;

  /* Get current if flags for the device */
  if (ioctl(fd, RMNET_IOCTL_EXTENDED, &ifr) < 0)
  {
    LOG_MSG_ERROR("set_mru: ioctl RMNET_IOCTL_SET_MRU failed. errno %d", errno, 0, 0);
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

FUNCTION QTI_RMNET_PH_SET_MODEM_STATE()

DESCRIPTION
  - sets the modem state on USB driver

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

SIDE EFFECTS
  None

==========================================================================*/

int qti_rmnet_ph_set_modem_state(int modem_state)
{
  int      ret;
/*--------------------------------------------------------------------------------*/
  LOG_MSG_INFO1("Received set modem state to %d", modem_state, 0, 0);

  if (modem_state)
  {
    ret = ioctl(rmnet_state_config->ph_iface_fd, FRMNET_CTRL_MODEM_ONLINE);
    if (ret < 0)
    {
      LOG_MSG_ERROR("Couldn't set FRMNET_CTRL_MODEM_ONLINE on driver err %d",
                    errno, 0, 0);
      return QTI_FAILURE;
    }
  }
  else
  {
    ret = ioctl(rmnet_state_config->ph_iface_fd, FRMNET_CTRL_MODEM_OFFLINE);
    if (ret < 0)
    {
      LOG_MSG_ERROR("Couldn't set FRMNET_CTRL_MODEM_OFFLINE on driver err %d",
                    errno, 0, 0);
      return QTI_FAILURE;
    }
  }

  LOG_MSG_INFO1("Successfully set modem state on USB driver file", 0, 0, 0);

  return QTI_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_DPL_PROCESS_PH_RESET()

DESCRIPTION
  - processes peripheral reset for DPL logging
  - upon USB cable plug-in : DPM open
  - upon USB cable plug-out : DPM close.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure


SIDE EFFECTS
  None

=========================================================================*/
static int qti_dpl_process_ph_reset()
{
  int                             ret_val;
  int                             ret;
  int                             line_state;

/*-------------------------------------------------------------------------*/

    LOG_MSG_INFO1("Processing DPL peripheral reset", 0, 0, 0);

    ret = ioctl(dpl_state_config->dpl_iface_fd,
                FRMNET_CTRL_GET_LINE_STATE,
                &line_state);
    if (ret)
    {
      LOG_MSG_INFO1("Couldn't get FRMNET LINE STATE from driver",
                     0, 0, 0);
      line_state = -1;
      return QTI_FAILURE;
    }
    else
    {
      LOG_MSG_INFO1("DPL line state %d", line_state, 0, 0);
      if(line_state == 1)
      {
        if(!dpl_state_config->dtr_enabled)
        {
          ret_val = qti_dpl_dpm_port_open();
          if (ret_val != QTI_SUCCESS)
          {
            LOG_MSG_ERROR("Failed to open DPL port through DPM", 0, 0, 0);
            return QTI_FAILURE;
          }
          dpl_state_config->dtr_enabled = 1;
        }
      }
/*----------------------------------------------------------------------------
  ph cable plug out happened
----------------------------------------------------------------------------*/
      else if(line_state == 0)
      {
        if(dpl_state_config->dtr_enabled)
        {
          ret_val = qti_dpl_dpm_port_close();
          if (ret_val != QTI_SUCCESS)
          {
            LOG_MSG_ERROR("Failed to close DPL port through DPM", 0, 0, 0);
          }
          dpl_state_config->dtr_enabled = 0;
        }
      }
    }

    LOG_MSG_INFO1("Completed DPL peripheral reset processing", 0, 0, 0);

    return QTI_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_RMNET_DPL_INIT()

DESCRIPTION

  This function
  - opens the ph device file for DPL logging
  - adds the ph fd to wait on select call

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/

int qti_dpl_ph_init
(
  qti_dpl_param           * dpl_state,
  qti_sk_fd_set_info_t    * fd_set,
  qti_sock_thrd_fd_read_f read_f
)
{
  int ret_val;
/*-------------------------------------------------------------------------*/
  LOG_MSG_INFO1("Open DPL file to receive QMI messages", 0, 0, 0);
  dpl_state_config = dpl_state;

  ret_val = qti_ph_file_open(dpl_state_config->dpl_iface_device_file,
                             &(dpl_state_config->dpl_iface_fd));

  if(ret_val == QTI_FAILURE)
  {
    LOG_MSG_ERROR("Failed to open DPL device file. Error %d",
                   errno, 0, 0);
    return QTI_FAILURE;
  }
  else
  {
    LOG_MSG_INFO1("Opened DPL file's fd is %d",
                  dpl_state_config->dpl_iface_fd, 0, 0);

    ret_val = qti_map_fd_read(fd_set,dpl_state_config->dpl_iface_fd, read_f);
    if(ret_val == QTI_FAILURE)
    {
      LOG_MSG_ERROR("Failed to map DPL fd with the read function",
                     0, 0, 0);
      close(dpl_state_config->dpl_iface_fd);
      return QTI_FAILURE;
    }

/*-------------------------------------------------------------------------
  On boot up since QTI comes up a bit late when compared to USB driver,
  we may miss the 0 byte data from USB. So in order to handle those cases
  query the DPL state in peripheral driver by processing it like a reset.
--------------------------------------------------------------------------*/
    qti_dpl_process_ph_reset();
  }

  return QTI_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_DPL_PH_RECV_MSG()

DESCRIPTION

  This function
  - receives data from peripheral interface for DPL logging.

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

=========================================================================*/


int qti_dpl_ph_recv_msg
(
   int dpl_ph_fd
)
{
  int        ret;
  int        ret_val;
  char       ph_rx_buf[RMNET_MAX_TRANSFER_SIZE];

 /*-------------------------------------------------------------------------*/

  memset(ph_rx_buf, 0, sizeof(ph_rx_buf));

  ret = read(dpl_ph_fd, ph_rx_buf, RMNET_MAX_TRANSFER_SIZE);

  LOG_MSG_INFO1("Read %d data from peripheral device file", ret, 0, 0);

  if (ret < 0)
  {
    LOG_MSG_ERROR("Failed to read from the dev file. Errno %d",
                   errno, 0, 0);
    return QTI_FAILURE;
  }
  else if (ret > RMNET_MAX_TRANSFER_SIZE)
  {
    LOG_MSG_ERROR("Received data larger than dest buff %d",
                   errno, 0, 0);
    return QTI_FAILURE;
  }
/*----------------------------------------------------------------------------
  Peripheral cable plug out/ plug in happened
----------------------------------------------------------------------------*/
  else if (ret == 0)
  {
    qti_dpl_process_ph_reset();
  }
  else
  {
    LOG_MSG_INFO1("Ignoring data read from DPL peripheral device file", 0, 0, 0);
  }

  return QTI_SUCCESS;
}



/*===========================================================================

FUNCTION QTI_DPL_PH_GET_EP_INFO()

DESCRIPTION
  Gets the ph, IPA EP information from ph

DEPENDENCIES
  None.

RETURN VALUE
  QTI_SUCCESS on success
  QTI_FAILURE on failure

SIDE EFFECTS
  None

=========================================================================*/

int qti_dpl_ph_get_ep_info()
{

  int ret_val= QTI_SUCCESS ;
  int retry_count = 0;

/*-------------------------------------------------------------------------
  peripheral(USB) needs to wait till pipes(IPA-USB) get connected to get the required
  IPA pipe info. This can take sometime and if we call IOCTL before peripheral
  gets the IPA pipe info, the IOCTL can fail. So we retry for 5 sec to allow
  peripheral to get the IPA pipe info.
--------------------------------------------------------------------------*/
  while (retry_count < QTI_USB_RETRY)
  {
    ret_val = ioctl(dpl_state_config->dpl_iface_fd,
                    FRMNET_CTRL_EP_LOOKUP,
                    &(dpl_state_config->ep_info));
    if(ret_val == QTI_SUCCESS)
      break;

    sleep(ONE_SEC);
    retry_count++;
    LOG_MSG_INFO1("Retrying to get EP info", 0, 0, 0);
  }

  if(retry_count == QTI_USB_RETRY)
  {
    LOG_MSG_ERROR("Unable to get EP info", 0, 0, 0);
    return QTI_FAILURE;
  }

  LOG_MSG_INFO1_6("Peripheral type %d, peripheral id %d, ipa consumer EP %d, ipa producer EP %d \n",
                  dpl_state_config->ep_info.ph_ep_info.ep_type,
                  dpl_state_config->ep_info.ph_ep_info.peripheral_iface_id,
                  dpl_state_config->ep_info.ipa_ep_pair.consumer_pipe_num,
                  dpl_state_config->ep_info.ipa_ep_pair.producer_pipe_num, 0, 0);

  return QTI_SUCCESS;
}
