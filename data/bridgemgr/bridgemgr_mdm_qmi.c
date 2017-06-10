/******************************************************************************

                 B R I D G E M G R _ M D M _ Q M I . C

******************************************************************************/

/******************************************************************************

  @file    bridgemgr_mdm_qmi.c
  @brief   Bridge Manager MDM QMI Functions Header File

  DESCRIPTION
  Header file for BridgeMgr MDM QMI functions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011-2013 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary and Confidential.

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
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include "bridgemgr_mdm_qmi.h"


/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define BRIDGEMGR_MDM_QMI_TETH_PORT "/dev/sdioctl8"


static int bridgemgr_mdm_qmi_fd = BRIDGEMGR_FD_INVALID;


/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  bridgemgr_mdm_qmi_process_cb
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
  This function is called in the context of cmdq thread 
*/
/*=========================================================================*/
static int bridgemgr_mdm_qmi_process_cb
(
  const void *data,
  int size
)
{
  int ret = BRIDGEMGR_FAILURE;


  if (NULL == data || size <= 0)
  {
    bridgemgr_log_err("bridgemgr_mdm_qmi_proccess_cb: bad params\n");
    goto bail;
  }

  /* Print the received QMUX message */
  bridgemgr_common_print_qmux_msg(BRIDGEMGR_SYS_MDM_QMI, data, size);

  /* We have received data from MDM, forward it to USB subsystem */
  ret = bridgemgr_forward_data(BRIDGEMGR_SYS_USB_QMI, data, size);

bail:
  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_mdm_qmi_cleanup_cb
===========================================================================*/
/*!
@brief
  This is the cleanup function registerd by BRIDGMGR_SYS_MDM_QMI module

@param
  cb - pointer to client registered callbacks

@return
  None

*/
/*=========================================================================*/
static void bridgemgr_mdm_qmi_cleanup_cb
(
  bridgemgr_sys_type               sys,
  bridgemgr_client_callbacks_type  *cb
)
{
  if (BRIDGEMGR_SYS_MDM_QMI != sys)
  {
    bridgemgr_log_err("bridgemgr_mdm_qmi_cleanup_cb: invalid sys=%s\n",
                      bridgemgr_common_get_sys_str(sys));
    return;
  }

  bridgemgr_common_cleanup_cb(sys, cb);
  bridgemgr_mdm_qmi_fd = BRIDGEMGR_FD_INVALID;
}


/*===========================================================================
  FUNCTION  bridgemgr_mdm_qmi_read
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
static void *bridgemgr_mdm_qmi_read
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
    bridgemgr_log_err("bridgemgr_mdm_qmi_read: sigaction failed\n");
  }

  bridgemgr_log_low("bridgemgr_mdm_qmi_read: reading from fd=%d\n", fd);

  /* Wait for initialization to complete */
  bridgemgr_common_wait_for_init(BRIDGEMGR_SYS_MDM_QMI);

  for (;;)
  {
     /* Allocate a new cmd */
     bridgemgr_cmdq_cmd_type *cmd = bridgemgr_cmdq_alloc_cmd(BRIDGEMGR_SYS_MDM_QMI);

    if (NULL == cmd)
    {
      bridgemgr_log_err("bridgemgr_mdm_qmi_read: cmd alloc failed\n");
      break;
    }

    /* Read the QMUX message into the command buffer */
    if (BRIDGEMGR_SUCCESS == bridgemgr_common_read(BRIDGEMGR_SYS_MDM_QMI,
                                                   fd,
                                                   cmd->cmd_data.data.qmux_nl.msg,
                                                   BRIDGEMGR_CMDQ_MAX_QMUX_NETLINK_MSG,
                                                   &cmd->cmd_data.data.qmux_nl.msg_len))
    {
      /* Enqueue the command in the command queue */
      if (BRIDGEMGR_SUCCESS != bridgemgr_cmdq_enqueue_cmd(cmd))
      {
        bridgemgr_log_err("bridgemgr_mdm_qmi_read: cmd enq failed\n");
        bridgemgr_cmdq_free_cmd(cmd);
      }
    }
    else
    {
      bridgemgr_sys_type sys;
      bridgemgr_reinit_type reinit_type = BRIDGEMGR_REINIT_TYPE_FULL;


      bridgemgr_log_err("bridgemgr_mdm_qmi_read: read failed\n");

      bridgemgr_cmdq_free_cmd(cmd);

      /* If read failed due to a modem restart, re-init SYS_PS_QMI_IND module as well */
      if (ENETRESET == errno)
      {
        sys = BRIDGEMGR_SYS_PS_QMI_IND;

        if (BRIDGEMGR_SUCCESS != bridgemgr_common_issue_reinit_request(sys,
                                                                       reinit_type))
        {
          bridgemgr_log_err("bridgemgr_mdm_qmi_read: failed to issue sys=%s reinit=%s request\n",
                            bridgemgr_common_get_sys_str(sys),
                            bridgemgr_common_get_reinit_str(reinit_type));
        }
      }

      /* Since read failed, re-init SYS_MDM_QMI module */
      sys = BRIDGEMGR_SYS_MDM_QMI;

      if (BRIDGEMGR_SUCCESS != bridgemgr_common_issue_reinit_request(sys,
                                                                      reinit_type))
      {
        bridgemgr_log_err("bridgemgr_mdm_qmi_read: failed to issue sys=%s reinit=%s request\n",
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
  FUNCTION  bridgemgr_mdm_qmi_init
===========================================================================*/
/*!
@brief
  This function is called to initialize the BRIDGEMGR_SYS_MDM_QMI module

@param
  client_cb - Set of callbacks to register

@return
  BRIDGEMGR_SUCCESS - Module initialization was successful
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
int bridgemgr_mdm_qmi_init
(
  bridgemgr_client_callbacks_type *client_cb
)
{
  int ret = BRIDGEMGR_FAILURE;
  int fd = BRIDGEMGR_FD_INVALID;
  int retry_cnt;
  pthread_t thr;


  if (NULL == client_cb)
  {
    bridgemgr_log_err("bridgemgr_mdm_qmi_init: bad params\n");
    goto bail;
  }

  for (retry_cnt = 1; retry_cnt <= BRIDGEMGR_MAX_RETRY_COUNT; ++retry_cnt)
  {
    if ((fd = open(BRIDGEMGR_MDM_QMI_TETH_PORT, O_RDWR)) < 0)
    {
      bridgemgr_log_err("bridgemgr_mdm_qmi_init: failed to open MDM tether "
                        "port %s for attempt: %d\n",
                        BRIDGEMGR_MDM_QMI_TETH_PORT,
                        retry_cnt);
      usleep(BRIDGEMGR_RETRY_DELAY);
    }
    else
    {
      break;
    }
  }

  if (retry_cnt > BRIDGEMGR_MAX_RETRY_COUNT)
  {
    bridgemgr_log_err("bridgemgr_mdm_qmi_init: failed to open MDM tether port %s\n",
                      BRIDGEMGR_MDM_QMI_TETH_PORT);
    goto bail;
  }

  if (pthread_create(&thr, NULL, &bridgemgr_mdm_qmi_read, (void *)fd))
  {
    bridgemgr_log_err("bridgemgr_mdm_qmi_init: failed to create read thread\n");
    goto bail;
  }

  /* Save the fd for issuing ioctl commands */
  bridgemgr_mdm_qmi_fd      = fd;

  client_cb->fd             = fd;
  client_cb->process_func   = bridgemgr_mdm_qmi_process_cb;
  client_cb->cleanup_func   = bridgemgr_mdm_qmi_cleanup_cb;
  client_cb->read_thread_id = thr;

  ret = BRIDGEMGR_SUCCESS;

bail:
  if (BRIDGEMGR_SUCCESS != ret)
  {
    BRIDGEMGR_FD_CLOSE(fd);
  }

  return ret;
}


/*===========================================================================
  FUNCTION  bridgemgr_mdm_qmi_ioctl
===========================================================================*/
/*!
@brief
  This function is called to issue ioctl commands on the MDM fd

@param
  cmd            - Command to issue
  result [inout] - Result to store or set

@return
  BRIDGEMGR_SUCCESS - on Success
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
int bridgemgr_mdm_qmi_ioctl
(
  int            cmd,
  unsigned long  *result
)
{
  int ret = BRIDGEMGR_FAILURE;
  int ioctl_ret;


  if (NULL == result)
  {
    bridgemgr_log_err("bridgemgr_mdm_qmi_ioctl: bad param(s)\n");
    goto bail;
  }

  if (BRIDGEMGR_FD_INVALID == bridgemgr_mdm_qmi_fd)
  {
    bridgemgr_log_err("bridgemgr_mdm_qmi_ioctl: USB fd is invalid\n");
    goto bail;
  }

  switch (cmd)
  {
    case TIOCMGET:
      /* For TIOCMGET, output is returned as the return value of ioctl
         instead of in result */
      if ((ioctl_ret = ioctl(bridgemgr_mdm_qmi_fd, cmd, *result)) < 0)
      {
        bridgemgr_log_err("bridgemgr_mdm_qmi_ioctl: ioctl failed for cmd=0x%x, "
                          "errno=%u, error=%s\n",
                          cmd, errno, strerror(errno));
        goto bail;
      }

      *result = (unsigned long)ioctl_ret;
      ret = BRIDGEMGR_SUCCESS;

      break;

    case TIOCMSET:
      if (ioctl(bridgemgr_mdm_qmi_fd, cmd, *result) < 0)
      {
        bridgemgr_log_err("bridgemgr_mdm_qmi_ioctl: ioctl failed for cmd=0x%x, "
                          "errno=%u, error=%s\n",
                          cmd, errno, strerror(errno));
        goto bail;
      }
      ret = BRIDGEMGR_SUCCESS;
      break;

    default:
      bridgemgr_log_err("bridgemgr_mdm_qmi_ioctl: unhandled ioctl cmd=0x%x",
                        cmd);
      break;
  }

bail:
  return ret;
}

