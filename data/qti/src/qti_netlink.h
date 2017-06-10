
/******************************************************************************

                        QTI_NETLINK.H

******************************************************************************/

/******************************************************************************

  @file    qti_netlink.h
  @brief   Qualcomm Tethering Interface Netlink Messaging module

  DESCRIPTION
  Header file for NetMgr Netlink messaging functions.

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
02/04/12   sb/mp      Added support for dynamic USB composition switching.
10/08/12   mp         Fix to recognize the tethering interface upon power on.
06/29/12   sc         Revised version 
05/24/12   sb         Initial version

******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_addr.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <netinet/in.h>
#include "comdef.h"
#include "ds_util.h"
#include "qmi_client.h"

#define MAX_NUM_OF_FD 10
#define QTI_NL_MSG_MAX_LEN (1024)
#define RNDIS_INTERFACE "rndis0"
#define ECM_INTERFACE "ecm0"
#define IF_NAME_LEN 16
#define QTI_INTERFACES 2
#define QTI_QMI_SVC_INIT_TIMEOUT_MS 500

#define QTI_SUCCESS 0
#define QTI_FAILURE (-1)

#define QTI_DEFAULT_INTERFACE_ID (-99999)


/*--------------------------------------------------------------------------- 
   Type representing function callback registered with a socket listener 
   thread for reading from a socket on receipt of an incoming message
---------------------------------------------------------------------------*/
typedef int (* qti_sock_thrd_fd_read_f) (int fd);

typedef enum
{
  QTI_INIT = 0,
  QTI_LINK_UP_WAIT,
  QTI_LINK_UP,
  QTI_LINK_DOWN_WAIT,
  QTI_LINK_DOWN
} qti_nl_state_e;


typedef struct
{
 char dev_name[IF_NAME_LEN];
 int if_index;
 boolean enabled;
} qti_ifi_dev_name_t;

typedef struct
{
 qti_nl_state_e state;
 qti_ifi_dev_name_t if_dev[QTI_INTERFACES];
 qmi_client_type qmi_qti_wda_handle;
 qmi_client_type qmi_qti_v4_wds_handle;
 qmi_client_type qmi_qti_v6_wds_handle;
 uint32 qmi_qti_v4_wds_call_handle;
 uint32 qmi_qti_v6_wds_call_handle; 
 boolean qti_qcmap_proceed;
} qti_conf_t;


typedef struct
{
 int sk_fd;
 qti_sock_thrd_fd_read_f read_func;
} qti_nl_sk_fd_map_info_t;

typedef struct
{
 qti_nl_sk_fd_map_info_t sk_fds[MAX_NUM_OF_FD];
 fd_set fdset;
 int num_fd;
 int max_fd;
} qti_nl_sk_fd_set_info_t;

typedef struct
{
 int                 sk_fd;       /* socket descriptor */
 struct sockaddr_nl  sk_addr_loc; /* local address of socket */
} qti_nl_sk_info_t;

typedef struct 
{
 struct ifinfomsg  metainfo;
} qti_nl_link_info_t;

typedef struct
{
 unsigned int type;
 boolean link_event;
 qti_nl_link_info_t nl_link_info;
} qti_nl_msg_t;


/*===========================================================================
  FUNCTION  qti_init 
===========================================================================*/
/*!
@brief
 Initialize qti setup

@return
  TO DO

@note

  - Dependencies
    - None 

  - Side Effects
    - None
*/
/*=========================================================================*/
int qti_init(void);


/*===========================================================================
  FUNCTION  qti_conf_state_init 
===========================================================================*/
/*!
@brief
  Initialize qti state

@return
  Void

@note

  - Dependencies
    - None 

  - Side Effects
    - None
*/
/*=========================================================================*/
int qti_conf_state_init(void);

/*===========================================================================
  FUNCTION  qti_nl_listener_init 
===========================================================================*/
/*!
@brief
  Initialization routine for listener on NetLink sockets interface. 

@return
  TO DO

@note

  - Dependencies
    - None 

  - Side Effects
    - Listening thread is created
*/
/*=========================================================================*/
int qti_nl_listener_init
(
  unsigned int nl_type,
  unsigned int nl_groups,
  qti_nl_sk_fd_set_info_t * sk_fdset,
  qti_sock_thrd_fd_read_f read_f
);


/*===========================================================================
  FUNCTION  qti_nl_recv_msg
===========================================================================*/
/*!
@brief
  Virtual function registered with the socket listener thread to receive
  incoming messages over the NETLINK routing socket.

@return
  QTI_SUCCESS or QTI_FALUIRE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int qti_nl_recv_msg(int fd);

/*===========================================================================
  FUNCTION  qti_nl_query_if
===========================================================================*/
/*!
@brief
  Send an RTM_GETLINK to kernel to get the current link configuration.

@return
  QTI_SUCCESS or QTI_FALUIRE

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int qti_nl_query_if
(
  qti_nl_sk_info_t *sk_info
);
