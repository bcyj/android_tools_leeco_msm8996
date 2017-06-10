/******************************************************************************

  @file    portbridge_ext_host_mon.h
  @brief   External Host Monitor

  DESCRIPTION
  Header for monitoring functions for external host

 ******************************************************************************/
/*===========================================================================

  Copyright (c) 2010,2012-2013 Qualcomm Technologies, Inc. All Rights Reserved

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
#ifndef __PORTBRIDGE_EXT_HOST_MON_H__
#define __PORTBRIDGE_EXT_HOST_MON_H__

#include <cutils/sockets.h>
#include <sys/un.h>

#define DUN_NUM_CONNECTIONS 1
#define DUN_SERVER "qcom.dun.server"



/*Starts the connection monitoring thread*/
extern int   pb_start_conn_mon_thread(void);
/*Stops the connection monitoring thread*/
extern void  pb_stop_conn_mon_thread(void);
extern void close_socket(int *sock);

/* Mutex for dun server and core synchronization */
extern pthread_mutex_t signal_mutex;
extern pthread_cond_t signal_cv;


#ifdef __cplusplus
extern "C" {
#endif

    /* This will disconnect the DUN connection
       by closing the socket */
    void disconnect_dun();

#ifdef __cplusplus
}
#endif

#endif /* __PORTBRIDGE_EXT_HOST_MON_H__ */
