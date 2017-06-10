/******************************************************************************

                        QTI_PPP_SMD_TTY.C

******************************************************************************/

/******************************************************************************

  @file    qti_ppp_smd_tty.c
  @brief   Qualcomm Tethering Interface for DUN tethering. This file contains
           QTI interaction with SMD for DUN tethering

  DESCRIPTION
  Implementation file for QTI inteaction with SMD for DUN tethering.

  ---------------------------------------------------------------------------
  Copyright (c) 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------


******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

when       who        what, where, why
--------   ---        -------------------------------------------------------
02/19/14   cp         Initial version

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

#include "qti_ppp.h"

static qti_smd_tty_config        * smd_tty_info;

/*===========================================================================
                          FUNCTION DEFINITIONS
============================================================================*/
/*===========================================================================

FUNCTION QTI_SMD_TTY_RECV_MSG()

DESCRIPTION

  This function
  - receives AT commands from MODEM.

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

/*=========================================================================*/


int qti_smd_tty_recv_msg
(
   int smd_tty_fd
)
{
  int        ret;
  char      smd_rx_buf[USB_MAX_TRANSFER_SIZE];


 /*-------------------------------------------------------------------------*/
  ret = read(smd_tty_fd, smd_rx_buf, USB_MAX_TRANSFER_SIZE);
  if (ret < 0)
  {
    LOG_MSG_ERROR("Failed to read from the dev file", 0, 0, 0);
    return QTI_PPP_FAILURE;
  }

  /* Sent to tethered host using USB TTY.. */
  qti_usb_tty_send_msg(smd_rx_buf, ret);

  return QTI_PPP_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_SMD_TTY_FILE_OPEN()

DESCRIPTION

  This function
  - opens the device file which is used for interfacing with MODEM

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

/*=========================================================================*/


static int qti_smd_tty_file_open
(
   int * smd_tty_fd
)
{
  int qti_smd_tty_fd;
  struct termios tty_port_settings;
  speed_t baud_rate;
/*--------------------------------------------------------------------------*/

  /* Check for NULL Args. */
  if (smd_tty_fd == NULL)
  {
    LOG_MSG_ERROR("NULL Args",0,0,0);
    return QTI_PPP_FAILURE;
  }

  qti_smd_tty_fd = open(SMD_DS_TTY_PORT, O_RDWR);
  if(qti_smd_tty_fd == -1)
  {
    LOG_MSG_ERROR("Could not open device file, errno : %d ", errno, 0, 0);
    return QTI_PPP_FAILURE;
  }

  /* Set the BAUD rate. */
  memset(&tty_port_settings, 0, sizeof(struct termios));
  baud_rate = DUN_TTY_BAUDRATE;
  cfsetspeed(&tty_port_settings, baud_rate);
  tcsetattr(qti_smd_tty_fd,TCSANOW,&tty_port_settings);

  LOG_MSG_INFO1("Successfully opened USB device file. FD is %d", qti_smd_tty_fd, 0, 0);
  *smd_tty_fd = qti_smd_tty_fd;

  return QTI_PPP_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_SMD_TTY_MAP_FD_READ()

DESCRIPTION

  This function
  - adds the SMD tty fd to the list of FD on which select call listens
  - maps the read fucntion for the SMD tty fd

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

/*=========================================================================*/

static int qti_smd_tty_map_fd_read
(
   qti_ppp_nl_sk_fd_set_info_t *fd_set,
   int                     smd_tty_fd,
   qti_ppp_sock_thrd_fd_read_f read_f
)
{
  /* Check for NULL Args. */
  if (fd_set == NULL)
  {
    LOG_MSG_ERROR("NULL Args",0,0,0);
    return QTI_PPP_FAILURE;
  }

  if( fd_set->num_fd < MAX_NUM_OF_FD )
  {
    FD_SET(smd_tty_fd, &(fd_set->fdset));
/*--------------------------------------------------------------------------
  Add fd to fdmap array and store read handler function ptr
-------------------------------------------------------------------------- */
    fd_set->sk_fds[fd_set->num_fd].sk_fd = smd_tty_fd;
    fd_set->sk_fds[fd_set->num_fd].read_func = read_f;
    LOG_MSG_INFO1("Added read function for fd %d", smd_tty_fd, 0, 0);

/*--------------------------------------------------------------------------
  Increment number of fds stored in fdmap
--------------------------------------------------------------------------*/
    fd_set->num_fd++;
    if(fd_set->max_fd < smd_tty_fd)
    {
      LOG_MSG_INFO1("Updating USB max fd %d", smd_tty_fd, 0, 0);
      fd_set->max_fd = smd_tty_fd;
    }
  }
  else
  {
    LOG_MSG_ERROR("Exceeds maximum num of FD", 0, 0, 0);
    return QTI_PPP_FAILURE;
  }

  return QTI_PPP_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_SMD_TTY_LISTENER_INIT()

DESCRIPTION

  This function
  - opens the SMD TTY device file
  - adds the SMD TTY fd to wait on select call

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

/*=========================================================================*/

int qti_smd_tty_listener_init
(
  qti_smd_tty_config  * smd_tty_config_info,
  qti_ppp_nl_sk_fd_set_info_t * fd_set,
  qti_ppp_sock_thrd_fd_read_f read_f
)
{
  int ret_val;
/*-------------------------------------------------------------------------*/
  /* Check for NULL Args. */
  if (smd_tty_config_info == NULL || fd_set == NULL)
  {
    LOG_MSG_ERROR("NULL Args",0,0,0);
    return QTI_PPP_FAILURE;
  }

  LOG_MSG_INFO1("Open SMD TTY file to receive AT commands from modem", 0, 0, 0);
  smd_tty_info = smd_tty_config_info;
  ret_val = qti_smd_tty_file_open(&(smd_tty_info->smd_fd));

  if(ret_val == QTI_PPP_FAILURE)
  {
    LOG_MSG_ERROR("Failed to open USB device file. Abort", 0, 0, 0);
    return QTI_PPP_FAILURE;
  }
  else
  {
    LOG_MSG_INFO1("Opened file's fd is %d", smd_tty_info->smd_fd, 0, 0);
    ret_val = qti_smd_tty_map_fd_read(fd_set,smd_tty_info->smd_fd, read_f);
    if(ret_val == QTI_PPP_FAILURE)
    {
      LOG_MSG_ERROR("Failed to map fd with the read function", 0, 0, 0);
      close(smd_tty_info->smd_fd);
      return QTI_PPP_FAILURE;
    }
  }
  return QTI_PPP_SUCCESS;
}


/*===========================================================================

FUNCTION QTI_SMD_TTY_SEND_MSG()

DESCRIPTION

  This function
  - send AT commands to Modem

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

/*=========================================================================*/

void qti_smd_tty_send_msg
(
   void      *data,
   uint32_t   len
)
{
  int ret;
  int i = 0;
/*-----------------------------------------------------------------------*/

  /* Check for NULL Args. */
  if (data == NULL)
  {
    LOG_MSG_ERROR("NULL Args",0,0,0);
    return;
  }

  /* Go through the AT command and replace carriage return with new line. */
  for(i=0; i < len; i++)
  {
    if ( *(char *)(data + i) == 0x0a )
      *(char *)(data + i) = 0x0d;
  }

  ret = write(smd_tty_info->smd_fd, (char*)data, len);
  if (ret == -1)
  {
    LOG_MSG_ERROR("Couldn't send message to modem: %d", errno, 0, 0);
  }
  else if (ret != len)
  {
    LOG_MSG_ERROR("Unexpected return value when writing to device file: got %d, "
                  "expected %d (errno %d)", ret, len, errno);
  }
  else
  {
    LOG_MSG_INFO1("Successfully sent message to modem\n", 0, 0, 0);
  }

  return;
}
