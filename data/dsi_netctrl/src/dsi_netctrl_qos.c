/*!
  @file
  dsi_netctrl_qos.c

  @brief
  This file provides the functionality to manage Quality of Service
  elements on Modem via QMI interface.

*/

/*===========================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved

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

$Header:$

when       who     what, where, why
--------   ---     ----------------------------------------------------------
04/14/11   ar      Initial version

===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdint.h>
#ifdef DSI_NETCTRL_OFFTARGET
#include <string.h>
#endif
#include <netinet/in.h>

#include "assert.h"
#include "dsi_netctrli.h" /* declarations shared internally */
#include "dsi_netctrl_qos.h"

/*---------------------------------------------------------------------------
                           DECLARATIONS
---------------------------------------------------------------------------*/
#define DS_PRIMARY_FLOW_ID   (0)

/*===========================================================================
                          PUBLIC FUNCTIONS
===========================================================================*/

/*===========================================================================
  FUNCTION:  dsi_request_qos
===========================================================================*/
/*!
    @brief
    Used to request new QOS flow and filter on network connection.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
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
  dsi_store_t * st_hndl = NULL;
  int ret = DSI_ERROR;
  int qmi_ret = QMI_NO_ERR;
  int qmi_err_code = QMI_NO_ERR;

  DSI_LOG_ENTRY;

  st_hndl = ( (dsi_store_t *) hndl );

  /* this do..while loop decides the overall return value.
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    ret = DSI_ERROR;

    if (!dsi_inited)
    {
      DSI_LOG_ERROR("%s","dsi_request_qos: dsi not inited");
      break;
    }

    if (!(DSI_IS_HNDL_VALID(st_hndl)))
    {
      DSI_LOG_ERROR( "request_qos invalid arg, st_hndl [0x%x]",
                     (unsigned int*)st_hndl );
      break;
    }
    if (!(DSI_IS_QOS_HNDL_VALID(st_hndl->priv.dsi_iface_id)))
    {
      DSI_LOG_ERROR( "request_qos invalid qmi qos hndl [0x%x]",
                     (unsigned int)DSI_GET_QOS_HNDL(st_hndl->priv.dsi_iface_id) );
      break;
    }

    /* Send command request to QMI layer */
    qmi_ret = qmi_qos_request_qos( DSI_GET_QOS_HNDL(st_hndl->priv.dsi_iface_id),
                                   (int)num_qos_specs,
                                   qos_spec_list,
                                   req_opcode,
                                   qos_id_list,
                                   qos_spec_err_list,
                                   &qmi_err_code );
    if( QMI_NO_ERR != qmi_ret )
    {
      DSI_LOG_ERROR( "qmi_qos_request_qos failed with error [%d][%d]",
                     qmi_ret, qmi_err_code);
      break;
    }

    ret = DSI_SUCCESS;
  } while(0);

  DSI_LOG_EXIT;
  return ret;
} /* dsi_request_qos() */


/*===========================================================================
  FUNCTION:  dsi_release_qos
===========================================================================*/
/*!
    @brief
    Used to delete existing QOS flow and filters from network connection.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_release_qos
(
  dsi_hndl_t              hndl,
  unsigned int            num_qos_ids,
  dsi_qos_id_type        *qos_id_list
)
{
  dsi_store_t * st_hndl = NULL;
  int ret = DSI_ERROR;
  int qmi_ret = QMI_NO_ERR;
  int qmi_err_code = QMI_NO_ERR;

  DSI_LOG_ENTRY;

  st_hndl = ( (dsi_store_t *) hndl );

  /* this do..while loop decides the overall return value.
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    ret = DSI_ERROR;

    if (!dsi_inited)
    {
      DSI_LOG_ERROR("%s","dsi_release_qos: dsi not inited");
      break;
    }

    if (!(DSI_IS_HNDL_VALID(st_hndl)))
    {
      DSI_LOG_ERROR( "dsi_release_qos invalid arg, st_hndl [%p]",
                       (unsigned int*)st_hndl );
      break;
    }

    if (!(DSI_IS_QOS_HNDL_VALID(st_hndl->priv.dsi_iface_id)))
    {
      DSI_LOG_ERROR( "dsi_release_qos invalid qmi qos hndl [0x%x]",
                     (unsigned int)DSI_GET_QOS_HNDL(st_hndl->priv.dsi_iface_id) );
      break;
    }

    /* Send command request to QMI layer */
    qmi_ret = qmi_qos_release_qos( DSI_GET_QOS_HNDL(st_hndl->priv.dsi_iface_id),
                                   (int)num_qos_ids,
                                   qos_id_list,
                                   &qmi_err_code );
    if( QMI_NO_ERR != qmi_ret )
    {
      DSI_LOG_ERROR( "qmi_qos_release_qos failed with error [%d][%d]",
                     qmi_ret, qmi_err_code);
      break;
    }

    ret = DSI_SUCCESS;
  } while(0);

  DSI_LOG_EXIT;
  return ret;
} /* dsi_release_qos() */


/*===========================================================================
  FUNCTION:  dsi_modify_qos
===========================================================================*/
/*!
    @brief
    Used to change existing QOS flows on network connection.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_modify_qos
(
  dsi_hndl_t              hndl,
  unsigned int            num_qos_specs,
  dsi_qos_spec_type      *qos_spec_list,
  dsi_qos_err_rsp_type   *qos_spec_err_list
)
{
  dsi_store_t * st_hndl = NULL;
  int ret = DSI_ERROR;
  int qmi_ret = QMI_NO_ERR;
  int qmi_err_code = QMI_NO_ERR;

  DSI_LOG_ENTRY;

  st_hndl = ( (dsi_store_t *) hndl );

  /* this do..while loop decides the overall return value.
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    ret = DSI_ERROR;

    if (!dsi_inited)
    {
      DSI_LOG_ERROR("%s","dsi_modify_qos: dsi not inited");
      break;
    }

    if (!(DSI_IS_HNDL_VALID(st_hndl)))
    {
      DSI_LOG_ERROR( "dsi_modify_qos invalid arg, st_hndl [%p]",
                       (unsigned int*)st_hndl );
      break;
    }

    if (!(DSI_IS_QOS_HNDL_VALID(st_hndl->priv.dsi_iface_id)))
    {
      DSI_LOG_ERROR( "dsi_modify_qos invalid qmi qos hndl [0x%x]",
                     (unsigned int)DSI_GET_QOS_HNDL(st_hndl->priv.dsi_iface_id) );
      break;
    }

    /* Check for primary flow spec */
    if( DS_PRIMARY_FLOW_ID == qos_spec_list->qos_identifier )
    {
      qmi_ret = qmi_qos_modify_primary_qos( DSI_GET_QOS_HNDL(st_hndl->priv.dsi_iface_id),
                                            qos_spec_list,
                                            qos_spec_err_list,
                                            &qmi_err_code );
    }
    else
    {
      qmi_ret = qmi_qos_modify_secondary_qos( DSI_GET_QOS_HNDL(st_hndl->priv.dsi_iface_id),
                                              (int)num_qos_specs,
                                              qos_spec_list,
                                              qos_spec_err_list,
                                              &qmi_err_code );
    }

    if( QMI_NO_ERR != qmi_ret )
    {
      DSI_LOG_ERROR( "qmi_qos_modify_qos failed with error [%d][%d]",
                     qmi_ret, qmi_err_code);
      break;
    }

    ret = DSI_SUCCESS;
  } while(0);

  DSI_LOG_EXIT;
  return ret;
} /* dsi_modify_qos() */


/*===========================================================================
  FUNCTION:  dsi_suspend_qos
===========================================================================*/
/*!
    @brief
    Used to disable prioritized packet handling for existing QOS flows
    on network connection.  Further packet transmission on those QOS
    flows is treated as best-effort traffic.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_suspend_qos
(
  dsi_hndl_t              hndl,
  unsigned int            num_qos_ids,
  dsi_qos_id_type        *qos_id_list
)
{
  dsi_store_t * st_hndl = NULL;
  int ret = DSI_ERROR;
  int qmi_ret = QMI_NO_ERR;
  int qmi_err_code = QMI_NO_ERR;

  DSI_LOG_ENTRY;

  st_hndl = ( (dsi_store_t *) hndl );

  /* this do..while loop decides the overall return value.
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    ret = DSI_ERROR;

    if (!dsi_inited)
    {
      DSI_LOG_ERROR("%s","dsi_suspend_qos: dsi not inited");
      break;
    }

    if (!(DSI_IS_HNDL_VALID(st_hndl)))
    {
      DSI_LOG_ERROR( "dsi_suspend_qos invalid arg, st_hndl [%p]",
                       (unsigned int*)st_hndl );
      break;
    }

    if (!(DSI_IS_QOS_HNDL_VALID(st_hndl->priv.dsi_iface_id)))
    {
      DSI_LOG_ERROR( "dsi_suspend_qos invalid qmi qos hndl [0x%x]",
                     (unsigned int)DSI_GET_QOS_HNDL(st_hndl->priv.dsi_iface_id) );
      break;
    }

    /* Send command request to QMI layer */
    qmi_ret = qmi_qos_suspend_qos( DSI_GET_QOS_HNDL(st_hndl->priv.dsi_iface_id),
                                   (unsigned char)num_qos_ids,
                                   qos_id_list,
                                   &qmi_err_code );
    if( QMI_NO_ERR != qmi_ret )
    {
      DSI_LOG_ERROR( "qmi_qos_suspend_qos failed with error [%d][%d]",
                     qmi_ret, qmi_err_code);
      break;
    }

    ret = DSI_SUCCESS;
  } while(0);

  DSI_LOG_EXIT;
  return ret;
} /* dsi_suspend_qos() */


/*===========================================================================
  FUNCTION:  dsi_resume_qos
===========================================================================*/
/*!
    @brief
    Used to enable prioritized packet handling for existing QOS flows
    on network connection.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_resume_qos
(
  dsi_hndl_t              hndl,
  unsigned int            num_qos_ids,
  dsi_qos_id_type        *qos_id_list
)
{
  dsi_store_t * st_hndl = NULL;
  int ret = DSI_ERROR;
  int qmi_ret = QMI_NO_ERR;
  int qmi_err_code = QMI_NO_ERR;

  DSI_LOG_ENTRY;

  st_hndl = ( (dsi_store_t *) hndl );

  /* this do..while loop decides the overall return value.
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    ret = DSI_ERROR;

    if (!dsi_inited)
    {
      DSI_LOG_ERROR("%s","dsi_resume_qos: dsi not inited");
      break;
    }

    if (!(DSI_IS_HNDL_VALID(st_hndl)))
    {
      DSI_LOG_ERROR( "dsi_resume_qos invalid arg, st_hndl [%p]",
                       (unsigned int*)st_hndl );
      break;
    }

    if (!(DSI_IS_QOS_HNDL_VALID(st_hndl->priv.dsi_iface_id)))
    {
      DSI_LOG_ERROR( "dsi_resume_qos invalid qmi qos hndl [0x%x]",
                     (unsigned int)DSI_GET_QOS_HNDL(st_hndl->priv.dsi_iface_id) );
      break;
    }

    /* Send command request to QMI layer */
    qmi_ret = qmi_qos_resume_qos( DSI_GET_QOS_HNDL(st_hndl->priv.dsi_iface_id),
                                  (unsigned char)num_qos_ids,
                                  qos_id_list,
                                  &qmi_err_code );
    if( QMI_NO_ERR != qmi_ret )
    {
      DSI_LOG_ERROR( "qmi_qos_resume_qos failed with error [%d][%d]",
                     qmi_ret, qmi_err_code);
      break;
    }

    ret = DSI_SUCCESS;
  } while(0);

  DSI_LOG_EXIT;
  return ret;
} /* dsi_resume_qos() */


/*===========================================================================
  FUNCTION:  dsi_get_granted_qos
===========================================================================*/
/*!
    @brief
    Used to query the QOS information for an existing flow on network
    connection.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_get_granted_qos
(
  dsi_hndl_t                 hndl,
  dsi_qos_id_type            qos_id,
  int                        ip_family,
  dsi_qos_granted_info_type *qos_info
)
{
  dsi_store_t * st_hndl = NULL;
  int ret = DSI_ERROR, i = 0;
  int qmi_ret = QMI_NO_ERR;
  int qmi_err_code = QMI_NO_ERR;
  qmi_ip_family_pref_type qmi_ip_pref = QMI_IP_FAMILY_PREF_UNSPECIFIED;

  DSI_LOG_ENTRY;

  st_hndl = ( (dsi_store_t *) hndl );

  /* this do..while loop decides the overall return value.
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    ret = DSI_ERROR;

    if (!dsi_inited)
    {
      DSI_LOG_ERROR("%s","dsi_get_granted_qos: dsi not inited");
      break;
    }

    if (!(DSI_IS_HNDL_VALID(st_hndl)))
    {
      DSI_LOG_ERROR( "dsi_get_granted_qos invalid arg, st_hndl [%p]",
                       (unsigned int*)st_hndl );
      break;
    }

    if (!(DSI_IS_QOS_HNDL_VALID(st_hndl->priv.dsi_iface_id)))
    {
      DSI_LOG_ERROR( "dsi_get_granted_qos invalid qmi qos hndl [0x%x]",
                     (unsigned int)DSI_GET_QOS_HNDL(st_hndl->priv.dsi_iface_id) );
      break;
    }

    switch(ip_family)
    {
      case AF_INET:
      {
        qmi_ip_pref = QMI_IP_FAMILY_PREF_IPV4;
        break;
      }
      case AF_INET6:
      {
        qmi_ip_pref = QMI_IP_FAMILY_PREF_IPV6;
        break;
      }
      default:
      {
        qmi_ip_pref = QMI_IP_FAMILY_PREF_IPV4;
        break;
      }
    }
    /* Set ip_family_pref for the qos handle */
    qmi_ret = qmi_qos_set_client_ip_pref(DSI_GET_QOS_HNDL(st_hndl->priv.dsi_iface_id),
                                         qmi_ip_pref,
                                         &qmi_err_code);
    if( QMI_NO_ERR != qmi_ret )
    {
      DSI_LOG_ERROR( "qmi_qos_set_client_ip_pref failed with error [%d][%d]",
                     qmi_ret, qmi_err_code);
      break;
    }

    /* Send command request to QMI layer */
    if( DS_PRIMARY_FLOW_ID == qos_id )
    {
      qmi_ret =
        qmi_qos_get_primary_granted_qos_info( DSI_GET_QOS_HNDL(st_hndl->priv.dsi_iface_id),
                                              qos_id,
                                              qos_info,
                                              &qmi_err_code );
    }
    else
    {
      qmi_ret =
        qmi_qos_get_secondary_granted_qos_info( DSI_GET_QOS_HNDL(st_hndl->priv.dsi_iface_id),
                                                qos_id,
                                                qos_info,
                                                &qmi_err_code );
    }

    if( QMI_NO_ERR != qmi_ret )
    {
      DSI_LOG_ERROR( "qmi_qos_get_granted_qos_info failed with error [%d][%d]",
                     qmi_ret, qmi_err_code);
      break;
    }

    ret = DSI_SUCCESS;
  } while(0);

  DSI_LOG_EXIT;
  return ret;
} /* dsi_get_granted_qos() */


/*===========================================================================
  FUNCTION:  dsi_get_qos_status
===========================================================================*/
/*!
    @brief
    Used to query the QOS activated/suspended/gone state for an
    existing flow on network connection.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_get_qos_status
(
  dsi_hndl_t           hndl,
  dsi_qos_id_type      qos_id,
  dsi_qos_status_type *qos_status
)
{
  dsi_store_t * st_hndl = NULL;
  int ret = DSI_ERROR, i = 0;
  int qmi_ret = QMI_NO_ERR;
  int qmi_err_code = QMI_NO_ERR;

  DSI_LOG_ENTRY;

  st_hndl = ( (dsi_store_t *) hndl );

  /* this do..while loop decides the overall return value.
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    ret = DSI_ERROR;

    if (!dsi_inited)
    {
      DSI_LOG_ERROR("%s","dsi_get_qos_status: dsi not inited");
      break;
    }

    if (!(DSI_IS_HNDL_VALID(st_hndl)))
    {
      DSI_LOG_ERROR( "dsi_get_qos_status invalid arg, st_hndl [%p]",
                       (unsigned int*)st_hndl );
      break;
    }

    if (!(DSI_IS_QOS_HNDL_VALID(st_hndl->priv.dsi_iface_id)))
    {
      DSI_LOG_ERROR( "dsi_get_qos_status invalid qmi qos hndl [0x%x]",
                     (unsigned int)DSI_GET_QOS_HNDL(st_hndl->priv.dsi_iface_id) );
      break;
    }

    /* Send command request to QMI layer */
    qmi_ret = qmi_qos_get_status( DSI_GET_QOS_HNDL(st_hndl->priv.dsi_iface_id),
                                  qos_id,
                                  qos_status,
                                  &qmi_err_code );
    if( QMI_NO_ERR != qmi_ret )
    {
      DSI_LOG_ERROR( "qmi_qos_get_status failed with error [%d][%d]",
                     qmi_ret, qmi_err_code);
      break;
    }

    ret = DSI_SUCCESS;
  } while(0);

  DSI_LOG_EXIT;
  return ret;
} /* dsi_get_qos_status() */


/*===========================================================================
  FUNCTION:  dsi_handle_qos_status_rpt_ind
===========================================================================*/
/*!
    @brief
    Handles packet service indications.  This routine is invoked within
    the DSI_NetCtrl command processing thread context.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
static int dsi_handle_qos_status_rpt_ind
(
  int qos_hndl,
  qmi_service_id_type sid,
  void * user_data,
  qmi_qos_indication_data_type * ind_data
)
{
  dsi_store_t * st = NULL;
  dsi_evt_payload_t payload;
  int ret = DSI_ERROR;
  int reti = DSI_SUCCESS;
  int i = (int)(intptr_t)user_data;

  DSI_LOG_ENTRY;

  do
  {
    DSI_LOG_DEBUG("received status_report with qos_hndl [0x%08x] "
                  "sid [%d] user_data [%p] ind_data [%p]",
                  qos_hndl, sid, user_data, ind_data);

    if (NULL == ind_data)
    {
      DSI_LOG_ERROR("%s", "NULL ind data received in qos_ind_cb");
      break;
    }

    if (!DSI_IS_ID_VALID(i))
    {
      DSI_LOG_ERROR("invalid user data (dsi iface id) "\
                    "[%d] received in qos_ind_cb", i);
      break;
    }

    st = (dsi_store_t *)dsi_store_table[i].dsi_store_ptr;
    if( NULL == st )
    {
      DSI_LOG_INFO("No call state for index [%d]", i);
      break;
    }
    if( NULL == st->net_ev_cb )
    {
      DSI_LOG_ERROR("No event callback for index [%d]", i);
      break;
    }

    memset( &payload, 0x0, sizeof(payload) );
    payload.qos_info.flow_id =
      ind_data->status_report.qos_status_information.qos_identifier;
    payload.qos_info.reason_code =
      (ind_data->status_report.qos_reason_is_valid)?
      ind_data->status_report.qos_reason : 0;

    switch( ind_data->status_report.qos_status_information.qos_event )
    {
      case QMI_QOS_ACTIVATED_EV:
        DSI_LOG_DEBUG( "qos flow [0x%08lu] activated on iface [%d] reason[0x%x]",
                       payload.qos_info.flow_id, st->priv.dsi_iface_id,
                       payload.qos_info.reason_code );
        /* Client notified in dsi_process_netmgr_ev() for network-initiated support */
        break;

      case QMI_QOS_GONE_EV:
        DSI_LOG_DEBUG( "qos flow [0x%08lu] released on iface [%d] reason[0x%x]",
                       payload.qos_info.flow_id, st->priv.dsi_iface_id,
                       payload.qos_info.reason_code );

        /* post indication to client */
        payload.qos_info.status_evt = DSI_QOS_GONE_EV;
        st->net_ev_cb( (dsi_hndl_t *)st,
                       st->user_data,
                       DSI_EVT_QOS_STATUS_IND,
                       &payload );
        break;

      case QMI_QOS_SUSPENDED_EV:
        DSI_LOG_DEBUG( "qos flow [0x%08lu] suspended on iface [%d] reason[0x%x]",
                       payload.qos_info.flow_id, st->priv.dsi_iface_id,
                       payload.qos_info.reason_code );

        /* post indication to client */
        payload.qos_info.status_evt = DSI_QOS_SUSPENDED_EV;
        st->net_ev_cb( (dsi_hndl_t *)st,
                       st->user_data,
                       DSI_EVT_QOS_STATUS_IND,
                       &payload );
        break;

      case QMI_QOS_MODIFY_ACCEPTED_EV:
        DSI_LOG_DEBUG( "qos flow [0x%08lu] modify accepted on iface [%d] reason[0x%x]",
                       payload.qos_info.flow_id, st->priv.dsi_iface_id,
                       payload.qos_info.reason_code );

        /* post indication to client */
        payload.qos_info.status_evt = DSI_QOS_MODIFY_ACCEPTED_EV;
        st->net_ev_cb( (dsi_hndl_t *)st,
                       st->user_data,
                       DSI_EVT_QOS_STATUS_IND,
                       &payload );
        break;

      case QMI_QOS_MODIFY_REJECTED_EV:
        DSI_LOG_DEBUG( "qos flow [0x%08lu] modify rejected on iface [%d] reason[0x%x]",
                       payload.qos_info.flow_id, st->priv.dsi_iface_id,
                       payload.qos_info.reason_code );

        /* post indication to client */
        payload.qos_info.status_evt = DSI_QOS_MODIFY_REJECTED_EV;
        st->net_ev_cb( (dsi_hndl_t *)st,
                       st->user_data,
                       DSI_EVT_QOS_STATUS_IND,
                       &payload );
        break;

      case QMI_QOS_INFO_CODE_UPDATED_EV:
        DSI_LOG_DEBUG( "qos flow [0x%08lu] info code update on iface [%d] reason[0x%x]",
                       payload.qos_info.flow_id, st->priv.dsi_iface_id,
                       payload.qos_info.reason_code );

        switch( ind_data->status_report.qos_status_information.qos_status )
        {
          case QMI_QOS_STATUS_ACTIVATED:
            payload.qos_info.status_evt = DSI_QOS_ACTIVATED_EV;
            break;
          case QMI_QOS_STATUS_GONE:
            payload.qos_info.status_evt = DSI_QOS_GONE_EV;
            break;
          case QMI_QOS_STATUS_SUSPENDED:
            payload.qos_info.status_evt = DSI_QOS_SUSPENDED_EV;
            break;
          default:
            DSI_LOG_ERROR( "unsupported qos status [%d]",
                           ind_data->status_report.qos_status_information.qos_status );
            reti = DSI_ERROR;
            break;
        }

        if( DSI_SUCCESS == reti )
        {
          /* post indication to client */
          st->net_ev_cb( (dsi_hndl_t *)st,
                         st->user_data,
                         DSI_EVT_QOS_STATUS_IND,
                         &payload );
        }
        break;

      default:
        DSI_LOG_ERROR( "unsupported qos event [%d]",
                       ind_data->status_report.qos_status_information.qos_event );
        reti = DSI_ERROR;
        break;
    }
    if( DSI_ERROR == reti )
    {
      break;
    }
    ret = DSI_SUCCESS;
  } while (0);

  DSI_LOG_EXIT;
  return ret;
}


/*===========================================================================
  FUNCTION:  dsi_process_qos_ind
===========================================================================*/
/*!
    @brief
    callback function registered for qos indications

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
void dsi_process_qos_ind
(
  int qos_hndl,
  qmi_service_id_type sid,
  void * user_data,
  qmi_qos_indication_id_type ind_id,
  qmi_qos_indication_data_type * ind_data
)
{
  int ret = DSI_ERROR;
  int reti = DSI_SUCCESS;

  DSI_LOG_ENTRY;
  DSI_GLOBAL_LOCK;

  do
  {
    DSI_LOG_VERBOSE("received ind_id [%d] with qos_hndl [0x%08x] "
                    "sid [%d] user_data [%p] ind_data [%p]",
                    ind_id, qos_hndl, sid, user_data, ind_data);

    switch(ind_id)
    {
    case QMI_QOS_SRVC_STATUS_REPORT_IND_MSG:
      reti = dsi_handle_qos_status_rpt_ind( qos_hndl,
                                            sid,
                                            user_data,
                                            ind_data );
      break;

    default:
      DSI_LOG_VERBOSE("dsi_process_qos_ind: IGNORE: do not handle [%d] "
                      "indication as of now", ind_id);
      /* this is not an error, we just ignore this indication*/
      break;
    }
    if( DSI_ERROR == reti )
    {
      break;
    }
    ret = DSI_SUCCESS;
  } while (0);

  DSI_GLOBAL_UNLOCK;
  DSI_LOG_EXIT;
}
