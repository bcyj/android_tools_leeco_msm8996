/******************************************************************************

                        B R I D G E M G R_ C M D Q . C

******************************************************************************/

/******************************************************************************

  @file    bridgemgr_cmdq.c
  @brief   Bridge Manager Command Queue

  DESCRIPTION
  Implementation of BridgeMgr command queue module.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved

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

#include "ds_list.h"
#include "ds_cmdq.h"
#include "bridgemgr.h"
#include "bridgemgr_cmdq.h"


/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

/* Max commands in the queue */
#define BRIDGEMGR_CMDQ_MAX_CMDS 25

/* BridgeMgr command queue */
static struct ds_cmdq_info_s bridgemgr_cmdq;


/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  bridgemgr_cmdq_free_cmd_cb
===========================================================================*/
/*!
@brief
 Callback function registered with the Command Thread to free a command buffer
 after execution of the command is complete

@param 
  ds_cmd - Command to be freed
  data   - Command data

@return
  void

*/
/*=========================================================================*/
static void bridgemgr_cmdq_free_cmd_cb
(
  ds_cmd_t *ds_cmd,
  void *data
)
{
  /* Free the memory allocated for the command */
  free((bridgemgr_cmdq_cmd_type *)ds_cmd);
}


/*===========================================================================
  FUNCTION  bridgemgr_cmdq_execute_cmd_cb
===========================================================================*/
/*!
@brief
 Callback function registered with the Command Thread to process a command

@param
  ds_cmd - Command to process
  data   - Command data

@return
  void

*/
/*=========================================================================*/
static void bridgemgr_cmdq_execute_cmd_cb
(
  ds_cmd_t *ds_cmd,
  void *data
)
{
  bridgemgr_cmdq_cmd_type *cmd = (bridgemgr_cmdq_cmd_type *)ds_cmd;

  if (NULL == cmd)
  {
    bridgemgr_log_err("bridgemgr_cmdq_execute_cmd_cb: bad params\n");
    return;
  }

  /* Call the bridgemgr cmd dispatcher to send it to the appropriate subsystem */
  bridgemgr_cmd_dispatcher(&cmd->cmd_data);
}


/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  bridgemgr_cmdq_alloc_cmd
===========================================================================*/
/*!
@brief
  Function to obtain a new command buffer for async processing

@param 
  sys - The module with which the command will be associated with
 
@return
  Pointer to the new command - on success
  NULL                       - otherwise

*/
/*=========================================================================*/
bridgemgr_cmdq_cmd_type *bridgemgr_cmdq_alloc_cmd
(
  bridgemgr_sys_type sys
)
{
  bridgemgr_cmdq_cmd_type *cmd = NULL;

  /* Allocate a new command */
  if (NULL == (cmd = malloc(sizeof(bridgemgr_cmdq_cmd_type))))
  {
    bridgemgr_log_err("bridgemgr_cmdq_alloc_cmd: alloc failed\n");
    return NULL;
  }

  cmd->cmd_data.sys = sys;
  cmd->ds_cmd.data  = NULL;

  /* Asssign default execution and free handlers */
  cmd->ds_cmd.execute_f = bridgemgr_cmdq_execute_cmd_cb;
  cmd->ds_cmd.free_f    = bridgemgr_cmdq_free_cmd_cb;

  return cmd;
}


/*===========================================================================
  FUNCTION  bridgemgr_cmdq_free_cmd
===========================================================================*/
/*!
@brief
  Function to release a previously allocated command buffer 

@param
  cmd - The command to free

@return 
  none 

*/
/*=========================================================================*/
void bridgemgr_cmdq_free_cmd
(
  bridgemgr_cmdq_cmd_type *cmd
)
{
  if (NULL == cmd)
  {
    bridgemgr_log_err("bridgemgr_cmdq_free_cmd: bad params\n");
    return;
  }

  if (NULL != cmd->ds_cmd.free_f)
  {
    cmd->ds_cmd.free_f(&cmd->ds_cmd, cmd->ds_cmd.data );
  }
  else
  {
    bridgemgr_log_err("bridgemgr_cmdq_free_cmd: free_f NULL\n");
  }
}


/*===========================================================================
  FUNCTION  bridgemgr_cmdq_enqueue_cmd
===========================================================================*/
/*!
@brief
  This function enqueues a command into the command queue for async processing

@param 
  cmd - The command to be enqueued
 
@return
  BRIDGEMGR_SUCCESS - If enqueue was successful
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
int bridgemgr_cmdq_enqueue_cmd
(
  const bridgemgr_cmdq_cmd_type *cmd
)
{
  if (NULL == cmd)
  {
    bridgemgr_log_err("bridgemgr_cmdq_enqueue_cmd: bad params\n");
    return BRIDGEMGR_FAILURE;
  }

  /* Enqueue the command */
  return ds_cmdq_enq( &bridgemgr_cmdq, (const ds_cmd_t *)cmd);
}


/*===========================================================================
  FUNCTION  bridgemgr_cmdq_init
===========================================================================*/
/*!
@brief
  This function initializes the command queue used for async processing

@param 
  none
 
@return
  BRIDGEMGR_SUCCESS - If initialization was successful
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
int bridgemgr_cmdq_init(void)
{
  /* Initialize the command queue */
  return ds_cmdq_init(&bridgemgr_cmdq, BRIDGEMGR_CMDQ_MAX_CMDS);
}


/*===========================================================================
  FUNCTION  bridgemgr_cmdq_deinit
===========================================================================*/
/*!
@brief
  This function deinitializes the command queue used for async processing

@param 
  None 

@return
  BRIDGEMGR_SUCCESS
  BRIDGEMGR_FAILURE

*/
/*=========================================================================*/
int bridgemgr_cmdq_deinit(void)
{
  /* Deinitialize the command queue */
  return ds_cmdq_deinit(&bridgemgr_cmdq);
}


/*===========================================================================
  FUNCTION  bridgemgr_cmdq_wait
===========================================================================*/
/*!
@brief
  This function waits for the cmdq thread to exit

@param
  None

@return
  None

@note
  The calling thread blocks indefinitely

*/
/*=========================================================================*/
void bridgemgr_cmdq_wait(void)
{
  ds_cmdq_join_thread(&bridgemgr_cmdq);
}
