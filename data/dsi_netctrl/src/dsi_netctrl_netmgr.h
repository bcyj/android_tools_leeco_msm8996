/*!
  @file
  dsi_netctrl_netmgr.h

  @brief
  This is an internal header file that declares netmgr related functions.
*/

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

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header:  $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
04/19/10   js      created
===========================================================================*/
#include "netmgr.h"

/* netmgr link to dsi iface id mapping */
typedef struct dsi_netmgr_link_map_s
{
  netmgr_link_id_t netmgr_link;
  int dsi_iface_id;
} dsi_netmgr_link_map_type;

extern dsi_netmgr_link_map_type dsi_netmgr_link_map_tbl[];

/* callback function called from dsi_netctrl_cb thread
 * when netmgr event is arrrived */
extern void dsi_process_netmgr_ev
(
  netmgr_nl_events_t event,
  netmgr_nl_event_info_t * info,
  void * data
);
