/*!
  @file
  dsi_netctrl_cb_thrd.h

  @brief
  header for dsi_netctrl_cb_thread

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
#include "qmi_wds_srvc.h"
#include "qmi_qos_srvc.h"
#include "qdi.h"
#include "netmgr.h"

#ifndef _DSI_NETCTRL_CB_THRD_
#define _DSI_NETCTRL_CB_THRD_

/*===========================================================================
                  QMI WDS service command callback
===========================================================================*/
/* async command callback. currently used for
 *  start_nw_if
 *  stop_nw_if
*/
extern void dsi_qmi_wds_cmd_cb
(
  int                           user_handle,
  qmi_service_id_type           service_id,
  int                           sys_err_code,
  int                           qmi_err_code,
  void                         *user_data,
  qmi_wds_async_rsp_id_type     rsp_id,
  qdi_wds_async_rsp_data_type  *rsp_data
);

/*===========================================================================
                  QMI WDS service event callback
===========================================================================*/
void dsi_qmi_wds_ind_cb
(
  int wds_hndl,
  qmi_service_id_type sid,
  void * user_data,
  qmi_wds_indication_id_type ind_id,
  qmi_wds_indication_data_type * ind_data
);

/*===========================================================================
                  QMI QOS service event callback
===========================================================================*/
void dsi_qmi_qos_ind_cb
(
  int qos_hndl,
  qmi_service_id_type sid,
  void * user_data,
  qmi_qos_indication_id_type ind_id,
  qmi_qos_indication_data_type * ind_data
);

/*===========================================================================
                   QMI SYSTEM IND CALLBACK
===========================================================================*/
void dsi_qmi_sys_cb
(
  qmi_sys_event_type event_id,
  const qmi_sys_event_info_type * event_info,
  void * user_data
);

/*===========================================================================
                   NETMGR CALLBACK
===========================================================================*/
void dsi_netmgr_cb
(
  netmgr_nl_events_t event,
  netmgr_nl_event_info_t * info,
  void * data
);

/*===========================================================================
                   INIT/DEINIT ROUTINES
===========================================================================*/
extern int dsi_netctrl_cb_init();
extern void dsi_netctrl_cb_deinit();

#endif /* _DSI_NETCTRL_CB_THRD_ */
