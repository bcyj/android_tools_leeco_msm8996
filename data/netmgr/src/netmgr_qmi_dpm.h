#ifndef _NETMGR_QMI_DPM_H_
#define _NETMGR_QMI_DPM_H_
/**
  @file
  netmgr_qmi_dpm.h

  @brief
  Netmgr QMI DPM (QMI Data Port Mapper service) component.

  @details
  Netmgr QMI DPM component is used to configure embedded ports on the modem.
  The configuration specifies how many ports to open, what are the control
  ports and what are the data ports (hw-accel and software data ports). It
  can also specify the mapping between control and data ports. The configuration
  is choosen through configdb or hard-coded.

  The following functions are exported:
  netmgr_qmi_dpm_open_port()
  netmgr_qmi_dpm_close_port()

  The ports are opened using QCCI APIs (they must go over IPC-router since
  QMUXD transport would not be ready yet).

*/
/*===========================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved
  Qualcomm Technologies Proprietary and Confidential

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/03/13   hm/nd   Initial version

===========================================================================*/

/*===========================================================================
                     GLOBAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

typedef enum
{
  NETMGR_QMI_DPM_INVALID_EV = -1,
  NETMGR_QMI_DPM_OOS_EV,
  NETMGR_QMI_DPM_IS_EV,
  NETMGR_QMI_DPM_MAX_EV
} netmgr_dpm_event_t;

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_init
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Initializes the QMI DPM module.

    @details
    Performs the QCCI client library initialization to be able to send
    and receive DPM messages.

    @params None.
    @return NETMGR_SUCCESS: in case of success.
    @return NETMGR_FAILURE: in case of error.
    @dependencies None.
*/
int netmgr_qmi_dpm_init(void);

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_port_open
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Forms a dpm_open_port_req_msg_v01 message and sends to the modem.

    @details
    This method forms a dpm_open_port_req_msg_v01 and sends it to Q6 modem.

    This method needs to be sent over IPC router transport since QMUX would
    not be available during boot time. Once this message is received by Q6
    modem, it is going to open the ports on its ends and QMUXD boot-up is
    unblocked.

    The configuration used is from configdb XML file if it exists. Otherwise
    default hardcoded configuration is utilized.

    @params None.
    @return NETMGR_SUCCESS: in case of success.
    @return NETMGR_FAILURE: in case of error.

    @dependencies
    netmgr_qmi_dpm_init should be complete.
*/
int netmgr_qmi_dpm_port_open(void);

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_port_close
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Forms a dpm_close_port_req_msg_v01 message and sends to the modem.

    @details
    This method forms a dpm_close_port_req_msg_v01 and sends it to Q6 modem.
    Message is sent using IPC router transport.

    @params None.
    @return NETMGR_SUCCESS: in case of success.
    @return NETMGR_FAILURE: in case of error.

    @dependencies
    netmgr_qmi_dpm_init should be complete.
    netmgr_qmi_dpm_port_open should be complete, otherwise this is a NOP.
*/
int netmgr_qmi_dpm_port_close(void);

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_log_state
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Prints debug messages about QMI DPM state in netmgr.

    @details
    Prints information on whether QMI DPM is inited, what was the last
    message sent etc.

    @params None.
    @return None.
*/
void netmgr_qmi_dpm_log_state(void);

/*===========================================================================
  FUNCTION:  netmgr_qmi_dpm_process_cmdq_event
===========================================================================*/
/** @ingroup netmgr_dpm

    @brief
    Called in response to DPM events as part of command executor module

    @params None.
    @return None.
*/
void netmgr_qmi_dpm_process_cmdq_event(netmgr_dpm_event_t evt);

#endif /* _NETMGR_QMI_DPM_H_ */

