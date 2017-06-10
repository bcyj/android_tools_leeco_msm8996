/******************************************************************************

                                D S _ C M D . H

******************************************************************************/

/******************************************************************************

  @file    ds_cmd.h
  @brief   Data Services command thread header file

  DESCRIPTION
  Header file for Data Services command processing thread.

  ---------------------------------------------------------------------------
  Copyright (c) 2010-2011 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/

/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id: ds_cmdq.h,v 1.4 2010/02/26 00:02:38 randrew Exp $

when       who        what, where, why
--------   ---        -------------------------------------------------------
02/11/10   ar         Initial version (derived from DSC file)

******************************************************************************/

#ifndef __DS_CMD_H__
#define __DS_CMD_H__

#include <pthread.h>
#include "comdef.h"
#include "ds_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

/*--------------------------------------------------------------------------- 
   Forward declaration needed by subsequent type definitions
---------------------------------------------------------------------------*/
struct ds_cmd_s;

/*--------------------------------------------------------------------------- 
   Type definition of virtual function to execute command
---------------------------------------------------------------------------*/
typedef void (* ds_cmd_execute_f) (struct ds_cmd_s *, void *);

/*--------------------------------------------------------------------------- 
   Type definition of virtual function to free (deallocate) command
---------------------------------------------------------------------------*/
typedef void (* ds_cmd_free_f)    (struct ds_cmd_s *, void *);

/*--------------------------------------------------------------------------- 
   Structure representing a generic command
---------------------------------------------------------------------------*/
typedef struct ds_cmd_s {
    ds_cmd_execute_f execute_f;
    ds_cmd_free_f    free_f;
    void            * data;
} ds_cmd_t;


/*--------------------------------------------------------------------------- 
   Collection of control info of the Command Thread
---------------------------------------------------------------------------*/
typedef struct ds_cmdq_info_s {
    ds_dll_el_t *   head;   /* Head node of cmd queue */
    ds_dll_el_t *   tail;   /* Tail node of cmd queue */
    int             nel;    /* Number of commands enqueued */
    int             nmax;   /* Maximum number of commands supported */
    pthread_t       thrd;   /* Command thread */
    pthread_cond_t  cond;   /* Condition variable for signaling */
    pthread_mutex_t mutx;   /* Mutex for protecting the list operations */
    boolean         running;/* Flag for processing thread state */ 
} ds_cmdq_info_t;

/*===========================================================================
                     GLOBAL FUNCTION DECLARATIONS
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
    - None
*/
/*=========================================================================*/
ds_cmd_t * ds_cmdq_alloc_cmd( void );

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
    - None
*/
/*=========================================================================*/
void ds_cmdq_release_cmd
(
  ds_cmd_t * cmd
);

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
);


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
);

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
);


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
ds_cmd_t * ds_cmdq_deq
(
  struct ds_cmdq_info_s * cmdq
);


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
);

#ifdef __cplusplus
}
#endif

#endif /* __DS_CMD_H__ */
