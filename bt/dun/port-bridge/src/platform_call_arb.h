/******************************************************************************

  @file    platform_call_arb.h
  @brief   Platform Call Arbitration State Machine Header

  DESCRIPTION
  Header for the Platform Arbitration State Machine.

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

#ifndef __PLATFORM_CALL_ARB_H__
#define __PLATFORM_CALL_ARB_H__

#define FIFO_MODE 0666

/*Platform States*/
typedef enum {
    EMB_DATA_CALL_ERROR = 0,
    EMB_DATA_CALL_IDLE,
    EMB_DATA_CALL_CONNECTING,
    EMB_DATA_CALL_CONNECTED,
    EMB_DATA_CALL_DISCONNECTING,
    EMB_DATA_CALL_RECONNECTING,
    EMB_STATE_MAX,
} EMB_DATA_CALL_STATE_S;

/*State Definition*/
extern EMB_DATA_CALL_STATE_S pb_platform_state;


/*Dispatcher routine for the Platform SM*/
extern void process_platform_event(platform_event_msg_m msg);
/*Starts the Platform SM thread*/
extern int  start_platform_sm_thread(void);
/*Stops the Platform SM thread*/
extern void stop_platform_sm_thread(void);
/*Joins the Platform SM thread to the main thread*/
extern void join_plat_thread(void);
/*Posts events to platform pipe*/
extern void post_rmnet_event_to_platform (PLATFORM_EVENT_E event);
/*Posts events to the ipc platform pipe*/
extern void post_core_event_to_platform(int);

#endif /* __PLATFORM_CALL_ARB_H__ */
