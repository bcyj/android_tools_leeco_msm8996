/******************************************************************************

  @file    platform_call_arb_kevents.h
  @brief   Platform Arbitration Rmnet Kernel Events Header

  DESCRIPTION
  Header for the rmnet kernel events.

 ******************************************************************************/
/*===========================================================================

  Copyright (c) 2010,2013 Qualcomm Technologies, Inc. All Rights Reserved

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

#ifndef __PLATFORM_CALL_ARB_KEVENTS_H__
#define __PLATFORM_CALL_ARB_KEVENTS_H__

#define DUN_INITIATED 0
#define DUN_END 1
#define SINGLE_PDP 1

/*RMNET STATES*/
typedef enum {
    RMNETSTATE_ERROR,
    RMNETSTATE_DOWN,
    RMNETSTATE_UP,
    RMNET_STATE_MAX,
} RMNETSTATE_EVENT_E;

/*Starts the rmnet monitoring thread*/
extern int   pb_start_rmnet_mon_thread(void);
/*Stops the rmnet monitoring thread*/
extern void  pb_stop_rmnet_mon_thread(void);
/*Disables the telephony call*/
extern void  pb_disable_embedded_data_call(void);
/*Enables the telephony*/
extern void  pb_enable_embedded_data_call(void);
/*Checks the status of the rmnet*/
extern RMNETSTATE_EVENT_E pb_rmnet_interface_check_status(void);

#endif /* __PLATFORM_CALL_ARB_KEVENTS_H__ */
