/******************************************************************************

                        N E T M G R _ P L A T F O R M . C

******************************************************************************/

/******************************************************************************

  @file    netmgr_platform.c
  @brief   Network Manager plafrom layer implementation

  DESCRIPTION
  Implementation of NetMgr's platform layer.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc. All Rights Reserved

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

#include "netmgr_defs.h"
#include "netmgr_util.h"
#include "netmgr_kif.h"
#include "netmgr_platform.h"

/*===========================================================================
                     FORWARD DECLARATIONS
===========================================================================*/


/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

/* Platform Virtual Function Table */
LOCAL struct platform_vtbl  netmgr_platform_vtable;

#define VERIFY_VTABLE_FUNC(table,func)                                  \
  if( NULL == netmgr_platform_vtable.table.func ) {                     \
    netmgr_log_err("platform vtable function not initialized: %s.%s\n", \
                   #table,#func);                                       \
    return NETMGR_FAILURE;                                              \
  }

/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/



/*===========================================================================
                            GLOBAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION  netmgr_platform_register_vtbl
===========================================================================*/
/*!
@brief
  Function to register virtual function table with the Platform layer.
  Use of vtbl will permit different implementations to be registered
  at runtime, support testing.

@return
  int

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_platform_register_vtbl(
  enum netmgr_platform_vtbl  id,
  void *                     table
)
{
  NETMGR_ASSERT( table );

  NETMGR_LOG_FUNC_ENTRY;

  /* Assign vtable to platform vtable */
  switch( id ) {
    case NETMGR_PLATFORM_VTBL_QMI:
      netmgr_platform_vtable.qmi = *(struct qmi_vtbl*)table;
      break;

    case NETMGR_PLATFORM_VTBL_KIF:
      netmgr_platform_vtable.kif = *(struct kif_vtbl*)table;
      break;

    case NETMGR_PLATFORM_VTBL_TC:
      netmgr_platform_vtable.tc = *(struct tc_vtbl*)table;
      break;

    case NETMGR_PLATFORM_VTBL_MAX:
    default:
      netmgr_log_err("Invalid vtable ID specified: %d", id);
      break;

  }

  NETMGR_LOG_FUNC_EXIT;

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_out_of_service
===========================================================================*/
/*!
@brief
  Function to handle modem out_of_service event in QMI module.

@return
  int

@note

  - Dependencies
    - None

  - Side Effects
    - qmi clients will be released
    - qmi links will be reset with default init values
*/
/*=========================================================================*/
int netmgr_qmi_out_of_service ( int link )
{
  VERIFY_VTABLE_FUNC( qmi, out_of_service );
  netmgr_platform_vtable.qmi.out_of_service(link);
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_qmi_reset
===========================================================================*/
/*!
@brief
  Function to reset connection to QMI Message Library

@return
  int

@note

  - Dependencies
    - netmgr_qmi_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_qmi_reset ( int link, netmgr_sm_events_t evt)
{
  VERIFY_VTABLE_FUNC( qmi, reset );
  return netmgr_platform_vtable.qmi.reset(link, evt);
}

/*===========================================================================
  FUNCTION  netmgr_qmi_verify
===========================================================================*/
/*!
@brief
  Function to verify if the given QMI link is active.

@return
  int

@note

  - Dependencies
    - netmgr_qmi_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_qmi_verify ( int link )
{
  VERIFY_VTABLE_FUNC( qmi, verify );
  return netmgr_platform_vtable.qmi.verify(link);
}

/*===========================================================================
  FUNCTION  netmgr_qmi_dispatch
===========================================================================*/
/*!
@brief
  Function to dispatch asynchronous command for Kernel interface module

@return
  int

@note

  - Dependencies
    - netmgr_kif_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_qmi_dispatch
(
  ds_cmd_t * cmd,
  void     * data
)
{
  VERIFY_VTABLE_FUNC( qmi, dispatch );
  return netmgr_platform_vtable.qmi.dispatch( cmd, data );
}

/*===========================================================================
  FUNCTION  netmgr_qmi_qos_get_flow_info
===========================================================================*/
/*!
@brief
  Function to query QoS flow info via QMI Message Library

@return
  int - NETMGR_SUCCESS on successful operation, NETMGR_FAILURE otherwise

@note

  - Dependencies
    - netmgr_qmi_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_qmi_qos_get_flow_info ( uint8     connection,
                                   uint32    flow_id,
                                   uint8   * priority,
                                   uint32  * datarate )
{
  VERIFY_VTABLE_FUNC( qmi, qos_get_flow_info );
  return netmgr_platform_vtable.qmi.qos_get_flow_info( connection,
                                                       flow_id,
                                                       priority,
                                                       datarate );
}

/*===========================================================================
  FUNCTION  netmgr_kif_out_of_service
===========================================================================*/
/*!
@brief
  Function to handle modem out_of_service event in KIF module.

@return
  int

@note

  - Dependencies
    - None

  - Side Effects
    - qmi clients will be released
    - qmi links will be reset with default init values
*/
/*=========================================================================*/
int netmgr_kif_out_of_service ( int link )
{
  VERIFY_VTABLE_FUNC( kif, out_of_service );
  netmgr_platform_vtable.kif.out_of_service(link);
  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_kif_reset
===========================================================================*/
/*!
@brief
  Function to reset connection to Kernel interface module

@return
  int

@note

  - Dependencies
    - netmgr_kif_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_kif_reset ( int link, netmgr_sm_events_t evt)
{
  VERIFY_VTABLE_FUNC( kif, reset );
  return netmgr_platform_vtable.kif.reset(link, evt);
}

/*===========================================================================
  FUNCTION  netmgr_kif_dispatch
===========================================================================*/
/*!
@brief
  Function to dispatch asynchronous command for Kernel interface module

@return
  int

@note

  - Dependencies
    - netmgr_kif_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_kif_dispatch
(
  ds_cmd_t * cmd,
  void     * data
)
{
  VERIFY_VTABLE_FUNC( kif, dispatch );
  return netmgr_platform_vtable.kif.dispatch( cmd, data );
}


/*===========================================================================
  FUNCTION  netmgr_kif_send_event_msg
===========================================================================*/
/*!
@brief
  Function to send Kernel interface module asynchronous event message.

@return
  int

@note

  - Dependencies
    - netmgr_kif_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_kif_send_event_msg
(
  const netmgr_nl_event_info_t *event_info
)
{
  VERIFY_VTABLE_FUNC( kif, send_event );
  return netmgr_platform_vtable.kif.send_event( event_info );
}


/*===========================================================================
  FUNCTION  netmgr_kif_iface_open
===========================================================================*/
/*!
@brief
  Function to open the Kernel network interface

@return
  int

@note

  - Dependencies
    - netmgr_kif_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_kif_iface_open ( uint8     link,
                            netmgr_address_set_t * addr_info_ptr,
                            const netmgr_kif_clntcb_t * clntcb,
                            void *    clnt_hdl )
{
  VERIFY_VTABLE_FUNC( kif, iface_open );
  return netmgr_platform_vtable.kif.iface_open( link,
                                                addr_info_ptr,
                                                clntcb,
                                                clnt_hdl );
}

/*===========================================================================
  FUNCTION  netmgr_kif_iface_close
===========================================================================*/
/*!
@brief
  Function to open the Kernel network interface

@return
  int

@note

  - Dependencies
    - netmgr_kif_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_kif_iface_close ( uint8     link,
                             netmgr_address_set_t * addr_info_ptr,
                             boolean   teardown_iface )
{
  VERIFY_VTABLE_FUNC( kif, iface_close );
  return netmgr_platform_vtable.kif.iface_close( link,
                                                 addr_info_ptr,
                                                 teardown_iface );
}

/*===========================================================================
  FUNCTION  netmgr_kif_iface_configure
===========================================================================*/
/*!
@brief
  Function to configure address for the Kernel network interface

@return
  int

@note

  - Dependencies
    - netmgr_kif_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_kif_iface_configure ( uint8             link,
                                 netmgr_ip_addr_t  addr_type )
{
  VERIFY_VTABLE_FUNC( kif, iface_configure );
  return netmgr_platform_vtable.kif.iface_configure( link, addr_type );
}

/*===========================================================================
  FUNCTION  netmgr_kif_iface_reconfigure
===========================================================================*/
/*!
@brief
  Function to reconfigure address for the Kernel network interface

@return
  int

@note

  - Dependencies
    - netmgr_kif_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_kif_iface_reconfigure ( uint8     link,
                                   netmgr_address_set_t * addr_info_ptr )
{
  VERIFY_VTABLE_FUNC( kif, iface_reconfigure );
  return netmgr_platform_vtable.kif.iface_reconfigure( link,
                                                       addr_info_ptr );
}

/*===========================================================================
  FUNCTION  netmgr_kif_flow_control
===========================================================================*/
/*!
@brief
  Function to control the state of the Traffic Control qdisc class
  mapped to the specified QoS flow.

@return
  int

@note

  - Dependencies
    - netmgr_kif_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_kif_flow_control ( uint8     connection,
                              void *    data )
{
  VERIFY_VTABLE_FUNC( kif, flow_control );
  return netmgr_platform_vtable.kif.flow_control( connection,
                                                  data );
}


/*===========================================================================
  FUNCTION  netmgr_tc_reset
===========================================================================*/
/*!
@brief
  Function to reset connection to Traffic Control Library

@return
  int

@note

  - Dependencies
    - netmgr_tc_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_tc_reset ( int link )
{
  VERIFY_VTABLE_FUNC( tc, reset );
  return netmgr_platform_vtable.tc.reset(link);
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_activate
===========================================================================*/
/*!
@brief
  Function to create/resume the Traffic Control objects for a QoS flow.

@return
  int

@note

  - Dependencies
    - netmgr_tc_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_tc_flow_activate ( int       link,
                              const netmgr_qmi_qos_flow_info_t  *qos_flow )
{
  VERIFY_VTABLE_FUNC( tc, flow_activate );
  return netmgr_platform_vtable.tc.flow_activate( link,
                                                  qos_flow );
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_modify
===========================================================================*/
/*!
@brief
  Function to change the Traffic Control object attributes for an
  existing QoS flow.

@return
  int

@note

  - Dependencies
    - netmgr_tc_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_tc_flow_modify ( int       link,
                            const netmgr_qmi_qos_flow_info_t  *qos_flow )
{
  VERIFY_VTABLE_FUNC( tc, flow_modify );
  return netmgr_platform_vtable.tc.flow_modify( link,
                                                qos_flow);

}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_delete
===========================================================================*/
/*!
@brief
  Function to destroy the Traffic Control objects for an existing QoS flow.

@return
  int

@note

  - Dependencies
    - netmgr_tc_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_tc_flow_delete ( int       link,
                            uint32    flow_id)
{
  VERIFY_VTABLE_FUNC( tc, flow_delete );
  return netmgr_platform_vtable.tc.flow_delete( link,
                                                flow_id);
}

/*===========================================================================
    FUNCTION  netmgr_tc_flow_suspend
===========================================================================*/
/*!
@brief
  Function to suspend the Traffic Control objects for an active QoS flow.

@return
  int

@note

  - Dependencies
    - netmgr_tc_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_tc_flow_suspend (int link,
                            const netmgr_qmi_qos_flow_info_t  *qos_flow )
{
  VERIFY_VTABLE_FUNC( tc, flow_suspend );
  return netmgr_platform_vtable.tc.flow_suspend(link,
                                                 qos_flow);
}

/*===========================================================================
    FUNCTION  netmgr_tc_flow_control
===========================================================================*/
/*!
@brief
  Function to control the Traffic Control objects for an active QoS flow.

@return
  int

@note

  - Dependencies
    - netmgr_tc_init() must have been called

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_tc_flow_control (int link,
                            uint32 flow_id,
                            netmgr_tc_flow_state_t state)
{
  VERIFY_VTABLE_FUNC( tc, flow_control );
  return netmgr_platform_vtable.tc.flow_control(link,
                                                flow_id,
                                                state);
}


/*===========================================================================
  FUNCTION  netmgr_platform_init
===========================================================================*/
/*!
@brief
  Function to initialize platform layer module.

@return
  int

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_platform_init(void)
{
  NETMGR_LOG_FUNC_ENTRY;

  /* Clear virtual function table */
  memset((void*)&netmgr_platform_vtable, 0x0, sizeof(netmgr_platform_vtable));

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}


