/******************************************************************************

                    B R I D G E M G R _ M D M _ Q M I . H

******************************************************************************/

/******************************************************************************

  @file    bridgemgr_mdm_qmi.h
  @brief   Bridge Manager MDM QMI Functions Header File

  DESCRIPTION
  Header file for BridgeMgr MDM QMI functions.

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

#ifndef __BRIDGEMGR_MDM_QMI_H__
#define __BRIDGEMGR_MDM_QMI_H__

#include "bridgemgr.h"


/*===========================================================================
                            GLOBAL FUNCTION DECLARATIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  bridgemgr_mdm_qmi_init
===========================================================================*/
/*!
@brief
  This function is called to initialize the BRIDGEMGR_SYS_MDM_QMI module

@param
  client_cb - Set of callbacks to register

@return
  BRIDGEMGR_SUCCESS - Module initialization was successful
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
int bridgemgr_mdm_qmi_init
(
  bridgemgr_client_callbacks_type *client_cb
);


/*===========================================================================
  FUNCTION  bridgemgr_mdm_qmi_ioctl
===========================================================================*/
/*!
@brief
  This function is called to issue ioctl commands on the MDM fd

@param
  cmd            - Command to issue
  result [inout] - Result to store or set

@return
  BRIDGEMGR_SUCCESS - on Success
  BRIDGEMGR_FAILURE - Otherwise

*/
/*=========================================================================*/
int bridgemgr_mdm_qmi_ioctl
(
  int            cmd,
  unsigned long  *result
);

#endif /* __BRIDGEMGR_MDM_QMI_H__ */
