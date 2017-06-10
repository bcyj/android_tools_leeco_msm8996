/******************************************************************************

                        QTI_PPP_USB_TTY.C

******************************************************************************/

/******************************************************************************

  @file    qti_ppp_usb_tty.c
  @brief   Qualcomm Tethering Interface for DUN tethering. This file contains
           QTI interaction with USB for DUN tethering

  DESCRIPTION
  Implementation file for QTI inteaction with USB for DUN tethering.

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
#include <sys/inotify.h>
#include <sys/un.h>
#include <sys/socket.h>

#include "qti_ppp.h"

static qti_usb_tty_config        * usb_tty_info;
static qti_usb_line_state_config *usb_line_state_info;

/*---------------------------------------------------------------------------
IOCTL to USB for Changing the mode to SMD.
---------------------------------------------------------------------------*/
typedef unsigned long   u32;
#define QTI_USB_XPORT_IOCTL_MAGIC 'G'
#define QTI_USB_CHANGE_XPORT_TYPE _IOW(QTI_USB_XPORT_IOCTL_MAGIC, 0, u32)
#define QTI_USB_SMD_WRITE_TYPE _IOW(QTI_USB_XPORT_IOCTL_MAGIC, 1, struct qti_ppp_ioctl_smd_write_arg_type)
/*===========================================================================
                          FUNCTION DEFINITIONS
============================================================================*/
/*===========================================================================

FUNCTION PRINT_AT_CMD()

DESCRIPTION

  This function
  - prints AT Commands.

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

/*=========================================================================*/
void print_at_cmd
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
  printf("\nCommand: %s\n", buf);
}

/*===========================================================================

FUNCTION QTI_USB_VALIDATE_DUN_PROFILE()

DESCRIPTION

  This function
  - Validates the profile information passed in ATD command.

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

/*=========================================================================*/
static int qti_usb_validate_dun_profile
(
   void      *atd_cmd,
   uint32_t   cmd_len
)
{
  char *tmp_ptr = NULL, *tmp_ptr1 = NULL, *tmp_ptr2 = NULL;
  char dun_profile[USB_MAX_TRANSFER_SIZE];
  int i = 0;
  uint8_t dun_profile_id = 0;
  qti_ppp_tech_pref_mask_v01 tech_pref = 0;

  if (atd_cmd == NULL || cmd_len == 0)
  {
    LOG_MSG_ERROR("NULL Arguements", 0, 0, 0);
    return QTI_PPP_FAILURE;
  }

  if ( strcasestr( (char *)atd_cmd, DUN_3GPP2_DIAL_STRING) )
  {
    /* Parsing is not required for 3GPP2 as DUN call is always made on default profile. */
    tech_pref |= QTI_PPP_MASK_TECH_PREF_3GPP2_V01;
    dun_profile_id = 0;
  }
  else if ( strcasestr( (char *)atd_cmd, DUN_3GPP_DIAL_STRING1) ||
            strcasestr( (char *)atd_cmd, DUN_3GPP_DIAL_STRING2) )
  {
    tech_pref |= QTI_PPP_MASK_TECH_PREF_3GPP_V01;

    /* Get the Profile ID. */

    /* Logic for parsing the profile id. */

    /* Profile is mentioned as follows 'atdt*98*1#'
    * where 1 is the profile id. And default profile is
    * specified as 'atdt*98#'. We check if profile id is specified
    * or not by looking for 2 '*' and if present we start from the
    * last '*' and traverse till the end # to get parse the profile id.
    */

    tmp_ptr = (char *)memchr(atd_cmd,DUN_ASTERISK_CHAR, cmd_len-1);
    tmp_ptr1 = (char *)memrchr(atd_cmd, DUN_HASH_CHAR, cmd_len-1);

    if ( tmp_ptr == NULL || tmp_ptr1 == NULL ||
         tmp_ptr1 < tmp_ptr ||
         ( (char *)memchr(atd_cmd, DUN_HASH_CHAR, cmd_len-1) !=
         (char *)memrchr(atd_cmd, DUN_HASH_CHAR, cmd_len-1)) )
    {
      LOG_MSG_ERROR("Invalid AT command: %s", atd_cmd, 0, 0);
      return QTI_PPP_FAILURE;
    }

    tmp_ptr2 = (char *)memrchr(atd_cmd, DUN_ASTERISK_CHAR, cmd_len-1);
    /* Check if '#' is at the end of atd. */
    if ( tmp_ptr2 == NULL || tmp_ptr1 < tmp_ptr2 )
    {
      LOG_MSG_ERROR("Invalid AT command: %s", atd_cmd, 0, 0);
      return QTI_PPP_FAILURE;
    }

    /* Check if profile id is not specified. i.e if we have only one '*' */
    if (tmp_ptr2 == tmp_ptr)
    {
      dun_profile_id = 0;
    }
    else
    {
      /* User has specified the Profile ID. */
      memset(dun_profile, 0, USB_MAX_TRANSFER_SIZE);
      for ( i = 1; (*(tmp_ptr2 + i) != DUN_HASH_CHAR); i++ )
      {
        dun_profile[i-1] = *(tmp_ptr2 + i);
      }
      dun_profile_id = atoi(dun_profile);
      LOG_MSG_INFO1("\nDun Profile ID: %d\n", dun_profile_id, 0, 0);
    }
  }
  else
  {
    LOG_MSG_ERROR("Invalid AT command", 0, 0, 0);
    return QTI_PPP_FAILURE;
  }

  return qti_ppp_validate_dun_profile(dun_profile_id, tech_pref);

}
/*===========================================================================

FUNCTION QTI_USB_TTY_RECV_MSG()

DESCRIPTION

  This function
  - receives AT commands from USB interface

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

/*=========================================================================*/


int qti_usb_tty_recv_msg
(
   int usb_tty_fd
)
{
  int        ret, i=0;
  char      usb_rx_buf[USB_MAX_TRANSFER_SIZE];
  int       ret_val;
  u32       usb_mode = 0;
  int       usb_serial_fd = 0;
  struct qti_ppp_ioctl_smd_write_arg_type atd_command;

 /*-------------------------------------------------------------------------*/
  memset(usb_rx_buf, 0, USB_MAX_TRANSFER_SIZE);
  ret = read(usb_tty_fd, usb_rx_buf, USB_MAX_TRANSFER_SIZE);
  if (ret <= 0)
  {
    LOG_MSG_ERROR("Failed to read from the dev file %d:%d", usb_tty_fd, errno, 0);
    if (ret == 0)
    {

      if ( usb_tty_info->is_ppp_active )
      {
        /*--------------------------------------------------------------------------
        Bring down PPP tethering
        ---------------------------------------------------------------------------*/
        qti_ppp_usb_link_down();

        /*--------------------------------------------------------------------------
        Disable QC Mobile AP
        ---------------------------------------------------------------------------*/
        qti_ppp_disable_mobile_ap();
      }

      LOG_MSG_INFO1("\nClosing USB FD %d %d\n",
                    usb_tty_fd, usb_tty_info->usb_fd, 0);
      close(usb_tty_info->usb_fd);
      usb_tty_info->is_ppp_active = 0;

      /* Clear the FD from fd set. */
      qti_ppp_clear_fd(&sk_fdset, usb_tty_info->usb_fd);
      if ( usb_tty_fd != usb_tty_info->usb_fd )
        qti_ppp_clear_fd(&sk_fdset, usb_tty_fd);
      usb_tty_info->usb_fd = 0;
    }
    return QTI_PPP_FAILURE;
  }

  if ( strcasestr(usb_rx_buf, DUN_DIAL_STRING) )
  {

/*--------------------------------------------------------------------------
  Enable QC Mobile AP
---------------------------------------------------------------------------*/
    ret_val = qti_ppp_enable_mobile_ap();
    if (ret_val == QTI_PPP_SUCCESS)
    {
      LOG_MSG_INFO1(" Mobile AP enable: successful", 0, 0, 0);
    }
    else
    {
      LOG_MSG_ERROR(" Mobile AP enable: unsuccessful.", 0, 0, 0);
      return QTI_PPP_FAILURE;
    }

#ifdef FEATURE_DUN_PROFILE_VALIDATION
    ret_val = qti_usb_validate_dun_profile(usb_rx_buf, ret);
    if (ret_val == QTI_PPP_SUCCESS)
    {
      LOG_MSG_INFO1(" Validate DUN profile: successful", 0, 0, 0);
    }
    else
    {
      ret_val = qti_ppp_disable_mobile_ap();
      if (ret_val == QTI_PPP_SUCCESS)
      {
        LOG_MSG_INFO1(" Mobile AP disable: successful", 0, 0, 0);
      }
      else
      {
        LOG_MSG_ERROR(" Mobile AP disable: unsuccessful.", 0, 0, 0);
      }
      LOG_MSG_ERROR(" Validate DUN Profile: unsuccessful.", 0, 0, 0);
      goto modem;
    }
#endif /* FEATURE_DUN_PROFILE_VALIDATION */

    /* Send CONNECT to DUN Client */
    qti_usb_tty_send_msg(DUN_CONNECT_STRING_WITH_CR,DUN_CONNECT_STRING_WITH_CR_LEN);

/*--------------------------------------------------------------------------
  Setup PPP tethering
---------------------------------------------------------------------------*/
    ret_val = qti_ppp_usb_link_up();
    if (ret_val == QTI_PPP_SUCCESS)
    {
      LOG_MSG_INFO1(" Setup USB tethering: successful.",
                    0, 0, 0);
    }
    else
    {
      LOG_MSG_ERROR(" Setup USB tethering: unsuccessful.",
                    0, 0, 0);
      ret_val = qti_ppp_disable_mobile_ap();
      if (ret_val == QTI_PPP_SUCCESS)
      {
        LOG_MSG_INFO1(" Mobile AP disable: successful", 0, 0, 0);
      }
      else
      {
        LOG_MSG_ERROR(" Mobile AP disable: unsuccessful.", 0, 0, 0);
      }
      goto send_no_carrier;
    }

    LOG_MSG_INFO1("\nClosing USB FD %d %d\n",
                  usb_tty_fd, usb_tty_info->usb_fd, 0);
    close(usb_tty_info->usb_fd);

    /* Clear the FD from fd set. */
    qti_ppp_clear_fd(&sk_fdset, usb_tty_info->usb_fd);
    usb_tty_info->usb_fd = 0;
    usb_tty_info->is_ppp_active = 1;

  }
  else
  {
    /* Send message to Modem */
    qti_smd_tty_send_msg(usb_rx_buf, ret);
  }
  return QTI_PPP_SUCCESS;

send_no_carrier:
  /* Send NO CARRIER to DUN Client */
  qti_usb_tty_send_msg(DUN_NO_CARRIER_STRING,DUN_NO_CARRIER_STRING_LEN);
  return QTI_PPP_SUCCESS;

#ifdef FEATURE_DUN_PROFILE_VALIDATION /* TODO */
modem:
  /* Open the serial device to call the IOCTL to switch to SMD. */
  usb_serial_fd = open(USB_SERIAL_PORT, O_RDWR);
  if(usb_serial_fd == -1)
  {
    LOG_MSG_ERROR("Could not open device file, errno : %d ", errno, 0, 0);
    return QTI_PPP_FAILURE;
  }
  /* Change the mode to SMD. */
  usb_mode = 1;
  if (ioctl(usb_serial_fd,
            QTI_USB_CHANGE_XPORT_TYPE,
            &usb_mode) < 0)
  {
    LOG_MSG_ERROR("ioctl failed something wrong check %d.",
                  errno,0,0);
    ret_val = QTI_PPP_FAILURE;
  }
  else
  {
    LOG_MSG_INFO1("ioctl success, mode changed to SMD",
                  0,0,0);
    ret_val = QTI_PPP_SUCCESS;

    /* Send the ATD command. */
    memset(&atd_command, 0, sizeof(atd_command));

    /* Go through the AT command and replace carriage return with new line. */
    for(i=0; i < ret; i++)
    {
      if ( *(char *)(usb_rx_buf + i) == 0x0a )
        *(char *)(usb_rx_buf + i) = 0x0d;
    }
    atd_command.buf = usb_rx_buf;
    atd_command.size = ret;

    if (ioctl(usb_serial_fd,
              QTI_USB_SMD_WRITE_TYPE,
              &atd_command) < 0)
    {
      LOG_MSG_ERROR("ioctl failed something wrong check %d.",
                    errno,0,0);
    }
    else
    {
      LOG_MSG_INFO1("ioctl success, mode changed to SMD",
                    0,0,0);
    }

    LOG_MSG_INFO1("Closing USB FD %d %d", usb_tty_fd, usb_tty_info->usb_fd, 0);

    /* Clear the FD from fd set. */
    qti_ppp_clear_fd(&sk_fdset, usb_tty_info->usb_fd);
    close(usb_tty_info->usb_fd);
    usb_tty_info->usb_fd = 0;

    /* Initialize WDS client to get DUN call events. */
    ret_val = qti_ppp_wds_init(&sk_fdset,
                                        qti_ppp_wds_recv_msg);
    if (ret_val != QTI_PPP_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to initialize QTI USB TTY listener",0,0,0);
    }
  }
  /* Close the Serial FD. */
  close(usb_serial_fd);
  return ret_val;
#endif /* FEATURE_DUN_PROFILE_VALIDATION */
}

/*===========================================================================

FUNCTION QTI_USB_TTY_FILE_OPEN()

DESCRIPTION

  This function
  - opens the device file which is used for interfacing with USB

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

/*=========================================================================*/


static int qti_usb_tty_file_open
(
   int * usb_tty_fd
)
{
  int qti_usb_tty_fd, usb_serial_fd;
  struct termios tty_port_settings;
  speed_t baud_rate;
  u32 usb_mode = 0;
  char command[MAX_COMMAND_STR_LEN];
  uint32_t time_out = 10;
  uint8_t j = 0;
/*--------------------------------------------------------------------------*/

  /* Check for NULL Args. */
  if (usb_tty_fd == NULL)
  {
    LOG_MSG_ERROR("NULL Args",0,0,0);
    return QTI_PPP_FAILURE;
  }

  /* Before opening the TTY device switch the USB mode. */
  /* Open the serial device to call the IOCTL. */
  usb_serial_fd = open(USB_SERIAL_PORT, O_RDWR);
  if(usb_serial_fd == -1)
  {
    LOG_MSG_ERROR("Could not open serial device file, errno : %d ", errno, 0, 0);
    return QTI_PPP_FAILURE;
  }
  /* Change the mode to TTY. */
  usb_mode = 0;
  if (ioctl(usb_serial_fd,
            QTI_USB_CHANGE_XPORT_TYPE,
            &usb_mode) < 0)
  {
    LOG_MSG_ERROR("ioctl failed something wrong check %d.",
                  errno,0,0);
    close(usb_serial_fd);
    return QTI_PPP_FAILURE;
  }
  else
  {
    LOG_MSG_INFO1("ioctl success, mode changed to SMD",
                  0,0,0);
  }
  /* Close the FD. */
  close(usb_serial_fd);

  while (j++ < time_out)
  {
    qti_usb_tty_fd = open(USB_DUN_TTY_PORT, O_RDWR | O_NOCTTY);
    if (qti_usb_tty_fd != -1)
    {
      break;
    }
    usleep(USB_DEV_INIT_DELAY);
  }

  memset(&tty_port_settings, 0, sizeof(struct termios));

  baud_rate = DUN_TTY_BAUDRATE;
  /* Set the Baud rate. */
  cfsetspeed(&tty_port_settings, baud_rate);

  /* Flush the existing data. */
  tcflush(qti_usb_tty_fd, TCIOFLUSH);

  /* Apply the atributes. */
  tcsetattr(qti_usb_tty_fd,TCSANOW,&tty_port_settings);

  LOG_MSG_INFO1("Successfully opened USB device file. FD is %d", qti_usb_tty_fd, 0, 0);
  *usb_tty_fd = qti_usb_tty_fd;

  return QTI_PPP_SUCCESS;

}

/*===========================================================================

FUNCTION QTI_USB_TTY_MAP_FD_READ()

DESCRIPTION

  This function
  - adds the USB tty fd to the list of FD on which select call listens
  - maps the read fucntion for the USB tty fd

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

/*=========================================================================*/

static int qti_usb_tty_map_fd_read
(
   qti_ppp_nl_sk_fd_set_info_t *fd_set_list,
   int                     usb_tty_fd,
   qti_ppp_sock_thrd_fd_read_f read_func
)
{
  /* Check for NULL Args. */
  if (fd_set_list == NULL)
  {
    LOG_MSG_ERROR("NULL Args",0,0,0);
    return QTI_PPP_FAILURE;
  }

  if( fd_set_list->num_fd < MAX_NUM_OF_FD )
  {
    FD_SET(usb_tty_fd, &(fd_set_list->fdset));
/*--------------------------------------------------------------------------
  Add fd to fdmap array and store read handler function ptr
-------------------------------------------------------------------------- */
    fd_set_list->sk_fds[fd_set_list->num_fd].sk_fd = usb_tty_fd;
    fd_set_list->sk_fds[fd_set_list->num_fd].read_func = read_func;
    LOG_MSG_INFO1("Added read function for fd %d", usb_tty_fd, 0, 0);

/*--------------------------------------------------------------------------
  Increment number of fds stored in fdmap
--------------------------------------------------------------------------*/
    fd_set_list->num_fd++;
    if(fd_set_list->max_fd < usb_tty_fd)
    {
      LOG_MSG_INFO1("Updating USB max fd %d", usb_tty_fd, 0, 0);
      fd_set_list->max_fd = usb_tty_fd;
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

FUNCTION QTI_USB_TTY_LISTENER_INIT()

DESCRIPTION

  This function
  - opens the USB device file
  - adds the USB fd to wait on select call

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

/*=========================================================================*/

int qti_usb_tty_listener_init
(
  qti_usb_tty_config  * config_info,
  qti_ppp_nl_sk_fd_set_info_t * fd_set,
  qti_ppp_sock_thrd_fd_read_f read_f
)
{
  int ret_val;
  FILE * fp = NULL;
  uint32_t is_connected = 0;
/*-------------------------------------------------------------------------*/

  /* Check for NULL Args. */
  if (config_info == NULL || fd_set == NULL)
  {
    LOG_MSG_ERROR("NULL Args",0,0,0);
    return QTI_PPP_FAILURE;
  }

#ifdef FEATURE_DUN_PROFILE_VALIDATION /* TODO */
  /* Clean the WDS handle if present. */
  qti_ppp_wds_exit();
#endif /* FEATURE_DUN_PROFILE_VALIDATION */

  /* First check whether we are in DUN+SoftAP Mode or not. */
  /* Open the file and read the connected value. */
  fp = fopen(USB_DUN_TTY_ACTIVE_FLAG, "r");

  if ( fp == NULL )
  {
    LOG_MSG_ERROR("Error opening TTY connected status file: %d.\n", errno, 0, 0);
    return QTI_PPP_FAILURE;
  }

  if (fscanf(fp, "%d", &is_connected) != 1) {
    LOG_MSG_ERROR("Error reading TTY connected status file: %d.\n", errno, 0, 0);
    fclose(fp);
    return QTI_PPP_FAILURE;
  }

  fclose(fp);

  if ( !is_connected )
  {
    LOG_MSG_ERROR("Not in DUN+SoftAP mode: %d. Do not start now\n", is_connected, 0, 0);
    return QTI_PPP_FAILURE;
  }

  LOG_MSG_INFO1("Open USB TTY file to receive AT commands", 0, 0, 0);
  /* Update the configuration. */
  usb_tty_info = config_info;
  usb_tty_info->is_ppp_active = 0;
  ret_val = qti_usb_tty_file_open(&(usb_tty_info->usb_fd));

  if(ret_val == QTI_PPP_FAILURE)
  {
    LOG_MSG_ERROR("Failed to open USB device file. Abort", 0, 0, 0);
    return QTI_PPP_FAILURE;
  }
  else
  {
    LOG_MSG_INFO1("Opened file's fd is %d", usb_tty_info->usb_fd, 0, 0);
    ret_val = qti_usb_tty_map_fd_read(fd_set,usb_tty_info->usb_fd, read_f);
    if(ret_val == QTI_PPP_FAILURE)
    {
      LOG_MSG_ERROR("Failed to map fd with the read function", 0, 0, 0);
      close(usb_tty_info->usb_fd);
      return QTI_PPP_FAILURE;
    }
  }
  return QTI_PPP_SUCCESS;
}


/*===========================================================================

FUNCTION QTI_USB_TTY_SEND_MSG()

DESCRIPTION

  This function
  - send AT commands to USB

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

/*=========================================================================*/

void qti_usb_tty_send_msg
(
   void      *data,
   uint32_t   len
)
{
  int ret;
/*-----------------------------------------------------------------------*/

  /* Check for NULL Args. */
  if (data == NULL)
  {
    LOG_MSG_ERROR("NULL Args",0,0,0);
    return;
  }

  ret = write(usb_tty_info->usb_fd, (char*)data, len);
  if (ret == -1)
  {
    LOG_MSG_ERROR("Couldn't send message to host: %d", errno, 0, 0);
  }
  else if (ret != len)
  {
    LOG_MSG_ERROR("Unexpected return value when writing to device file: got %d, "
                  "expected %d (errno %d)", ret, len, errno);
  }
  else
  {
    LOG_MSG_INFO1("Successfully sent message to host\n", 0, 0, 0);
  }

  return;
}

/*=========================================================================*/
static int qti_usb_line_state_open
(
   int * usb_line_state_fd
)
{
  int qti_usb_line_state_fd, val;
  struct sockaddr_un qti_ppp_usb_line_state;
  int len;
  struct timeval rcv_timeo;
/*--------------------------------------------------------------------------*/

  /* Check for NULL Args. */
  if (usb_line_state_fd == NULL)
  {
    LOG_MSG_ERROR("NULL Args",0,0,0);
    return QTI_PPP_FAILURE;
  }

  if ((qti_usb_line_state_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == QTI_PPP_FAILURE)
  {
    LOG_MSG_ERROR("Error creating socket, errno: %d", errno, 0, 0);
    return QTI_PPP_FAILURE;
  }

  if(fcntl(qti_usb_line_state_fd, F_SETFD, FD_CLOEXEC) < 0)
  {
    LOG_MSG_ERROR("Couldn't set Close on Exec, errno: %d", errno, 0, 0);
  }

  rcv_timeo.tv_sec = 0;
  rcv_timeo.tv_usec = 100000;
  setsockopt(qti_usb_line_state_fd, SOL_SOCKET, SO_RCVTIMEO, (void *)&rcv_timeo, sizeof(rcv_timeo));
  val = fcntl(qti_usb_line_state_fd, F_GETFL, 0);
  fcntl(qti_usb_line_state_fd, F_SETFL, val | O_NONBLOCK);

  qti_ppp_usb_line_state.sun_family = AF_UNIX;
  strlcpy(qti_ppp_usb_line_state.sun_path, QTI_PPP_LS_FILE, QTI_PPP_UNIX_PATH_MAX);
  unlink(qti_ppp_usb_line_state.sun_path);
  len = strlen(qti_ppp_usb_line_state.sun_path) + sizeof(qti_ppp_usb_line_state.sun_family);
  if (bind(qti_usb_line_state_fd, (struct sockaddr *)&qti_ppp_usb_line_state, len) == QTI_PPP_FAILURE)
  {
    LOG_MSG_ERROR("Error binding the socket, errno: %d", errno, 0, 0);
    close(qti_usb_line_state_fd);
    return QTI_PPP_FAILURE;
  }

  *usb_line_state_fd = qti_usb_line_state_fd;

  return QTI_PPP_SUCCESS;

}

/*===========================================================================

FUNCTION QTI_USB_LINE_STATE_INIT()

DESCRIPTION

  This function
  - Adds fd to get line state notifications from QTI.

DEPENDENCIES
  None.

RETURN VALUE

SIDE EFFECTS
  None

/*=========================================================================*/

int qti_usb_line_state_init
(
  qti_usb_line_state_config *line_state_config,
  qti_ppp_nl_sk_fd_set_info_t * fd_set,
  qti_ppp_sock_thrd_fd_read_f read_f
)
{

  int ret_val;
/*-------------------------------------------------------------------------*/

  /* Check for NULL Args. */
  if (line_state_config == NULL || fd_set == NULL)
  {
    LOG_MSG_ERROR("NULL Args",0,0,0);
    return QTI_PPP_FAILURE;
  }

  LOG_MSG_INFO1("Open USB line state fd to get line status indications", 0, 0, 0);
  usb_line_state_info = line_state_config;
  ret_val = qti_usb_line_state_open(&(line_state_config->line_state_fd));

  if(ret_val == QTI_PPP_FAILURE)
  {
    LOG_MSG_ERROR("Failed to open Notify object. Abort", 0, 0, 0);
    return QTI_PPP_FAILURE;
  }
  else
  {
    LOG_MSG_INFO1("Opened file's fd is %d", usb_line_state_info->line_state_fd, 0, 0);
    ret_val = qti_usb_tty_map_fd_read(fd_set,usb_line_state_info->line_state_fd, read_f);
    if(ret_val == QTI_PPP_FAILURE)
    {
      LOG_MSG_ERROR("Failed to map fd with the read function", 0, 0, 0);
      /* Close the Notify fd. Watch fd will be cleared automatically. */
      close(usb_line_state_info->line_state_fd);
      return QTI_PPP_FAILURE;
    }
  }
  return QTI_PPP_SUCCESS;
}

/*===========================================================================

FUNCTION QTI_USB_LINE_STATE_RECV_MSG()

DESCRIPTION

  This function
  - receives notifications about line state.

DEPENDENCIES
  None.

RETURN VALUE
  QTI_PPP_SUCCESS on success
  QTI_PPP_FAILURE on failure


SIDE EFFECTS
  None

/*=========================================================================*/
int qti_usb_line_state_recv_msg
(
  int usb_line_state_fd
)
{
  int line_state = 0, ret, ret_val = QTI_PPP_SUCCESS;

  ret = read(usb_line_state_fd, &line_state, sizeof(line_state));
  if (ret <= 0)
  {
    LOG_MSG_ERROR("Failed to read from the dev file %d:%d", usb_line_state_fd, errno, 0);
    return QTI_PPP_FAILURE;
  }

  if ( line_state == 1 )
  {
    /*---------------------------------------------------------------------
      Call into the USB TTY listener init function which sets up QTI to
      listen to AT Commands coming in from the USB device file for DUN
    ---------------------------------------------------------------------*/
    if (usb_tty_info == NULL ||
        (usb_tty_info->is_ppp_active == 0 && usb_tty_info->usb_fd == 0))
    {
      ret_val = qti_usb_tty_listener_init(&usb_tty_config_info,
                                          &sk_fdset,
                                          qti_usb_tty_recv_msg);
      if (ret_val != QTI_PPP_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to initialize QTI USB TTY listener",0,0,0);
      }
    }
  }
  else
  {
    /* USB cable is disconnected. Clear the TTY FD. */
    if ( usb_tty_info != NULL && usb_tty_info->usb_fd != 0)
    {

      if ( usb_tty_info->is_ppp_active )
      {
        /*--------------------------------------------------------------------------
        Bring down PPP tethering
        ---------------------------------------------------------------------------*/
        qti_ppp_usb_link_down();

        /*--------------------------------------------------------------------------
        Disable QC Mobile AP
        ---------------------------------------------------------------------------*/
        qti_ppp_disable_mobile_ap();
      }

      /* Update the PPP flag. */
      usb_tty_info->is_ppp_active = 0;

      LOG_MSG_INFO1("\nClosing USB FD %d\n", usb_tty_info->usb_fd, 0, 0);
      close(usb_tty_info->usb_fd);
      /* Clear the FD from fd set. */
      qti_ppp_clear_fd(&sk_fdset, usb_tty_info->usb_fd);
      usb_tty_info->usb_fd = 0;
    }
  }
  return QTI_PPP_SUCCESS;
}
