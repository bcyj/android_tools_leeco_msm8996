/******************************************************************************

                          N E T M G R _ E X E C . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_exec.h
  @brief   Network Manager executive header file

  DESCRIPTION
  Header file for NetMgr executive control module.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010-2013,2015 Qualcomm Technologies, Inc. All Rights Reserved

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
02/17/10   ar         Initial version

******************************************************************************/

#ifndef __NETMGR_EXEC_H__
#define __NETMGR_EXEC_H__

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#include "comdef.h"

#include "ds_cmdq.h"
#include "netmgr.h"
#include "netmgr_defs.h"
#include "netmgr_netlink.h"
#include "netmgr_main.h"
#include "netmgr_qmi.h"
#include "netmgr_tc.h"
#include "netmgr_sm.h"

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

/*---------------------------------------------------------------------------
   Type of a Executive event data
---------------------------------------------------------------------------*/
typedef struct netmgr_exec_cmd_data_s {
  netmgr_sm_events_t            type;            /* Event type */
  int                           link;
  union {
    netmgr_qmi_cmd_t            qmi_msg;
    netmgr_qmi_qos_flow_info_t  qos_flow;
    netmgr_msg_t                kif_msg;
    struct connect_msg_s {
      netmgr_nl_msg_t           nlmsg_info;
      netmgr_address_set_t     *addr_info_ptr;
      boolean                   reconfig_required;
    } connect_msg;
    struct disconnect_msg_s {
      netmgr_address_set_t     *addr_info_ptr;
      boolean                   teardown_iface;
    } disconnect_msg;
    unsigned int               netd_restart_count;
  } info;                                        /* Payload info */
} netmgr_exec_cmd_data_t;

/*---------------------------------------------------------------------------
   Type of a Executive command
---------------------------------------------------------------------------*/
typedef struct netmgr_exec_cmd_s {
  ds_cmd_t                  cmd;                 /* Command object         */
  netmgr_exec_cmd_data_t    data;                /* Command data           */
  int                       tracker;             /* 1 if alloc, else 0     */
  int                       repost_count;        /* Number of time command
                                                    has been reposted      */
} netmgr_exec_cmd_t;


/*---------------------------------------------------------------------------
   Type representing collection of state information for each network link
---------------------------------------------------------------------------*/
struct netmgr_exec_link_s {
  struct stm_state_machine_s *sm;
};


/*---------------------------------------------------------------------------
   Type representing collection of state information for module
---------------------------------------------------------------------------*/
struct netmgr_exec_state_s {
  struct netmgr_exec_link_s   links[NETMGR_MAX_LINK];   /* Link state info */
  struct ds_cmdq_info_s       cmdq;  /* Command queue for async processing */
  int                         nlink;            /* Number of network links */
};


/*===========================================================================
                            GLOBAL FUNCTION DECLARATIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  netmgr_exec_get_cmd
===========================================================================*/
/*!
@brief
  Function to get a command buffer for asynchronous processing

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - Allocated heap memory
*/
/*=========================================================================*/
netmgr_exec_cmd_t * netmgr_exec_get_cmd ( void );


/*===========================================================================
  FUNCTION  netmgr_exec_release_cmd
===========================================================================*/
/*!
@brief
  Function to release a command buffer

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - Returns memory to heap
*/
/*=========================================================================*/
void netmgr_exec_release_cmd ( netmgr_exec_cmd_t * );


/*===========================================================================
  FUNCTION  netmgr_exec_put_cmd
===========================================================================*/
/*!
@brief
  Function to post a command buffer for asynchronous processing

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_exec_put_cmd ( const netmgr_exec_cmd_t * cmdbuf );


/*===========================================================================
  FUNCTION  netmgr_exec_wait
===========================================================================*/
/*!
@brief
  Forces calling thread to wait on exit of command queue thread.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - Calling thread is blocked indefinitely
*/
/*=========================================================================*/
void netmgr_exec_wait ( void );


/*===========================================================================
  FUNCTION  netmgr_exec_init
===========================================================================*/
/*!
@brief
 Main initialization routine of the executive control module.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - Initializes the QMI Driver
*/
/*=========================================================================*/
void netmgr_exec_init (int nlink, netmgr_ctl_port_config_type links[]);

#endif /* __NETMGR_EXEC_H__ */
