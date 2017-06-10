/******************************************************************************

                  Q C R I L _ D A T A _ S T U B S . C

******************************************************************************/

/******************************************************************************

  @file    qcril_data_stubs.C
  @brief   QCRIL Data test stubs

  DESCRIPTION
  QCRIL_Data test stub functions.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2011,2014 Qualcomm Technologies, Inc. All Rights Reserved

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
04/22/11   ar         Initial version

******************************************************************************/

#ifdef QCRIL_DATA_OFFTARGET

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#ifdef QCRIL_DATA_OFFTARGET
#include <netinet/in.h>
#include <errno.h>
#endif
#include <string.h>
#include <signal.h>
#include <time.h>

#include "qmi.h"
#include "qmi_wds_srvc.h"
#include "qmi_wds_utils.h"
#include "qmi_qos_srvc.h"
#include "dsi_netctrl.h"
#include "qcril_arb.h"
#include "qcril_log.h"
#include "qcril_reqlist.h"
#include "qcril_data_test.h"
#include "qcril_data_stubs.h"

#define STUB_ENTRY  fprintf( stderr, "STUB %s: entry\n", __func__ );
#define STUB_EXIT   fprintf( stderr, "STUB %s: exit\n", __func__ );

#define QOS_FLOW_ID (0x01020304)

#define HANDLE_INIT ((void*)0x1000)
static dsi_hndl_t next_handle = HANDLE_INIT;

static struct client_s
{
  qcril_stub_response_cb resp_cb_fn;
  void                  *resp_user_data;
  dsi_net_ev_cb          dsi_cb_fn;
  void                  *dsi_user_data;
  dsi_hndl_t             handle;
} qcril_data_stub_client;

extern qcril_data_test_client_info_t qcril_data_client_info;

dsi_qos_granted_info_type dummy_qos =
{
  .tx_granted_flow_data_is_valid = TRUE,
  .tx_granted_flow_data.ip_flow_index = 0,
  .tx_granted_flow_data.qos_flow_granted.umts_flow_desc.param_mask = ( QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE |
                                                                       QMI_QOS_UMTS_FLOW_PARAM_TRAFFIC_CLASS |
                                                                       QMI_QOS_UMTS_FLOW_PARAM_MAX_DELAY |
                                                                       QMI_QOS_UMTS_FLOW_PARAM_3GPP2_FLOW_PRIO ),
  .tx_granted_flow_data.qos_flow_granted.umts_flow_desc.traffic_class = QMI_QOS_UMTS_TC_INTERACTIVE,
  .tx_granted_flow_data.qos_flow_granted.umts_flow_desc.data_rate.guaranteed_rate = 64000,
  .tx_granted_flow_data.qos_flow_granted.umts_flow_desc.data_rate.max_rate = 64000,
  .tx_granted_flow_data.qos_flow_granted.umts_flow_desc.max_delay = 100,
  .tx_granted_flow_data.qos_flow_granted.umts_flow_desc.flow_priority_3gpp2 = 5,

  .tx_granted_flow_data.qos_flow_granted.cdma_flow_desc.param_mask = QMI_QOS_CDMA_FLOW_PARAM_PROFILE_ID,
  .tx_granted_flow_data.qos_flow_granted.cdma_flow_desc.profile_id = 42,


  .rx_granted_flow_data_is_valid = TRUE,
  .rx_granted_flow_data.ip_flow_index = 0,
  .rx_granted_flow_data.qos_flow_granted.umts_flow_desc.param_mask = QMI_QOS_UMTS_FLOW_PARAM_DATA_RATE,
  .rx_granted_flow_data.qos_flow_granted.umts_flow_desc.data_rate.guaranteed_rate = 512000,
  .rx_granted_flow_data.qos_flow_granted.umts_flow_desc.data_rate.max_rate = 1024000,

  .dss_errno_type = 0
};


/*-----------------------------------------------------------------------*/

int dsi_init(int mode)
{
  (void)mode;
  STUB_ENTRY;
  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

dsi_hndl_t dsi_get_data_srvc_hndl
(
  dsi_net_ev_cb dsi_cb_fn,
  void * dsi_user_data
)
{
  (void)dsi_cb_fn; (void)dsi_user_data;
  STUB_ENTRY;

  qcril_data_stub_client.dsi_cb_fn = dsi_cb_fn;
  qcril_data_stub_client.dsi_user_data = dsi_user_data;
  qcril_data_stub_client.handle = next_handle++;

  STUB_EXIT;
  return qcril_data_stub_client.handle;
}

void dsi_rel_data_srvc_hndl(dsi_hndl_t hndl)
{
  (void)hndl;
  STUB_ENTRY;

  qcril_data_stub_client.dsi_cb_fn = NULL;
  qcril_data_stub_client.dsi_user_data = NULL;
  qcril_data_stub_client.handle = 0;

  STUB_EXIT;
  return;
}

int dsi_start_data_call(dsi_hndl_t hndl)
{
  (void)hndl;
  STUB_ENTRY;
  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

int dsi_stop_data_call(dsi_hndl_t hndl)
{
  (void)hndl;
  STUB_ENTRY;
  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

int dsi_set_data_call_param
(
  dsi_hndl_t hndl,
  dsi_call_param_identifier_t identifier,
  dsi_call_param_value_t *info
)
{
  (void)hndl; (void)identifier; (void)info;
  STUB_ENTRY;
  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

int dsi_get_device_name(dsi_hndl_t hndl, char * buf, int len)
{
  (void)hndl; (void)buf; (void)len;
  STUB_ENTRY;
  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

int dsi_get_call_end_reason
(
  dsi_hndl_t hndl,
  dsi_ce_reason_t * ce_reason,
  dsi_ip_family_t ipf
)
{
  (void)hndl; (void)ce_reason; (void)ipf;
  STUB_ENTRY;
  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

int dsi_get_ip_addr
(
  dsi_hndl_t hndl,
  dsi_addr_info_t * info_ptr,
  int len
)
{
  (void)hndl; (void)info_ptr; (void)len;
  STUB_ENTRY;
  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}


unsigned int dsi_get_ip_addr_count
(
  dsi_hndl_t hndl
)
{
  (void)hndl;
  STUB_ENTRY;
  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

int dsi_get_addr(dsi_hndl_t hndl, sockaddr_storage_t *addr_ptr)
{
  (void)hndl; (void)addr_ptr;
  STUB_ENTRY;
  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

int dsi_request_qos
(
  dsi_hndl_t              hndl,
  unsigned int            num_qos_specs,
  dsi_qos_spec_type      *qos_spec_list,
  dsi_qos_req_opcode_type req_opcode,
  dsi_qos_id_type        *qos_id_list,
  dsi_qos_err_rsp_type   *qos_spec_err_list
)
{
  dsi_evt_payload_t  payload;

  (void)hndl; (void)num_qos_specs; (void)qos_spec_list;
  (void)req_opcode; (void)qos_id_list; (void)qos_spec_err_list;
  STUB_ENTRY;

  if( -1 == qcril_data_client_info.handle )
  {
    /* Error case */
    fprintf( stderr, "STUB %s: return general error\n", __func__ );
    STUB_EXIT;
    return QCRIL_DATA_FAILURE;
  }
  else
  {
    /* Invoke registered callback */
    if( qcril_data_stub_client.dsi_cb_fn )
    {
      if( -2 == qcril_data_client_info.handle )
      {
        fprintf( stderr, "STUB %s: flow gone error\n", __func__ );
        payload.qos_info.flow_id = QOS_FLOW_ID;
        payload.qos_info.status_evt = DSI_QOS_GONE_EV;
      }
      else
      {
        /* Success case */
        payload.qos_info.flow_id = QOS_FLOW_ID;
        payload.qos_info.status_evt = DSI_QOS_ACTIVATED_EV;
        qos_id_list[0] = QOS_FLOW_ID;
      }

      /* Override dsi_user_data with qos_spec_list for validation */
      qcril_data_stub_client.dsi_cb_fn( hndl,
                                        (void*)qos_spec_list,
                                        DSI_EVT_QOS_STATUS_IND,
                                        &payload );
    }
  }

  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

int dsi_release_qos
(
  dsi_hndl_t              hndl,
  unsigned int            num_qos_ids,
  dsi_qos_id_type        *qos_id_list
)
{
  dsi_evt_payload_t  payload;

  (void)hndl; (void)num_qos_ids; (void)qos_id_list;
  STUB_ENTRY;

  if( -1 == qcril_data_client_info.handle )
  {
    /* Error case */
    fprintf( stderr, "STUB %s: return general error\n", __func__ );
    STUB_EXIT;
    return QCRIL_DATA_FAILURE;
  }
  else
  {
    /* Invoke registered callback */
    if( qcril_data_stub_client.dsi_cb_fn )
    {
      payload.qos_info.flow_id = qos_id_list[0];
      payload.qos_info.status_evt = DSI_QOS_GONE_EV;
      qcril_data_stub_client.dsi_cb_fn( qcril_data_stub_client.handle,
                                        qcril_data_stub_client.dsi_user_data,
                                        DSI_EVT_QOS_STATUS_IND,
                                        &payload );
    }
  }

  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

int dsi_modify_qos
(
  dsi_hndl_t              hndl,
  unsigned int            num_qos_specs,
  dsi_qos_spec_type      *qos_spec_list,
  dsi_qos_err_rsp_type   *qos_spec_err_list
)
{
  dsi_evt_payload_t  payload;

  (void)hndl; (void)num_qos_specs; (void)qos_spec_list; (void)qos_spec_err_list;
  STUB_ENTRY;

  if( -1 == qcril_data_client_info.handle )
  {
    /* Error case */
    fprintf( stderr, "STUB %s: return general error\n", __func__ );
    STUB_EXIT;
    return QCRIL_DATA_FAILURE;
  }
  else
  {
    /* Invoke registered callback */
    if( qcril_data_stub_client.dsi_cb_fn )
    {
      if( -2 == qcril_data_client_info.handle )
      {
        fprintf( stderr, "STUB %s: flow gone error\n", __func__ );
        payload.qos_info.flow_id = QOS_FLOW_ID;
        payload.qos_info.status_evt = DSI_QOS_GONE_EV;
      }
      else if( -3 == qcril_data_client_info.handle )
      {
        fprintf( stderr, "STUB %s: flow modify rejected\n", __func__ );
        payload.qos_info.flow_id = QOS_FLOW_ID;
        payload.qos_info.status_evt = DSI_QOS_MODIFY_REJECTED_EV;
      }
      else
      {
        /* Success case */
        fprintf( stderr, "STUB %s: flow modify accespted\n", __func__ );
        payload.qos_info.flow_id = QOS_FLOW_ID;
        payload.qos_info.status_evt = DSI_QOS_MODIFY_ACCEPTED_EV;
      }

      /* Override dsi_user_data with qos_spec_list for validation */
      qcril_data_stub_client.dsi_cb_fn( hndl,
                                        (void*)qos_spec_list,
                                        DSI_EVT_QOS_STATUS_IND,
                                        &payload );
    }
  }

  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

int dsi_suspend_qos
(
  dsi_hndl_t              hndl,
  unsigned int            num_qos_ids,
  dsi_qos_id_type        *qos_id_list
)
{
  dsi_evt_payload_t  payload;

  (void)hndl; (void)num_qos_ids; (void)qos_id_list;
  STUB_ENTRY;

  if( -1 == qcril_data_client_info.handle )
  {
    /* Error case */
    fprintf( stderr, "STUB %s: return general error\n", __func__ );
    STUB_EXIT;
    return QCRIL_DATA_FAILURE;
  }
  else
  {
    /* Invoke registered callback */
    if( qcril_data_stub_client.dsi_cb_fn )
    {
      /* Success case */
      payload.qos_info.flow_id = qos_id_list[0];
      payload.qos_info.status_evt = DSI_QOS_SUSPENDED_EV;
      qcril_data_stub_client.dsi_cb_fn( qcril_data_stub_client.handle,
                                        qcril_data_stub_client.dsi_user_data,
                                        DSI_EVT_QOS_STATUS_IND,
                                        &payload );
    }
  }

  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

int dsi_resume_qos
(
  dsi_hndl_t              hndl,
  unsigned int            num_qos_ids,
  dsi_qos_id_type        *qos_id_list
)
{
  dsi_evt_payload_t  payload;

  (void)hndl; (void)num_qos_ids; (void)qos_id_list;
  STUB_ENTRY;

  if( -1 == qcril_data_client_info.handle )
  {
    /* Error case */
    fprintf( stderr, "STUB %s: return general error\n", __func__ );
    STUB_EXIT;
    return QCRIL_DATA_FAILURE;
  }
  else
  {
    /* Invoke registered callback */
    if( qcril_data_stub_client.dsi_cb_fn )
    {
      if( -2 == qcril_data_client_info.handle )
      {
        fprintf( stderr, "STUB %s: flow gone error\n", __func__ );
        payload.qos_info.flow_id = qos_id_list[0];
        payload.qos_info.status_evt = DSI_QOS_GONE_EV;
      }
      else
      {
        /* Success case */
        payload.qos_info.flow_id = qos_id_list[0];
        payload.qos_info.status_evt = DSI_QOS_ACTIVATED_EV;
      }

      /* Override dsi_user_data with qos_spec_list for validation */
      qcril_data_stub_client.dsi_cb_fn( hndl,
                                        qcril_data_stub_client.dsi_user_data,
                                        DSI_EVT_QOS_STATUS_IND,
                                        &payload );
    }
  }

  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

int dsi_get_granted_qos
(
  dsi_hndl_t                 hndl,
  dsi_qos_id_type            qos_id,
  dsi_qos_granted_info_type *qos_info
)
{
  int ret = QCRIL_DATA_SUCCESS;
  (void)hndl; (void)qos_info;
  STUB_ENTRY;

  if( QOS_FLOW_ID == qos_id )
    memcpy( qos_info, &dummy_qos, sizeof(dsi_qos_granted_info_type) );
  else
    ret = QCRIL_DATA_FAILURE;

  STUB_EXIT;
  return ret;
}


int dsi_get_qos_status
(
  dsi_hndl_t           hndl,
  dsi_qos_id_type      qos_id,
  dsi_qos_status_type *qos_status
)
{
  (void)hndl; (void)qos_status;
  STUB_ENTRY;

  if( QOS_FLOW_ID == qos_id )
    *qos_status = QMI_QOS_STATUS_ACTIVATED;
  else
    *qos_status = QMI_QOS_STATUS_GONE;

  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

/*-----------------------------------------------------------------------*/

int qdp_profile_look_up
(
  const char  ** param_strings,    /* the order of params must match with the
                                      order specified in qdp_ril_param_idx_t */
  unsigned int * profile_id_3gpp,  /* value (not pointer it-self) must
                                      be set to zero by caller */
  qdp_profile_pdn_type *profile_pdn_type_3gpp,  /* 3gpp profile PDN type */
  unsigned int * profile_id_3gpp2, /* value must be set to zero by caller */
  qdp_profile_pdn_type *profile_pdn_type_3gpp2, /* 3gpp2 profile PDN type */
  qdp_error_info_t * error_info
)
{
  (void)param_strings; (void)profile_id_3gpp; (void)profile_id_3gpp2;
  (void)profile_pdn_type_3gpp; (void)profile_pdn_type_3gpp2;
  STUB_ENTRY;
  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

int qdp_profile_look_up_by_param
(
  const char  ** param_strings,    /* the order of params must match with the
                                      order specified in qdp_ril_param_idx_t */
  int            param_to_match,
  int            param_value,
  unsigned int * profile_id_3gpp,  /* value (not pointer it-self) must
                                      be set to zero by caller */
  qdp_profile_pdn_type *profile_pdn_type_3gpp,  /* 3gpp profile PDN type */
  unsigned int * profile_id_3gpp2, /* value must be set to zero by caller */
  qdp_profile_pdn_type *profile_pdn_type_3gpp2, /* 3gpp2 profile PDN type */
  qmi_wds_profile_params_type * p_params,  /* out placeholder */
  qdp_tech_t                *   tech_type,
  qdp_error_info_t * error_info
)
{
  (void)param_strings; (void)profile_id_3gpp; (void)profile_id_3gpp2;
  (void)profile_pdn_type_3gpp; (void)profile_pdn_type_3gpp2;
  STUB_ENTRY;
  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

int qdp_3gpp_profile_update_ex
(
  qmi_wds_profile_params_type * p_params,
  unsigned int                  profile_id,
  int                         * error_code
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}
int qdp_profile_release
(
  unsigned int profile_id
)
{
  (void)profile_id;
  STUB_ENTRY;
  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

int qdp_profile_release_ex
(
 unsigned int profile_id
)
{
  (void)profile_id;
  STUB_ENTRY;
  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

void qdp_deinit(void)
{
  STUB_ENTRY;
  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

/*-----------------------------------------------------------------------*/
int qmi_wds_client = 0;
int qmi_qos_client = 0;

int
qmi_init
(
  qmi_sys_event_rx_hdlr   event_rx_hdlr,
  void                    *event_user_data
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return QMI_NO_ERR;
}

int
qmi_release( int init_handle )
{
  (void)init_handle;
  STUB_ENTRY;
  STUB_EXIT;
  return QMI_NO_ERR;
}


int
qmi_dev_connection_init
(
  const char  *dev_id,
  int         *qmi_err_code
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return QMI_NO_ERR;
}

int
qmi_set_port_data_format
(
  const char                            *dev_id,
  qmi_data_format_qos_hdr_state_type    qos_hdr_state,
  qmi_link_layer_protocol_type          *link_protocol,
  int                                   *qmi_err_code
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return QMI_NO_ERR;
}


qmi_client_handle_type
qmi_wds_srvc_init_client
(
  const char                    *dev_id,
  qmi_wds_indication_hdlr_type  user_ind_msg_hdlr,
  void                          *user_ind_msg_hdlr_user_data,
  int                           *qmi_err_code
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return qmi_wds_client++;;
}

int
qmi_wds_srvc_release_client
(
  int      user_handle,
  int      *qmi_err_code
)
{
  STUB_ENTRY;
  STUB_EXIT;
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
  STUB_ENTRY;
  STUB_EXIT;
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
  STUB_ENTRY;
  call_settings->mask = (QMI_WDS_CURR_CALL_INFO_IP_FAMILY|
                         QMI_WDS_CURR_CALL_INFO_IPV4_ADDR);
  call_settings->ip_family = QMI_WDS_IP_FAMILY_PREF_IPV4;
  call_settings->ipv4_addr = 0x01020304;
  STUB_EXIT;
  return QMI_NO_ERR;
}

int
qmi_wds_get_pref_data_system
(
  int                         user_handle,
  qmi_wds_pref_data_sys_type  *cur_pref_sys,
  int                         *qmi_err_code
)
{
  STUB_ENTRY;
  STUB_EXIT;
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
  STUB_ENTRY;
  STUB_EXIT;
  return qmi_qos_client++;
}


int
qmi_qos_srvc_release_client
(
  qmi_client_handle_type  client_handle,
  int                     *qmi_err_code
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return QMI_NO_ERR;
}

int
qmi_qos_set_event_report_state
(
  int                               client_handle,
  qmi_qos_event_report_state_type   *report_state,
  int                               *qmi_err_code
)
{
  STUB_ENTRY;
  STUB_EXIT;
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
  STUB_ENTRY;

  // Fake CDMA profile
  granted_info->tx_granted_flow_data_is_valid = TRUE;
  granted_info->tx_granted_flow_data.qos_flow_granted.cdma_flow_desc.param_mask = QMI_QOS_CDMA_FLOW_PARAM_PROFILE_ID;
  granted_info->tx_granted_flow_data.qos_flow_granted.cdma_flow_desc.profile_id = 5;

  STUB_EXIT;
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
  STUB_ENTRY;

  // Fake CDMA profile
  granted_info->tx_granted_flow_data_is_valid = TRUE;
  granted_info->tx_granted_flow_data.qos_flow_granted.cdma_flow_desc.param_mask = QMI_QOS_CDMA_FLOW_PARAM_PROFILE_ID;
  granted_info->tx_granted_flow_data.qos_flow_granted.cdma_flow_desc.profile_id = 7;

  STUB_EXIT;
  return QMI_NO_ERR;
}

int
qmi_wds_utils_get_profile_list
(
  int                             user_handle,
  qmi_wds_profile_tech_type       *profile_tech,
  qmi_wds_profile_node_list_type  *profile_search_list,
  qmi_wds_profile_list_type       *profile_list,
  int                             *num_profile_list_elements,
  int                             *qmi_err_code
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return QMI_NO_ERR;
}

int
qmi_wds_utils_get_profile_list2
(
  int                              user_handle,
  qmi_wds_profile_tech_type        profile_tech,
  uint32                           param_mask,
  void                            *param_value,
  qmi_wds_profile_list_type       *profile_list,
  int                             *num_profile_list_elements,
  int                             *qmi_err_code
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return QMI_NO_ERR;
}


int
qmi_wds_set_event_report
(
  int                               user_handle,
  qmi_wds_event_report_params_type  *event_params,
  int                               *qmi_err_code
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return QMI_NO_ERR;
}

int
qmi_wds_go_dormant_req
(
  int   user_handle,
  int   *qmi_err_code
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return QMI_NO_ERR;
}

int
qmi_wds_get_dormancy_status
(
  int                       user_handle,
  qmi_wds_dorm_status_type  *dorm_status,
  int                       *qmi_err_code
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return QMI_NO_ERR;
}

int
qmi_wds_get_current_bearer_tech
(
  int                            user_handle,
  qmi_wds_data_bearer_tech_type  *rsp_data,
  int                            *qmi_err_code
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return QMI_NO_ERR;
}

/*-----------------------------------------------------------------------*/

int qmi_ril_is_feature_supported(int feature)
{
  STUB_ENTRY;
  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

qcril_instance_id_e_type qmi_ril_get_process_instance_id(void)
{
  STUB_ENTRY;
  STUB_EXIT;
  return (qcril_instance_id_e_type)1;
}

void qcril_send_unsol_response
(
  qcril_unsol_resp_params_type *param_ptr
)
{
  STUB_ENTRY;

  qcril_data_stub_client.resp_cb_fn( QCRIL_STUB_RESPONSE_UNS,
                                     (void*)param_ptr,
                                     qcril_data_stub_client.resp_user_data );

  STUB_EXIT;
  return;
}

void qcril_send_request_response
(
  qcril_request_resp_params_type *param_ptr
)
{
  STUB_ENTRY;

  qcril_data_stub_client.resp_cb_fn( QCRIL_STUB_RESPONSE_REQ,
                                     (void*)param_ptr,
                                     qcril_data_stub_client.resp_user_data );

  STUB_EXIT;
  return;
}


void qcril_format_log_msg
(
  char *buf_ptr,
  int buf_size,
  char *fmt,
  ...
)
{
  va_list arg_list;

  va_start(arg_list, fmt);
  (void)vsnprintf(buf_ptr, buf_size, fmt, arg_list);
  va_end(arg_list);

  return;
}

int qcril_log_get_token_id
(
  RIL_Token t
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

const char *qcril_log_lookup_event_name
(
  int event_id
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return NULL;
}

void qcril_log_msg_to_adb
(
  int  lvl,
  char *msg_ptr
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return;
}

void qcril_reqlist_default_entry
(
  RIL_Token t,
  int request,
  qcril_modem_id_e_type modem_id,
  qcril_req_state_e_type state,
  qcril_evt_e_type pending_event_id,
  qcril_reqlist_u_type *sub_ptr,
  qcril_reqlist_public_type *req_ptr
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return;
}


IxErrnoType qcril_reqlist_free
(
  qcril_instance_id_e_type instance_id,
  RIL_Token t
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return (IxErrnoType)0;
}

IxErrnoType qcril_reqlist_new
(
  qcril_instance_id_e_type instance_id,
  qcril_reqlist_public_type *entry_ptr
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return (IxErrnoType)0;
}

IxErrnoType qcril_reqlist_query
(
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  qcril_reqlist_public_type *info_ptr
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return (IxErrnoType)0;
}

void qcril_event_queue
(
  qcril_instance_id_e_type instance_id,
  qcril_modem_id_e_type modem_id,
  qcril_data_src_e_type data_src,
  qcril_evt_e_type event_id,
  void *data,
  size_t datalen,
  RIL_Token t
)
{
  STUB_ENTRY;

  qcril_data_event_hdlr( qcril_data_client_info.params_ptr,
                         qcril_data_client_info.response_ptr );

  STUB_EXIT;
  return;
}


void qcril_arb_set_pref_data_tech
(
  qcril_instance_id_e_type instance_id,
  qcril_arb_pref_data_tech_e_type pref_data_tech
)
{
  STUB_ENTRY;
  STUB_EXIT;
  return;
}

void qcril_default_request_resp_params
(
  qcril_instance_id_e_type instance_id,
  RIL_Token t,
  int request_id,
  RIL_Errno ril_err_no,
  qcril_request_resp_params_type *param_ptr
)
{
  STUB_ENTRY;

  param_ptr->instance_id = instance_id;
  param_ptr->t = t;
  param_ptr->request_id = request_id;
  param_ptr->ril_err_no = ril_err_no;
  param_ptr->resp_pkt = NULL;
  param_ptr->resp_len = 0;
  param_ptr->logstr = NULL;

  STUB_EXIT;
  return;
}

void qcril_default_unsol_resp_params
(
  qcril_instance_id_e_type instance_id,
  int response_id,
  qcril_unsol_resp_params_type *param_ptr
)
{
  STUB_ENTRY;

  param_ptr->instance_id = instance_id;
  param_ptr->response_id = response_id;
  param_ptr->resp_pkt = NULL;
  param_ptr->resp_len = 0;
  param_ptr->logstr = NULL;

  STUB_EXIT;
  return;
}


int qcril_dsds_ipc_send_func(int lvl,char *log_buffer)
{
  STUB_ENTRY;
  STUB_EXIT;
  return QCRIL_DATA_SUCCESS;
}

/*-----------------------------------------------------------------------*/

int property_get(const char *key, char *value, const char *default_value)
{
  STUB_ENTRY;
  STUB_EXIT;
  return 0;
}


/*-----------------------------------------------------------------------*/

int atexit(void (*function)(void))
{
  return 0;
}


void qcril_stubs_init( qcril_stub_response_cb resp_cb_fn, void* user_data )
{
  qcril_data_stub_client.resp_cb_fn = resp_cb_fn;
  qcril_data_stub_client.resp_user_data = user_data;
}

void qcril_stubs_release( void )
{
  qcril_data_stub_client.resp_cb_fn = NULL;
  qcril_data_stub_client.resp_user_data = NULL;
}

#endif /* QCRIL_DATA_OFFTARGET */
