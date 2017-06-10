/******************************************************************************

  @file    portbridge_core.h
  @brief   Portbridge Core State Machine Header

  DESCRIPTION
  Header for the Portbridge Core State Machine.

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

#ifndef __PORTBRIDGE_CORE_H__
#define __PORTBRIDGE_CORE_H__

/*Condition variable and associated mutex which are by CORE SM to wait to pass
  on the AT cmd to the mode, until it recvs rmnet down from PLATFORM SM*/
extern pthread_mutex_t pb_signal_ext_host_mutex;
extern pthread_cond_t  pb_signal_ext_host;
extern pthread_mutex_t fd_close_mutex;

/*Core States*/
typedef enum {
    DUN_STATE_ERROR = 0,
    DUN_STATE_DISCONNECTED,
    DUN_STATE_IDLE,
    DUN_STATE_CONNECTING,
    DUN_STATE_CONNECTED,
    DUN_STATE_MAX,
} DUN_STATE_S;

/*State Definition*/
extern DUN_STATE_S pb_dun_state;

/*DUN COMMANDS*/
typedef enum {
    DUN_ATCMD_INVALID,
    DUN_ATCMD_START,
    DUN_ATCMD_STOP,
    DUN_ATCMD_MAX
} DUN_ATCMD_E;

/*Dispatcher routine for the CORE SM*/
extern void process_dun_event (dun_event_msg_m msg);
/*Starts the CORE SM thread*/
extern int  start_core_sm_thread(void);
/*Stops the CORE SM thread*/
extern void stop_core_sm_thread(void);
/*Joins the CORE SM thread to the main thread*/
extern void join_core_thread(void);
/*Posts events to core pipe*/
extern void post_ext_host_event_to_core(DUN_EVENT_E event);
/*Posts events to the ipc core pipe*/
extern void post_platform_event_to_core(int);

#endif  /* __PORTBRIDGE_CORE_H__ */
