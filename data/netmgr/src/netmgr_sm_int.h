/*=============================================================================

    netmgr_sm_int.h

Description:
  This file contains the machine generated header file for the state machine
  specified in the file:
  ./netmgr_sm.stm

=============================================================================*/

/*===========================================================================

  Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved

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


#ifndef NETMGR_SM_INT_H
#define NETMGR_SM_INT_H

#ifdef __cplusplus
/* If compiled into a C++ file, ensure symbols names are not mangled */
extern "C"
{
#endif

/* Include external state machine header */
#include "netmgr_sm_ext.h"

/* Begin machine generated internal header for state machine array: NETMGR_SM[] */

/* Suppress Lint suggestions to const-ify state machine and payload ptrs */
/*lint -esym(818,sm,payload) */

/* Define a macro for the number of SM instances */
#define NETMGR_SM_NUM_INSTANCES 34

/* Define a macro for the number of SM states */
#define NETMGR_SM_NUM_STATES 8

/* Define a macro for the number of SM inputs */
#define NETMGR_SM_NUM_INPUTS 18

#ifndef STM_DATA_STRUCTURES_ONLY
/* State Machine entry/exit function prototypes */
void netmgr_sm_entry(stm_state_machine_t *sm,void *payload);
void netmgr_sm_exit(stm_state_machine_t *sm,void *payload);


/* State entry/exit function prototypes */
void netmgr_sm_state_down_entry(stm_state_machine_t *sm,stm_state_t _state,void *payload);
void netmgr_sm_state_down_exit(stm_state_machine_t *sm,stm_state_t _state,void *payload);
void netmgr_sm_state_inited_entry(stm_state_machine_t *sm,stm_state_t _state,void *payload);
void netmgr_sm_state_inited_exit(stm_state_machine_t *sm,stm_state_t _state,void *payload);
void netmgr_sm_state_comingup_entry(stm_state_machine_t *sm,stm_state_t _state,void *payload);
void netmgr_sm_state_comingup_exit(stm_state_machine_t *sm,stm_state_t _state,void *payload);
void netmgr_sm_state_configuring_entry(stm_state_machine_t *sm,stm_state_t _state,void *payload);
void netmgr_sm_state_configuring_exit(stm_state_machine_t *sm,stm_state_t _state,void *payload);
void netmgr_sm_state_reconfiguring_entry(stm_state_machine_t *sm,stm_state_t _state,void *payload);
void netmgr_sm_state_reconfiguring_exit(stm_state_machine_t *sm,stm_state_t _state,void *payload);
void netmgr_sm_state_up_entry(stm_state_machine_t *sm,stm_state_t _state,void *payload);
void netmgr_sm_state_up_exit(stm_state_machine_t *sm,stm_state_t _state,void *payload);
void netmgr_sm_state_goingdown_entry(stm_state_machine_t *sm,stm_state_t _state,void *payload);
void netmgr_sm_state_goingdown_exit(stm_state_machine_t *sm,stm_state_t _state,void *payload);
void netmgr_sm_state_goingdowntocomeup_entry(stm_state_machine_t *sm,stm_state_t _state,void *payload);
void netmgr_sm_state_goingdowntocomeup_exit(stm_state_machine_t *sm,stm_state_t _state,void *payload);


/* Transition function prototypes */
stm_state_t netmgr_sm_inited_msg(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_modem_is_msg(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_modem_oos_msg(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_dispatch_kif_msg(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_dispatch_qmi_msg(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_modem_reset_msg(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_modem_connected(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_qos_delete(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_repost_modem_connected(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_modem_disconnected(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_kif_opened(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_kif_closed(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_kif_configured(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_modem_reconfig(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_qos_activate(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_qos_modify(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_qos_suspend(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_qos_flow_control(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_modem_connected_while_going_down(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_kif_opened_while_going_down(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_kif_down_to_come_up(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_modem_disconnected_in_inited(stm_state_machine_t *sm, void *payload);
stm_state_t netmgr_sm_netd_restart(stm_state_machine_t *sm, void *payload);


/* State enumeration */
enum
{
  NETMGR_STATE_DOWN,
  NETMGR_STATE_INITED,
  NETMGR_STATE_COMING_UP,
  NETMGR_STATE_CONFIGURING,
  NETMGR_STATE_RECONFIGURING,
  NETMGR_STATE_UP,
  NETMGR_STATE_GOING_DOWN,
  NETMGR_STATE_GOING_DOWN_TO_COME_UP,
};

#endif /* !STM_DATA_STRUCTURES_ONLY */

/* End machine generated internal header for state machine array: NETMGR_SM[] */


#ifdef __cplusplus
} /* extern "C" {...} */
#endif

#endif /* ! NETMGR_SM_INT_H */
