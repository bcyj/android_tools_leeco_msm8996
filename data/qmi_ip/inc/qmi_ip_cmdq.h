#ifndef __QMI_IP_CMDQ_H__
#define __QMI_IP_CMDQ_H__
/******************************************************************************

                          QMI_IP_CMDQ.H

******************************************************************************/

/******************************************************************************

  @file    qmi_ip_cmdq.h
  @brief   QMI IP command queues header file

  DESCRIPTION
  Header file for QMI IP command queues.

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

#ifdef __cplusplus
extern "C"
{
#endif

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#include "comdef.h"
#include "ds_cmdq.h"
#include "qmi_ip.h"


/*===========================================================================
                      DEFINITIONS AND DECLARATIONS
===========================================================================*/

/*---------------------------------------------------------------------------
   Type of a Executive event data
---------------------------------------------------------------------------*/
typedef struct qmi_ip_cmdq_cmd_data_s {
	qmi_ip_event_e event;
  int            mode;
} qmi_ip_cmdq_cmd_data_t;

/*---------------------------------------------------------------------------
   Type of a Executive command
---------------------------------------------------------------------------*/
typedef struct qmi_ip_cmdq_cmd_s {
  ds_cmd_t                  cmd;                /* Command object         */
  qmi_ip_cmdq_cmd_data_t    data;               /* Command data           */
  int                       tracker;            /* 1 if alloc, else 0     */
} qmi_ip_cmdq_cmd_t;

/*---------------------------------------------------------------------------
   Type representing collection of state information for module
---------------------------------------------------------------------------*/
struct qmi_ip_cmdq_state_s {
  struct ds_cmdq_info_s       cmdq;  /* Command queue for async processing */
};


/*===========================================================================
                            GLOBAL FUNCTION DECLARATIONS
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
qmi_ip_cmdq_cmd_t * qmi_ip_cmdq_get_cmd ( void );

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
void qmi_ip_cmdq_release_cmd ( qmi_ip_cmdq_cmd_t * );

/*===========================================================================
  FUNCTION  qmi_ip_cmdq_put_cmd
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
int qmi_ip_cmdq_put_cmd ( const qmi_ip_cmdq_cmd_t * cmdbuf );

/*===========================================================================
  FUNCTION  qmi_ip_cmdq_wait
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
void qmi_ip_cmdq_wait ( void );

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
extern void qmi_ip_cmdq_init ( void );

#ifdef __cplusplus
}
#endif

#endif /* __QCMAP_CMDQ_H__ */

