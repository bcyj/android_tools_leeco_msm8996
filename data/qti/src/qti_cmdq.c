/******************************************************************************

                         QTI _ C M D Q . C

******************************************************************************/


/******************************************************************************

  @file    qti_cmdq.c
  @brief   QTI command queues

  DESCRIPTION
  Implementation of QTI command queues.
 
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
5/29/2012  sb         Initial version

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

#include "qti_cmdq.h"
#include "ds_Utils_DebugMsg.h"



/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/


/*--------------------------------------------------------------------------- 
   Constant representing maximum number of command buffers used by this 
   module
---------------------------------------------------------------------------*/
#define QTI_CMDQ_MAX_CMDS 5

/*--------------------------------------------------------------------------- 
   Executive control state information
---------------------------------------------------------------------------*/
LOCAL struct qti_cmdq_state_s  qti_cmdq_state_info;

/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  qti_cmdq_cmd_free
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
qti_cmdq_cmd_free( ds_cmd_t * cmd, void * data )
{
  qti_cmdq_cmd_t * cmd_buf;
  
  /* Get qmi cmd ptr from user data ptr */
  cmd_buf = (qti_cmdq_cmd_t *)data;

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
qti_cmdq_cmd_process (ds_cmd_t * cmd, void * data)
{
  qti_cmdq_cmd_t * cmd_buf;

  printf("Processing command event\n");
 
  /* Get qmi cmd ptr from user data ptr */
  cmd_buf = (qti_cmdq_cmd_t *)data;
     
  qti_qmi_cmd_exec(cmd_buf->data.event, cmd_buf->data.interface);

  printf("Sucessfully processed command event\n");

  return;
}


/*===========================================================================
  FUNCTION  qti_cmdq_cleanup
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
qti_cmdq_cleanup
(
  void
)
{
  /* Purge command queue to release heap memory */
  (void)ds_cmdq_deinit( &qti_cmdq_state_info.cmdq );
}

/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  qti_cmdq_get_cmd
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
qti_cmdq_cmd_t * qti_cmdq_get_cmd ( void )
{
  qti_cmdq_cmd_t * cmd_buf = NULL;

  /* Allocate command buffer */
  if((cmd_buf = malloc(sizeof(qti_cmdq_cmd_t))) == NULL ) 
  {
    LOG_MSG_INFO1("qcmap_cmdq: malloc failed\n",0,0,0);  
    return NULL;
  } 

  /* Assign self-reference in DS cmd payload */
  cmd_buf->cmd.data      = (void*)cmd_buf;
  cmd_buf->tracker       = 1;

  /* Asssign default execution and free handlers */
  cmd_buf->cmd.execute_f = qti_cmdq_cmd_process;
  cmd_buf->cmd.free_f    = qti_cmdq_cmd_free;

  return cmd_buf;
}

/*===========================================================================
  FUNCTION  qti_cmdq_release_cmd
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
void qti_cmdq_release_cmd ( qti_cmdq_cmd_t * cmd_buf )
{
  if( cmd_buf->cmd.free_f ) 
  {
    cmd_buf->cmd.free_f( &cmd_buf->cmd, cmd_buf->cmd.data );
  } 
  else 
  {
    LOG_MSG_INFO1("qcmap_cmdq: Specified buffer not valid, ignoring\n",0,0,0);
  }
  return;
}

/*===========================================================================
  FUNCTION  qti_cmdq_put_cmd
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
int qti_cmdq_put_cmd ( const qti_cmdq_cmd_t * cmdbuf )
{
  /* Append command buffer to the command queue */
  int result = ds_cmdq_enq( &qti_cmdq_state_info.cmdq, &cmdbuf->cmd );

  printf("Successfully put command \n");

  return result;
}

/*===========================================================================
  FUNCTION  qti_cmdq_wait
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
void qti_cmdq_wait ( void )
{
  ds_cmdq_join_thread( &qti_cmdq_state_info.cmdq );
}


/*===========================================================================
  FUNCTION  qti_cmdq_init
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
void qti_cmdq_init ( void )
{
  /* Register process termination cleanup handler */
  atexit( qti_cmdq_cleanup );
   
  /*-------------------------------------------------------------------------
    Initialize command queue for asynch processing
  -------------------------------------------------------------------------*/
  ds_cmdq_init( &qti_cmdq_state_info.cmdq, QTI_CMDQ_MAX_CMDS );

  return;
}
