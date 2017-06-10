/******************************************************************************

                        N E T M G R _ S M . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_sm.c
  @brief   Network Manager state machine implementation

  DESCRIPTION
  Implementation of NetMgr's state machine.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010-2015 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary and Confidential.

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

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h> /* open, read */

#include "netmgr_util.h"
#include "netmgr_defs.h"
#include "netmgr_main.h"
#include "netmgr_exec.h"
#include "netmgr_platform.h"

/*===========================================================================
                              FORWARD DECLARATIONS
===========================================================================*/


/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

/*===========================================================================

         STM COMPILER GENERATED PROTOTYPES AND DATA STRUCTURES

===========================================================================*/

/* Include STM compiler generated internal data structure file */
#include "netmgr_sm_int.h"

#define NETMGR_EXEC_MAX_CMD_REPOST_LIMIT  (10)
#define NETMGR_EXEC_REPOST_STEP_MICROSECS (5000)
#define NETMGR_EXEC_MICROSECS_PER_SEC     (1000000)

/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/


/*! @brief Structure for state-machine per-instance local variables
*/
typedef struct
{
  int   internal_var;  /*!< My internal variable */
  void *internal_ptr;  /*!< My internal pointer */
} netmgr_sm_stub_instance_type;

/*! @brief All variables internal to module netmgr_sm_stub.c
*/
typedef struct
{
  /*! My array of per-instance internal variables. */
  netmgr_sm_stub_instance_type instance[ 3 ];
} netmgr_sm_stub_type;


/*===========================================================================

                 STATE MACHINE: NETMGR_SM

===========================================================================*/

/*===========================================================================

  STATE MACHINE ENTRY FUNCTION:  netmgr_sm_entry

===========================================================================*/
/*!
    @brief
    Entry function for state machine NETMGR_SM

    @detail
    Called upon activation of this state machine, with optional
    user-passed payload pointer parameter.

    @return
    None

*/
/*=========================================================================*/
void netmgr_sm_entry
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  STM_UNUSED( payload );
  STM_NULL_CHECK( sm );

} /* netmgr_sm_entry() */


/*===========================================================================

  STATE MACHINE EXIT FUNCTION:  netmgr_sm_exit

===========================================================================*/
/*!
    @brief
    Exit function for state machine NETMGR_SM

    @detail
    Called upon deactivation of this state machine, with optional
    user-passed payload pointer parameter.

    @return
    None

*/
/*=========================================================================*/
void netmgr_sm_exit
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  STM_UNUSED( payload );
  STM_NULL_CHECK( sm );
} /* netmgr_sm_exit() */


/*===========================================================================

  STATE MACHINE ERROR HOOK FUNCTION: netmgr_sm_error_hook

===========================================================================*/
void netmgr_sm_error_hook
(
  stm_status_t error,
  const char *filename,
  uint32 line,
  struct stm_state_machine_s *sm
)
{
  STM_UNUSED( error );
  STM_UNUSED( filename );
  STM_UNUSED( line );
  STM_UNUSED( sm );

  /* STUB */
  return;
}


/*===========================================================================

  STATE MACHINE DEBUG HOOK FUNCTION:  netmgr_sm_debug_hook

===========================================================================*/
void netmgr_sm_debug_hook
(
  stm_debug_event_t debug_event,
  struct stm_state_machine_s *sm,
  stm_state_t state_info,
  void *payload
)
{
  STM_UNUSED( payload );

  if( netmgr_main_cfg.debug ) {
    /* NOTE: the ENTRY/EXIT states seem reversed here but that is due
     * to STM2 providing the prev/next state value respectively. The
     * ordering here seems to make more sense. */
    switch( debug_event ) {
      case STM_STATE_EXIT_FN:
        netmgr_log_med("netmgr_sm_debug_hook: enter state %s\n",
                       stm_get_state_name(sm, state_info));
        break;
      case STM_STATE_ENTRY_FN:
        netmgr_log_med("netmgr_sm_debug_hook: exit state %s\n",
                       stm_get_state_name(sm, state_info));
        break;
      default:
        break;
    }
  }
  return;
}



/*===========================================================================

     (State Machine: NETMGR_SM)
     STATE ENTRY/EXIT/TRANSITION FUNCTIONS: NETMGR_STATE_DOWN

===========================================================================*/

/*===========================================================================

  STATE ENTRY FUNCTION:  netmgr_sm_state_down_entry

===========================================================================*/
/*!
    @brief
    Entry function for state machine NETMGR_SM,
    state NETMGR_STATE_DOWN

    @detail
    Called upon entry into this state of the state machine, with optional
    user-passed payload pointer parameter.  The prior state of the state
    machine is also passed as the prev_state parameter.

    @return
    None

*/
/*=========================================================================*/
void netmgr_sm_state_down_entry
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  stm_state_t         prev_state,  /*!< Previous state */
  void                *payload     /*!< Payload pointer */
)
{
  NETMGR_LOG_FUNC_ENTRY;

  STM_UNUSED( sm );
  STM_UNUSED( prev_state );
  STM_UNUSED( payload );

  NETMGR_LOG_FUNC_EXIT;
} /* netmgr_sm_state_down_entry() */


/*===========================================================================

  STATE EXIT FUNCTION:  netmgr_sm_state_down_exit

===========================================================================*/
/*!
    @brief
    Exit function for state machine NETMGR_SM,
    state NETMGR_STATE_DOWN

    @detail
    Called upon exit of this state of the state machine, with optional
    user-passed payload pointer parameter.  The impending state of the state
    machine is also passed as the next_state parameter.

    @return
    None

*/
/*=========================================================================*/
void netmgr_sm_state_down_exit
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  stm_state_t         next_state,  /*!< Next state */
  void                *payload     /*!< Payload pointer */
)
{
  STM_UNUSED( payload );
  STM_UNUSED( next_state );
  STM_UNUSED( sm );

  /* Verify program initialized */
  if( !netmgr_main_cfg.initialized ) {
    netmgr_log_err("netmgr initialization incomplete!\n");
    NETMGR_ABORT("netmgr initialization incomplete!");
  }

} /* netmgr_sm_state_down_exit() */

/*===========================================================================

  STATE EXIT FUNCTION:  netmgr_sm_state_inited_entry

===========================================================================*/
/*!
    @brief
    Entry function for state machine NETMGR_SM,
    state NETMGR_STATE_INITED

    @detail
    Called upon entry of this state of the state machine, with optional
    user-passed payload pointer parameter. The prior state of the state
    machine is also passed as the prev_state parameter.

    @return
    None

*/
/*=========================================================================*/
void netmgr_sm_state_inited_entry
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  stm_state_t          prev_state, /*!< Prev State */
  void                *payload     /*!< Payload pointer */
)
{
  netmgr_exec_cmd_t * cmd_buf = NULL;
  netmgr_nl_event_info_t * event_info = NULL;
  const char *link_name;

  STM_UNUSED( sm );

  /* Verify program initialized */
  if( !netmgr_main_cfg.initialized ) {
    netmgr_log_err("netmgr initialization incomplete!\n");
    NETMGR_ABORT("netmgr initialization incomplete!");
  }

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  /* Check for initialization state */
  if( NETMGR_STATE_DOWN != prev_state ||
      /* if netmgrd was restarted, below check is TRUE */
      ( NETMGR_KIF_LINK_POWERUP_STATE_UP ==
        netmgr_kif_get_link_powerup_state(cmd_buf->data.link))
    )
  {
    /* Send the config status for a reverse Rmnet link */
#ifdef FEATURE_DATA_IWLAN
    if (NETMGR_KIF_IS_REV_RMNET_LINK(cmd_buf->data.link))
    {
      int ip_family, config_status;

      if (NETMGR_SUCCESS != netmgr_qmi_retrieve_rev_ip_config_status(cmd_buf->data.link, &ip_family, &config_status))
      {
        netmgr_log_med("netmgr_sm_state_inited_entry: failed to get rev IP config status for link=%d, ignoring\n",
                       cmd_buf->data.link);
      }
      else
      {
        netmgr_qmi_send_rev_ip_config_complete(NETMGR_QMI_IWLAN_CALL_CLEANUP,
                                               cmd_buf->data.link,
                                               ip_family,
                                               config_status);
      }
    }
#endif /* FEATURE_DATA_IWLAN */

    /* Call the function to remove the link-local interface from the
     * custom network. Usually this would be done inside the OOS handler
     * registered with netmgr_kif. However we might get a NETMGR_KIF_CLOSED_EV
     * on the link before OOS_EV is received depending on when global
     * rmnet cleanup happens. If CLOSED_EV is received then the link will
     * move to inited state and we need to do the cleanup here */
    netmgr_kif_remove_link_network(cmd_buf->data.link);

    /* Reset the QMI link WDS data */
    netmgr_qmi_reset_link_wds_data(cmd_buf->data.link);

    /* reset link_powerup state to DOWN */
    netmgr_kif_set_link_powerup_state(cmd_buf->data.link,
                                      NETMGR_KIF_LINK_POWERUP_STATE_DOWN);

    link_name = netmgr_kif_get_name(cmd_buf->data.link);
    if (NULL == link_name)
    {
      netmgr_log_err("%s(): unable to determine name for link=%d\n",
                      __func__, cmd_buf->data.link);
    }
    else
    {
      /* Post event indication to clients */
      event_info = netmgr_malloc( sizeof(netmgr_nl_event_info_t) );
      if( NULL == event_info ) {
        netmgr_log_err("failed to allocate event buffer!\n");
      } else {
        memset( event_info, 0x0, sizeof(netmgr_nl_event_info_t) );
        event_info->event = NET_PLATFORM_DOWN_EV;
        event_info->link = cmd_buf->data.link;
        event_info->param_mask |= NETMGR_EVT_PARAM_LINK;
        strlcpy( event_info->dev_name,
                 link_name,
                 sizeof(event_info->dev_name) );
        event_info->param_mask |= NETMGR_EVT_PARAM_DEVNAME;

        if( NETMGR_SUCCESS != netmgr_kif_send_event_msg( event_info ) ) {
          netmgr_log_err("failed on kif_send_event DOWN\n");
        }

        netmgr_free( event_info );
      }
    }
  }

} /* netmgr_sm_state_inited_entry() */

/*===========================================================================

  STATE EXIT FUNCTION:  netmgr_sm_state_inited_exit

===========================================================================*/
/*!
    @brief
    Exit function for state machine NETMGR_SM,
    state NETMGR_STATE_INITED

    @detail
    Called upon exit of this state of the state machine, with optional
    user-passed payload pointer parameter. The prior state of the state
    machine is also passed as the prev_state parameter.

    @return
    None

*/
/*=========================================================================*/
void netmgr_sm_state_inited_exit
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  stm_state_t         next_state,  /*!< Next state */
  void                *payload     /*!< Payload pointer */
)
{
  STM_UNUSED( payload );
  STM_UNUSED( next_state );
  STM_UNUSED( sm );

} /* netmgr_sm_state_inited_exit() */

/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_modem_connected

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_DOWN,
    upon receiving input NETMGR_WDS_CONNECTED_EV

    @detail
    Called upon receipt of input NETMGR_WDS_CONNECTED_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_modem_connected
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
  netmgr_exec_cmd_t * cmd_buf;

  NETMGR_LOG_FUNC_ENTRY;

  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  netmgr_log_med("Recvd NETMGR_WDS_CONNECTED_EV on link=%d\n",
                 cmd_buf->data.link);

  if (0 != cmd_buf->repost_count) {
    netmgr_log_err("ignoring stale modem connect indication on link=%d\n",
                   cmd_buf->data.link);
    goto bail;
  }

  /* Initiate KIF interface open process */
  if( NETMGR_SUCCESS !=
      netmgr_kif_iface_open( (uint8)cmd_buf->data.link,
                             cmd_buf->data.info.connect_msg.addr_info_ptr,
                             NULL, NULL ) ) {
    netmgr_log_err("failed on kif_iface_open\n");

    /* Force platform interface to close */
    (void)netmgr_kif_iface_close( (uint8)cmd_buf->data.link,
                                  cmd_buf->data.info.connect_msg.addr_info_ptr,
                                  TRUE );
    next_state = NETMGR_STATE_INITED;
  }
  else {
    /* Intial Modem interface connect, transition to COMING_UP state */
    next_state = NETMGR_STATE_COMING_UP;
  }

bail:
  /* Release heap memory allocated in QMI module */
  if (NULL != cmd_buf
      && NULL != cmd_buf->data.info.connect_msg.addr_info_ptr)
  {
    ds_free( cmd_buf->data.info.connect_msg.addr_info_ptr );
  }

  NETMGR_LOG_FUNC_EXIT;
  return( next_state );

} /* netmgr_sm_modem_connected() */


/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_modem_disconnected_in_inited

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_INITED,
    upon receiving input NETMGR_WDS_DISCONNECTED_EV

    @detail
    Called upon receipt of input NETMGR_WDS_DISCONNECTED_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_modem_disconnected_in_inited
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
  netmgr_exec_cmd_t * cmd_buf;

  NETMGR_LOG_FUNC_ENTRY;

  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

#ifdef FEATURE_DATA_IWLAN
  if (NETMGR_KIF_IS_REV_RMNET_LINK(cmd_buf->data.link) )
  {
    /* We don't expect NETMGR_WDS_DISCONNECTED_EV for reverse links in NETMGR_STATE_INITED
     * Hoever if we do receive it we need to send a config complete event with CLEANUP so that
     * all previous transactions are cleaned up and the state machine moves back to a working
     * condition */

     /* Send CLEANUP command for V4 */
     netmgr_qmi_send_rev_ip_config_complete(NETMGR_QMI_IWLAN_CALL_CLEANUP,
                                            cmd_buf->data.link,
                                            AF_INET,
                                            QMI_WDS_REV_IP_TRANSPORT_CONFIG_SUCCESS);

     /* Send CLEANUP command for V6 */
     netmgr_qmi_send_rev_ip_config_complete(NETMGR_QMI_IWLAN_CALL_CLEANUP,
                                            cmd_buf->data.link,
                                            AF_INET6,
                                            QMI_WDS_REV_IP_TRANSPORT_CONFIG_SUCCESS);
  }
#endif /* FEATURE_DATA_IWLAN */

  /* Getting NETMGR_WDS_DISCONNECTED event in INITED state should keep the state
   * machine in inited state */
  next_state = NETMGR_STATE_INITED;

bail:
  /* Release heap memory allocated in QMI module */
  if (NULL != cmd_buf
      && NULL != cmd_buf->data.info.connect_msg.addr_info_ptr)
  {
    ds_free( cmd_buf->data.info.connect_msg.addr_info_ptr );
  }

  NETMGR_LOG_FUNC_EXIT;
  return( next_state );

} /* netmgr_sm_modem_connected() */

/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_modem_connected_while_going_down

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_GOING_DOWN,
    upon receiving input NETMGR_WDS_CONNECTED_EV

    @detail
    Called upon receipt of input NETMGR_WDS_CONNECTED_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_modem_connected_while_going_down
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
  netmgr_exec_cmd_t * cmd_buf;

  NETMGR_LOG_FUNC_ENTRY;

  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

#ifdef FEATURE_DATA_IWLAN
  if (NETMGR_KIF_IS_REV_RMNET_LINK(cmd_buf->data.link) )
  {
    /* The reverse Rmnet link would be kept in the UP state as long as there's an
       associated forward Rmnet (i.e with same address configuration). In such a
       case we can configure the address and directly transition the reverse Rmnet
       SM to UP state */
    if (NETMGR_LINK_MAX != netmgr_qmi_iwlan_get_link_assoc(cmd_buf->data.link))
    {
      /* Initiate KIF interface configuration process */
      if( NETMGR_SUCCESS !=
          netmgr_kif_iface_configure( (uint8)cmd_buf->data.link,
                                      netmgr_qmi_get_ip_addr_type_first_conn_clnt( cmd_buf->data.link ) ) )
      {
        netmgr_log_err("failed on kif_iface_configure\n");

        /* Force platform interface to close */
        (void)netmgr_kif_iface_close( (uint8)cmd_buf->data.link, NULL, TRUE );
        next_state = NETMGR_STATE_INITED;
      }
      else
      {
        next_state = NETMGR_STATE_UP;
      }
    }
    else
    {
      if(cmd_buf->repost_count == 0 )
      {
        /*  Modem interface connect while going down,
            transition to GOING_DOWN_TO_COME_UP state
            This is a valid connected ev coming from modem */
        next_state = NETMGR_STATE_GOING_DOWN_TO_COME_UP;
      }
      /* else {
         This event is being reposted locally from netmgr
         Ignore this connected ev when in going down state
      } */
    }
  }
  else
#endif /* FEATURE_DATA_IWLAN */
  {
    if( 0 == cmd_buf->repost_count)
    {
      /*  Modem interface connect while going down,
          transition to GOING_DOWN_TO_COME_UP state.
          This is a valid connected ev coming from modem */
      next_state = NETMGR_STATE_GOING_DOWN_TO_COME_UP;
    }
    /* else {
       This event is being reposted locally from netmgr
       Ignore this connected ev when in going down state
    } */
  }

  /* Release heap memory allocated in QMI module */
  if (NULL != cmd_buf
      && NULL != cmd_buf->data.info.connect_msg.addr_info_ptr)
  {
    ds_free( cmd_buf->data.info.connect_msg.addr_info_ptr );
  }

  NETMGR_LOG_FUNC_EXIT;
  return( next_state );

} /* netmgr_sm_modem_connected_while_going_down() */

/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_dispatch_kif_msg

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_DOWN,
    upon receiving input NETMGR_KIF_MSG_CMD

    @detail
    Called upon receipt of input NETMGR_KIF_MSG_CMD, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_dispatch_kif_msg
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
  netmgr_exec_cmd_t * cmd_buf;

  NETMGR_LOG_FUNC_ENTRY;

  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  /* Post message to the KIF module for local processing */
  if( NETMGR_SUCCESS !=
      netmgr_kif_dispatch( &cmd_buf->cmd,
                           &cmd_buf->data.info.kif_msg ) ) {
    netmgr_log_err("kif dispatch failed\n");
  }

  NETMGR_LOG_FUNC_EXIT;
  return( next_state );

} /* netmgr_sm_dispatch_kif_msg() */


/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_dispatch_qmi_msg

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_DOWN,
    upon receiving input NETMGR_QMI_MSG_CMD

    @detail
    Called upon receipt of input NETMGR_QMI_MSG_CMD, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_dispatch_qmi_msg
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
  netmgr_exec_cmd_t * cmd_buf;

  NETMGR_LOG_FUNC_ENTRY;

  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  /* Post message to the QMI module for local processing */
  if( NETMGR_SUCCESS !=
      netmgr_qmi_dispatch( &cmd_buf->cmd,
                           (void*)&cmd_buf->data.info.qmi_msg ) ) {
    netmgr_log_err("failed on qmi_dispatch\n");
  }

  NETMGR_LOG_FUNC_EXIT;
  return( next_state );

} /* netmgr_sm_dispatch_qmi_msg() */


/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_inited_msg

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_INITED,
    upon receiving input NETMGR_INITED_EV

    @detail
    Called upon receipt of input NETMGR_INITED_EV, with optional
    user-passed payload pointer. This function checks if the given
    link is initialized, and if so, it moves the SM forward to
    NETMGR_STATE_INITED

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_inited_msg
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
  netmgr_exec_cmd_t * cmd_buf = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
  STM_UNUSED( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  /* reset qmi module */
  if( NETMGR_SUCCESS != netmgr_qmi_verify(cmd_buf->data.link)) {
    netmgr_log_err("qmi verification failed on link %d\n",
                   cmd_buf->data.link);
  } else {
    netmgr_log_high("qmi verification succeeded on link %d\n",
                    cmd_buf->data.link);
    next_state = NETMGR_STATE_INITED;
  }

  NETMGR_LOG_FUNC_EXIT;
  return( next_state );

} /* netmgr_sm_inited_msg() */

/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_modem_oos_msg

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_DOWN,
    upon receiving input NETMGR_MODEM_OOS_EV

    @detail
    Called upon receipt of input NETMGR_MODEM_OOS_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_modem_oos_msg
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
  netmgr_exec_cmd_t * cmd_buf = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  /* perform kif clean-up */
  if (NETMGR_SUCCESS != netmgr_kif_out_of_service(cmd_buf->data.link))
  {
    netmgr_log_err("failed to execute kif out_of_service\n");
  }

  /* release qmi clients */
  if (NETMGR_SUCCESS != netmgr_qmi_out_of_service(cmd_buf->data.link))
  {
    netmgr_log_err("failed to execute qmi out_of_service\n");
  }

  next_state = NETMGR_STATE_DOWN;

  NETMGR_LOG_FUNC_EXIT;
  return( next_state );

} /* netmgr_sm_modem_oos_msg() */

/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_modem_is_msg

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_DOWN,
    upon receiving input NETMGR_MODEM_IS_EV

    @detail
    Called upon receipt of input NETMGR_MODEM_IS_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_modem_is_msg
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
  netmgr_exec_cmd_t * cmd_buf = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  /* reset qmi module */
  if( NETMGR_SUCCESS != netmgr_qmi_reset(cmd_buf->data.link, cmd_buf->data.type) ) {
    netmgr_log_err("failed on qmi_reset\n");
  }

  /* Note: we intentionally do not reset kif when modem is out
   * of service, because doing so seems to cause err_fatal on the
   * modem side. Moving this operation when modem is resumed, however
   * seems to be working */
  /* reset kif ifaces */
  if( NETMGR_SUCCESS != netmgr_kif_reset(cmd_buf->data.link, cmd_buf->data.type) ) {
    netmgr_log_err("failed on kif_reset\n");
  }

  next_state = NETMGR_STATE_INITED;

  NETMGR_LOG_FUNC_EXIT;
  return( next_state );

} /* netmgr_sm_modem_is_msg() */

/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_modem_reset_msg

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_DOWN,
    upon receiving input NETMGR_RESET_MSG_CMD

    @detail
    Called upon receipt of input NETMGR_RESET_MSG_CMD, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_modem_reset_msg
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
  netmgr_exec_cmd_t * cmd_buf = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  /* Reset each modoule */
  if( NETMGR_SUCCESS != netmgr_qmi_reset(cmd_buf->data.link, cmd_buf->data.type) ) {
    netmgr_log_err("failed on qmi_reset\n");
  }
  if( NETMGR_SUCCESS != netmgr_kif_reset(cmd_buf->data.link, cmd_buf->data.type) ) {
    netmgr_log_err("failed on kif_reset\n");
  }
#ifdef NETMGR_QOS_ENABLED
  if( netmgr_main_get_qos_enabled() &&
      !NETMGR_KIF_IS_REV_RMNET_LINK(cmd_buf->data.link) )
  {
    if( NETMGR_SUCCESS != netmgr_tc_reset(cmd_buf->data.link) ) {
      netmgr_log_err("failed on tc_reset\n");
    }
  }
#endif

  next_state = NETMGR_STATE_INITED;

  NETMGR_LOG_FUNC_EXIT;
  return( next_state );

} /* netmgr_sm_modem_reset_msg() */


/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_netd_restart

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_UP,
    upon receiving input NETMGR_NETD_RESTART_EV

    @detail
    Called upon receipt of input NETMGR_NETD_RESTART_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_netd_restart
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
  netmgr_exec_cmd_t * cmd_buf = NULL;
  static unsigned int prev_netd_restart_count = 0;
  boolean force = FALSE;

  NETMGR_LOG_FUNC_ENTRY;

  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
  STM_NULL_CHECK( payload );

#if defined (NETMGR_OFFTARGET) || defined (FEATURE_HANDLE_NETD_RESTART)
  cmd_buf = (netmgr_exec_cmd_t *)payload;

  /* Each time netd gets restarted, the 'netd_restart_count' will be
   * incremented. We will compare this to the previously saved value
   * of the counter. If the values are different then we know its a
   * new instance of netd restart and we will use this to re-create
   * the custom network */
  if (prev_netd_restart_count
      != cmd_buf->data.info.netd_restart_count)
  {
    /* Save the new counter value to ensure that we creating custom
     * network only once */
    prev_netd_restart_count = cmd_buf->data.info.netd_restart_count;
    force = TRUE;
  }

  netmgr_kif_process_netd_restart(cmd_buf->data.link,
                                  force);
#endif

  /* There is no state transition required after handling netd restart
   * We can remain in the same state */
  next_state = STM_SAME_STATE;

  NETMGR_LOG_FUNC_EXIT;
  return( next_state );
} /*netmgr_sm_netd_restart*/

/*===========================================================================

     (State Machine: NETMGR_SM)
     STATE ENTRY/EXIT/TRANSITION FUNCTIONS: NETMGR_STATE_COMING_UP

===========================================================================*/

/*===========================================================================

  STATE ENTRY FUNCTION:  netmgr_sm_state_comingup_entry

===========================================================================*/
/*!
    @brief
    Entry function for state machine NETMGR_SM,
    state NETMGR_STATE_COMING_UP

    @detail
    Called upon entry into this state of the state machine, with optional
    user-passed payload pointer parameter.  The prior state of the state
    machine is also passed as the prev_state parameter.

    @return
    None

*/
/*=========================================================================*/
void netmgr_sm_state_comingup_entry
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  stm_state_t         prev_state,  /*!< Previous state */
  void                *payload     /*!< Payload pointer */
)
{
  STM_UNUSED( payload );
  STM_UNUSED( prev_state );
  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
} /* netmgr_sm_state_comingup_entry() */


/*===========================================================================

  STATE EXIT FUNCTION:  netmgr_sm_state_comingup_exit

===========================================================================*/
/*!
    @brief
    Exit function for state machine NETMGR_SM,
    state NETMGR_STATE_COMING_UP

    @detail
    Called upon exit of this state of the state machine, with optional
    user-passed payload pointer parameter.  The impending state of the state
    machine is also passed as the next_state parameter.

    @return
    None

*/
/*=========================================================================*/
void netmgr_sm_state_comingup_exit
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  stm_state_t         next_state,  /*!< Next state */
  void                *payload     /*!< Payload pointer */
)
{
  NETMGR_LOG_FUNC_ENTRY;

  STM_UNUSED( sm );
  STM_UNUSED( next_state );
  STM_UNUSED( payload );

  NETMGR_LOG_FUNC_EXIT;
  return;

} /* netmgr_sm_state_comingup_exit() */


/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_modem_disconnected

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_COMING_UP,
    upon receiving input NETMGR_WDS_DISCONNECTED_EV

    @detail
    Called upon receipt of input NETMGR_WDS_DISCONNECTED_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_modem_disconnected
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
  netmgr_exec_cmd_t * cmd_buf = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  STM_NULL_CHECK( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  /* Initiate KIF interface close process */
  if( NETMGR_SUCCESS !=
      netmgr_kif_iface_close( (uint8)cmd_buf->data.link,
                              cmd_buf->data.info.disconnect_msg.addr_info_ptr,
                              cmd_buf->data.info.disconnect_msg.teardown_iface ) ) {
    netmgr_log_err("failed on kif_iface_close\n");

    next_state = NETMGR_STATE_INITED;
  }
  else {
    /* For some cases, leave interface state unchanged on disconnect;
     * otherwise trigger teardown. */
    if( cmd_buf->data.info.disconnect_msg.teardown_iface )
    {
      next_state = NETMGR_STATE_GOING_DOWN;
    }
  }

  NETMGR_LOG_FUNC_EXIT;
  return( next_state );

} /* netmgr_sm_modem_disconnected() */


/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_kif_opened

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_COMING_UP,
    upon receiving input NETMGR_KIF_OPENED_EV

    @detail
    Called upon receipt of input NETMGR_KIF_OPENED_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_kif_opened
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
  netmgr_exec_cmd_t * cmd_buf = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  STM_NULL_CHECK( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  /* skip iface config if tech name is EMBMS */
  if( QMI_WDS_IFACE_NAME_EMBMS == netmgr_qmi_wds_get_tech_name(cmd_buf->data.link) )
  {
    next_state = NETMGR_STATE_UP;
  }
  else
  {
    /* Initiate KIF interface configuration process */
    if( NETMGR_SUCCESS !=
        netmgr_kif_iface_configure( (uint8)cmd_buf->data.link,
                                    netmgr_qmi_get_ip_addr_type_first_conn_clnt( cmd_buf->data.link ) ) ) {
      netmgr_log_err("failed on kif_iface_configure\n");

      /* Force platform interface to close */
      (void)netmgr_kif_iface_close( (uint8)cmd_buf->data.link, NULL, TRUE );
      next_state = NETMGR_STATE_INITED;
    }
    else {
      next_state = NETMGR_STATE_CONFIGURING;
    }
  }

  NETMGR_LOG_FUNC_EXIT;
  return( next_state );

} /* netmgr_sm_kif_opened() */

/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_kif_opened_while_going_down

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_GOING_DOWN_TO_COME_UP,
    upon receiving input NETMGR_KIF_OPENED_EV

    @detail
    Called upon receipt of input NETMGR_KIF_OPENED_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_kif_opened_while_going_down
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
  netmgr_exec_cmd_t * cmd_buf = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  STM_NULL_CHECK( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  /* Force platform interface to close */
  (void)netmgr_kif_iface_close( (uint8)cmd_buf->data.link, NULL, TRUE );

  NETMGR_LOG_FUNC_EXIT;
  return( next_state );

} /* netmgr_sm_kif_opened_while_going_down() */

/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_kif_closed

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_COMING_UP,
    upon receiving input NETMGR_KIF_CLOSED_EV

    @detail
    Called upon receipt of input NETMGR_KIF_CLOSED_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_kif_closed
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
  netmgr_exec_cmd_t * cmd_buf = NULL;
  netmgr_address_info_t *addr_info = NULL;
  int i;
  NETMGR_LOG_FUNC_ENTRY;

  STM_NULL_CHECK( sm );
  STM_UNUSED( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

#ifdef NETMGR_QOS_ENABLED
  if( netmgr_main_get_qos_enabled() &&
      !NETMGR_KIF_IS_REV_RMNET_LINK(cmd_buf->data.link) )
  {
    /* Initiate Traffic Control root flow delete process */
    if (NETMGR_SUCCESS !=
        netmgr_tc_flow_delete(cmd_buf->data.link,
                              NETMGR_QMI_PRIMARY_FLOW_ID))
    {
      netmgr_log_err("failed on tc_flow_delete\n");
    }

    /*Delete the cached qos flows and filters*/
    for (i =0 ; i < QMI_QOS_MAX_FLOW_FILTER; i++)
    {
      if (netmgr_qmi_cfg.links[cmd_buf->data.link].qos_info.qos_flows[i])
      {
        free(netmgr_qmi_cfg.links[cmd_buf->data.link].qos_info.qos_flows[i]);
        netmgr_qmi_cfg.links[cmd_buf->data.link].qos_info.qos_flows[i] = NULL;
      }
    }

  }
#endif /* NETMGR_QOS_ENABLED */

  /* If the reverse rmnet interface moves to CLOSED state before receiving
   * OOS event then we will loose the WDS related state information for that
   * link. If this happens we cannot remove the SA rules in the global kif
   * cleanup function. We need to do it here */
#ifdef FEATURE_DATA_IWLAN
  if (NETMGR_KIF_IS_REV_RMNET_LINK(cmd_buf->data.link)
      && NETMGR_STATE_UP == stm_get_state(&NETMGR_SM[cmd_buf->data.link]))
  {
    netmgr_log_med("%s(): Performing cleanup for link=%d\n",
                   __func__, cmd_buf->data.link);

    /* Get address info ptr */
    addr_info = netmgr_qmi_get_addr_info(cmd_buf->data.link);

    if (NULL != addr_info)
    {
      netmgr_kif_remove_sa_and_routing_rules(cmd_buf->data.link,
                                             AF_INET,
                                             &addr_info->ipv4);
      netmgr_kif_remove_sa_and_routing_rules(cmd_buf->data.link,
                                             AF_INET6,
                                             &addr_info->ipv6);
    }
    else
    {
      netmgr_log_err("%s(): invalid addr_info for link=%d\n",
                     __func__, cmd_buf->data.link);
    }
  }
#endif /* FEATURE_DATA_IWLAN */

  next_state = NETMGR_STATE_INITED;

  NETMGR_LOG_FUNC_EXIT;
  return( next_state );
} /* netmgr_sm_kif_closed() */


/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_kif_down_to_come_up

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_COMING_UP,
    upon receiving input NETMGR_KIF_CLOSED_EV

    @detail
    Called upon receipt of input NETMGR_KIF_CLOSED_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_kif_down_to_come_up
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
  netmgr_exec_cmd_t * cmd_buf = NULL;
  netmgr_address_info_t * addr_info = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  STM_NULL_CHECK( sm );
  STM_UNUSED( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

#ifdef NETMGR_QOS_ENABLED
  if( netmgr_main_get_qos_enabled() &&
      !NETMGR_KIF_IS_REV_RMNET_LINK(cmd_buf->data.link) )
  {
    /* Initiate Traffic Control root flow delete process */
    if (NETMGR_SUCCESS !=
        netmgr_tc_flow_delete(cmd_buf->data.link,
                              NETMGR_QMI_PRIMARY_FLOW_ID))
    {
      netmgr_log_err("failed on tc_flow_delete\n");
    }
  }
#endif /* NETMGR_QOS_ENABLED */

#ifdef FEATURE_DATA_IWLAN
    if (NETMGR_KIF_IS_REV_RMNET_LINK(cmd_buf->data.link))
    {
      int ip_family, config_status;

      if (NETMGR_SUCCESS != netmgr_qmi_retrieve_rev_ip_config_status(cmd_buf->data.link, &ip_family, &config_status))
      {
        netmgr_log_med("netmgr_sm_state_inited_entry: failed to get rev IP config status for link=%d, ignoring\n",
                       cmd_buf->data.link);
      }
      else
      {
        netmgr_qmi_send_rev_ip_config_complete(NETMGR_QMI_IWLAN_CALL_CLEANUP,
                                               cmd_buf->data.link,
                                               ip_family,
                                               config_status);
      }
    }
#endif /* FEATURE_DATA_IWLAN */

  cmd_buf = (netmgr_exec_cmd_t *)payload;
  addr_info = netmgr_qmi_get_addr_info(cmd_buf->data.link);
  if (NULL == addr_info)
  {
    netmgr_log_err("failed to get addr_info\n");
    next_state = NETMGR_STATE_DOWN;
  }
  /* Initiate KIF interface open process */
  else if( NETMGR_SUCCESS !=
      netmgr_kif_iface_open( (uint8)cmd_buf->data.link,
                             &addr_info->ipv4,
                             NULL, NULL ) ) {
    netmgr_log_err("failed on kif_iface_open\n");

    /* Force platform interface to close */
    (void)netmgr_kif_iface_close( (uint8)cmd_buf->data.link,
                                  &addr_info->ipv4,
                                  TRUE );
    next_state = NETMGR_STATE_DOWN;
  }
  else
  {
    next_state = NETMGR_STATE_COMING_UP;
  }

  NETMGR_LOG_FUNC_EXIT;
  return( next_state );
} /* netmgr_sm_kif_down_to_come_up() */

/*===========================================================================

     (State Machine: NETMGR_SM)
     STATE ENTRY/EXIT/TRANSITION FUNCTIONS: NETMGR_STATE_CONFIGURING

===========================================================================*/

/*===========================================================================

  STATE ENTRY FUNCTION:  netmgr_sm_state_configuring_entry

===========================================================================*/
/*!
    @brief
    Entry function for state machine NETMGR_SM,
    state NETMGR_STATE_CONFIGURING

    @detail
    Called upon entry into this state of the state machine, with optional
    user-passed payload pointer parameter.  The prior state of the state
    machine is also passed as the prev_state parameter.

    @return
    None

*/
/*=========================================================================*/
void netmgr_sm_state_configuring_entry
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  stm_state_t         prev_state,  /*!< Previous state */
  void                *payload     /*!< Payload pointer */
)
{
  STM_UNUSED( payload );
  STM_UNUSED( prev_state );
  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
} /* netmgr_sm_state_configuring_entry() */


/*===========================================================================

  STATE EXIT FUNCTION:  netmgr_sm_state_configuring_exit

===========================================================================*/
/*!
    @brief
    Exit function for state machine NETMGR_SM,
    state NETMGR_STATE_CONFIGURING

    @detail
    Called upon exit of this state of the state machine, with optional
    user-passed payload pointer parameter.  The impending state of the state
    machine is also passed as the next_state parameter.

    @return
    None

*/
/*=========================================================================*/
void netmgr_sm_state_configuring_exit
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  stm_state_t         next_state,  /*!< Next state */
  void                *payload     /*!< Payload pointer */
)
{
  STM_UNUSED( payload );
  STM_UNUSED( next_state );
  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
} /* netmgr_sm_state_configuring_exit() */

/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_repost_modem_connected

===========================================================================*/
/*!
    @brief

    @detail
    Called upon receipt of input NETMGR_WDS_CONNECTED_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_repost_modem_connected
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
  netmgr_exec_cmd_t * cmd_buf;
  netmgr_exec_cmd_t * new_cmd;
  unsigned int repost_delay = 0;

  NETMGR_LOG_FUNC_ENTRY;

  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  if( cmd_buf->repost_count < NETMGR_EXEC_MAX_CMD_REPOST_LIMIT )
  {
    new_cmd = netmgr_exec_get_cmd();
    NETMGR_ASSERT(new_cmd);

    new_cmd->data = cmd_buf->data;
    new_cmd->repost_count = cmd_buf->repost_count + 1;

    /* Use exponential back off starting with NETMGR_EXEC_REPOST_STEP_MICROSECS */
    repost_delay = (unsigned int)((1 << cmd_buf->repost_count) * NETMGR_EXEC_REPOST_STEP_MICROSECS);

    /* Limit the max repost_delay to 1 second */
    repost_delay = (repost_delay > NETMGR_EXEC_MICROSECS_PER_SEC) ?
                   NETMGR_EXEC_MICROSECS_PER_SEC :
                   repost_delay;

    netmgr_log_med("Reposting connect event on link[%d] repost_count[%d] repost_delay[%u] usec\n",
                   cmd_buf->data.link,
                   cmd_buf->repost_count,
                   repost_delay);

    usleep(repost_delay);

    if( NETMGR_SUCCESS != netmgr_exec_put_cmd( new_cmd ) ) {
      NETMGR_ABORT("netmgr_sm_repost_modem_connected: failed to put commmand\n");
      if (NULL != new_cmd
          && NULL != new_cmd->data.info.connect_msg.addr_info_ptr)
      {
        ds_free( new_cmd->data.info.connect_msg.addr_info_ptr );
      }
      netmgr_exec_release_cmd(new_cmd);
    }
  }
  else
  {
    if (NULL != cmd_buf
        && NULL != cmd_buf->data.info.connect_msg.addr_info_ptr)
    {
      ds_free( cmd_buf->data.info.connect_msg.addr_info_ptr );
      cmd_buf->data.info.connect_msg.addr_info_ptr = NULL;
    }
  }

  NETMGR_LOG_FUNC_EXIT;
  return( next_state );
} /* netmgr_sm_repost_modem_connected() */


/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_kif_configured

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_CONFIGURING,
    upon receiving input NETMGR_KIF_CONFIGURED_EV

    @detail
    Called upon receipt of input NETMGR_KIF_CONFIGURED_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_kif_configured
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
  netmgr_exec_cmd_t * cmd_buf = NULL;
#ifdef NETMGR_QOS_ENABLED
  netmgr_qmi_qos_flow_info_t  qos_flow;
#endif /* NETMGR_QOS_ENABLED */

  NETMGR_LOG_FUNC_ENTRY;

  STM_NULL_CHECK( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

#ifdef NETMGR_QOS_ENABLED
  if( netmgr_main_get_qos_enabled() &&
      !NETMGR_KIF_IS_REV_RMNET_LINK(cmd_buf->data.link) )
  {
    /* Initialize query data structure */
    qos_flow.flow_id = NETMGR_QMI_PRIMARY_FLOW_ID;
    qos_flow.num_filter = 0;
    qos_flow.is_new = TRUE;
    qos_flow.priority = NETMGR_TC_DEFAULT_PRIORITY;
    qos_flow.datarate = NETMGR_TC_DEFAULT_DATARATE;

    /* Query QMI for primary flow QoS parameters*/
    if( NETMGR_SUCCESS !=
        netmgr_qmi_qos_get_flow_info( (uint8)cmd_buf->data.link,
                                      qos_flow.flow_id,
                                      &qos_flow.priority,
                                      &qos_flow.datarate ) ) {
      netmgr_log_err("failed on qmi_qos_get_flow_info\n");
    }

    /* Initiate Traffic Control flow create process */
    if( NETMGR_SUCCESS !=
        netmgr_tc_flow_activate( cmd_buf->data.link,
                                 &qos_flow ) ) {
      netmgr_log_err("failed on tc_flow_activate\n");
    }

    {
      int i = 0;

      netmgr_log_med("activating cached secondary flows");

      for (i =0; i < QMI_QOS_MAX_FLOW_FILTER; i++)
      {
        if (netmgr_qmi_cfg.links[cmd_buf->data.link].qos_info.qos_flows[i] != NULL)
        {
          /* Initiate Traffic Control flow create process */
          if( NETMGR_SUCCESS !=
              netmgr_tc_flow_activate( cmd_buf->data.link,
                                       netmgr_qmi_cfg.links[cmd_buf->data.link].qos_info.qos_flows[i] ) )
          {
            netmgr_log_err("failed on tc_flow_activate\n");
          }
          free(netmgr_qmi_cfg.links[cmd_buf->data.link].qos_info.qos_flows[i]);
          netmgr_qmi_cfg.links[cmd_buf->data.link].qos_info.qos_flows[i] = NULL;
        }
      }
    }
  }
#endif /* NETMGR_QOS_ENABLED */

  next_state = NETMGR_STATE_UP;

  NETMGR_LOG_FUNC_EXIT;
  return( next_state );

} /* netmgr_sm_kif_configured() */


/*===========================================================================

     (State Machine: NETMGR_SM)
     STATE ENTRY/EXIT/TRANSITION FUNCTIONS: NETMGR_STATE_RECONFIGURING

===========================================================================*/

/*===========================================================================

  STATE ENTRY FUNCTION:  netmgr_sm_state_reconfiguring_entry

===========================================================================*/
/*!
    @brief
    Entry function for state machine NETMGR_SM,
    state NETMGR_STATE_RECONFIGURING

    @detail
    Called upon entry into this state of the state machine, with optional
    user-passed payload pointer parameter.  The prior state of the state
    machine is also passed as the prev_state parameter.

    @return
    None

*/
/*=========================================================================*/
void netmgr_sm_state_reconfiguring_entry
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  stm_state_t         prev_state,  /*!< Previous state */
  void                *payload     /*!< Payload pointer */
)
{
  STM_UNUSED( payload );
  STM_UNUSED( prev_state );
  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
} /* netmgr_sm_state_reconfiguring_entry() */


/*===========================================================================

  STATE EXIT FUNCTION:  netmgr_sm_state_reconfiguring_exit

===========================================================================*/
/*!
    @brief
    Exit function for state machine NETMGR_SM,
    state NETMGR_STATE_RECONFIGURING

    @detail
    Called upon exit of this state of the state machine, with optional
    user-passed payload pointer parameter.  The impending state of the state
    machine is also passed as the next_state parameter.

    @return
    None

*/
/*=========================================================================*/
void netmgr_sm_state_reconfiguring_exit
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  stm_state_t         next_state,  /*!< Next state */
  void                *payload     /*!< Payload pointer */
)
{
  STM_UNUSED( payload );
  STM_UNUSED( next_state );
  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
} /* netmgr_sm_state_reconfiguring_exit() */


/*===========================================================================

     (State Machine: NETMGR_SM)
     STATE ENTRY/EXIT/TRANSITION FUNCTIONS: NETMGR_STATE_UP

===========================================================================*/

/*===========================================================================

  STATE ENTRY FUNCTION:  netmgr_sm_state_up_entry

===========================================================================*/
/*!
    @brief
    Entry function for state machine NETMGR_SM,
    state NETMGR_STATE_UP

    @detail
    Called upon entry into this state of the state machine, with optional
    user-passed payload pointer parameter.  The prior state of the state
    machine is also passed as the prev_state parameter.

    @return
    None

*/
/*=========================================================================*/
void netmgr_sm_state_up_entry
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  stm_state_t         prev_state,  /*!< Previous state */
  void                *payload     /*!< Payload pointer */
)
{
  netmgr_exec_cmd_t * cmd_buf = NULL;
  netmgr_nl_events_t event = NET_PLATFORM_INVALID_EV;
  netmgr_nl_event_info_t * event_info = NULL;
  netmgr_address_set_t * modem_addr_ptr = NULL;
  netmgr_nl_addr_info_t * nl_addr_ptr = NULL;
  const char *dev_name = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  STM_NULL_CHECK( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;
  modem_addr_ptr = cmd_buf->data.info.connect_msg.addr_info_ptr;
  nl_addr_ptr = &cmd_buf->data.info.connect_msg.nlmsg_info.addr_info;

  dev_name = netmgr_kif_get_name(cmd_buf->data.link);
  if(dev_name == NULL)
  {
    netmgr_log_err("%s(): unable to determine name for link=%d\n", __func__, link);
    goto bail;
  }

  event = (NETMGR_STATE_RECONFIGURING == prev_state)?
          NET_PLATFORM_RECONFIGURED_EV :  NET_PLATFORM_UP_EV;

  /* Post event indication to clients */
  event_info = netmgr_malloc( sizeof(netmgr_nl_event_info_t) );
  if( NULL == event_info ) {
    netmgr_log_err("failed to allocate event buffer!\n");
  } else {
    memset( event_info, 0x0, sizeof(netmgr_nl_event_info_t) );
    event_info->event = event;
    event_info->link = cmd_buf->data.link;
    event_info->param_mask |= NETMGR_EVT_PARAM_LINK;
    strlcpy( event_info->dev_name,
             dev_name,
             sizeof(event_info->dev_name) );
    event_info->param_mask |= NETMGR_EVT_PARAM_DEVNAME;

    /* Populate address elements */
    if( modem_addr_ptr )
    {
      if( NETMGR_IP_ANY_ADDR != modem_addr_ptr->gateway.type ) {
        NETMGR_CONV_TO_SOCKADDR( &modem_addr_ptr->gateway, &event_info->gtwy_info.ip_addr );
        event_info->gtwy_info.mask = modem_addr_ptr->gw_mask;
        event_info->param_mask |= NETMGR_EVT_PARAM_GTWYINFO;
      }

      if( NETMGR_IP_ANY_ADDR != modem_addr_ptr->dns_primary.type ) {
        NETMGR_CONV_TO_SOCKADDR( &modem_addr_ptr->dns_primary, &event_info->dnsp_addr );
        event_info->param_mask |= NETMGR_EVT_PARAM_DNSPADDR;
      }

      if( NETMGR_IP_ANY_ADDR != modem_addr_ptr->dns_secondary.type ) {
        NETMGR_CONV_TO_SOCKADDR( &modem_addr_ptr->dns_secondary, &event_info->dnss_addr );
        event_info->param_mask |= NETMGR_EVT_PARAM_DNSSADDR;
      }

      if( NETMGR_NLA_PARAM_PREFIXADDR & nl_addr_ptr->attr_info.param_mask ) {
        memcpy( &event_info->addr_info.addr.ip_addr,
                &nl_addr_ptr->attr_info.prefix_addr,
                sizeof(event_info->addr_info.addr.ip_addr) );
        event_info->addr_info.addr.mask = (unsigned int)ds_get_num_bits_set_count(modem_addr_ptr->if_mask);
        event_info->param_mask |= NETMGR_EVT_PARAM_IPADDR;
      }
      if( NETMGR_NLA_PARAM_CACHEINFO & nl_addr_ptr->attr_info.param_mask ) {
        memcpy( &event_info->addr_info.cache_info,
                &nl_addr_ptr->attr_info.cache_info,
                sizeof(event_info->addr_info.cache_info) );
        event_info->param_mask |= NETMGR_EVT_PARAM_CACHE;
      }

      event_info->mtu = netmgr_kif_get_mtu(cmd_buf->data.link);
      event_info->param_mask |= NETMGR_EVT_PARAM_MTU;

#ifdef FEATURE_DATA_IWLAN
      if( NETMGR_NLA_PARAM_PREFIXADDR & nl_addr_ptr->attr_info.param_mask ) {
        (void)netmgr_qmi_iwlan_update_link_assoc(cmd_buf->data.link, NULL);
      }
#endif /* FEATURE_DATA_IWLAN */
    }

    if( NETMGR_SUCCESS != netmgr_kif_send_event_msg( event_info ) ) {
      netmgr_log_err("failed on kif_send_event UP\n");
    }

    netmgr_free( event_info );
  }

bail:
  NETMGR_LOG_FUNC_EXIT;
} /* netmgr_sm_state_up_entry() */


/*===========================================================================

  STATE EXIT FUNCTION:  netmgr_sm_state_up_exit

===========================================================================*/
/*!
    @brief
    Exit function for state machine NETMGR_SM,
    state NETMGR_STATE_UP

    @detail
    Called upon exit of this state of the state machine, with optional
    user-passed payload pointer parameter.  The impending state of the state
    machine is also passed as the next_state parameter.

    @return
    None

*/
/*=========================================================================*/
void netmgr_sm_state_up_exit
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  stm_state_t         next_state,  /*!< Next state */
  void                *payload     /*!< Payload pointer */
)
{
  STM_UNUSED( payload );
  STM_UNUSED( next_state );
  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
} /* netmgr_sm_state_up_exit() */


/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_modem_reconfig

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_UP,
    upon receiving input NETMGR_WDS_CONNECTED_EV

    @detail
    Called upon receipt of input NETMGR_WDS_CONNECTED_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_modem_reconfig
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
  netmgr_exec_cmd_t * cmd_buf;

  NETMGR_LOG_FUNC_ENTRY;

  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  /* Initiate KIF interface reconfiguration process */
  if( NETMGR_SUCCESS !=
      netmgr_kif_iface_reconfigure( (uint8)cmd_buf->data.link,
                                    cmd_buf->data.info.connect_msg.addr_info_ptr ) ) {
    netmgr_log_err("failed on kif_iface_configure\n");

    /* Force platform interface to close */
    (void)netmgr_kif_iface_close( (uint8)cmd_buf->data.link, NULL, TRUE );
    next_state = NETMGR_STATE_INITED;
  }
  else {
    /* Reconfiguration due to Modem interface address change,
     * transition to RECONFIGURING state */
    next_state = NETMGR_STATE_RECONFIGURING;
  }

  /* Release heap memory allocated in QMI module */
  ds_free( cmd_buf->data.info.connect_msg.addr_info_ptr );

  NETMGR_LOG_FUNC_EXIT;
  return( next_state );

} /* netmgr_sm_modem_reconfig() */


/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_qos_activate

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_UP,
    upon receiving input NETMGR_QOS_ACTIVATE_EV

    @detail
    Called upon receipt of input NETMGR_QOS_ACTIVATE_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_qos_activate
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
#ifdef NETMGR_QOS_ENABLED
  netmgr_exec_cmd_t * cmd_buf = NULL;
  netmgr_nl_event_info_t * event_info = NULL;
  int i =0;
  const char *dev_name = NULL;
  NETMGR_LOG_FUNC_ENTRY;

  STM_UNUSED( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  /* Special handling where QOS_ACTIVATE event comes very soon
   * after WDS_CONNECTED. In this case we store the flow and filter
   * spec and configure them later when the kernel interface is UP
   * and running.
   */
  if (sm->current_state == NETMGR_STATE_COMING_UP ||
        sm->current_state == NETMGR_STATE_INITED ||
        sm->current_state == NETMGR_STATE_CONFIGURING ||
        sm->current_state == NETMGR_STATE_RECONFIGURING)
  {
    for (i = 0; i < QMI_QOS_MAX_FLOW_FILTER; i++)
    {
      if (netmgr_qmi_cfg.links[cmd_buf->data.link].qos_info.qos_flows[i] == NULL)
      {

        netmgr_qmi_cfg.links[cmd_buf->data.link].qos_info.qos_flows[i] =
          malloc(sizeof(netmgr_qmi_qos_flow_info_t));

        if (!netmgr_qmi_cfg.links[cmd_buf->data.link].qos_info.qos_flows[i])
        {
          netmgr_log_err( "Cannot allocate memory for storing QOS flow\n");
          return next_state;
        }

        memcpy(netmgr_qmi_cfg.links[cmd_buf->data.link].qos_info.qos_flows[i],
               &cmd_buf->data.info.qos_flow,
               sizeof(netmgr_qmi_qos_flow_info_t));

       break;
      }

    }
    if (i == QMI_QOS_MAX_FLOW_FILTER)
    {
      netmgr_log_err( "Cache limit reached for storing qos_flow. "
                      "flow will not be activated.\n");
    }

    return next_state;
  }

  dev_name = netmgr_kif_get_name(cmd_buf->data.link);

  if( netmgr_main_get_qos_enabled() &&
     !NETMGR_KIF_IS_REV_RMNET_LINK(cmd_buf->data.link) )
  {
    /* Initiate Traffic Control flow create process */
    if( NETMGR_SUCCESS !=
        netmgr_tc_flow_activate( cmd_buf->data.link,
                                 &cmd_buf->data.info.qos_flow ) ) {
      netmgr_log_err("failed on tc_flow_create\n");
    }
    else if(NULL == dev_name) {
      netmgr_log_err("%s(): unable to determine name for link=%d\n", __func__, link);
    }
    else {
      /* Post event indication to clients */
      event_info = netmgr_malloc( sizeof(netmgr_nl_event_info_t) );
      if( NULL == event_info ) {
        netmgr_log_err("failed to allocate event buffer!\n");
      } else {
        memset( event_info, 0x0, sizeof(netmgr_nl_event_info_t) );
        event_info->event = NET_PLATFORM_FLOW_ACTIVATED_EV;
        event_info->link = cmd_buf->data.link;
        event_info->flow_info.flow_id =  (unsigned int)cmd_buf->data.info.qos_flow.flow_id;
        event_info->flow_info.flow_type =  cmd_buf->data.info.qos_flow.flow_type;
        event_info->param_mask |= ( NETMGR_EVT_PARAM_LINK | NETMGR_EVT_PARAM_FLOWINFO );
        strlcpy( event_info->dev_name,
                 dev_name,
                 sizeof(event_info->dev_name) );
        event_info->param_mask |= NETMGR_EVT_PARAM_DEVNAME;

        if( NETMGR_SUCCESS !=
            netmgr_kif_send_event_msg( event_info ) ) {
          netmgr_log_err("failed on kif_send_event ACTIVATED\n");
        }

        netmgr_free( event_info );
      }
    }
  }

  NETMGR_LOG_FUNC_EXIT;
#else
  STM_UNUSED( sm );
  STM_UNUSED( payload );
#endif /* NETMGR_QOS_ENABLED */
  return( next_state );

} /* netmgr_sm_qos_activate() */


/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_qos_modify

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_UP,
    upon receiving input NETMGR_QOS_MODIFY_EV

    @detail
    Called upon receipt of input NETMGR_QOS_MODIFY_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_qos_modify
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
#ifdef NETMGR_QOS_ENABLED
  netmgr_exec_cmd_t * cmd_buf = NULL;
  netmgr_nl_event_info_t * event_info = NULL;
  const char *dev_name = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  STM_UNUSED( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  if( netmgr_main_get_qos_enabled() &&
      !NETMGR_KIF_IS_REV_RMNET_LINK(cmd_buf->data.link) )
  {
    /* Initiate Traffic Control flow modify process */
    if( NETMGR_SUCCESS !=
        netmgr_tc_flow_modify(cmd_buf->data.link,
                              &cmd_buf->data.info.qos_flow))
    {
      netmgr_exec_cmd_t * new_cmd = NULL;

      netmgr_log_err("failed on tc_flow_modify, post command to "
                      "executive to delete\n");

      new_cmd = netmgr_exec_get_cmd();
      NETMGR_ASSERT(new_cmd);

      new_cmd->data.type = NETMGR_QOS_DELETE_EV;
      new_cmd->data.link = cmd_buf->data.link;
      new_cmd->data.info.qos_flow.flow_id = cmd_buf->data.info.qos_flow.flow_id;

      if (NETMGR_SUCCESS != netmgr_exec_put_cmd(new_cmd))
      {
        NETMGR_ABORT("failed to put command");
        netmgr_exec_release_cmd(new_cmd);
      }
    }
    else if(NULL == (dev_name = netmgr_kif_get_name(cmd_buf->data.link))) {
      netmgr_log_err("%s(): unable to determine name for link=%d\n",
                      __func__, cmd_buf->data.link);
    }
    else
    {
      /* Post event indication to clients */
      event_info = netmgr_malloc( sizeof(netmgr_nl_event_info_t) );
      if( NULL == event_info ) {
        netmgr_log_err("failed to allocate event buffer!\n");
      } else {
        memset( event_info, 0x0, sizeof(netmgr_nl_event_info_t) );
        event_info->event = NET_PLATFORM_FLOW_MODIFIED_EV;
        event_info->link = cmd_buf->data.link;
        event_info->flow_info.flow_id =  (unsigned int) cmd_buf->data.info.qos_flow.flow_id;
        event_info->flow_info.flow_type =  cmd_buf->data.info.qos_flow.flow_type;
        event_info->param_mask |= ( NETMGR_EVT_PARAM_LINK | NETMGR_EVT_PARAM_FLOWINFO );
        strlcpy( event_info->dev_name,
                 dev_name,
                 sizeof(event_info->dev_name) );
        event_info->param_mask |= NETMGR_EVT_PARAM_DEVNAME;

        if( NETMGR_SUCCESS !=
            netmgr_kif_send_event_msg( event_info ) ) {
          netmgr_log_err("failed on kif_send_event MODIFIED\n");
        }

        netmgr_free( event_info );
      }
    }
  }

  NETMGR_LOG_FUNC_EXIT;
#else
  STM_UNUSED( sm );
  STM_UNUSED( payload );
#endif /* NETMGR_QOS_ENABLED */

  return next_state;

} /* netmgr_sm_qos_modify() */


/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_qos_delete

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_UP,
    upon receiving input NETMGR_QOS_DELETE_EV

    @detail
    Called upon receipt of input NETMGR_QOS_DELETE_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_qos_delete
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
#ifdef NETMGR_QOS_ENABLED
  netmgr_exec_cmd_t * cmd_buf = NULL;
  netmgr_nl_event_info_t * event_info = NULL;
  const char * dev_name = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  STM_UNUSED( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  if( netmgr_main_get_qos_enabled() &&
      !NETMGR_KIF_IS_REV_RMNET_LINK(cmd_buf->data.link) &&
      !NETMGR_IS_DEFAULT_FLOW(cmd_buf->data.info.qos_flow.flow_id) )
  {
    /* Initiate Traffic Control flow delete process */
    if( NETMGR_SUCCESS !=
        netmgr_tc_flow_delete( cmd_buf->data.link,
                               cmd_buf->data.info.qos_flow.flow_id) )
    {
      netmgr_log_err("failed on tc_flow_delete\n");
    }
    else if(NULL == (dev_name = netmgr_kif_get_name(cmd_buf->data.link))) {
      netmgr_log_err("%s(): unable to determine name for link=%d\n",
                      __func__, cmd_buf->data.link);
    }
    else
    {
      /* Post event indication to clients */
      event_info = netmgr_malloc( sizeof(netmgr_nl_event_info_t) );
      if( NULL == event_info ) {
        netmgr_log_err("failed to allocate event buffer!\n");
      } else {
        memset( event_info, 0x0, sizeof(netmgr_nl_event_info_t) );
        event_info->event = NET_PLATFORM_FLOW_DELETED_EV;
        event_info->link = cmd_buf->data.link;
        event_info->flow_info.flow_id =  (unsigned int)cmd_buf->data.info.qos_flow.flow_id;
        event_info->flow_info.flow_type =  cmd_buf->data.info.qos_flow.flow_type;
        event_info->param_mask |= ( NETMGR_EVT_PARAM_LINK | NETMGR_EVT_PARAM_FLOWINFO );
        strlcpy( event_info->dev_name,
                 dev_name,
                 sizeof(event_info->dev_name) );
        event_info->param_mask |= NETMGR_EVT_PARAM_DEVNAME;

        if( NETMGR_SUCCESS !=
            netmgr_kif_send_event_msg( event_info ) ) {
          netmgr_log_err("failed on kif_send_event DELETED\n");
        }

        netmgr_free( event_info );
      }
    }
  }

  NETMGR_LOG_FUNC_EXIT;
#else
  STM_UNUSED( sm );
  STM_UNUSED( payload );
#endif /* NETMGR_QOS_ENABLED */
  return( next_state );

} /* netmgr_sm_qos_delete() */

/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_qos_suspend

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_UP,
    upon receiving input NETMGR_QOS_SUSPEND_EV

    @detail
    Called upon receipt of input NETMGR_QOS_SUSPEND_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_qos_suspend
(
 stm_state_machine_t *sm,         /*!< State Machine instance pointer */
 void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
#ifdef NETMGR_QOS_ENABLED
  netmgr_exec_cmd_t * cmd_buf = NULL;
  netmgr_nl_event_info_t * event_info = NULL;
  int i;
  const char *dev_name = NULL;
  NETMGR_LOG_FUNC_ENTRY;

  STM_UNUSED( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  /* Special handling where QOS_SUSPEND event comes very soon
   * after WDS_CONNECTED. In this case we store the flow and filter
   * spec and configure them later when the kernel interface is UP
   * and running.
   */
  if (sm->current_state == NETMGR_STATE_COMING_UP ||
        sm->current_state == NETMGR_STATE_INITED ||
        sm->current_state == NETMGR_STATE_CONFIGURING ||
        sm->current_state == NETMGR_STATE_RECONFIGURING)
  {
    for (i = 0; i < QMI_QOS_MAX_FLOW_FILTER; i++)
    {
      if (netmgr_qmi_cfg.links[cmd_buf->data.link].qos_info.qos_flows[i] == NULL)
      {

        netmgr_qmi_cfg.links[cmd_buf->data.link].qos_info.qos_flows[i] =
          malloc(sizeof(netmgr_qmi_qos_flow_info_t));

        if (!netmgr_qmi_cfg.links[cmd_buf->data.link].qos_info.qos_flows[i])
        {
          netmgr_log_err( "Cannot allocate memory for storing QOS flow\n");
          return next_state;
        }

        memcpy(netmgr_qmi_cfg.links[cmd_buf->data.link].qos_info.qos_flows[i],
               &cmd_buf->data.info.qos_flow,
               sizeof(netmgr_qmi_qos_flow_info_t));

       break;
      }

    }
    if (i == QMI_QOS_MAX_FLOW_FILTER)
    {
      netmgr_log_err( "Cache limit reached for storing qos_flow. "
                      "flow will not be activated.\n");
    }

    return next_state;
  }


  if( netmgr_main_get_qos_enabled() &&
      !NETMGR_KIF_IS_REV_RMNET_LINK(cmd_buf->data.link) )
  {
    /* Initiate Traffic Control flow suspend process */
    if( NETMGR_SUCCESS !=
        netmgr_tc_flow_suspend(cmd_buf->data.link,
                               &cmd_buf->data.info.qos_flow))
    {
      netmgr_log_err("failed on tc_flow_suspend\n");
    }
    else if(NULL == (dev_name = netmgr_kif_get_name(cmd_buf->data.link)))
    {
      netmgr_log_err("%s(): unable to determine name for link=%d\n",
                      __func__, cmd_buf->data.link);
    }
    else
    {
      /* Post event indication to clients */
      event_info = netmgr_malloc(sizeof(netmgr_nl_event_info_t));

      if (!event_info)
      {
        netmgr_log_err("failed to allocate event buffer!\n");
      }
      else
      {
        memset(event_info, 0x0, sizeof(netmgr_nl_event_info_t));
        event_info->event = NET_PLATFORM_FLOW_SUSPENDED_EV;
        event_info->link = cmd_buf->data.link;
        event_info->flow_info.flow_id = (unsigned int)cmd_buf->data.info.qos_flow.flow_id;
        event_info->flow_info.flow_type =  cmd_buf->data.info.qos_flow.flow_type;
        event_info->param_mask |= NETMGR_EVT_PARAM_LINK | NETMGR_EVT_PARAM_FLOWINFO;
        strlcpy( event_info->dev_name,
                 dev_name,
                 sizeof(event_info->dev_name) );
        event_info->param_mask |= NETMGR_EVT_PARAM_DEVNAME;

        if (NETMGR_SUCCESS != netmgr_kif_send_event_msg(event_info))
        {
          netmgr_log_err("failed on kif_send_event SUSPENDED\n");
        }

        netmgr_free(event_info);
      }
    }
  }

  NETMGR_LOG_FUNC_EXIT;
#else
  STM_UNUSED( sm );
  STM_UNUSED( payload );
#endif /* NETMGR_QOS_ENABLED */

  return next_state;
} /* netmgr_sm_qos_suspend() */


/*===========================================================================

  TRANSITION FUNCTION:  netmgr_sm_qos_flow_control

===========================================================================*/
/*!
    @brief
    Transition function for state machine NETMGR_SM,
    state NETMGR_STATE_UP,
    upon receiving input NETMGR_QOS_FLOCNTRL_EV

    @detail
    Called upon receipt of input NETMGR_QOS_FLOCNTRL_EV, with optional
    user-passed payload pointer.

    @return
    Returns the next state that the state machine should transition to
    upon receipt of the input.  This state must be a valid state for this
    state machine.

*/
/*=========================================================================*/
stm_state_t netmgr_sm_qos_flow_control
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  void                *payload     /*!< Payload pointer */
)
{
  stm_state_t next_state = STM_SAME_STATE; /* Default 'next' state */
#ifdef NETMGR_QOS_ENABLED
  netmgr_exec_cmd_t * cmd_buf = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  STM_UNUSED( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  if( netmgr_main_get_qos_enabled() &&
      !NETMGR_KIF_IS_REV_RMNET_LINK(cmd_buf->data.link) )
  {
    /* Initiate Traffic Control flow control process */
    if( NETMGR_SUCCESS !=
        netmgr_tc_flow_control(cmd_buf->data.link,
                               cmd_buf->data.info.qos_flow.flow_id,
                               cmd_buf->data.info.qos_flow.state))
    {
      netmgr_log_err("failed on tc_flow_control\n");
    }
  }

  NETMGR_LOG_FUNC_EXIT;
#else
  STM_UNUSED( sm );
  STM_UNUSED( payload );
#endif /* NETMGR_QOS_ENABLED */

  return next_state;
} /* netmgr_sm_qos_flow_control() */


/*===========================================================================

     (State Machine: NETMGR_SM)
     STATE ENTRY/EXIT/TRANSITION FUNCTIONS: NETMGR_STATE_GOING_DOWN

===========================================================================*/

/*===========================================================================

  STATE ENTRY FUNCTION:  netmgr_sm_state_goingdown_entry

===========================================================================*/
/*!
    @brief
    Entry function for state machine NETMGR_SM,
    state NETMGR_STATE_GOING_DOWN

    @detail
    Called upon entry into this state of the state machine, with optional
    user-passed payload pointer parameter.  The prior state of the state
    machine is also passed as the prev_state parameter.

    @return
    None

*/
/*=========================================================================*/
void netmgr_sm_state_goingdown_entry
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  stm_state_t         prev_state,  /*!< Previous state */
  void                *payload     /*!< Payload pointer */
)
{
#ifdef FEATURE_DATA_IWLAN
  netmgr_exec_cmd_t * cmd_buf = NULL;
  STM_UNUSED( prev_state );

  NETMGR_LOG_FUNC_ENTRY;

  STM_NULL_CHECK( sm );
  STM_NULL_CHECK( payload );

  cmd_buf = (netmgr_exec_cmd_t *)payload;

  /* When a forward Rmnet interface is going down, bring down the corresponding
     reverse Rmnet as well (if it's still UP) */
  if (!NETMGR_KIF_IS_REV_RMNET_LINK(cmd_buf->data.link))
  {
    int rev_link;

    /* If the forward link is associated with a valid reverse link and
       reverse link had already been disconnected, inititae a teardown */
    if (NETMGR_LINK_MAX != (rev_link = netmgr_qmi_iwlan_get_link_assoc(cmd_buf->data.link)))
    {
      netmgr_log_med("netmgr_sm_state_goingdown_entry: link=%d going down, bringing down associated rev_link=%d\n",
                     cmd_buf->data.link,
                     rev_link);
      (void)netmgr_qmi_iwlan_clear_link_assoc(cmd_buf->data.link, rev_link);
      if (NETMGR_STATE_GOING_DOWN == stm_get_state(&NETMGR_SM[rev_link]))
      {
        netmgr_kif_iface_close((uint8)rev_link, NULL, TRUE);
      }
    }
  }
#else
  STM_UNUSED( payload );
  STM_UNUSED( prev_state );
  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
#endif /* FEATURE_DATA_IWLAN */
} /* netmgr_sm_state_goingdown_entry() */


/*===========================================================================

  STATE EXIT FUNCTION:  netmgr_sm_state_goingdown_exit

===========================================================================*/
/*!
    @brief
    Exit function for state machine NETMGR_SM,
    state NETMGR_STATE_GOING_DOWN

    @detail
    Called upon exit of this state of the state machine, with optional
    user-passed payload pointer parameter.  The impending state of the state
    machine is also passed as the next_state parameter.

    @return
    None

*/
/*=========================================================================*/
void netmgr_sm_state_goingdown_exit
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  stm_state_t         next_state,  /*!< Next state */
  void                *payload     /*!< Payload pointer */
)
{
  STM_UNUSED( payload );
  STM_UNUSED( next_state );
  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
} /* netmgr_sm_state_goingdown_exit() */

/*===========================================================================

     (State Machine: NETMGR_SM)
     STATE ENTRY/EXIT/TRANSITION FUNCTIONS: NETMGR_STATE_GOING_DOWN_TO_COME_UP

===========================================================================*/

/*===========================================================================

  STATE ENTRY FUNCTION:  netmgr_sm_state_goingdowntocomeup_entry

===========================================================================*/
/*!
    @brief
    Entry function for state machine NETMGR_SM,
    state NETMGR_STATE_GOING_DOWN_TO_COME_UP

    @detail
    Called upon entry into this state of the state machine, with optional
    user-passed payload pointer parameter.  The prior state of the state
    machine is also passed as the prev_state parameter.

    @return
    None

*/
/*=========================================================================*/
void netmgr_sm_state_goingdowntocomeup_entry
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  stm_state_t         prev_state,  /*!< Previous state */
  void                *payload     /*!< Payload pointer */
)
{
  STM_UNUSED( payload );
  STM_UNUSED( prev_state );
  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
} /* netmgr_sm_state_goingdowntocomeup_entry() */


/*===========================================================================

  STATE EXIT FUNCTION:  netmgr_sm_state_goingdowntocomeup_exit

===========================================================================*/
/*!
    @brief
    Exit function for state machine NETMGR_SM,
    state NETMGR_STATE_GOING_DOWN_TO_COME_UP

    @detail
    Called upon exit of this state of the state machine, with optional
    user-passed payload pointer parameter.  The impending state of the state
    machine is also passed as the next_state parameter.

    @return
    None

*/
/*=========================================================================*/
void netmgr_sm_state_goingdowntocomeup_exit
(
  stm_state_machine_t *sm,         /*!< State Machine instance pointer */
  stm_state_t         next_state,  /*!< Next state */
  void                *payload     /*!< Payload pointer */
)
{
  STM_UNUSED( payload );
  STM_UNUSED( next_state );
  /* Ensure that the state machine instance pointer passed is valid */
  STM_NULL_CHECK( sm );
} /* netmgr_sm_state_goingdowntocomeup_exit() */


