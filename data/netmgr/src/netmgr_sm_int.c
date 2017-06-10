/*=============================================================================

    netmgr_sm_int.c

Description:
  This file contains the machine generated source file for the state machine
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


/* Include STM compiler generated external and internal header files */
#include "netmgr_sm_ext.h"
#include "netmgr_sm_int.h"

/* Include INPUT_DEF_FILE specified files */
#include <netmgr_sm.h>

/* Begin machine generated internal source for state machine array: NETMGR_SM[] */

#ifndef STM_DATA_STRUCTURES_ONLY
/* Transition table */
static const stm_transition_fn_t
  NETMGR_SM_transitions[ NETMGR_SM_NUM_STATES * NETMGR_SM_NUM_INPUTS ] =
{
  /* Transition functions for state NETMGR_STATE_DOWN */
  netmgr_sm_inited_msg,    /* NETMGR_INITED_EV */
  netmgr_sm_modem_is_msg,    /* NETMGR_MODEM_IS_EV */
  netmgr_sm_modem_oos_msg,    /* NETMGR_MODEM_OOS_EV */
  netmgr_sm_dispatch_kif_msg,    /* NETMGR_KIF_MSG_CMD */
  netmgr_sm_dispatch_qmi_msg,    /* NETMGR_QMI_MSG_CMD */
  netmgr_sm_modem_reset_msg,    /* NETMGR_RESET_MSG_CMD */
  NULL,    /* NETMGR_WDS_CONNECTED_EV */
  NULL,    /* NETMGR_QOS_DELETE_EV */
  NULL,    /* NETMGR_WDS_DISCONNECTED_EV */
  NULL,    /* NETMGR_KIF_OPENED_EV */
  NULL,    /* NETMGR_KIF_CLOSED_EV */
  NULL,    /* NETMGR_KIF_CONFIGURED_EV */
  NULL,    /* NETMGR_KIF_RECONFIGURED_EV */
  NULL,    /* NETMGR_QOS_ACTIVATE_EV */
  NULL,    /* NETMGR_QOS_MODIFY_EV */
  NULL,    /* NETMGR_QOS_SUSPEND_EV */
  NULL,    /* NETMGR_QOS_FLOCNTRL_EV */
  netmgr_sm_netd_restart,    /* NETMGR_NETD_RESTART_EV */

  /* Transition functions for state NETMGR_STATE_INITED */
  NULL,    /* NETMGR_INITED_EV */
  NULL,    /* NETMGR_MODEM_IS_EV */
  netmgr_sm_modem_oos_msg,    /* NETMGR_MODEM_OOS_EV */
  netmgr_sm_dispatch_kif_msg,    /* NETMGR_KIF_MSG_CMD */
  netmgr_sm_dispatch_qmi_msg,    /* NETMGR_QMI_MSG_CMD */
  netmgr_sm_modem_reset_msg,    /* NETMGR_RESET_MSG_CMD */
  netmgr_sm_modem_connected,    /* NETMGR_WDS_CONNECTED_EV */
  netmgr_sm_qos_delete,    /* NETMGR_QOS_DELETE_EV */
  netmgr_sm_modem_disconnected_in_inited,    /* NETMGR_WDS_DISCONNECTED_EV */
  NULL,    /* NETMGR_KIF_OPENED_EV */
  NULL,    /* NETMGR_KIF_CLOSED_EV */
  NULL,    /* NETMGR_KIF_CONFIGURED_EV */
  NULL,    /* NETMGR_KIF_RECONFIGURED_EV */
  netmgr_sm_qos_activate,    /* NETMGR_QOS_ACTIVATE_EV */
  NULL,    /* NETMGR_QOS_MODIFY_EV */
  netmgr_sm_qos_suspend,    /* NETMGR_QOS_SUSPEND_EV */
  NULL,    /* NETMGR_QOS_FLOCNTRL_EV */
  netmgr_sm_netd_restart,    /* NETMGR_NETD_RESTART_EV */

  /* Transition functions for state NETMGR_STATE_COMING_UP */
  NULL,    /* NETMGR_INITED_EV */
  NULL,    /* NETMGR_MODEM_IS_EV */
  netmgr_sm_modem_oos_msg,    /* NETMGR_MODEM_OOS_EV */
  netmgr_sm_dispatch_kif_msg,    /* NETMGR_KIF_MSG_CMD */
  netmgr_sm_dispatch_qmi_msg,    /* NETMGR_QMI_MSG_CMD */
  netmgr_sm_modem_reset_msg,    /* NETMGR_RESET_MSG_CMD */
  netmgr_sm_repost_modem_connected,    /* NETMGR_WDS_CONNECTED_EV */
  NULL,    /* NETMGR_QOS_DELETE_EV */
  netmgr_sm_modem_disconnected,    /* NETMGR_WDS_DISCONNECTED_EV */
  netmgr_sm_kif_opened,    /* NETMGR_KIF_OPENED_EV */
  netmgr_sm_kif_closed,    /* NETMGR_KIF_CLOSED_EV */
  NULL,    /* NETMGR_KIF_CONFIGURED_EV */
  NULL,    /* NETMGR_KIF_RECONFIGURED_EV */
  netmgr_sm_qos_activate,    /* NETMGR_QOS_ACTIVATE_EV */
  NULL,    /* NETMGR_QOS_MODIFY_EV */
  netmgr_sm_qos_suspend,    /* NETMGR_QOS_SUSPEND_EV */
  NULL,    /* NETMGR_QOS_FLOCNTRL_EV */
  netmgr_sm_netd_restart,    /* NETMGR_NETD_RESTART_EV */

  /* Transition functions for state NETMGR_STATE_CONFIGURING */
  NULL,    /* NETMGR_INITED_EV */
  NULL,    /* NETMGR_MODEM_IS_EV */
  netmgr_sm_modem_oos_msg,    /* NETMGR_MODEM_OOS_EV */
  netmgr_sm_dispatch_kif_msg,    /* NETMGR_KIF_MSG_CMD */
  netmgr_sm_dispatch_qmi_msg,    /* NETMGR_QMI_MSG_CMD */
  netmgr_sm_modem_reset_msg,    /* NETMGR_RESET_MSG_CMD */
  netmgr_sm_repost_modem_connected,    /* NETMGR_WDS_CONNECTED_EV */
  NULL,    /* NETMGR_QOS_DELETE_EV */
  netmgr_sm_modem_disconnected,    /* NETMGR_WDS_DISCONNECTED_EV */
  NULL,    /* NETMGR_KIF_OPENED_EV */
  netmgr_sm_kif_closed,    /* NETMGR_KIF_CLOSED_EV */
  netmgr_sm_kif_configured,    /* NETMGR_KIF_CONFIGURED_EV */
  NULL,    /* NETMGR_KIF_RECONFIGURED_EV */
  netmgr_sm_qos_activate,    /* NETMGR_QOS_ACTIVATE_EV */
  NULL,    /* NETMGR_QOS_MODIFY_EV */
  netmgr_sm_qos_suspend,    /* NETMGR_QOS_SUSPEND_EV */
  NULL,    /* NETMGR_QOS_FLOCNTRL_EV */
  netmgr_sm_netd_restart,    /* NETMGR_NETD_RESTART_EV */

  /* Transition functions for state NETMGR_STATE_RECONFIGURING */
  NULL,    /* NETMGR_INITED_EV */
  NULL,    /* NETMGR_MODEM_IS_EV */
  netmgr_sm_modem_oos_msg,    /* NETMGR_MODEM_OOS_EV */
  netmgr_sm_dispatch_kif_msg,    /* NETMGR_KIF_MSG_CMD */
  netmgr_sm_dispatch_qmi_msg,    /* NETMGR_QMI_MSG_CMD */
  netmgr_sm_modem_reset_msg,    /* NETMGR_RESET_MSG_CMD */
  netmgr_sm_modem_reconfig,    /* NETMGR_WDS_CONNECTED_EV */
  NULL,    /* NETMGR_QOS_DELETE_EV */
  netmgr_sm_modem_disconnected,    /* NETMGR_WDS_DISCONNECTED_EV */
  NULL,    /* NETMGR_KIF_OPENED_EV */
  netmgr_sm_kif_closed,    /* NETMGR_KIF_CLOSED_EV */
  NULL,    /* NETMGR_KIF_CONFIGURED_EV */
  netmgr_sm_kif_configured,    /* NETMGR_KIF_RECONFIGURED_EV */
  netmgr_sm_qos_activate,    /* NETMGR_QOS_ACTIVATE_EV */
  NULL,    /* NETMGR_QOS_MODIFY_EV */
  netmgr_sm_qos_suspend,    /* NETMGR_QOS_SUSPEND_EV */
  netmgr_sm_qos_flow_control,    /* NETMGR_QOS_FLOCNTRL_EV */
  netmgr_sm_netd_restart,    /* NETMGR_NETD_RESTART_EV */

  /* Transition functions for state NETMGR_STATE_UP */
  NULL,    /* NETMGR_INITED_EV */
  NULL,    /* NETMGR_MODEM_IS_EV */
  netmgr_sm_modem_oos_msg,    /* NETMGR_MODEM_OOS_EV */
  netmgr_sm_dispatch_kif_msg,    /* NETMGR_KIF_MSG_CMD */
  netmgr_sm_dispatch_qmi_msg,    /* NETMGR_QMI_MSG_CMD */
  netmgr_sm_modem_reset_msg,    /* NETMGR_RESET_MSG_CMD */
  netmgr_sm_modem_reconfig,    /* NETMGR_WDS_CONNECTED_EV */
  netmgr_sm_qos_delete,    /* NETMGR_QOS_DELETE_EV */
  netmgr_sm_modem_disconnected,    /* NETMGR_WDS_DISCONNECTED_EV */
  netmgr_sm_kif_opened,    /* NETMGR_KIF_OPENED_EV */
  netmgr_sm_kif_closed,    /* NETMGR_KIF_CLOSED_EV */
  NULL,    /* NETMGR_KIF_CONFIGURED_EV */
  NULL,    /* NETMGR_KIF_RECONFIGURED_EV */
  netmgr_sm_qos_activate,    /* NETMGR_QOS_ACTIVATE_EV */
  netmgr_sm_qos_modify,    /* NETMGR_QOS_MODIFY_EV */
  netmgr_sm_qos_suspend,    /* NETMGR_QOS_SUSPEND_EV */
  netmgr_sm_qos_flow_control,    /* NETMGR_QOS_FLOCNTRL_EV */
  netmgr_sm_netd_restart,        /* NETMGR_NETD_RESTART_EV */

  /* Transition functions for state NETMGR_STATE_GOING_DOWN */
  NULL,    /* NETMGR_INITED_EV */
  NULL,    /* NETMGR_MODEM_IS_EV */
  netmgr_sm_modem_oos_msg,    /* NETMGR_MODEM_OOS_EV */
  netmgr_sm_dispatch_kif_msg,    /* NETMGR_KIF_MSG_CMD */
  netmgr_sm_dispatch_qmi_msg,    /* NETMGR_QMI_MSG_CMD */
  netmgr_sm_modem_reset_msg,    /* NETMGR_RESET_MSG_CMD */
  netmgr_sm_modem_connected_while_going_down,    /* NETMGR_WDS_CONNECTED_EV */
  netmgr_sm_qos_delete,    /* NETMGR_QOS_DELETE_EV */
  NULL,    /* NETMGR_WDS_DISCONNECTED_EV */
  netmgr_sm_kif_opened_while_going_down,    /* NETMGR_KIF_OPENED_EV */
  netmgr_sm_kif_closed,    /* NETMGR_KIF_CLOSED_EV */
  NULL,    /* NETMGR_KIF_CONFIGURED_EV */
  NULL,    /* NETMGR_KIF_RECONFIGURED_EV */
  NULL,    /* NETMGR_QOS_ACTIVATE_EV */
  NULL,    /* NETMGR_QOS_MODIFY_EV */
  netmgr_sm_qos_suspend,    /* NETMGR_QOS_SUSPEND_EV */
  NULL,    /* NETMGR_QOS_FLOCNTRL_EV */
  netmgr_sm_netd_restart,    /* NETMGR_NETD_RESTART_EV */

  /* Transition functions for state NETMGR_STATE_GOING_DOWN_TO_COME_UP */
  NULL,    /* NETMGR_INITED_EV */
  NULL,    /* NETMGR_MODEM_IS_EV */
  netmgr_sm_modem_oos_msg,    /* NETMGR_MODEM_OOS_EV */
  netmgr_sm_dispatch_kif_msg,    /* NETMGR_KIF_MSG_CMD */
  netmgr_sm_dispatch_qmi_msg,    /* NETMGR_QMI_MSG_CMD */
  netmgr_sm_modem_reset_msg,    /* NETMGR_RESET_MSG_CMD */
  netmgr_sm_repost_modem_connected,    /* NETMGR_WDS_CONNECTED_EV */
  netmgr_sm_qos_delete,    /* NETMGR_QOS_DELETE_EV */
  netmgr_sm_modem_disconnected,    /* NETMGR_WDS_DISCONNECTED_EV */
  netmgr_sm_kif_opened_while_going_down,    /* NETMGR_KIF_OPENED_EV */
  netmgr_sm_kif_down_to_come_up,    /* NETMGR_KIF_CLOSED_EV */
  NULL,    /* NETMGR_KIF_CONFIGURED_EV */
  NULL,    /* NETMGR_KIF_RECONFIGURED_EV */
  NULL,    /* NETMGR_QOS_ACTIVATE_EV */
  NULL,    /* NETMGR_QOS_MODIFY_EV */
  netmgr_sm_qos_suspend,    /* NETMGR_QOS_SUSPEND_EV */
  NULL,    /* NETMGR_QOS_FLOCNTRL_EV */
  netmgr_sm_netd_restart,    /* NETMGR_NETD_RESTART_EV */

};
#endif /* !STM_DATA_STRUCTURES_ONLY */

/* State { name, entry, exit, child SM } table */
static const stm_state_map_t
  NETMGR_SM_states[ NETMGR_SM_NUM_STATES ] =
{
  {"NETMGR_STATE_DOWN",
#ifndef STM_DATA_STRUCTURES_ONLY
    netmgr_sm_state_down_entry, netmgr_sm_state_down_exit,
#else /* STM_DATA_STRUCTURES_ONLY */
    NULL, NULL,
#endif /* STM_DATA_STRUCTURES_ONLY */
    NULL},
  {"NETMGR_STATE_INITED",
#ifndef STM_DATA_STRUCTURES_ONLY
    netmgr_sm_state_inited_entry, netmgr_sm_state_inited_exit,
#else /* STM_DATA_STRUCTURES_ONLY */
    NULL, NULL,
#endif /* STM_DATA_STRUCTURES_ONLY */
    NULL},
  {"NETMGR_STATE_COMING_UP",
#ifndef STM_DATA_STRUCTURES_ONLY
    netmgr_sm_state_comingup_entry, netmgr_sm_state_comingup_exit,
#else /* STM_DATA_STRUCTURES_ONLY */
    NULL, NULL,
#endif /* STM_DATA_STRUCTURES_ONLY */
    NULL},
  {"NETMGR_STATE_CONFIGURING",
#ifndef STM_DATA_STRUCTURES_ONLY
    netmgr_sm_state_configuring_entry, netmgr_sm_state_configuring_exit,
#else /* STM_DATA_STRUCTURES_ONLY */
    NULL, NULL,
#endif /* STM_DATA_STRUCTURES_ONLY */
    NULL},
  {"NETMGR_STATE_RECONFIGURING",
#ifndef STM_DATA_STRUCTURES_ONLY
    netmgr_sm_state_reconfiguring_entry, netmgr_sm_state_reconfiguring_exit,
#else /* STM_DATA_STRUCTURES_ONLY */
    NULL, NULL,
#endif /* STM_DATA_STRUCTURES_ONLY */
    NULL},
  {"NETMGR_STATE_UP",
#ifndef STM_DATA_STRUCTURES_ONLY
    netmgr_sm_state_up_entry, netmgr_sm_state_up_exit,
#else /* STM_DATA_STRUCTURES_ONLY */
    NULL, NULL,
#endif /* STM_DATA_STRUCTURES_ONLY */
    NULL},
  {"NETMGR_STATE_GOING_DOWN",
#ifndef STM_DATA_STRUCTURES_ONLY
    netmgr_sm_state_goingdown_entry, netmgr_sm_state_goingdown_exit,
#else /* STM_DATA_STRUCTURES_ONLY */
    NULL, NULL,
#endif /* STM_DATA_STRUCTURES_ONLY */
    NULL},
  {"NETMGR_STATE_GOING_DOWN_TO_COME_UP",
#ifndef STM_DATA_STRUCTURES_ONLY
    netmgr_sm_state_goingdowntocomeup_entry, netmgr_sm_state_goingdowntocomeup_exit,
#else /* STM_DATA_STRUCTURES_ONLY */
    NULL, NULL,
#endif /* STM_DATA_STRUCTURES_ONLY */
    NULL},
};

/* Input { name, value } table */
static const stm_input_map_t
  NETMGR_SM_inputs[ NETMGR_SM_NUM_INPUTS ] =
{
  { "NETMGR_INITED_EV" , (stm_input_t) NETMGR_INITED_EV },
  { "NETMGR_MODEM_IS_EV" , (stm_input_t) NETMGR_MODEM_IS_EV },
  { "NETMGR_MODEM_OOS_EV" , (stm_input_t) NETMGR_MODEM_OOS_EV },
  { "NETMGR_KIF_MSG_CMD" , (stm_input_t) NETMGR_KIF_MSG_CMD },
  { "NETMGR_QMI_MSG_CMD" , (stm_input_t) NETMGR_QMI_MSG_CMD },
  { "NETMGR_RESET_MSG_CMD" , (stm_input_t) NETMGR_RESET_MSG_CMD },
  { "NETMGR_WDS_CONNECTED_EV" , (stm_input_t) NETMGR_WDS_CONNECTED_EV },
  { "NETMGR_QOS_DELETE_EV" , (stm_input_t) NETMGR_QOS_DELETE_EV },
  { "NETMGR_WDS_DISCONNECTED_EV" , (stm_input_t) NETMGR_WDS_DISCONNECTED_EV },
  { "NETMGR_KIF_OPENED_EV" , (stm_input_t) NETMGR_KIF_OPENED_EV },
  { "NETMGR_KIF_CLOSED_EV" , (stm_input_t) NETMGR_KIF_CLOSED_EV },
  { "NETMGR_KIF_CONFIGURED_EV" , (stm_input_t) NETMGR_KIF_CONFIGURED_EV },
  { "NETMGR_KIF_RECONFIGURED_EV" , (stm_input_t) NETMGR_KIF_RECONFIGURED_EV },
  { "NETMGR_QOS_ACTIVATE_EV" , (stm_input_t) NETMGR_QOS_ACTIVATE_EV },
  { "NETMGR_QOS_MODIFY_EV" , (stm_input_t) NETMGR_QOS_MODIFY_EV },
  { "NETMGR_QOS_SUSPEND_EV" , (stm_input_t) NETMGR_QOS_SUSPEND_EV },
  { "NETMGR_QOS_FLOCNTRL_EV" , (stm_input_t) NETMGR_QOS_FLOCNTRL_EV },
  { "NETMGR_NETD_RESTART_EV" , (stm_input_t) NETMGR_NETD_RESTART_EV },
};


/* Constant all-instance state machine data */
static const stm_state_machine_constdata_t NETMGR_SM_constdata =
{
  NETMGR_SM_NUM_INSTANCES, /* number of state machine instances */
  NETMGR_SM_NUM_STATES, /* number of states */
  NETMGR_SM_states, /* array of state mappings */
  NETMGR_SM_NUM_INPUTS, /* number of inputs */
  NETMGR_SM_inputs, /* array of input mappings */
#ifndef STM_DATA_STRUCTURES_ONLY
  NETMGR_SM_transitions, /* array of transition function mappings */
  netmgr_sm_entry, /* state machine entry function */
  netmgr_sm_exit, /* state machine exit function */
  netmgr_sm_error_hook, /* state machine error hook function */
  netmgr_sm_debug_hook, /* state machine debug hook function */
  NETMGR_STATE_DOWN /* state machine initial state */
#else /* STM_DATA_STRUCTURES_ONLY */
  NULL, /* array of transition function mappings */
  NULL, /* state machine entry function */
  NULL, /* state machine exit function */
  NULL, /* state machine error hook function */
  NULL, /* state machine debug hook function */
  0 /* state machine initial state */
#endif /* STM_DATA_STRUCTURES_ONLY */
};

/* Constant per-instance state machine data */
static const stm_state_machine_perinst_constdata_t
  NETMGR_SM_perinst_constdata[ NETMGR_SM_NUM_INSTANCES ] =
{
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[0]", /* state machine name */
    0x44271596, /* state machine unique ID (md5("NETMGR_SM[0]") & 0xFFFFFFFF) */
    0  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[1]", /* state machine name */
    0xc8d6ffd9, /* state machine unique ID (md5("NETMGR_SM[1]") & 0xFFFFFFFF) */
    1  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[2]", /* state machine name */
    0x4bc06da2, /* state machine unique ID (md5("NETMGR_SM[2]") & 0xFFFFFFFF) */
    2  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[3]", /* state machine name */
    0xad6fc688, /* state machine unique ID (md5("NETMGR_SM[3]") & 0xFFFFFFFF) */
    3  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[4]", /* state machine name */
    0x392d818b, /* state machine unique ID (md5("NETMGR_SM[4]") & 0xFFFFFFFF) */
    4  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[5]", /* state machine name */
    0x97aeb583, /* state machine unique ID (md5("NETMGR_SM[5]") & 0xFFFFFFFF) */
    5  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[6]", /* state machine name */
    0x396bebe3, /* state machine unique ID (md5("NETMGR_SM[6]") & 0xFFFFFFFF) */
    6  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[7]", /* state machine name */
    0xa3c457aa, /* state machine unique ID (md5("NETMGR_SM[7]") & 0xFFFFFFFF) */
    7  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[8]", /* state machine name */
    0x0dd549f9, /* state machine unique ID (md5("NETMGR_SM[8]") & 0xFFFFFFFF) */
    8  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[9]", /* state machine name */
    0x50a92935, /* state machine unique ID (md5("NETMGR_SM[9]") & 0xFFFFFFFF) */
    9  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[10]", /* state machine name */
    0xa0f3a0ed, /* state machine unique ID (md5("NETMGR_SM[10]") & 0xFFFFFFFF) */
    10  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[11]", /* state machine name */
    0x626b55d0, /* state machine unique ID (md5("NETMGR_SM[11]") & 0xFFFFFFFF) */
    11  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[12]", /* state machine name */
    0x000576f1, /* state machine unique ID (md5("NETMGR_SM[12]") & 0xFFFFFFFF) */
    12  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[13]", /* state machine name */
    0x47ba20be, /* state machine unique ID (md5("NETMGR_SM[13]") & 0xFFFFFFFF) */
    13  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[14]", /* state machine name */
    0x912ae3eb, /* state machine unique ID (md5("NETMGR_SM[14]") & 0xFFFFFFFF) */
    14  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[15]", /* state machine name */
    0xf2cd2bea, /* state machine unique ID (md5("NETMGR_SM[15]") & 0xFFFFFFFF) */
    15  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[16]", /* state machine name */
    0x994fd171, /* state machine unique ID (md5("NETMGR_SM[16]") & 0xFFFFFFFF) */
    16  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[17]", /* state machine name */
    0x548cce2f, /* state machine unique ID (md5("NETMGR_SM[17]") & 0xFFFFFFFF) */
    17  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[18]", /* state machine name */
    0x121f1b57, /* state machine unique ID (md5("NETMGR_SM[18]") & 0xFFFFFFFF) */
    18  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[19]", /* state machine name */
    0x28601000, /* state machine unique ID (md5("NETMGR_SM[19]") & 0xFFFFFFFF) */
    19  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[20]", /* state machine name */
    0x36a4de02, /* state machine unique ID (md5("NETMGR_SM[20]") & 0xFFFFFFFF) */
    20  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[21]", /* state machine name */
    0x8b464de5, /* state machine unique ID (md5("NETMGR_SM[21]") & 0xFFFFFFFF) */
    21  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[22]", /* state machine name */
    0x37f45308, /* state machine unique ID (md5("NETMGR_SM[22]") & 0xFFFFFFFF) */
    22  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[23]", /* state machine name */
    0x2a5bde6a, /* state machine unique ID (md5("NETMGR_SM[23]") & 0xFFFFFFFF) */
    23  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[24]", /* state machine name */
    0xa9b21a14, /* state machine unique ID (md5("NETMGR_SM[24]") & 0xFFFFFFFF) */
    24  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[25]", /* state machine name */
    0x09077fb5, /* state machine unique ID (md5("NETMGR_SM[25]") & 0xFFFFFFFF) */
    25  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[26]", /* state machine name */
    0x8325b4ad, /* state machine unique ID (md5("NETMGR_SM[26]") & 0xFFFFFFFF) */
    26  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[27]", /* state machine name */
    0x9bed4957, /* state machine unique ID (md5("NETMGR_SM[27]") & 0xFFFFFFFF) */
    27  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[28]", /* state machine name */
    0x30290bb0, /* state machine unique ID (md5("NETMGR_SM[28]") & 0xFFFFFFFF) */
    28  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[29]", /* state machine name */
    0xb30e9292, /* state machine unique ID (md5("NETMGR_SM[29]") & 0xFFFFFFFF) */
    29  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[30]", /* state machine name */
    0xbc61b434, /* state machine unique ID (md5("NETMGR_SM[30]") & 0xFFFFFFFF) */
    30  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[31]", /* state machine name */
    0x8e6be16e, /* state machine unique ID (md5("NETMGR_SM[31]") & 0xFFFFFFFF) */
    31  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[32]", /* state machine name */
    0xa5098781, /* state machine unique ID (md5("NETMGR_SM[32]") & 0xFFFFFFFF) */
    32  /* this state machine instance */
  },
  {
    &NETMGR_SM_constdata, /* state machine constant data */
    "NETMGR_SM[33]", /* state machine name */
    0x1c2babe0, /* state machine unique ID (md5("NETMGR_SM[33]") & 0xFFFFFFFF) */
    33  /* this state machine instance */
  },
};

/* State machine instance array definition */
stm_state_machine_t
  NETMGR_SM[ NETMGR_SM_NUM_INSTANCES ] =
{
  {
    &NETMGR_SM_perinst_constdata[ 0 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 1 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 2 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 3 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 4 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 5 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 6 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 7 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 8 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 9 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 10 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 11 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 12 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 13 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 14 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 15 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 16 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 17 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 18 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 19 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 20 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 21 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 22 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 23 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 24 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 25 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 26 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 27 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 28 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 29 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 30 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 31 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 32 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
  {
    &NETMGR_SM_perinst_constdata[ 33 ], /* per instance constant data array */
    STM_DEACTIVATED_STATE, /* current state */
    -1, /* current input index */
    NULL, /* calling SM instance */
    FALSE, /* propagate input to parent */
    FALSE, /* locked flag */
    NULL, /* user defined per-instance data */
    0, /* user defined debug mask */
  },
};

#ifndef STM_DATA_STRUCTURES_ONLY
/* User called 'reset' routine.  Should never be needed, but can be used to
   effect a complete reset of all a given state machine's instances. */
void NETMGR_SM_reset(void)
{
  uint32 idx;
  void **tricky;

  /* Reset all the child SMs (if any) */


  /* Reset the parent */
  for( idx = 0; idx < NETMGR_SM_NUM_INSTANCES; idx++)
  {
    tricky = (void **)&NETMGR_SM[ idx ].pi_const_data; /* sleight of hand to assign to const ptr below */
    *tricky = (void *)&NETMGR_SM_perinst_constdata[ idx ]; /* per instance constant data array */
    NETMGR_SM[ idx ].current_state = STM_DEACTIVATED_STATE; /* current state */
    NETMGR_SM[ idx ].curr_input_index = -1; /* current input index */
    NETMGR_SM[ idx ].propagate_input = FALSE; /* propagate input to parent */
    NETMGR_SM[ idx ].is_locked = FALSE; /* locked flag */
    NETMGR_SM[ idx ].user_data = NULL; /* user defined per-instance data */
    NETMGR_SM[ idx ].debug_mask = 0; /* user defined debug mask */
  }

}
#endif /* !STM_DATA_STRUCTURES_ONLY */

/* End machine generated internal source for state machine array: NETMGR_SM[] */


