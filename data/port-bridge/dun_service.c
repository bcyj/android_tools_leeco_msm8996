/**
  @file
  dun_service.c

  @brief
  This file handles serial communication between host device and
  modem device
*/

/*===========================================================================

  Copyright (c) 2011,2013-2014 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

===========================================================================*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/wait.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <signal.h>

#include "port_bridge.h"

#define DUN_CTRL_POLLING_INTERVAL (200 * 1000)
#define DUN_WAIT_BEFORE_CLEANUP   (500 * 1000)
#define MAX(x,y) ((x) > (y) ? (x) : (y))

/*===========================================================================
  FUNCTION   dun_parse_atcmd
============================================================================*/
/*!
@brief
  Parses AT commands

@param
  buf  Raw data received from the host device

@return
  DUN_ATCMD_START    If a valid AT start command is received
  DUN_ATCMD_INVALID  Invalid AT command is received
*/
/*===========================================================================*/
static dun_atcmd_t dun_parse_atcmd(const char *buf)
{
  if( (strncasecmp(buf,"ATDT*98", 7) == 0)
     || (strncasecmp(buf,"ATDT*99", 7) == 0)
     || (strncasecmp(buf,"ATD*98", 6) == 0)
     || (strncasecmp(buf,"ATD*99", 6) == 0)
     || (strncasecmp(buf,"ATDT#777", 8) == 0)
     || (strncasecmp(buf,"ATD#777", 7) == 0) )
  {
    LOGI("dun_parse_atcmd buf %s\n", buf);
    return DUN_ATCMD_START;
  }
  else
  {
    return DUN_ATCMD_INVALID;
  }
}

/*===========================================================================
  FUNCTION  dun_transmit_dev_status_info
============================================================================*/
/*!
@brief
  Get the status of the given device node and update it on the other end.
  Change gets reported only if there is a difference between the previous
  and current status.

@param
  from_fd  File descriptor of the device from which status needs to be read
  to_fd    File descriptoor of the device to which status is to be reported
  prev_status_bits  Previous status bits of the device

@return
  int - New status bits
*/
/*============================================================================*/
static int dun_transmit_dev_status_info
(
  int from_fd,
  int to_fd,
  int prev_status_bits
)
{
  int new_status_bits = 0;
  int cmd_bits = 0;

  /* Get the current status bits of from_fd */
  if (ioctl(from_fd, TIOCMGET, &new_status_bits) < 0)
  {
    LOGE("dun_transmit_dev_status_info: TIOCMGET failed for fd [%d] [%s]\n",
         from_fd, strerror(errno));
    return DUN_ERROR;
  }

  /* Check if status has changed */
  if (prev_status_bits != new_status_bits)
  {
    /* Turn ON the bits which are present in new but not in old */
    cmd_bits = new_status_bits & ~prev_status_bits;
    if (ioctl(to_fd, TIOCMBIS, &cmd_bits) < 0)
    {
      LOGE("dun_transmit_dev_status_info: Error setting new status bits on the target"
           " device [%d] [%s]\n",
           to_fd, strerror(errno));
      return DUN_ERROR;
    }

    /* Turn OFF the bits that are not there in new but are there in old */
    cmd_bits = ~new_status_bits & prev_status_bits;
    if (ioctl(to_fd, TIOCMBIC, &cmd_bits) < 0)
    {
      LOGE("dun_transmit_dev_status_info: Error clearing new status bits on the target"
           " device [%d] [%s]\n",
           to_fd, strerror(errno));
      return DUN_ERROR;
    }
  }

  return new_status_bits;
}

/*===========================================================================
  FUNCTION  dun_port_thread_exit_handler
============================================================================*/
/*!
@brief
  Signal handler for threads

@param
  sig   The signal which is received

@return
  None
*/
/*===========================================================================*/
static void dun_port_thread_exit_handler(int sig)
{
  LOGD("dun_port_thread_exit_handler: Received signal %d", sig);
  pthread_exit(NULL);
}

/*===========================================================================
  FUNCTION  dun_monitor_ports
============================================================================*/
/*!
@brief
  Thread which monitors the ports and report any change of status to both
  sides of the bridge

@param
  arg  Argument passed during thread creation

@return
  None
*/
/*============================================================================*/
static void *dun_monitor_ports(void *arg)
{
  int prev_host_bits = 0;
  struct sigaction actions;

  dun_portparams_t *pportparams = (dun_portparams_t *) arg;

  memset(&actions, 0, sizeof(actions));
  sigemptyset(&actions.sa_mask);
  actions.sa_flags = 0;
  actions.sa_handler = dun_port_thread_exit_handler;

  if (sigaction(SIGUSR1, &actions, NULL) < 0)
  {
    LOGE("dun_monitor_ports: Error in sigaction [%s]\n",
         strerror(errno));
    pthread_exit(NULL);
  }

  while (1)
  {
    usleep(DUN_CTRL_POLLING_INTERVAL);
    prev_host_bits = dun_transmit_dev_status_info(pportparams->host_fd,
                                                  pportparams->modem_fd,
                                                  prev_host_bits);
    if (prev_host_bits < 0)
    {
      LOGE("dun_monitor_ports: Error in getting status bits\n");
      continue;
    }
  }

  /* Terminate thread */
  pthread_exit(NULL);
  return NULL;
}

/*===========================================================================
  FUNCTION  dun_dataxfr_thread
============================================================================*/
/*!
@brief
  Thread is responsible for data transfer

@param
  arg   Argument passed during thread creation

@return
  None
*/
/*===========================================================================*/
static void* dun_dataxfr_thread(void* arg)
{
  extern dun_state_t dun_state;
  dun_atcmd_t atcmd;
  ssize_t num_read;
  ssize_t num_written;
  int current_host_bits = 0;
  int cmd_bits = 0;
  char *host_xfr_buf = NULL;
  char *modem_xfr_buf = NULL;
  struct sigaction actions;
  dun_portparams_t *pportparams = (dun_portparams_t *)arg;

  fd_set readfds;
  int max_fd = MAX(pportparams->host_fd, pportparams->modem_fd);

  if (sigaction(SIGUSR1, &actions, NULL) < 0)
  {
    LOGE("dun_dataxfr_thread: Error in sigaction [%s]\n", strerror(errno));
    dun_post_event(DUN_EVENT_ERROR);
    pthread_exit(NULL);
    return NULL;
  }

  host_xfr_buf = (char *) malloc(DUN_MAXBUFSIZE);
  if (NULL == host_xfr_buf)
  {
    LOGE("dun_dataxfr_thread: Could not allocate host_xfr_buf buffer for data transfer! [%s]",
         strerror(errno));
    dun_post_event(DUN_EVENT_ERROR);
    pthread_exit(NULL);
    return NULL;
  }

  modem_xfr_buf = (char *) malloc(DUN_MAXBUFSIZE);
  if (NULL == modem_xfr_buf)
  {
    LOGE("dun_dataxfr_thread: Could not allocate modem_xfr_buf buffer for data transfer! [%s]",
         strerror(errno));
    dun_post_event(DUN_EVENT_ERROR);
    pthread_exit(NULL);
    return NULL;
  }

  while (1)
  {
    FD_ZERO(&readfds);
    FD_SET(pportparams->host_fd, &readfds);
    FD_SET(pportparams->modem_fd, &readfds);

    if (select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0)
    {
      LOGE("dun_dataxfr_thread: select failed [%s]", strerror(errno));
    }

    if (FD_ISSET(pportparams->modem_fd, &readfds))
    {
      memset(modem_xfr_buf, 0, DUN_MAXBUFSIZE);
      /* Data transfer from device port to external port */
      if (dun_state == DUN_STATE_CONNECTED ||
          dun_state == DUN_STATE_IDLE)
      {
        LOGI("dun_dataxfr_thread: Reading data from device port...");
        num_read = read(pportparams->modem_fd, (void *)modem_xfr_buf,
                        DUN_MAXBUFSIZE);
        if (num_read == -ENETRESET)
        {
          /* This is the case where SSR will be sent from MHI device */
          /* Get current host status bits */
          if (ioctl(pportparams->host_fd, TIOCMGET, &current_host_bits) < 0)
          {
            LOGE("dun_dataxfr_thread: Error in getting host status bits while"
                 " processing ENETRESET [%s]\n",
                 strerror(errno));
            continue;
          }

          /* Send CD bit to host */
          cmd_bits = current_host_bits & ~TIOCM_CD;

          /* Set the bits on the host */
          if (ioctl(pportparams->host_fd, TIOCMBIS, &cmd_bits) < 0)
          {
            LOGE("dun_dataxfr_thread: Error in clearing DTR bit on host while"
                 " processing ENETRESET [%s]\n",
                 strerror(errno));
            continue;
          }
        }
        else if (num_read < 0 || num_read > DUN_MAXBUFSIZE)
        {
          LOGE("dun_dataxfr_dlink: Read from device port failed [%s]\n",
               strerror(errno));
          continue;
        }
        else if (num_read > 0)
        {
          LOGI("dun_dataxfr_thread: num_read [%zd]", num_read);
          LOGI("dun_dataxfr_thread: Writing data to host port...");
          num_written = write(pportparams->host_fd, (void *)modem_xfr_buf, (size_t)num_read);
          LOGI("dun_dataxfr_thread: num_written [%zd]", num_written);

          if (num_written < 0)
          {
            LOGE("dun_dataxfr_thread: Write to external port failed [%s]\n",
                 strerror(errno));
            continue;
          }
        }
      }
    }

    if (FD_ISSET(pportparams->host_fd, &readfds))
    {
      memset(host_xfr_buf, 0, DUN_MAXBUFSIZE);
      /* Data transfer from external port to device port */
      LOGI("dun_dataxfr_thread: Reading data from host port...");
      num_read = read(pportparams->host_fd, (void *)host_xfr_buf,
                      DUN_MAXBUFSIZE);
      if (num_read < 0 || num_read > DUN_MAXBUFSIZE)
      {
        LOGE("dun_dataxfr_thread: External port read failed [%s]",
             strerror(errno));
        continue;
      }
      else
      {
        /* Parse the data received from external port */
        LOGD("dun_dataxfr_thread: num_read [%zd]", num_read);
        atcmd = dun_parse_atcmd(host_xfr_buf);

        /* If DUN state was idle and we receive and AT cmd start
         * we should send DUN start event to the state machine */
        if (dun_state == DUN_STATE_IDLE
            && atcmd == DUN_ATCMD_START)
        {
          dun_post_event(DUN_EVENT_START);
        }

        LOGI("dun_dataxfr_thread: Writing data to device port...");
        num_written = write(pportparams->modem_fd, (void *)host_xfr_buf, (size_t)num_read);
        LOGD("dun_dataxfr_thread: num_written [%zd]", num_written);

        /* Write the AT command to device port as-is */
        if (num_written < 0)
        {
          LOGE("dun_dataxfr_thread: Write to device port failed [%s]",
               strerror(errno));
          continue;
        }
      }
    }
  }

  LOGI("dun_dataxfr_thread: Exiting dataxfr thread");
  if (NULL != host_xfr_buf)
  {
    free(host_xfr_buf);
  }

  if (NULL != modem_xfr_buf)
  {
    free(modem_xfr_buf);
  }

  return NULL;
}

/*===========================================================================
  FUNCTION  dun_init_ports
============================================================================*/
/*!
@brief
  Initialize the host and modem device ports

@param
  pportparams  Structure holding information on the devices

@return
  int - DUN_SUCCESS if successful
        DUN_ERROR if there was an error
*/
/*===========================================================================*/
static int dun_init_ports(dun_portparams_t *pportparms)
{
  struct termios term_params;
  struct termios orig_term_params;

  /* Open the modem port */
  if ((pportparms->modem_fd = open(pportparms->modem, O_RDWR)) < 0)
  {
    LOGE("dun_init_ports: Unable to open device port %s : %s\n",
         pportparms->modem, strerror(errno));
    return DUN_ERROR;
  }

  /* Open host port */
  if ((pportparms->host_fd = open(pportparms->host, O_RDWR)) < 0)
  {
    LOGE("dun_init_ports: Unable to open external port %s : %s\n",
         pportparms->host, strerror(errno));
    return DUN_ERROR;
  }

  LOGI("dun_init_ports: Successfully opened modem port: %x, host port: %x\n",
       pportparms->modem_fd, pportparms->host_fd);

  /* Put host port into RAW mode.  Note that this is not
   * necessary for modem port as a raw mode is the only one available
   */
  if (tcgetattr (pportparms->host_fd, &term_params) < 0)
  {
    LOGE ("dun_init_ports: tcgetattr() call fails : %s\n", strerror(errno));
    return DUN_ERROR;
  }

  orig_term_params = term_params;
  term_params.c_lflag &= (unsigned)(~(ECHO | ICANON | IEXTEN | ISIG));
  term_params.c_iflag &= (unsigned)(~(BRKINT | ICRNL | INPCK | ISTRIP | IXON));
  term_params.c_cflag &= (unsigned)(~(CSIZE | PARENB));
  term_params.c_cflag |= CS8;
  term_params.c_oflag &= (unsigned)(~(OPOST));
  term_params.c_cc[VMIN] = 1;
  term_params.c_cc[VTIME] = 0;

  if (tcsetattr (pportparms->host_fd, TCSAFLUSH, &term_params) < 0)
  {
    LOGE ("dun_init_ports: tcsetattr() call fails : %s\n", strerror(errno));
    return DUN_ERROR;
  }

  LOGI("dun_init_ports: Configured external host port in RAW mode\n");
  return DUN_SUCCESS;
}

/*===========================================================================
  FUNCTION  dun_start_ports_threads
============================================================================*/
/*!
@brief
  Starts the ports monitor and data transfer threads

@param
  pportparams  Structure holding information on the devices

@return
  int - DUN_SUCCESS if successful
        DUN_ERROR if there was an error
*/
/*==========================================================================*/
int dun_start_ports_threads(dun_portparams_t *pportparams)
{
  if( dun_init_ports(pportparams) < 0)
  {
    LOGE("dun_start_ports_threads: Error while initiating ports \n");
    dun_post_event(DUN_EVENT_ERROR);
    return DUN_ERROR;
  }

  /* Create threads monitors external port and data transfer */
  if(pthread_create(&(pportparams->portsmonitor_thread), NULL,
                     dun_monitor_ports, (void *)pportparams) < 0)
  {
    LOGE("dun_start_ports_threads: Unable to create extportmonitor : %s\n",
         strerror(errno));
    dun_post_event(DUN_EVENT_ERROR);
    return DUN_ERROR;
  }

  /* Create data xfr thread */
  if (pthread_create(&(pportparams->portdataxfr_thread), NULL,
                     dun_dataxfr_thread, (void *) pportparams) < 0)
  {
    LOGE("dun_start_ports_threads: Unable to create dataxfr thread [%s]\n",
         strerror(errno));
    dun_post_event(DUN_EVENT_ERROR);
    return DUN_ERROR;
  }

  return DUN_SUCCESS;
}

/*===========================================================================
  FUNCTION  dun_stop_ports_threads
============================================================================*/
/*!
@brief
  Stops the threads

@param
  pportparams  Structure holding information on the devices

@return
  int - DUN_SUCCESS if successful
        DUN_ERROR if there was an error
*/
/*===========================================================================*/
int dun_stop_ports_threads(dun_portparams_t *pportparams)
{
  int status;

  /* Allowing portsmonitor thread to transfer CD bit from SMD to ext */
  usleep(DUN_WAIT_BEFORE_CLEANUP);

  /* kill thread which monitors external port bits */
  if((status = pthread_kill(pportparams->portsmonitor_thread,
                            SIGUSR1)) != 0)
  {
    LOGE("dun_stop_ports_threads: Error cancelling thread %d, error = %d (%s)",
         (int)pportparams->portsmonitor_thread, status, strerror(status));
    return DUN_ERROR;
  }

  if((status = pthread_kill(pportparams->portdataxfr_thread, SIGUSR1)) != 0)
  {
    LOGE("dun_stop_ports_threads: Error cancelling thread [%d], error [%d][%s]",
         (int)pportparams->portdataxfr_thread, status, strerror(status));
    return DUN_ERROR;
  }

  pthread_join(pportparams->portsmonitor_thread, NULL);
  LOGI("dun_stop_ports_threads: Joined port monitor thread \n");

  pthread_join(pportparams->portdataxfr_thread, NULL);
  LOGI("dun_stop_ports_threads: Joined data txf thread \n");

  close(pportparams->modem_fd);
  close(pportparams->host_fd);
  return DUN_SUCCESS;
}
