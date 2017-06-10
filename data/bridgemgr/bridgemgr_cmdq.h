/******************************************************************************

                      B R I D G E M G R _ C M D Q . H

******************************************************************************/

/******************************************************************************

  @file    bridgemgr_cmdq.h
  @brief   Bridge Manager Command Queue Functions Header File

  DESCRIPTION
  Header file for BridgeMgr command queue functions.

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

#ifndef __BRIDGEMGR_CMDQ_H__
#define __BRIDGEMGR_CMDQ_H__

#include "ds_cmdq.h"
#include "bridgemgr_common.h"
#include "qmi_wds_srvc.h"


/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define BRIDGEMGR_CMDQ_MAX_QMUX_NETLINK_MSG 2048

/* Message buffer big enough to hold a QMUX or NETLINK message */
typedef struct
{
  unsigned char  msg[BRIDGEMGR_CMDQ_MAX_QMUX_NETLINK_MSG]; /* USB, MDM, PROXY*/
  int            msg_len;
} bridgemgr_cmdq_qmux_nl_msg_type;

/* Command for module re-initialization */
typedef struct
{
  bridgemgr_sys_type     sys;
  bridgemgr_reinit_type  type;
} bridgemgr_cmdq_reinit_cmd_type;

/* Typedef for command data */
typedef struct
{
  bridgemgr_sys_type sys;
  union
  {
    qmi_wds_indication_data_type     wds_ind;
    bridgemgr_cmdq_qmux_nl_msg_type  qmux_nl;
    bridgemgr_cmdq_reinit_cmd_type   reinit;
  } data;
} bridgemgr_cmdq_cmd_data_type;

/* Typedef for bridgemgr command type */
typedef struct
{
  ds_cmd_t                      ds_cmd;   /* Should be the first entry in the structure */
  bridgemgr_cmdq_cmd_data_type  cmd_data;
} bridgemgr_cmdq_cmd_type;


/*===========================================================================
                            GLOBAL FUNCTION DECLARATIONS
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
);


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
);


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
);


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
int bridgemgr_cmdq_init(void);


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
int bridgemgr_cmdq_deinit(void);


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
void bridgemgr_cmdq_wait(void);

#endif /* __BRIDGEMGR_CMDQ_H__ */

