/******************************************************************************

                        D S I _ N E T C T R L _ S T U B S . C

******************************************************************************/

/******************************************************************************

  @file    dsi_netctrl_stubs.C
  @brief   Network Manager test stubs

  DESCRIPTION
  Dsi_Netctrl test stub functions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved

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
03/10/11   js         Initial version (copied from netmgr_stubs.c)

******************************************************************************/

#ifdef DSI_NETCTRL_OFFTARGET

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "qdi.h"
#include "qmi.h"
#include "qmi_wds_srvc.h"
#include "qmi_nas_srvc.h"

#include "dsi_netctrli.h"
#include "netmgr.h"

int atexit(void (*function)(void))
{
  return 0;
}

/*-----------------------------------------------------------------------*/
#define  DSI_NETCTRL_LOG_FUNC_ENTRY printf("%s ENTRY\n", __FUNCTION__)
#define  DSI_NETCTRL_LOG_FUNC_EXIT printf("%s EXIT\n", __FUNCTION__)

#ifdef DSI_NETCTRL_USE_STUBS

static int qmi_wds_client = 0xffff;

int
qmi_init
(
  qmi_sys_event_rx_hdlr   event_rx_hdlr,
  void                    *event_user_data
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}

int
qmi_release
(
  int init_handle
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}

int
qmi_dev_connection_init
(
  const char  *dev_id,
  int         *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}

#if 0
qmi_client_handle_type
qmi_wds_srvc_init_client
(
  const char                    *dev_id,
  qmi_wds_indication_hdlr_type  user_ind_msg_hdlr,
  void                          *user_ind_msg_hdlr_user_data,
  int                           *qmi_err_code
)
{
    return qmi_wds_client++;
}
#endif

int
qmi_wds_srvc_release_client
(
  int      user_handle,
  int      *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}


int
qmi_wds_query_profile
(
  int                         user_handle,
  qmi_wds_profile_id_type     *profile_id,
  qmi_wds_profile_params_type *profile_params,
  int                         *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}

int
qmi_wds_reset_pkt_statistics
(
  int                               user_handle,
  int                               *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}

int
qmi_wds_get_pkt_statistics
(
int                               user_handle,
unsigned long                     stats_mask,
qmi_wds_xfer_stats                *stats_resp,
int                               *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}

int
qmi_wds_set_client_ip_pref
(
  int                          user_handle,
  qmi_wds_ip_family_pref_type  ip_family,
  int                          *qmi_err_code
)

{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}

int
qmi_wds_get_curr_call_info
(
  int                                        user_handle,
  qmi_wds_req_runtime_settings_params_type   requested_settings,
  qmi_wds_profile_id_type                    *profile_id,
  qmi_wds_profile_params_type                *profile_params,
  qmi_wds_curr_call_info_type                *call_settings,
  int                                        *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  call_settings->mask = (QMI_WDS_CURR_CALL_INFO_IP_FAMILY|
                         QMI_WDS_CURR_CALL_INFO_IPV4_ADDR);
  call_settings->ip_family = QMI_WDS_IP_FAMILY_PREF_IPV4;
  call_settings->ipv4_addr = 0x01020304;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}

int
qmi_wds_get_current_channel_rate
(
  int                         user_handle,
  qmi_wds_channel_rate_type   *resp_data,
  int                         *qmi_err_code
)
{
  int qmi_ret = QMI_NO_ERR;
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  if(NULL != resp_data)
  {
    resp_data->current_channel_tx_rate = 0;
    resp_data->current_channel_rx_rate = 0;
  }
  else
    qmi_ret = QMI_INTERNAL_ERR;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return qmi_ret;
}


int
qmi_wds_stop_nw_if
(
  int                         client_handle,
  qmi_wds_user_async_cb_type  user_cb,
  void                        *user_data,
  int                         *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}

int
qmi_wds_abort
(
  int                         client_handle,
  int                         txn_handle,
  qmi_wds_user_async_cb_type  user_cb,
  void                        *user_data,
  int                         *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  if (txn_handle == 0x01)
  {
    return QMI_NO_ERR;
  }
  else
  {
    printf("ERR: txn handle %d does not match with 0x01\n", txn_handle);
    return QMI_INTERNAL_ERR;
  }
}

int
qmi_wds_get_pkt_srvc_status
(
  int                       user_handle,
  qmi_wds_link_status_type  *pkt_srvc_status,
  int                       *qmi_err_code
)
{
  if (NULL == pkt_srvc_status ||
      NULL == qmi_err_code)
  {
    printf("ERR: NULL params rcvd\n");
    return QMI_INTERNAL_ERR;
  }
  /* there is no data call UP at this port */
  pkt_srvc_status = QMI_WDS_PACKET_DATA_DISCONNECTED;
  return QMI_NO_ERR;
}


int
qmi_wds_create_profile
(
  int                         user_handle,
  qmi_wds_profile_id_type     *profile_id,
  qmi_wds_profile_params_type *profile_params,
  int                         *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}

int
qmi_wds_delete_profile
(
  int                         user_handle,
  qmi_wds_profile_id_type     *profile_id,
  int                         *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}

qmi_client_handle_type
qmi_qos_srvc_init_client
(
  const char                    *dev_id,
  qmi_qos_indication_hdlr_type  ind_hdlr,
  void                          *ind_hdlr_user_data,
  int                           *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return 0x01;
}

int
qmi_qos_srvc_release_client
(
  qmi_client_handle_type  client_handle,
  int                     *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}

int
qmi_qos_request_qos
(
  qmi_client_handle_type        client_handle,
  int                           num_qos_specs,
  qmi_qos_spec_type             *qos_spec_array,
  qmi_qos_req_opcode_type       req_opcode_type,
  unsigned long                 *qos_id_array,
  qmi_qos_err_rsp_type          *qos_spec_errs,
  int                           *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}

int
qmi_qos_release_qos
(
  qmi_client_handle_type   client_handle,
  int                      num_qos_identifiers,
  unsigned long            *qos_id_array,
  int                      *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}


int
qmi_qos_modify_primary_qos
(
  qmi_client_handle_type        client_handle,
  qmi_qos_spec_type             *qos_spec,
  qmi_qos_err_rsp_type          *qos_spec_err,
  int                           *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}


int
qmi_qos_modify_secondary_qos
(
  qmi_client_handle_type        client_handle,
  int                           num_qos_specs,
  qmi_qos_spec_type             *qos_spec_array,
  qmi_qos_err_rsp_type          *qos_spec_errs,
  int                           *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}


int
qmi_qos_get_nw_supported_qos_profiles
(
  qmi_client_handle_type                         client_handle,
  qmi_qos_technology_type                        tech_pref,
  qmi_qos_nw_supported_qos_profiles_rsp_type     *rsp_data,
  int                                            *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}

int
qmi_qos_resume_qos
(
  qmi_client_handle_type    client_handle,
  unsigned char             num_qos_ids,
  unsigned long             *qos_id_array,
  int                       *qmi_err_code
  )
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}

int
qmi_qos_suspend_qos
(
  qmi_client_handle_type    client_handle,
  unsigned char             num_qos_ids,
  unsigned long             *qos_id_array,
  int                       *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}


int
qmi_qos_get_primary_granted_qos_info
(
  qmi_client_handle_type                  client_handle,
  unsigned long                           qos_identifier,
  qmi_qos_granted_info_rsp_type           *granted_info,
  int                                     *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}

int
qmi_qos_get_secondary_granted_qos_info
(
  qmi_client_handle_type                  client_handle,
  unsigned long                           qos_identifier,
  qmi_qos_granted_info_rsp_type           *granted_info,
  int                                     *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}


int
qmi_qos_does_nw_support_qos
(
  qmi_client_handle_type            client_handle,
  qmi_qos_nw_status_type            *status_resp,
  int                               *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}

int
qmi_qos_get_status
(
  qmi_client_handle_type            client_handle,
  unsigned long                     qos_identifier,
  qmi_qos_status_info               *status_resp,
  int                               *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QMI_NO_ERR;
}


qmi_client_handle_type
qmi_nas_srvc_init_client
(
  const char                    *dev_id,
  qmi_nas_indication_hdlr_type  user_ind_msg_hdlr,
  void                          *user_ind_msg_hdlr_user_data,
  int                           *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return 0x01;
}

int
qmi_nas_get_serving_system
(
  int                               client_handle,
  qmi_nas_serving_system_info_type  *resp_data,
  int                               *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  /* don't support this function call yet */
  return QMI_INTERNAL_ERR;
}


int netmgr_client_register
(
  netmgr_event_ind_f    cb_f,
  void                * data,
  netmgr_client_hdl_t * client_hndl
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

int netmgr_client_release
(
  const netmgr_client_hdl_t client_hndl
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

int netmgr_client_send_ping_msg()
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

int
qdi_init (void)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QDI_SUCCESS;
}

int
qdi_release (void)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return QDI_SUCCESS;
}

qdi_call_handle_t
qdi_alloc_call_handle
(
  qdi_client_handle_t  user_handle
)
{
  static qdi_call_handle_t handle = (qdi_call_handle_t)(0xFFFF);
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return handle;
}

void
qdi_release_call_handle
(
  qdi_call_handle_t  call_hndl
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
}

int qdi_get_qmi_wds_handle
(
  qdi_client_handle_t user_handle
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

qdi_client_handle_t qdi_wds_srvc_init_client
(
  const char                             *dev_id,
  qmi_wds_indication_hdlr_type           user_ind_msg_hdlr,
  void                                   *user_ind_msg_hdlr_user_data,
  qmi_wds_bind_mux_data_port_params_type *bind_params,
  qmi_wds_bind_subscription_type          subs_id,
  int                                    *qmi_err_code
)
{
  static qdi_client_handle_t handle = 1;

  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return handle;
}


int qdi_wds_srvc_release_client
(
  qdi_client_handle_t  user_handle,
  int                  *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

int qdi_get_addr_info
(
  qdi_client_handle_t  user_handle,
  const char           *ifname,
  int                  ipfamily,
  dsi_addr_info_t      *addr_info,
  qmi_wds_iface_name_type tech_name
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

int qdi_wds_start_nw_if
(
  qdi_client_handle_t              user_handle,
  qdi_call_handle_t                call_hndl,
  qmi_wds_start_nw_if_params_type  *params,
  boolean                          partial_retry,
  qdi_wds_user_async_cb_type       user_cb,
  void                             *user_data,
  qmi_wds_call_end_reason_type     *call_end_reason_resp,
  int                              *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

int qdi_wds_stop_nw_if
(
  qdi_client_handle_t         user_handle,
  qdi_call_handle_t           call_hndl,
  qdi_wds_user_async_cb_type  user_cb,
  void                        *user_data,
  int                         *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

int qdi_wds_abort
(
  qdi_client_handle_t         user_handle,
  qdi_call_handle_t           call_hndl,
  int                         txn_handle,
  qdi_wds_user_async_cb_type  user_cb,
  void                        *user_data,
  int                         *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

int qmi_wds_indication_register
(
  int                               user_handle,
  qmi_wds_indication_reg_req_type   ind_type,
  int                               *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

int qmi_wds_embms_tmgi_deactivate
(
  int                                   user_handle,
  qmi_wds_embms_actdeact_tmgi_info_type *tmgi,
  int                                   *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

int qmi_wds_embms_tmgi_activate
(
  int                                    user_handle,
  qmi_wds_embms_actdeact_tmgi_info_type  *tmgi,
  int                                    *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

int qmi_wds_embms_tmgi_activate_deactivate
(
  int                                     user_handle,
  qmi_wds_embms_actdeact_tmgi_info_type  *params,
  int                                    *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

int qmi_wds_embms_tmgi_list_query
(
  int                               user_handle,
  qmi_wds_embms_tmgi_list_type      list_type,
  qmi_wds_embms_tmgi_list_ind_type  *list_query,
  int                               *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

int
qmi_wds_get_current_bearer_tech
(
  int                            user_handle,
  qmi_wds_data_bearer_tech_type  *rsp_data,
  int                            *qmi_err_code
)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

#endif /* DSI_NETCTRL_USE_STUBS */

int property_get(const char *key, char *value, const char *default_value)
{
  DSI_NETCTRL_LOG_FUNC_ENTRY;
  DSI_NETCTRL_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;
}

#endif /* DSI_NETCTRL_OFFTARGET */
