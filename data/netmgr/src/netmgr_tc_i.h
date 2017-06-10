/******************************************************************************

                          N E T M G R _ T C _ I . H

******************************************************************************/

/******************************************************************************

  @file    netmgr_tc_i.h
  @brief   Network Manager traffic control internal header file

  DESCRIPTION
  Header file for NetMgr Linux traffic control interface.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010 Qualcomm Technologies, Inc. All Rights Reserved

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
02/23/10   ar         Initial version

******************************************************************************/

#ifndef __NETMGR_TC_I_H__
#define __NETMGR_TC_I_H__

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/
#include "netmgr_tc.h"

/*===========================================================================
                        DEFINITIONS AND DECLARATIONS
===========================================================================*/

/*---------------------------------------------------------------------------
   Type representing TC handle information
---------------------------------------------------------------------------*/
typedef struct netmgr_tc_handle_info_s {
  int major;
  int minor;
} netmgr_tc_handle_info_t;

/*---------------------------------------------------------------------------
   Type representing collection of a flow's state information
---------------------------------------------------------------------------*/
typedef struct netmgr_tc_flow_info_s {
  netmgr_qmi_qos_flow_info_t  qos_flow; /* Modem flow attributes */
  netmgr_tc_handle_info_t* class_handle; /* class handle */
} netmgr_tc_flow_info_t;

/*---------------------------------------------------------------------------
   Type representing collection of a flow's state information
---------------------------------------------------------------------------*/
typedef struct netmgr_tc_link_info_s {
  netmgr_tc_handle_info_t *root_qdisc;    /* root qdisc handle             */
  netmgr_tc_handle_info_t *root_class;    /* root class handle             */
  netmgr_tc_handle_info_t *default_class; /* default class handle          */
  netmgr_tc_handle_info_t *tcp_ack_class; /* tcp ack class handle          */
  ds_dll_el_t *flow_list;                 /* Flow objects list head        */
  ds_dll_el_t *filter_list;                /* Filter precedence list head */
  int next_class_minor;                   /* Class minor ID sequence       */
} netmgr_tc_link_info_t;

/*---------------------------------------------------------------------------
   Collection of configuration information for the module
---------------------------------------------------------------------------*/
struct netmgr_tc_cfg_s {
  int                   nlink;                     /* number of qmi links  */
  netmgr_ctl_port_config_type *link_array;         /* link enabled array   */
  netmgr_tc_link_info_t links[NETMGR_MAX_LINK];    /* link state info      */
  boolean               is_initialized;            /* Flag init completed  */
  boolean               postrouting_chain_available;  /* chain to only mark qcom qos*/
};


/*===========================================================================
                            FUNCTION DECLARATIONS
===========================================================================*/

#endif /* __NETMGR_TC_I_H__ */
