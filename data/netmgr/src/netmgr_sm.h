/******************************************************************************

                          N E T M G R _ S M . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_sm.h
  @brief   Network Manager state machine header file

  DESCRIPTION
  Header file containing definition of NetMgr's state machine.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010,2015 Qualcomm Technologies, Inc. All Rights Reserved

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
02/08/10   ar         Initial version

******************************************************************************/

#ifndef __NETMGR_SM_H__
#define __NETMGR_SM_H__

#include "netmgr_defs.h"
#include "netmgr_sm_int.h"

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

typedef enum netmgr_sm_events_e {
  NETMGR_KIF_MSG_CMD,            /* Kernel socket interface message      */
  NETMGR_QMI_MSG_CMD,            /* QMI library message                  */
  NETMGR_RESET_MSG_CMD,          /* netmgr reset message                 */
  
  NETMGR_INITED_EV,              /* netmgr initialized event             */
  NETMGR_MODEM_IS_EV,            /* Modem in service event               */
  NETMGR_MODEM_OOS_EV,           /* Modem out of service event           */
  NETMGR_WDS_CONNECTED_EV,       /* Modem interface connected            */
  NETMGR_WDS_DISCONNECTED_EV,    /* Modem interface disconnected         */
  NETMGR_KIF_OPENED_EV,          /* Kernel network interface up          */
  NETMGR_KIF_CLOSED_EV,          /* Kernel network interface down        */
  NETMGR_KIF_CONFIGURED_EV,      /* Kernel network interface configured  */
  NETMGR_KIF_RECONFIGURED_EV,    /* Kernel network interface reconfigured*/
  NETMGR_QOS_ACTIVATE_EV,        /* Modem QoS flow activated             */
  NETMGR_QOS_MODIFY_EV,          /* Modem QoS flow modified              */
  NETMGR_QOS_DELETE_EV,          /* Modem QoS flow deleted               */
  NETMGR_QOS_SUSPEND_EV,         /* modem QoS flow suspended             */
  NETMGR_QOS_FLOCNTRL_EV,        /* modem QoS flow control change        */
  NETMGR_NETD_RESTART_EV,        /* netd restart event                   */
  NETMGR_INVALID_EV              /* Internal value                       */
} netmgr_sm_events_t;


/*===========================================================================
                            GLOBAL FUNCTION DECLARATIONS
===========================================================================*/

/*===========================================================================

  STATE MACHINE ERROR HOOK FUNCTION: netmgr_sm_error_hook

===========================================================================*/
void netmgr_sm_error_hook
(
  stm_status_t     error,
  const char      *filename,
  uint32           line,
  struct stm_state_machine_s *sm
);


/*===========================================================================

  STATE MACHINE DEBUG HOOK FUNCTION:  netmgr_sm_debug_hook

===========================================================================*/
void netmgr_sm_debug_hook
(
  stm_debug_event_t debug_event,
  struct stm_state_machine_s *sm,
  stm_state_t state_info,
  void *payload
);



#endif /* __NETMGR_SM_H__ */
