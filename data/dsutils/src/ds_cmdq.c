/******************************************************************************

                               D S _ C M D Q . C

******************************************************************************/

/******************************************************************************

  @file    ds_cmdq.c
  @brief   Data Services command queue

  DESCRIPTION
  Implementation of DS command queue and processing thread.

  ---------------------------------------------------------------------------
  Copyright (c) 2010-2011,2014-2015 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id: ds_cmdq.c,v 1.3 2010/02/19 22:32:27 randrew Exp $

when       who        what, where, why
--------   ---        -------------------------------------------------------
02/11/10   ar         Initial version (derived from DSC file)

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>

#include "ds_util.h"
#include "ds_cmdq.h"

/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/


/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  ds_cmdthrd_main
===========================================================================*/
/*!
@brief
  The main function of Command Thread. Sits in a forever loop, waiting for
  commands to be enqueued, and executes them one by one in a FIFO manner.

@return
  void * - 0 on success, -1 on failure

@note

  - Dependencies
    - Requires Command Thread data structures to have been initialized

  - Side Effects
    - None
*/
/*=========================================================================*/
static void * ds_cmdthrd_main (void * info)
{
  ds_cmd_t * cmd;
  struct ds_cmdq_info_s * cmdq = (struct ds_cmdq_info_s *)info;

  /* validate input param */
  if( NULL == cmdq ) {
    ds_log_err("ds_cmdthrd_main: Bad Param cmdq NULL");
    return (void*)(intptr_t)-1;
  }

  cmdq->running = TRUE;

  /* Loop indefinitely, processing commands one by one */
  while( cmdq->running ) {
    /* Acquire mutex before dequeuing command */
    if (pthread_mutex_lock(&cmdq->mutx) != 0) {
      ds_log_sys_err("pthread_mutex_lock failed:");
      return (void*)(intptr_t)-1;
    }

    /* Try to dequeue command */
    if ((cmd = ds_cmdq_deq(cmdq)) == NULL) {
      /* No command in queue. Go to sleep on condition variable */
      if (pthread_cond_wait(&cmdq->cond, &cmdq->mutx)
          != 0)
      {
        ds_log_sys_err("pthread_cond_wait failed:");
        /* Release mutex*/
        if (pthread_mutex_unlock(&cmdq->mutx) != 0)
        {
          ds_log_sys_err("pthread_mutex_unlock failed:");
        }

        return (void*)(intptr_t)-1;
      }

      /* Signaled to wake up. Release mutex and continue with loop */
      if (pthread_mutex_unlock(&cmdq->mutx) != 0) {
        ds_log_sys_err("pthread_mutex_unlock failed:");
        return(void*)(intptr_t) -1;
      }
    } else {
      /* Dequeued valid command. Release mutex first before processing
      ** command.
      */
      if (pthread_mutex_unlock(&cmdq->mutx) != 0) {
        ds_log_sys_err("pthread_mutex_unlock failed:");
        return (void*)(intptr_t)-1;
      }

      /* Double check that execute func ptr is set in the cmd object */
      if( NULL == cmd->execute_f) {
        ds_log_err("ds_cmdthrd_main: NULL for cmd->execute_f");
        return (void*)(intptr_t)-1;
      }

      /* Execute command */
      (*(cmd->execute_f))(cmd, cmd->data);

      /* Call free handler, if set */
      if (cmd->free_f)
        (*(cmd->free_f))(cmd, cmd->data);
    }
  } /* end of while */

  return (void*)(intptr_t)0;
}

/*===========================================================================
  FUNCTION  ds_cmdthrd_init
===========================================================================*/
/*!
@brief
  Function for initializing and starting Command Thread. Must be called
  before clients can post commands for execution in Command Thread context.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static void ds_cmdthrd_init( struct ds_cmdq_info_s * cmdq )
{
  /* validate input param */
  if(NULL == cmdq) {
    ds_log_err("ds_cmdthrd_init: Bad Param cmdq NULL");
    return;
  }

  /* Start command thread */
  if( 0 != pthread_create( &cmdq->thrd,
                           NULL,
                           ds_cmdthrd_main,
                           (void*)cmdq ) )
  {
    ds_log_sys_err("Cannot start cmdthrd:");
    return;
  }
}

/*===========================================================================
  FUNCTION  ds_cmdthrd_deinit
===========================================================================*/
/*!
@brief
  Set flag to trigger Command Thread to exit.

@return
  None

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
static void ds_cmdthrd_deinit( struct ds_cmdq_info_s * cmdq )
{
  /* validate input param */
  if(NULL == cmdq) {
    ds_log_err("ds_cmdthrd_deinit: Bad Param cmdq NULL");
    return;
  }

  /* Terminate command thread */
  cmdq->running = FALSE;
  if( 0 > pthread_cond_signal( &cmdq->cond ) )
  {
    ds_log_sys_err("pthread_cond_signal failed:");
  }
}


/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  ds_cmdq_alloc_cmd
===========================================================================*/
/*!
@brief
  Allocate a generic command buffer

@return
  Pointer to command buffer, NULL if allocation failed

@note

  - Dependencies
    - None

  - Side Effects
    - Memory taken from heap
*/
/*=========================================================================*/
ds_cmd_t * ds_cmdq_alloc_cmd( void )
{
  ds_cmd_t * cmd = NULL;

  /* Allocate command buffer from heap */
  cmd = ds_malloc( sizeof(ds_cmd_t) );
  return cmd;
}

/*===========================================================================
  FUNCTION  ds_cmdq_release_cmd
===========================================================================*/
/*!
@brief
  Release a generic command buffer

@return
  None

@note

  - Dependencies
    - None

  - Side Effects
    - Memory returned to heap
*/
/*=========================================================================*/
void ds_cmdq_release_cmd
(
  ds_cmd_t * cmd
)
{
  /* Release command buffer back to heap */
  ds_free( cmd );
  return;
}

/*===========================================================================
  FUNCTION  ds_cmdq_enq
===========================================================================*/
/*!
@brief
  Used by clients to enqueue a command to the Command Thread's list of
  pending commands and execute it in the Command Thread context.

@return
  int - 0 on success, -1 on failure

@note

  - Dependencies
    - Assumes Command Thread has been initialized and is running.

  - Side Effects
    - None
*/
/*=========================================================================*/
int ds_cmdq_enq
(
  struct ds_cmdq_info_s * cmdq,
  const ds_cmd_t *        cmd
)
{
  ds_dll_el_t * node;

  /* Doesn't make sense to have a null execute function ptr */
  if(NULL == cmd->execute_f) {
    ds_log_err("ds_cmdq_enq: Bad Param cmd->execute_f NULL");
    return -1;
  }

  /* Make sure we have space for enqueuing the command */
  if (cmdq->nel > cmdq->nmax) {
    /* Configured length of command list exceeded, generate warning */
    ds_log_high( "Command queue exceeds configured maximum! %d > %d\n",
                 cmdq->nel, cmdq->nmax );
  }

  /* Acquire mutex before enqueuing to list */
  if (pthread_mutex_lock(&cmdq->mutx) < 0) {
    ds_log_sys_err("pthread_mutex_lock failed:");
    return -1;
  }

  /* Enqueue command to the tail of the command list */
  if ((node = ds_dll_enq(cmdq->tail, NULL, cmd)) == NULL)
  {
    ds_log_err("Failed to insert into cmdq\n");
    /* Release mutex*/
    if (pthread_mutex_unlock(&cmdq->mutx) != 0)
    {
      ds_log_sys_err("pthread_mutex_unlock failed:");
    }

    return -1;
  }
  cmdq->tail = node;

  /* Increment number of commands in list */
  ++cmdq->nel;

  /* If list was empty before we enqueued this command, signal the command
  ** thread to wake up and process the command.
  */
  if (cmdq->nel == 1) {
    if (pthread_cond_signal(&cmdq->cond) != 0)
    {
      ds_log_sys_err("pthread_cond_signal failed:");
      /* Release mutex*/
      if (pthread_mutex_unlock(&cmdq->mutx) != 0)
      {
        ds_log_sys_err("pthread_mutex_unlock failed:");
      }

      return -1;
    }
  }

  /* Release the mutex before returning from the function */
  if (pthread_mutex_unlock(&cmdq->mutx) < 0) {
    ds_log_sys_err("pthread_mutex_unlock failed:");
    return -1;
  }

  return 0;
}


/*===========================================================================
  FUNCTION  ds_cmdq_deq
===========================================================================*/
/*!
@brief
  Dequeues the first command from the FIFO list of commands pending
  execution and returns a pointer to it.  Caller must handle NULL return,
  indicating no command buffer was available.

@return
  ds_cmd_t * - pointer to command if one is enqueued, NULL otherwise

@note

  - Dependencies
    - Caller must acquire the Command Thread mutex before calling this
      function, as this function does not do any locking itself.

  - Side Effects
    - None
*/
/*=========================================================================*/
ds_cmd_t * ds_cmdq_deq( struct ds_cmdq_info_s * cmdq )
{
  ds_dll_el_t * node = NULL;
  ds_cmd_t * cmd = NULL;

  /* validate input param */
  if(NULL == cmdq) {
    ds_log_err("ds_cmdq_deq: Bad Param cmdq NULL");
    return NULL;
  }


  /* Dequeue next command from the head of the list */
  if (NULL !=
      (node = ds_dll_deq( cmdq->head,
                          &cmdq->tail,
                          (void *)&cmd) ) )
  {
    /* Valid command dequeued. Decrement number of cmds in list */
    --cmdq->nel;

    /* Free memory for this list node */
    ds_dll_free(node);
  } else {
    /* Do nothing as this is case handled by caller. */
  }

  /* Return cmd ptr, or null if no cmd was dequeued */
  return cmd;
}


/*===========================================================================
  FUNCTION  ds_cmdq_join_thread
===========================================================================*/
/*!
@brief
  Execute pthread_join on the command queue thread.  This causes the
  calling thread to wait on the command thread to exit (which may be
  never).

@return
  int - 0 on success, -1 on failure

@note

  - Dependencies
    - ds_cmdq_init() must have been invoked.

  - Side Effects
    - Calling thread is blocked indefinitely
*/
/*=========================================================================*/
int ds_cmdq_join_thread
(
  const struct ds_cmdq_info_s * cmdq
)
{
  /* validate input param */
  if(NULL == cmdq) {
    ds_log_err("ds_cmdq_join_thread: Bad Param cmdq NULL\n");
    return -1;
  }

  if( 0 == cmdq->thrd ) {
    ds_log_err("Command thread not initialized\n");
    return -1;
  }

  /* Join on the command thread */
  if( 0 != pthread_join( cmdq->thrd, NULL ) ) {
    ds_log_sys_err("pthread_join failed:");
    return -1;
  }

  return 0;
}

/*===========================================================================
  FUNCTION  ds_cmdq_deinit
===========================================================================*/
/*!
@brief
  Terminates the Command Thread and purges all pending commands in queue.

@return
  int - 0 on success, -1 on failure

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int ds_cmdq_deinit
(
  struct ds_cmdq_info_s * cmdq
)
{
  ds_cmd_t * cmd = NULL;

  /* validate input param */
  if(NULL == cmdq) {
    ds_log_err("ds_cmdq_join_thread: Bad Param cmdq NULL\n");
    return -1;
  }

  /* Kill command processing thread */
  ds_cmdthrd_deinit( cmdq );

  /* Wait for the thread to exit */
  ds_cmdq_join_thread( cmdq );

  /* Purge command queue to release heap memory */
  cmd = ds_cmdq_deq( cmdq );
  while( cmd )
  {
    /* Invoke free handler if defined */
    if( cmd->free_f )
      cmd->free_f( cmd, cmd->data );

    /* Get next command */
    cmd = ds_cmdq_deq( cmdq );
  }

  return 0;
}



/*===========================================================================
  FUNCTION  ds_cmdq_init
===========================================================================*/
/*!
@brief
  Initializes the command queueue data structures and spawns the
  Command Thread.

@return
  int - 0 on success, -1 on failure

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int ds_cmdq_init
(
  struct ds_cmdq_info_s * cmdq,
  unsigned int            nmax
)
{
  /* validate input param */
  if(NULL == cmdq) {
    ds_log_err("ds_cmdq_init: Bad Param cmdq NULL\n");
    return -1;
  }

  memset((void*)cmdq, 0x0, sizeof(struct ds_cmdq_info_s));

  /* Initialize the list of commands */
  if( (cmdq->head = ds_dll_init(NULL)) == NULL) {
    /* Error in initializing list. Abort.. */
    ds_log_err("Failed to allocate memory for cmdq\n");
    return -1;
  }
  cmdq->tail = cmdq->head;

  /* Initialize number of elements in the list and limit */
  cmdq->nel = 0;
  cmdq->nmax = (int)nmax;

  /* Initialize the mutex and condition variables */
  if( 0 != pthread_mutex_init(&cmdq->mutx, NULL) ) {
    ds_log_sys_err("pthread_mutex_init failed:");
    return -1;
  }
  if( 0 != pthread_cond_init(&cmdq->cond, NULL) ) {
    ds_log_sys_err("pthread_cond_init failed:");
    return -1;
  }

  /* Spawn command processing thread */
  ds_cmdthrd_init( cmdq );

  return 0;
}

