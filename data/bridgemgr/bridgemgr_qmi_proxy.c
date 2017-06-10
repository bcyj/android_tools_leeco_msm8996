/******************************************************************************

                 B R I D G E M G R _ Q M I _ P R O X Y . C

******************************************************************************/

/******************************************************************************

  @file    bridgemgr_qmi_proxy.c
  @brief   Bridge Manager QMI Proxy Functions Header File

  DESCRIPTION
  Header file for BridgeMgr QMI Proxy functions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011-2012 Qualcomm Technologies, Inc. All Rights Reserved

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

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
04/25/11   sg         Initial version

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "ds_string.h"
#include "bridgemgr_common.h"
#include "bridgemgr_qmi_proxy.h"


/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define BRIDGEMGR_QMI_PROXY_UD_SOCKET        "/dev/socket/qmux_radio/proxy_tether_connect_socket"
#define BRIDGEMGR_QMI_PROXY_CLIENT_UD_SOCKET "/dev/socket/qmux_radio/tether_client_connect_socket"


/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  bridgemgr_qmi_proxy_process_cb
===========================================================================*/
/*!
@brief
  This is the processing callback registered by BRIDGMGR_SYS_MDM_QMI module

@param
  data - data to be processed
  size - size of the data

@return
  BRIDGEMGR_SUCCESS - If processing was successful
  BRIDGEMGR_FAILURE - Otherwise

@note
  This function gets called in the context of cmdq thread

*/
/*=========================================================================*/
static int bridgemgr_qmi_proxy_process_cb
(
  const void *data,
  int size
)
{
  int ret = BRIDGEMGR_FAILURE;


  if (NULL == data || size <= 0)
  {
    bridgemgr_log_err("bridgemgr_qmi_proxy_proccess_cb: bad params\n");
    goto bail;
  }

  /* Print the received QMUX message */
  bridgemgr_common_print_qmux_msg(BRIDGEMGR_SYS_QMI_PROXY, data, size);

  /* We have received data from the QMI Proxy, forward it to USB subsystem */
  ret = bridgemgr_forward_data(BRIDGEMGR_SYS_USB_QMI, data, size);

bail:
  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_qmi_proxy_read
===========================================================================*/
/*!
@brief
  This function is called by the read thread spawned during init

@param
  arg - argument passed during thread creation

@return
  NULL

*/
/*=========================================================================*/
static void *bridgemgr_qmi_proxy_read
(
  void *arg
)
{
  int fd = (int)arg;
  struct sigaction action;


  memset(&action, 0, sizeof(action));
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  action.sa_handler = bridgemgr_common_signal_handler;

  /* Register to USR2 signal so as to exit when necessary */
  if (sigaction(SIGUSR2, &action, NULL) < 0)
  {
    bridgemgr_log_err("bridgemgr_qmi_proxy_read: sigaction failed\n");
  }

  bridgemgr_log_low("bridgemgr_qmi_proxy_read: reading from fd=%d\n", fd);

  /* Wait for initialization to complete */
  bridgemgr_common_wait_for_init(BRIDGEMGR_SYS_QMI_PROXY);

  for (;;)
  {
     /* Allocate a new cmd */
     bridgemgr_cmdq_cmd_type *cmd = bridgemgr_cmdq_alloc_cmd(BRIDGEMGR_SYS_QMI_PROXY);

    if (NULL == cmd)
    {
      bridgemgr_log_err("bridgemgr_qmi_proxy_read: cmd alloc failed\n");
      break;
    }

    /* Read the QMUX message into the command buffer */
    if (BRIDGEMGR_SUCCESS == bridgemgr_common_read(BRIDGEMGR_SYS_QMI_PROXY,
                                                   fd,
                                                   cmd->cmd_data.data.qmux_nl.msg,
                                                   BRIDGEMGR_CMDQ_MAX_QMUX_NETLINK_MSG,
                                                   &cmd->cmd_data.data.qmux_nl.msg_len))
    {
      /* Enqueue the command in the command queue */
      if (BRIDGEMGR_SUCCESS != bridgemgr_cmdq_enqueue_cmd(cmd))
      {
        bridgemgr_log_err("bridgemgr_qmi_proxy_read: cmd enq failed\n");
        bridgemgr_cmdq_free_cmd(cmd);
      }
    }
    else
    {
      bridgemgr_sys_type     sys         = BRIDGEMGR_SYS_QMI_PROXY;
      bridgemgr_reinit_type  reinit_type = BRIDGEMGR_REINIT_TYPE_FULL;


      bridgemgr_log_err("bridgemgr_qmi_proxy_read: read failed\n");

      /* Read failed, issue re-init request for SYS_QMI_PROXY */
      bridgemgr_cmdq_free_cmd(cmd);

      if (BRIDGEMGR_SUCCESS != bridgemgr_common_issue_reinit_request(sys,
                                                                     reinit_type))
      {
        bridgemgr_log_err("bridgemgr_qmi_proxy_read: failed to issue sys=%s reinit=%s request\n",
                          bridgemgr_common_get_sys_str(sys),
                          bridgemgr_common_get_reinit_str(reinit_type));
        break;
      }

      /* Wait for the reinit to happen */
      for (;;)
      {
        usleep(BRIDGEMGR_RETRY_DELAY);
      }
    }
  }

  return NULL;
}


/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  bridgemgr_qmi_proxy_init
===========================================================================*/
/*!
@brief
  This function is called to initialize the BRIDGEMGR_SYS_QMI_PROXY module

@param
  client_cb - Set of callbacks to register

@return
  BRIDGEMGR_SUCCESS - Module initialization was successful
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
int bridgemgr_qmi_proxy_init
(
  bridgemgr_client_callbacks_type *client_cb
)
{
  int ret = BRIDGEMGR_FAILURE;
  int sock_fd = BRIDGEMGR_FD_INVALID;
  struct sockaddr_un addr;
  int retry_cnt;
  pthread_t thr;

  if (NULL == client_cb)
  {
    bridgemgr_log_err("bridgemgr_qmi_proxy_init: bad params\n");
    goto bail;
  }

  /* Create a Unix Domain socket */
  if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
  {
    bridgemgr_log_err("bridgemgr_qmi_proxy_init: socket creation failed\n");
    goto bail;
  }

  bridgemgr_log_low("bridgemgr_qmi_proxy_init: removing... %s\n",
                    BRIDGEMGR_QMI_PROXY_CLIENT_UD_SOCKET);

  /* Remove the entry if it already exists */
  if (remove(BRIDGEMGR_QMI_PROXY_CLIENT_UD_SOCKET) == -1 && errno != ENOENT)
  {
    bridgemgr_log_err("bridgemgr_qmi_proxy_init: remove %s failed\n",
                      BRIDGEMGR_QMI_PROXY_CLIENT_UD_SOCKET);
    goto bail;
  }

  /* Initialize the address structure */
  memset(&addr, 0, sizeof(struct sockaddr_un));

  addr.sun_family = AF_UNIX;
  strlcpy(addr.sun_path, BRIDGEMGR_QMI_PROXY_CLIENT_UD_SOCKET, sizeof(addr.sun_path));

  if (bind(sock_fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) < 0)
  {
    bridgemgr_log_err("bridgemgr_qmi_proxy_init: bind() failed for client sock=%s\n",
                      BRIDGEMGR_QMI_PROXY_CLIENT_UD_SOCKET);
    goto bail;
  }

  /* Initialize the address structure */
  memset(&addr, 0, sizeof(struct sockaddr_un));

  addr.sun_family = AF_UNIX;
  strlcpy(addr.sun_path, BRIDGEMGR_QMI_PROXY_UD_SOCKET, sizeof(addr.sun_path));

  for (retry_cnt = 1; retry_cnt <= BRIDGEMGR_MAX_RETRY_COUNT; ++retry_cnt)
  {
    /* Connect to the QMI Proxy daemon */
    if (connect(sock_fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) < 0)
    {
      bridgemgr_log_err("bridgemgr_qmi_proxy_init: connect() failed for attempt: %d\n", retry_cnt);
      usleep(BRIDGEMGR_RETRY_DELAY);
    }
    else
    {
      break;
    }
  }

  if (retry_cnt > BRIDGEMGR_MAX_RETRY_COUNT)
  {
    bridgemgr_log_err("bridgemgr_qmi_proxy_init: connect() failed\n");
    goto bail;
  }

  if (pthread_create(&thr, NULL, &bridgemgr_qmi_proxy_read, (void *)sock_fd))
  {
    bridgemgr_log_err("bridgemgr_qmi_proxy_init: failed to create read thread\n");
    goto bail;
  }

  /* Update the client callback info */
  client_cb->fd             = sock_fd;
  client_cb->process_func   = bridgemgr_qmi_proxy_process_cb;
  client_cb->cleanup_func   = bridgemgr_common_cleanup_cb;
  client_cb->read_thread_id = thr;

  bridgemgr_log_med("bridgemgr_qmi_proxy_init: complete\n");

  ret = BRIDGEMGR_SUCCESS;

bail:
  if (BRIDGEMGR_SUCCESS != ret)
  {
    BRIDGEMGR_FD_CLOSE(sock_fd);
  }

  return ret;
}

