/******************************************************************************

                         QMI_IP_CMDQ.C

******************************************************************************/


/******************************************************************************

  @file    qmi_ip_cmdq.c
  @brief   QMI IP command queues

  DESCRIPTION
  Implementation of QMI IP command queues.

  ---------------------------------------------------------------------------
  Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

 ******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
10/23/2013 tw         Initial version

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <pthread.h>
#include <stdlib.h>
#include <assert.h>

#include "stm2.h"
#include "ds_list.h"
#include "ds_cmdq.h"

#include "qmi_ip_cmdq.h"



/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

int device_mode;
/*---------------------------------------------------------------------------
   Constant representing maximum number of command buffers used by this
   module
---------------------------------------------------------------------------*/
#define qmi_ip_cmdq_MAX_CMDS 5

/*---------------------------------------------------------------------------
   Executive control state information
---------------------------------------------------------------------------*/
LOCAL struct qmi_ip_cmdq_state_s  qmi_ip_cmdq_state_info;

/*===========================================================================
                            FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  qmi_ip_cmdq_cmd_free
===========================================================================*/
/*!
@brief
 Virtual function registered with the Command Thread to free a
 command buffer, after execution of the command is complete.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
qmi_ip_cmdq_cmd_free( ds_cmd_t * cmd, void * data )
{
  qmi_ip_cmdq_cmd_t * cmd_buf;

  /* Get qmi cmd ptr from user data ptr */
  cmd_buf = (qmi_ip_cmdq_cmd_t *)data;

  /* Unset tracker for debug purposes */
  cmd_buf->tracker = 0;

  /* Release dynamic memory */
  free( cmd_buf );

  LOG_MSG_INFO1("qcmap_cmdq: free one commmand data",0,0,0);

  return;
}

/*===========================================================================
  FUNCTION  qcmap_cmdq_cmd_process
===========================================================================*/
/*!
@brief
 Virtual function registered with the Command Thread to process a
 command buffer.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
qmi_ip_cmdq_cmd_process (ds_cmd_t * cmd, void * data)
{
  qmi_ip_cmdq_cmd_t * cmd_buf;

  /* Get qmi cmd ptr from user data ptr */
  cmd_buf = (qmi_ip_cmdq_cmd_t *)data;
  LOG_MSG_INFO1("Received QTI indication %d\n", cmd_buf->data.event, 0, 0);
  qmi_ip_process_link_event(cmd_buf->data.event, cmd_buf->data.mode);

  return;
}


/*===========================================================================
  FUNCTION  qmi_ip_cmdq_cleanup
===========================================================================*/
/*!
@brief
  Performs cleanup of executive module.  Invoked at process termination.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL void
qmi_ip_cmdq_cleanup
(
  void
)
{
  /* Purge command queue to release heap memory */
  (void)ds_cmdq_deinit( &qmi_ip_cmdq_state_info.cmdq );
}

/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  qmi_ip_cmdq_get_cmd
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
qmi_ip_cmdq_cmd_t * qmi_ip_cmdq_get_cmd ( void )
{
  qmi_ip_cmdq_cmd_t * cmd_buf = NULL;

  /* Allocate command buffer */
  if((cmd_buf = malloc(sizeof(qmi_ip_cmdq_cmd_t))) == NULL )
  {
    LOG_MSG_INFO1("qmi_ip_cmdq: malloc failed",0,0,0);
    return NULL;
  }

  /* Assign self-reference in DS cmd payload */
  cmd_buf->cmd.data      = (void*)cmd_buf;
  cmd_buf->tracker       = 1;

  /* Asssign default execution and free handlers */
  cmd_buf->cmd.execute_f = qmi_ip_cmdq_cmd_process;
  cmd_buf->cmd.free_f    = qmi_ip_cmdq_cmd_free;

  return cmd_buf;
}

/*===========================================================================
  FUNCTION  qmi_ip_cmdq_release_cmd
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
void qmi_ip_cmdq_release_cmd ( qmi_ip_cmdq_cmd_t * cmd_buf )
{
  if( cmd_buf->cmd.free_f )
  {
    cmd_buf->cmd.free_f( &cmd_buf->cmd, cmd_buf->cmd.data );
  }
  else
  {
    LOG_MSG_INFO1("qmi_ip_cmdq: Specified buffer not valid, ignoring\n",0,0,0);
  }
  return;
}

/*===========================================================================
  FUNCTION  qmi_ip_cmdq_put_cmd
===========================================================================*/
/*!
@brief
  Function to post a command buffer for asynchronous processing

@return
  int - 0 on successful operation, -1 otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int qmi_ip_cmdq_put_cmd ( const qmi_ip_cmdq_cmd_t * cmdbuf )
{
  /* Append command buffer to the command queue */
  int result = ds_cmdq_enq( &qmi_ip_cmdq_state_info.cmdq, &cmdbuf->cmd );

  return result;
}

/*===========================================================================
  FUNCTION  qmi_ip_cmdq_wait
===========================================================================*/
/*!
@brief
  Forces calling thread to wait on exit of command processing thread.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - Calling thread is blocked indefinitely
*/
/*=========================================================================*/
void qmi_ip_cmdq_wait ( void )
{
  ds_cmdq_join_thread( &qmi_ip_cmdq_state_info.cmdq );
}


/*===========================================================================
  FUNCTION  qmi_ip_cmdq_init
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
    - None
*/
/*=========================================================================*/
void qmi_ip_cmdq_init ( void )
{
  /* Register process termination cleanup handler */
  atexit( qmi_ip_cmdq_cleanup );

  LOG_MSG_INFO1("qmi_ip_cmdq: cmdq init\n",0,0,0);

  /*-------------------------------------------------------------------------
    Initialize command queue for asynch processing
  -------------------------------------------------------------------------*/
  ds_cmdq_init( &qmi_ip_cmdq_state_info.cmdq, qmi_ip_cmdq_MAX_CMDS );

  return;
}
