/******************************************************************************

                         Q C M A P _ C M D Q . C

******************************************************************************/
/*===========================================================================

Copyright (c) 2012-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/******************************************************************************

  @file    qcmap_cmdq.c
  @brief   QCMAP command queues

  DESCRIPTION
  Implementation of QCMAP command queues.

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
7/11/2012  gk         9x25

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

#include "qcmap_cmdq.h"
#include "qmi_client.h"
#include "qualcomm_mobile_access_point_msgr_v01.h"
#include "network_access_service_v01.h"
#include "qcmap_cm_api.h"


/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/


/*--------------------------------------------------------------------------- 
   Constant representing maximum number of command buffers used by this 
   module
---------------------------------------------------------------------------*/
#define QCMAP_CMDQ_MAX_CMDS 5

/*--------------------------------------------------------------------------- 
   Executive control state information
---------------------------------------------------------------------------*/
LOCAL struct qcmap_cmdq_state_s  qcmap_cmdq_state_info;
void  process_connectbackhaul_cmd();
void qcmap_auto_con_release_cmd ( ds_cmd_t *cmd);
LOCAL void qcmap_auto_con_cmd_process (ds_cmd_t * cmd, void * data);
/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  qcmap_cmdq_cmd_free
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
qcmap_cmdq_cmd_free( ds_cmd_t * cmd, void * data )
{
  qcmap_cmdq_cmd_t * cmd_buf;
  
  /* Get qmi cmd ptr from user data ptr */
  cmd_buf = (qcmap_cmdq_cmd_t *)data;

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
qcmap_cmdq_cmd_process (ds_cmd_t * cmd, void * data)
{
  qcmap_cmdq_cmd_t * cmd_buf;
  qcmap_cmdq_cmd_t * cmd_buf;
 
  /* Get qmi cmd ptr from user data ptr */
  cmd_buf = (qcmap_cmdq_cmd_t *)data;
     
  qcmap_cm_dsi_net_cb_fcn(cmd_buf->data.dsi_nethandle, cmd_buf->data.user_data, cmd_buf->data.evt, cmd_buf->data.payload_ptr);

  return;
}

/*===========================================================================
  FUNCTION  qcmap_auto_con_cmd_process 
===========================================================================*/
/*!
@brief
 Virtual function registered with the Command Thread to process a  
 auto connect request. 

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
qcmap_auto_con_cmd_process (ds_cmd_t * cmd, void * data)
{
  qcmap_cmdq_cmd_t * cmd_buf;
  /* Get qmi cmd ptr from user data ptr */
  process_connectbackhaul_cmd();
  return;
}


/*===========================================================================
  FUNCTION  qcmap_cmdq_cleanup
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
qcmap_cmdq_cleanup
(
  void
)
{
  /* Purge command queue to release heap memory */
  (void)ds_cmdq_deinit( &qcmap_cmdq_state_info.cmdq );
}

/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  qcmap_cmdq_get_cmd
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
qcmap_cmdq_cmd_t * qcmap_cmdq_get_cmd ( void )
{
  qcmap_cmdq_cmd_t * cmd_buf = NULL;

  /* Allocate command buffer */
  if((cmd_buf = malloc(sizeof(qcmap_cmdq_cmd_t))) == NULL ) 
  {
    LOG_MSG_INFO1("qcmap_cmdq: malloc failed\n",0,0,0);  
    return NULL;
  } 

  /* Assign self-reference in DS cmd payload */
  cmd_buf->cmd.data      = (void*)cmd_buf;
  cmd_buf->tracker       = 1;

  /* Asssign default execution and free handlers */
  cmd_buf->cmd.execute_f = qcmap_cmdq_cmd_process;
  cmd_buf->cmd.free_f    = qcmap_cmdq_cmd_free;

  return cmd_buf;
}

ds_cmd_t * qcmap_auto_con_get_cmd ( void )
{
 ds_cmd_t * cmd = NULL;

  /* Allocate command buffer */
  if((cmd = malloc(sizeof(ds_cmd_t))) == NULL ) 
  {
    LOG_MSG_INFO1("qcmap_cmdq: malloc failed\n",0,0,0);  
    return NULL;
  } 

  /* Assign self-reference in DS cmd payload */
  cmd->data      = NULL;

  /* Asssign default execution and free handlers */
  cmd->execute_f = qcmap_auto_con_cmd_process ;
  cmd->free_f    = qcmap_auto_con_release_cmd;

  return cmd;
}



/*===========================================================================
  FUNCTION  qcmap_cmdq_release_cmd
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
void qcmap_cmdq_release_cmd ( qcmap_cmdq_cmd_t * cmd_buf )
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

void qcmap_auto_con_release_cmd ( ds_cmd_t *cmd)
{
  if(cmd)
  free(cmd);
  cmd=NULL; 
  return;
}

/*===========================================================================
  FUNCTION  qcmap_cmdq_put_cmd
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
int qcmap_cmdq_put_cmd ( const qcmap_cmdq_cmd_t * cmdbuf )
{
  /* Append command buffer to the command queue */
  int result = ds_cmdq_enq( &qcmap_cmdq_state_info.cmdq, &cmdbuf->cmd );

  return result;
}


int qcmap_auto_con_put_cmd (ds_cmd_t *cmd )
{
  /* Append command buffer to the command queue */
  int result = ds_cmdq_enq( &qcmap_cmdq_state_info.cmdq, cmd );

  return result;
}


/*===========================================================================
  FUNCTION  qcmap_cmdq_wait
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
void qcmap_cmdq_wait ( void )
{
  ds_cmdq_join_thread( &qcmap_cmdq_state_info.cmdq );
}


/*===========================================================================
  FUNCTION  qcmap_cmdq_init
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
void qcmap_cmdq_init ( void )
{
  /* Register process termination cleanup handler */
  atexit( qcmap_cmdq_cleanup );
   
  /*-------------------------------------------------------------------------
    Initialize command queue for asynch processing
  -------------------------------------------------------------------------*/
  ds_cmdq_init( &qcmap_cmdq_state_info.cmdq, QCMAP_CMDQ_MAX_CMDS );

  return;
}

