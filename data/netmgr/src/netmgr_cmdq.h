#ifndef _NETMGR_CMDQ_H_
#define _NETMGR_CMDQ_H_
/**
  @file
  netmgr_cmdq.h

  @brief
  Netmgr command executor component.

  @details
  The netmgr command executor component is used for
  asynchronous event processing. This module is defined
  to be seperate from the existing executor module to support
  events that cannot be handled by the state machine.

  The ports are opened using QCCI APIs (they must go over IPC-router since
  QMUXD transport would not be ready yet).

*/
/*===========================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
07/01/14   kannana   Initial version

===========================================================================*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ds_util.h"
#include "ds_cmdq.h"
#include "netmgr_qmi_dpm.h"

#ifdef FEATURE_DATA_IWLAN
#include "netmgr_iwlan_client.h"
#endif /* FEATURE_DATA_IWLAN */

#include "netmgr_util.h"

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

/* Definitions for netmgr util command queue framework */
#define NETMGR_CB_MAX_CMDS      (20)

/* The command executor module should support multiple
 * types of messages */
typedef enum
{
  NETMGR_CB_CMD_TYPE_INVALID = -1,
  NETMGR_CB_CMD_TYPE_DPM,
#ifdef FEATURE_DATA_IWLAN
  NETMGR_CB_CMD_TYPE_IWLAN,
#endif /* FEATURE_DATA_IWLAN */
  NETMGR_CB_CMD_TYPE_MAX
} netmgr_cmdq_cb_cmd_type_t;

typedef struct
{
  netmgr_cmdq_cb_cmd_type_t  cmd_type;
  union
  {
    netmgr_dpm_event_t  dpm_event;
#ifdef FEATURE_DATA_IWLAN
    netmgr_client_event_t iwlan_client_event;
#endif /* FEATURE_DATA_IWLAN */
  } netmgr_union;
} netmgr_cmdq_cb_cmd_data_t;

typedef struct
{
  ds_cmd_t                   cmd;
  netmgr_cmdq_cb_cmd_data_t  cmd_data;
} netmgr_cmdq_cb_cmd_t;

ds_cmdq_info_t               cb_cmdq;

/*=========================================================================
  FUNCTION:  netmgr_cmdq_init

===========================================================================*/
/*!
    @brief
    Initializes netmgr command queue

    @details
    For handling use-cases which do not fit into the main netmgr state
    machine, we will use a separate command executor thread.

    @return
    NETMGR_SUCCESS
    NETMGR_FAILURE
*/
/*=========================================================================*/
int netmgr_cmdq_init( void );

/*=========================================================================
  FUNCTION:  netmgr_cmdq_get_cmd

===========================================================================*/
/*!
    @brief
    Function to get a command object

    @return
    netmgr_cmdq_cb_cmd_t* type object on success
    NULL on failure
*/
/*=========================================================================*/
netmgr_cmdq_cb_cmd_t* netmgr_cmdq_get_cmd( void );

/*=========================================================================
  FUNCTION:  netmgr_cmdq_release_cmd

===========================================================================*/
/*!
    @brief
    Function to release a command object

    @return
    None
*/
/*=========================================================================*/
void netmgr_cmdq_release_cmd( netmgr_cmdq_cb_cmd_t *cmd_buf );

/*=========================================================================
  FUNCTION:  netmgr_cmdq_put_cmd

===========================================================================*/
/*!
    @brief
    Function to post to command queue

    @return
    NETMGR_SUCCESS
    NETMGR_FAILURE
*/
/*=========================================================================*/
int netmgr_cmdq_put_cmd( netmgr_cmdq_cb_cmd_t *cmd_buf );

#endif /* _NETMGR_CMDQ_H_ */
