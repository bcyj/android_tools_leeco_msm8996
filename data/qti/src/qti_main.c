/******************************************************************************

                        QTI_MAIN.C

******************************************************************************/

/******************************************************************************

  @file    qti_main.c
  @brief   Qualcomm Tethering Interface Module

  DESCRIPTION
  Implementation of Qualcomm Tethering Interface.

  ---------------------------------------------------------------------------
  Copyright (c) 2012 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
 
 
******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
10/03/12   mp         Fix to make QTI use QCMAP WWAN config.
07/07/12   sc         Revised version
05/24/12   sb         Initial version

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>


#include "qti_cmdq.h"
#include "ds_Utils_DebugMsg.h"

/*========================================================================== 
  
FUNCTION SIGHANDLER()

DESCRIPTION

  Registered signal handler used to catch signals posted to QTI and perform
  some useful operations

DEPENDENCIES
  None.

RETURN VALUE
  None


SIDE EFFECTS 
  None

==========================================================================*/
void sighandler(int signal)
{
  qti_cmdq_cmd_t * cmd_buf = NULL;

  switch (signal)
  {
    /*------------------------------------------------------------------- 
      USR1 signal is posted to QTI when QCMAP LAN is up. Based on this
      signal we initialize QTI so that it will be in a ready state to
      bring up data call once we get USB plug in event through netlink
      sockets
    --------------------------------------------------------------------*/
    case SIGUSR1:
      LOG_MSG_INFO1("Got SIGUSR1",0,0,0);
      qti_qmi_rcv_msg();
      qti_conf_state_init();
      break;

    default:
      LOG_MSG_INFO1("Received unexpected signal %s",signal,0,0);
      break;
  }
}


/*========================================================================== 
  
FUNCTION MAIN()

DESCRIPTION

  The main function for QTI which is first called when QTI gets started on
  boot up.

DEPENDENCIES
  None.

RETURN VALUE
  0 on SUCCESS
  -1 on FAILURE

SIDE EFFECTS 
  None

==========================================================================*/
int main(int argc, char ** argv)
{
  int ret_val;
  qti_nl_sk_fd_set_info_t sk_fdset;

  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  printf("Staring QTI main\n");

  /* ---------------------------------------------------------------------
     Register signal handler to catch required signals
  -----------------------------------------------------------------------*/
  signal(SIGUSR1, sighandler);

  /*----------------------------------------------------------------------
  Initialize QTI netlink socket socket descriptor set
  ----------------------------------------------------------------------*/
  memset(&sk_fdset, 0, sizeof(qti_nl_sk_fd_set_info_t));

  /*---------------------------------------------------------------------
   Initilaize QTI
  ----------------------------------------------------------------------*/
  ret_val = qti_init();

  if(ret_val != QTI_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to initialize QTI",0,0,0);
    return 0;
  }

  /*---------------------------------------------------------------------
   Initialize QTI command queue
  ----------------------------------------------------------------------*/
  qti_cmdq_init();

  /*---------------------------------------------------------------------
  Call into the netlink listener init function which sets up QTI to 
  listen to netlink events 
  ---------------------------------------------------------------------*/
  ret_val = qti_nl_listener_init(NETLINK_ROUTE, 
                                 RTMGRP_LINK, 
                                 &sk_fdset, 
                                 qti_nl_recv_msg);
  if(ret_val != QTI_SUCCESS) 
  {
    LOG_MSG_ERROR("Failed to initialize QTI netlink event listener",0,0,0);
    return 0;
  }

  /*--------------------------------------------------------------------
  Wait for the QTI command queue to finish before exiting QTI
  ---------------------------------------------------------------------*/
  qti_cmdq_wait();

  return 0;
}


