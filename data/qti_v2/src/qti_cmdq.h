/******************************************************************************

                          QTI _ C M D Q . H

******************************************************************************/

/******************************************************************************

  @file    qti_cmdq.h
  @brief   Tethering interface module command queues header file

  DESCRIPTION
  Header file for Tethering Interface module command queues.

  ---------------------------------------------------------------------------
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------

******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
5/29/2012  sb         Initial version

******************************************************************************/

#ifndef __QTI_CMDQ_H__
#define __QTI_CMDQ_H__

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#include "ds_cmdq.h"
#include "qti.h"


/*===========================================================================
                      DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define QTI_CMDQ_SUCCESS (0)

/*---------------------------------------------------------------------------
   Type of a Executive event data
---------------------------------------------------------------------------*/
typedef struct qti_cmdq_cmd_data_s {
	qti_event_e event;
	qti_interface_e interface;
} qti_cmdq_cmd_data_t;

/*---------------------------------------------------------------------------
   Type of a Executive command
---------------------------------------------------------------------------*/
typedef struct qti_cmdq_cmd_s {
  ds_cmd_t               cmd;                /* Command object         */
  qti_cmdq_cmd_data_t    data;               /* Command data           */
  int                    tracker;            /* 1 if alloc, else 0     */
} qti_cmdq_cmd_t;

/*---------------------------------------------------------------------------
   Type representing collection of state information for module
---------------------------------------------------------------------------*/
struct qti_cmdq_state_s {
  struct ds_cmdq_info_s       cmdq;  /* Command queue for async processing */
};


/*===========================================================================
                            GLOBAL FUNCTION DECLARATIONS
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
qti_cmdq_cmd_t * qti_cmdq_get_cmd ( void );

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
void qti_cmdq_release_cmd ( qti_cmdq_cmd_t * );

/*===========================================================================
  FUNCTION  qti_cmdq_put_cmd
===========================================================================*/
/*!
@brief
  Function to post a command buffer for asynchronous processing

@return
  int - QCMAP_SUCCESS on successful operation, QCMAP_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int qti_cmdq_put_cmd ( const qti_cmdq_cmd_t * cmdbuf );

/*===========================================================================
  FUNCTION  qcmap_cmdq_wait
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
void qti_cmdq_wait ( void );

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
extern void qti_cmdq_init (void);

/*===========================================================================
  FUNCTION qti_get_cmdq_length
===========================================================================*/
/*!
@brief
 Function returns number of outstanding messages in command queue.

@return
  void

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int qti_get_cmdq_length( void );

#endif /* __QTI_CMDQ_H__ */

