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
#include "netmgr_cmdq.h"

/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

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
int netmgr_cmdq_init()
{
  int rc;
  int ret = NETMGR_SUCCESS;

  netmgr_log_med("netmgr_cmdq_init(): Initiliazing netmgr_dpm cmdq");
  rc = ds_cmdq_init(&cb_cmdq, NETMGR_CB_MAX_CMDS);
  if (0 != rc)
  {
    netmgr_log_err("netmgr_cmdq_init(): Failed to initialize netmgr_dpm"
                   " command queue [%d]", rc);
    ret = NETMGR_FAILURE;
  }

  return ret;
}

/*===========================================================================
  FUNCTION:  netmgr_cmdq_cmd_exec
===========================================================================*/
/*!
    @brief
    Virtual function registered with the command thread to process
    a command

    @return None.
*/
/*=========================================================================*/
void netmgr_cmdq_cmd_exec
(
 ds_cmd_t *cmd,
 void *data
)
{
  netmgr_cmdq_cb_cmd_t*  cmd_buf;

  if ( NULL == cmd || NULL == data )
  {
    netmgr_log_err("netmgr_cmdq_cmd_exec(): Input parameters invalid");
    return;
  }

  cmd_buf = (netmgr_cmdq_cb_cmd_t*) data;

  switch (cmd_buf->cmd_data.cmd_type)
  {
  case NETMGR_CB_CMD_TYPE_DPM:
    netmgr_qmi_dpm_process_cmdq_event(cmd_buf->cmd_data.netmgr_union.dpm_event);
    break;
#ifdef FEATURE_DATA_IWLAN
  case NETMGR_CB_CMD_TYPE_IWLAN:
    netmgrIwlanClientProcessEvent(cmd_buf->cmd_data.netmgr_union.iwlan_client_event);
    break;
#endif /* FEATURE_DATA_IWLAN */
  default:
    netmgr_log_err("netmgr_cmdq_cmd_exec(): Invalid command type received!");
    break;
  }
}

/*===========================================================================
  FUNCTION:  netmgr_cmdq_cmd_free
===========================================================================*/
/*!
    @brief
    Virtual function registered with the command thread to free command
    buffer, after execution of the command is complete.

    @return None.
*/
/*=========================================================================*/
void netmgr_cmdq_cmd_free
(
  ds_cmd_t *cmd,
  void *data
)
{
  if (NULL == data || NULL == cmd)
  {
    netmgr_log_err("netmgr_cmdq_cmd_free(): Received invalid data!");
    return;
  }

  netmgr_cmdq_cb_cmd_t *cmd_buf = (netmgr_cmdq_cb_cmd_t*) data;

  free(cmd_buf);
}

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
netmgr_cmdq_cb_cmd_t* netmgr_cmdq_get_cmd( void )
{
  netmgr_cmdq_cb_cmd_t*  cmd_buf = NULL;

  /* Allocate command buffer */
  cmd_buf = (netmgr_cmdq_cb_cmd_t*) malloc( sizeof(netmgr_cmdq_cb_cmd_t) );
  if ( NULL == cmd_buf )
  {
    netmgr_log_err("netmgr_cmdq_get_cmd(): Command buffer allocation failed!");
    return NULL;
  }

  memset(cmd_buf, 0x0, sizeof(netmgr_cmdq_cb_cmd_t));

  /* Assign default execution and free handlers */
  cmd_buf->cmd.execute_f      = netmgr_cmdq_cmd_exec;
  cmd_buf->cmd.free_f         = netmgr_cmdq_cmd_free;
  cmd_buf->cmd.data           = (void*) cmd_buf;

  return cmd_buf;
}

/*=========================================================================
  FUNCTION:  netmgr_cmdq_release_cmd
===========================================================================*/
/*!
    @brief
    Function to get a command object

    @return
    None
*/
/*=========================================================================*/
void netmgr_cmdq_release_cmd( netmgr_cmdq_cb_cmd_t *cmd_buf )
{
  if (NULL != cmd_buf
      && cmd_buf->cmd.free_f ) {
    cmd_buf->cmd.free_f(&cmd_buf->cmd, cmd_buf->cmd.data);
  } else {
    netmgr_log_err("netmgr_cmdq_release_cmd(): Specified buffer not valid, ignoring");
  }
}

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
int netmgr_cmdq_put_cmd( netmgr_cmdq_cb_cmd_t *cmd_buf )
{
  int result = NETMGR_SUCCESS;

  if ( NULL == cmd_buf )
  {
    netmgr_log_err("netmgr_cmdq_put_cmd(): Command buffer is NULL!");
    return NETMGR_FAILURE;
  }

  /* Check to verify if the command obtained is valid */
  if ( cmd_buf->cmd_data.cmd_type > NETMGR_CB_CMD_TYPE_INVALID
       && cmd_buf->cmd_data.cmd_type < NETMGR_CB_CMD_TYPE_MAX)
  {
    netmgr_log_med("netmgr_cmdq_put_cmd(): Received command [%d]",
                   cmd_buf->cmd_data.cmd_type);

    /* Add to command queue */
    result = ds_cmdq_enq(&cb_cmdq, &cmd_buf->cmd);
  }
  else
  {
    netmgr_log_err("netmgr_cmdq_put_cmd(): Invalid command type received!");
    result = NETMGR_FAILURE;
  }

  return result;
}
