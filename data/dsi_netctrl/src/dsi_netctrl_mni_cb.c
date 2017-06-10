/*!
  @file
  dsi_netctrl_mni_cb.c

  @brief
  implements dsi_netctrl callback processing functions

*/

/*===========================================================================

  Copyright (c) 2010-2014 Qualcomm Technologies, Inc. All Rights Reserved

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

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header:  $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
04/19/10   js      created

===========================================================================*/
#include "dsi_netctrli.h"
#include "qmi_wds_srvc.h"
#include "ds_cmdq.h"
#include "qdi.h"
#include "dsi_netctrl.h"
#include "dsi_netctrl_mni.h"
#include <stdint.h>

/*===========================================================================
              FUNCTIONS EXECUTED IN dsi_netctrl_cb THREAD
===========================================================================*/
/*===========================================================================
  FUNCTION:  dsi_process_stop_nw_async_rsp
===========================================================================*/
/*!
    @brief
    processes asynchronous response to the stop_nw qmi wds request
    i is dsi iface id
    st is the store handle pointer
    sys_err_code is the sys_err_code we got from qmi wds

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
static int dsi_process_stop_nw_async_rsp
(
  dsi_store_t * st,
  int i,
  int sys_err_code,
  int qmi_err_code,
  qdi_wds_async_rsp_data_type * rsp_data
)
{
  int ret = DSI_ERROR;
  int port_incall = FALSE;
  qmi_wds_link_status_type link_state_v4 = QMI_WDS_PACKET_DATA_DISCONNECTED;
  qmi_wds_link_status_type link_state_v6 = QMI_WDS_PACKET_DATA_DISCONNECTED;
  dsi_ce_reason_t ce_reason;
  boolean send_no_net = FALSE;
  boolean release_iface = FALSE;
  boolean handle_found = FALSE;
  int j;
  int ip_family = QMI_IP_FAMILY_PREF_IPV4;
  dsi_ip_family_t ipf = DSI_IP_FAMILY_V4;

  DSI_LOG_DEBUG( "%s", "dsi_process_stop_nw_async_rsp: ENTRY" );

  do
  {
    if (NULL == st)
    {
      DSI_LOG_ERROR("%s","NULL st received");
      break;
    }

    if(NULL == rsp_data)
    {
      DSI_LOG_ERROR("%s","received NULL rsp_data");
      break;
    }

    if (rsp_data->flags == QDI_RSP_DATA_V4_V6)
    {
      ip_family = QMI_IP_FAMILY_PREF_UNSPECIFIED;
      ipf = DSI_NUM_IP_FAMILIES;
    }
    else if (rsp_data->flags == QDI_RSP_DATA_V4)
    {
      ip_family = QMI_IP_FAMILY_PREF_IPV4;
      ipf = DSI_IP_FAMILY_V4;
    }
    else
    {
      ip_family = QMI_IP_FAMILY_PREF_IPV6;
      ipf = DSI_IP_FAMILY_V6;
    }
    DSI_LOG_DEBUG(  "stop_nw_async_rsp: rsp_data->flags:%d, ip_family:%d, ipf:%d ",
                    rsp_data->flags,ip_family,ipf );
    /* check if txn was completed successfully */
    if(QMI_NO_ERR == sys_err_code)
    {
      DSI_LOG_DEBUG("%s", "qmi txn " \
                    "QMI_WDS_SRVC_STOP_NW_ASYNC_RSP_MSG successful");

      /* Need to check for data call active on QMI port in another
       * process domain.  In such cases, the Modem will not send
       * WDS_PKT_SRVC_IND.  Need to locally generate call end
       * notification to clients as there will be no PLATFORM_DOWN
       * indication forthcoming. */
      if ( (DSI_IS_V4_CALL_DOWN(ipf, i) ||
            DSI_IS_V6_CALL_DOWN(ipf, i) ||
            DSI_IS_V4_V6_CALL_DOWN(ipf, i)
           ) &&
           DSI_SUCCESS == dsi_is_qmiport_incall
                          ( i, &port_incall,&link_state_v4,&link_state_v6 ) &&
           TRUE == port_incall )
      {


        if ( (DSI_IS_V4_CALL_DOWN(ipf, i) &&
               (link_state_v4 == QMI_WDS_PACKET_DATA_CONNECTED)) ||
             (DSI_IS_V6_CALL_DOWN(ipf, i) &&
               (link_state_v6 == QMI_WDS_PACKET_DATA_CONNECTED)) ||
             (DSI_IS_V4_V6_CALL_DOWN(ipf, i) &&
               ( (link_state_v4 == QMI_WDS_PACKET_DATA_CONNECTED) ||
                 (link_state_v6 == QMI_WDS_PACKET_DATA_CONNECTED)
               )))
        {
          send_no_net = TRUE;
          release_iface = TRUE;
        }
        else
        {
          send_no_net = FALSE;
          release_iface = FALSE;
        }
        DSI_LOG_DEBUG( " [%d] iface [%d] ip_family with [%d/%d]" \
                       "link_state_v4/link_state_v6 send_no_net:%d",
                      i,ip_family,link_state_v4,link_state_v6,send_no_net);
      }

      /* NOTE: if call was not active on some other qmi port, only then
       * we expect pkt_srvc_ind DISCONNECTED */
    }
    else if(QMI_SERVICE_ERR == sys_err_code &&
            QMI_SERVICE_ERR_NO_EFFECT == qmi_err_code)
    {
      /* we need to generate call end if we receive NO_EFFECT */
      send_no_net = TRUE;
      release_iface = TRUE;
    }
    else
    {
      /* we received sys err code that is not handled right now */
      DSI_LOG_DEBUG("%s", "qmi txn " \
                    "QMI_WDS_SRVC_STOP_NW_ASYNC_RSP_MSG unsuccessful");
      break;
    }

    if (TRUE == send_no_net)
    {
      /* Send the event to all clients on this interface */
      for (j = 0; j < DSI_MAX_DATA_CALLS; j++)
      {
        dsi_store_t *hndl = (dsi_store_t *)dsi_store_table[j].dsi_store_ptr;
        if ((hndl != NULL &&
            i == hndl->priv.dsi_iface_id) &&
           ((int)hndl->priv.start_nw_params.ip_family_pref == ip_family))
        {
          /* notify user that network is down (even though it's not) */
          if( NULL != hndl->net_ev_cb )
          {
            DSI_LOG_DEBUG( "notifying user of st_hndl [%p] of event NET_NO_NET for [%d] ip_family",
                           hndl,ip_family );
            hndl->net_ev_cb( (dsi_hndl_t *)hndl,
                           hndl->user_data,
                           DSI_EVT_NET_NO_NET,
                           NULL );
          }
          else
          {
            DSI_LOG_ERROR( "callback on store pointer [%p] is NULL", hndl );
          }
        }
      }
    }

    /* release the interface is call is already down and there
     * are no further pkt_srvc_ind expected on it */
    if (TRUE == release_iface)
    {
      /* detach this interface from the client store handle */
      DSI_LOG_DEBUG( "st_hndl [%p] detached from interface [%d]",
                     st, i );
      DSI_LOCK_MUTEX( &(st->priv.mutex) );
      dsi_detach_dsi_iface( st );
      DSI_UNLOCK_MUTEX( &(st->priv.mutex) );

      /* we determine ce reason from QMI
       * set reason_code to UNKNOWN here */
      ce_reason.reason_code = DSI_CE_REASON_UNKNOWN;
      ce_reason.reason_type = DSI_CE_TYPE_UNINIT;

      /* set this interface free... */
      dsi_release_dsi_iface( i, &ce_reason ,ipf);

      /* Release the IFACE clients only if this store does not have any handle sharing the IFACE */
      if(send_no_net == TRUE)
      {
        for (j = 0; j < DSI_MAX_DATA_CALLS; j++)
        {
          dsi_store_t *hndl = (dsi_store_t *)dsi_store_table[j].dsi_store_ptr;
          if ((hndl != NULL && i == hndl->priv.dsi_iface_id) )
          {
            handle_found = TRUE;
            break;
          }
        }
        if(!handle_found)
        {

/* Since DUAL IP call has two txn's, mni_client needs to be released only
   after async response for both families is complete.  */

          if( (DSI_IP_VERSION_4_6 != st->priv.ip_version) ||
              ( (DSI_GET_WDS_V4_TXN(i) == DSI_INVALID_WDS_TXN) &&
                (DSI_GET_WDS_V6_TXN(i) == DSI_INVALID_WDS_TXN) ) )
          {
            dsi_mni_release_client(i);
          }
        }
      }
    }

    ret = DSI_SUCCESS;
  } while (0);

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_DEBUG( "%s", "dsi_process_stop_nw_async_rsp: " \
                   "EXIT with suc" );
  }
  else
  {
    DSI_LOG_DEBUG( "%s", "dsi_process_stop_nw_async_rsp: " \
                   "EXIT with err" );
  }

  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_query_if_addr_from_qdi
===========================================================================*/
/*!
    @brief
    Queries QDI to obtain the interface addresses (IP, DNS, Gateway)

    @param
    iface_id   - dsi iface id
    ip_version - the IP version of the call (V4, V6 or V4V6)

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
static int dsi_query_if_addr_from_qdi
(
  int iface_id,
  int ip_version
)
{
  int ret = DSI_SUCCESS;
  dsi_store_t *st = NULL;
  int i;
  boolean client_found = FALSE;
  qmi_wds_iface_name_type tech_name = QMI_WDS_IFACE_NAME_NOT_REPORTED;

  if (!DSI_IS_ID_VALID(iface_id))
  {
    DSI_LOG_ERROR("dsi_query_if_addr_from_qdi: invalid iface_id=%d", iface_id);
    return DSI_ERROR;
  }

  /* Check for any valid client for the iface */
  for (i = 0; i < DSI_MAX_DATA_CALLS; i++)
  {
    st = dsi_store_table[i].dsi_store_ptr;

    if ((st != NULL && iface_id == st->priv.dsi_iface_id) &&
        ((st->priv.ip_version == DSI_IP_VERSION_4_6) ||
         ((st->priv.ip_version == DSI_IP_VERSION_4) && (ip_version == DSI_IP_VERSION_4)) ||
         ((st->priv.ip_version == DSI_IP_VERSION_6) && (ip_version == DSI_IP_VERSION_6))  )
       )
    {
      client_found = TRUE;
      break;
    }
  }
  if (TRUE == client_found)
  {
    DSI_LOG_VERBOSE("dsi_query_if_addr_from_qdi: valid client=0x%p found at "
                    "store index=%d", st, i);
    tech_name = st->priv.call_tech;
  }
  else
  {
    DSI_LOG_VERBOSE("%s", "dsi_query_if_addr_from_qdi: valid client not found");
    return DSI_ERROR;
  }

  /* Since we won't be getting any platform events from netmgr, query for
     all the interface addresses directly from QDI */
  switch (ip_version)
  {
    case DSI_IP_VERSION_4:
      DSI_LOG_VERBOSE("%s","IPv4 call - querying addresses from QDI");
      if (QDI_SUCCESS != qdi_get_addr_info(DSI_GET_QDI_HNDL(iface_id),
                                           DSI_GET_DEV_STR(iface_id),
                                           AF_INET,
                                           &DSI_GET_ADDRINFO(iface_id, DSI_IP_FAMILY_V4),
                                           tech_name))
      {
        DSI_LOG_ERROR("%s","failed to get IPv4 addr info from QDI");
        ret = DSI_ERROR;
      }
      else
      {
        DSI_LOCK_MUTEX(DSI_GET_MUTEX(iface_id));
        DSI_INCR_COUNT(iface_id,DSI_IP_VERSION_4);
        /* let go the lock before calling user cb */
        DSI_UNLOCK_MUTEX(DSI_GET_MUTEX(iface_id));
      }
      break;

    case DSI_IP_VERSION_6:
      DSI_LOG_VERBOSE("%s","IPv6 call - querying addresses from QDI");
      if (QDI_SUCCESS != qdi_get_addr_info(DSI_GET_QDI_HNDL(iface_id),
                                           DSI_GET_DEV_STR(iface_id),
                                           AF_INET6,
                                           &DSI_GET_ADDRINFO(iface_id, DSI_IP_FAMILY_V6),
                                           tech_name))
      {
        DSI_LOG_ERROR("%s","failed to get IPv6 addr info from QDI");
        ret = DSI_ERROR;
      }
      else
      {
        DSI_LOCK_MUTEX(DSI_GET_MUTEX(iface_id));
        DSI_INCR_COUNT(iface_id,DSI_IP_VERSION_6);
        /* let go the lock before calling user cb */
        DSI_UNLOCK_MUTEX(DSI_GET_MUTEX(iface_id));
      }
      break;

    case DSI_IP_VERSION_4_6:
    {
      int ret_v4 = QDI_FAILURE, ret_v6 = QDI_FAILURE;

      DSI_LOG_VERBOSE("%s","IPv4v6 call - querying addresses from QDI");
      if (QDI_SUCCESS != (ret_v4 = qdi_get_addr_info(DSI_GET_QDI_HNDL(iface_id),
                                                     DSI_GET_DEV_STR(iface_id),
                                                     AF_INET,
                                                     &DSI_GET_ADDRINFO(iface_id, DSI_IP_FAMILY_V4),
                                                     tech_name)))
      {
        DSI_LOG_ERROR("%s","failed to get IPv4 addr info from QDI");
      }
      else
      {
        DSI_LOCK_MUTEX(DSI_GET_MUTEX(iface_id));
        DSI_INCR_COUNT(iface_id,DSI_IP_VERSION_4);
        /* let go the lock before calling user cb */
        DSI_UNLOCK_MUTEX(DSI_GET_MUTEX(iface_id));
      }
      if (QDI_SUCCESS != (ret_v6 = qdi_get_addr_info(DSI_GET_QDI_HNDL(iface_id),
                                                     DSI_GET_DEV_STR(iface_id),
                                                     AF_INET6,
                                                     &DSI_GET_ADDRINFO(iface_id, DSI_IP_FAMILY_V6),
                                                     tech_name)))
      {
        DSI_LOG_ERROR("%s","failed to get IPv6 addr info from QDI");
      }
      else
      {
        DSI_LOCK_MUTEX(DSI_GET_MUTEX(iface_id));
        DSI_INCR_COUNT(iface_id,DSI_IP_VERSION_6);
        /* let go the lock before calling user cb */
        DSI_UNLOCK_MUTEX(DSI_GET_MUTEX(iface_id));
      }

      /* If we failed to get both addresses, return failure */
      if (QDI_SUCCESS != ret_v4 && QDI_SUCCESS != ret_v6)
      {
        ret = DSI_ERROR;
      }
      break;
    }

    default:
      DSI_LOG_ERROR("dsi_query_if_addr_from_qdi: invalid ip_version=%d", ip_version);
      ret = DSI_ERROR;
      break;
  }

  return ret;
}
/*===========================================================================
  FUNCTION:  dsi_query_if_tech_from_qdi
===========================================================================*/
/*!
    @brief
    Queries QDI to obtain the technology

    @param
    iface_id   - dsi iface id

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
static int dsi_query_if_tech_from_qdi
(
  int iface_id,
  int ip_family
)
{
  dsi_store_t *st = NULL;
  int i;
  boolean client_found = FALSE;
  qmi_wds_iface_name_type tech_name = QMI_WDS_IFACE_NAME_NOT_REPORTED;

  if (!DSI_IS_ID_VALID(iface_id))
  {
    DSI_LOG_ERROR("dsi_query_if_tech_from_qdi: invalid iface_id=%d", iface_id);
    return DSI_ERROR;
  }

  DSI_LOCK_MUTEX(DSI_GET_MUTEX(iface_id));

  /* Check for any valid client for the iface */
  for (i = 0; i < DSI_MAX_DATA_CALLS; i++)
  {
    st = dsi_store_table[i].dsi_store_ptr;

    if ((st != NULL && iface_id == st->priv.dsi_iface_id) &&
        ((st->priv.ip_version == DSI_IP_VERSION_4_6) ||
         ((st->priv.ip_version == DSI_IP_VERSION_4) && (ip_family == DSI_IP_FAMILY_V4)) ||
         ((st->priv.ip_version == DSI_IP_VERSION_6) && (ip_family == DSI_IP_FAMILY_V6))  )
       )
    {
      client_found = TRUE;
      break;
    }
  }

  if (TRUE == client_found)
  {
    DSI_LOG_VERBOSE("dsi_query_if_tech_from_qdi: valid client=0x%p found at "
                    "store index=%d", st, i);
    tech_name = st->priv.call_tech;
  }
  else
  {
    DSI_LOG_VERBOSE("%s", "dsi_query_if_tech_from_qdi: valid client not found");
  }
  DSI_UNLOCK_MUTEX(DSI_GET_MUTEX(iface_id));

  return tech_name;
}
/*===========================================================================
  FUNCTION:  dsi_update_call_end_reason
===========================================================================*/
/*!
    @brief
    Determines if a pending DSI_EVT_NET_IS_CONN event should be sent to the
    upper layers for a Dual-IP call

    @param
    st       - the store handle pointer
    rsp_data - the response data we got from qmi wds

    @return
    None
*/
/*=========================================================================*/
static void dsi_update_call_end_reason
(
  dsi_store_t                 *st,
  qdi_wds_async_rsp_data_type *rsp_data
)
{
  qmi_wds_call_end_reason_type* reason = NULL;

  if (!st || !rsp_data)
  {
    DSI_LOG_ERROR("%s", "dsi_update_call_end_reason: bad parameter(s)");
    return;
  }

  /* Update the V4 call end reason */
  if (rsp_data->flags & QDI_RSP_DATA_V4)
  {
    reason = &rsp_data->rsp_data_v4.rsp_data.start_nw_rsp.call_end_reason;
    st->priv.ce_reason[DSI_IP_FAMILY_V4].reason_code =
      reason->call_end_reason_verbose.verbose_reason;
    st->priv.ce_reason[DSI_IP_FAMILY_V4].reason_type =
      dsi_map_qmi_to_dsi_ce_reason_type(
      reason->call_end_reason_verbose.verbose_reason_type);

    DSI_LOG_DEBUG("set call end reason V4 code/type [%d/%d] on handle [%p]",
                  st->priv.ce_reason[DSI_IP_FAMILY_V4].reason_code,
                  st->priv.ce_reason[DSI_IP_FAMILY_V4].reason_type, st);
  }

  /* Update the V6 call end reason */
  if (rsp_data->flags & QDI_RSP_DATA_V6)
  {
    reason = &rsp_data->rsp_data_v6.rsp_data.start_nw_rsp.call_end_reason;
    st->priv.ce_reason[DSI_IP_FAMILY_V6].reason_code =
      reason->call_end_reason_verbose.verbose_reason;
    st->priv.ce_reason[DSI_IP_FAMILY_V6].reason_type =
      dsi_map_qmi_to_dsi_ce_reason_type(
      reason->call_end_reason_verbose.verbose_reason_type);

    DSI_LOG_DEBUG("set call end reason V6 code/type [%d/%d] on handle [%p]",
                  st->priv.ce_reason[DSI_IP_FAMILY_V6].reason_code,
                  st->priv.ce_reason[DSI_IP_FAMILY_V6].reason_type, st);
  }
}

/*===========================================================================
  FUNCTION:  dsi_query_iface_addr_and_incr_ref_count
===========================================================================*/
/*!
    @brief
    This function queries the interface addresses from QDI and if they are
    valid then increments the iface ref count and clears any pending flags.
    Also, returns the appropriate DSI event to send to the clients.

    @param
    st       - the store handle pointer
    iface    - interface id

    @return
    DSI_EVT_NET_IS_CONN
    DSI_EVT_NET_NO_NET
    DSI_EVT_INVALID
*/
/*=========================================================================*/
static dsi_net_evt_t
dsi_query_iface_addr_and_incr_ref_count
(
  dsi_store_t  *st,
  int          iface,
  int          ip_family
)
{
  dsi_net_evt_t event = DSI_EVT_INVALID;

  if (!st)
  {
    DSI_LOG_ERROR("%s","dsi_query_iface_addr_and_incr_ref_count: invalid param");
    return event;
  }

  /* Since we won't be getting any platform events from netmgr, query for
     all the interface addresses directly from QDI */
  /* special case for eMBMS since there is no IP family associated with
     its interface */
  if ( (QMI_WDS_IFACE_NAME_EMBMS != dsi_query_if_tech_from_qdi(iface,ip_family)) &&
       (QMI_WDS_IFACE_NAME_EMBMS != st->priv.start_nw_params.xtended_tech_pref)  &&
       (DSI_SUCCESS != dsi_query_if_addr_from_qdi(iface, st->priv.ip_version))
     )
  {
    /* No valid address found on the interface */
    DSI_LOG_ERROR("No valid address found on iface=%d, ip_version=%d, returning NET_NO_NET",
                  iface,
                  st->priv.ip_version);

    /* Return a NET_NO_NET event */
    event = DSI_EVT_NET_NO_NET;
  }
  /* If we already have a valid ref_count */
  else if (0 != st->priv.ref_count)
  {
    DSI_LOG_ERROR("Valid ref_count=%d found for st=0x%p",
                  st->priv.ref_count,
                  st);

    if (TRUE == st->priv.partial_retry)
    {
      event = DSI_EVT_NET_PARTIAL_CONN;
    }
    else
    {
      event = DSI_EVT_NET_NO_NET;
    }
  }
  else
  {
    /* Valid address found on the interface */
    DSI_LOG_VERBOSE("Valid address found on iface=%d, ip_version=%d, returning NET_IS_CONN",
                    iface,
                    st->priv.ip_version);


    DSI_LOCK_MUTEX(&(st->priv.mutex));
    /* Increment the client ref_count */
    ++st->priv.ref_count;
    DSI_UNLOCK_MUTEX(&(st->priv.mutex));

    DSI_LOG_VERBOSE("clearing pending flags call_state=%d, rl_qmi_inst",
                    st->priv.call_state,
                    st->priv.rl_qmi_inst);

    /* Clear out the pending flags */
    DSI_UPDATE_CALL_STATE(st, DSI_STATE_CALL_CONNECTED);

    st->priv.rl_qmi_inst = DSI_INVALID_QMI_INST;

    /* Updating the call tech of store handle for a shared call */
    if( (QMI_WDS_IFACE_NAME_EMBMS == st->priv.start_nw_params.xtended_tech_pref) &&
        (QMI_WDS_IFACE_NAME_EMBMS != st->priv.call_tech) )
    {
       st->priv.call_tech = QMI_WDS_IFACE_NAME_EMBMS;
    }

    /* Return a NET_IS_CONN event */
    event = DSI_EVT_NET_IS_CONN;
  }

  return event;
}

/*===========================================================================
  FUNCTION:  dsi_process_start_nw_async_rsp
===========================================================================*/
/*!
    @brief
    processes asynchronous response to start_nw request
    i is dsi iface id
    st is the store handle pointer
    sys_err_code is the sys_err_code we got from qmi wds
    qmi_err_code is the qmi_err_code we got from qmi wds
    rsp_data is the response data we got from qmi wds

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
static int dsi_process_start_nw_async_rsp
(
  dsi_store_t * st,
  int i,
  int sys_err_code,
  int qmi_err_code,
  qdi_wds_async_rsp_data_type * rsp_data
)
{
  int ret = DSI_ERROR;
  int reti = DSI_SUCCESS;
  /* notify event if notify_user is set to TRUE */
  boolean notify_user = DSI_FALSE;
  dsi_net_evt_t event = DSI_EVT_INVALID;
  int rl_qmi_inst;
  int port_incall = FALSE;
  qmi_wds_link_status_type link_state_v4 = QMI_WDS_PACKET_DATA_DISCONNECTED;
  qmi_wds_link_status_type link_state_v6 = QMI_WDS_PACKET_DATA_DISCONNECTED;
  int ip_family = QMI_IP_FAMILY_PREF_IPV4;
  boolean handle_found = FALSE;
  int j;

  DSI_LOG_DEBUG( "%s", "dsi_process_start_nw_async_rsp: ENTRY" );


  /* this do..while loop decides the overall return value
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    ret = DSI_ERROR;

    if(NULL == st)
    {
      DSI_LOG_ERROR("%s","received NULL st");
      break;
    }

    if(NULL == rsp_data)
    {
      DSI_LOG_ERROR("%s","received NULL rsp_data");
      break;
    }

    if (rsp_data->flags == QDI_RSP_DATA_V4_V6)
    {
      ip_family = QMI_IP_FAMILY_PREF_UNSPECIFIED;
      if((DSI_GET_WDS_V4_TXN(i) == DSI_INVALID_WDS_TXN)||
         (DSI_GET_WDS_V6_TXN(i) == DSI_INVALID_WDS_TXN))
      {
        DSI_LOG_ERROR("%s","dsi_qmi_wds_cmd_cb called in invalid state");
        break;
      }
      else
      {
        DSI_SET_WDS_V6_TXN(i,DSI_INVALID_WDS_TXN);
        DSI_SET_WDS_V4_TXN(i,DSI_INVALID_WDS_TXN);
      }
    }
    else if (rsp_data->flags == QDI_RSP_DATA_V4)
    {
      ip_family = QMI_IP_FAMILY_PREF_IPV4;
      if(DSI_GET_WDS_V4_TXN(i) == DSI_INVALID_WDS_TXN)
      {
        DSI_LOG_ERROR("%s","dsi_qmi_wds_cmd_cb called in invalid state");
        break;
      }
      else
      {
        DSI_SET_WDS_V4_TXN(i,DSI_INVALID_WDS_TXN);
      }
    }
    else
    {
      ip_family = QMI_IP_FAMILY_PREF_IPV6;
      if(DSI_GET_WDS_V6_TXN(i) == DSI_INVALID_WDS_TXN)
      {
        DSI_LOG_ERROR("%s","dsi_qmi_wds_cmd_cb called in invalid state");
        break;
      }
      else
      {
        DSI_SET_WDS_V6_TXN(i,DSI_INVALID_WDS_TXN);
      }
    }

    /* Update the call end reason */
    dsi_update_call_end_reason(st, rsp_data);

    /* check if txn was completed successfully */
    if(QMI_NO_ERR == sys_err_code)
    {
      /* Check for case where interface is already up in another process (ie
       * Modem has just incremented its ref-count).  Currently the Modem does
       * not provide an explicit indication that packet service indication
       * should be expected after start/stop command response.  This poses
       * problem with start case in that we wait indefinitely for pkt_srvc_ind
       * event which will never arrive.  For *most* cases we can detect this
       * condition by looking at the QMI handle in routing results, which
       * indicates if Modem interface is already in UP state.  There is a race
       * condition however if interface teardown occurs between our route lookup
       * and start interface; need Modem change to address this case. */

      DSI_LOG_ERROR("rl_qmi_inst = [%d]",st->priv.rl_qmi_inst);

      if( ( DSI_INVALID_QMI_INST != st->priv.rl_qmi_inst) &&
           (st->priv.companion_ip_call != TRUE))
      {
        /* some other process had already started this interface on modem */
        DSI_LOG_DEBUG("dsi interface [%d] already open " \
                      "by some other process", i);

        /* Query the interface addresses and increment the interface ref_count
           if a valid address is found */
        event = dsi_query_iface_addr_and_incr_ref_count(st, i,ip_family);

        if (DSI_EVT_INVALID != event)
        {
          DSI_LOG_DEBUG("notifying user of event=%d",
                        event);
          notify_user = DSI_TRUE;
        }
      }
      else
      {
        /* successfull !! */
        DSI_LOG_DEBUG("%s", "qmi txn success: rsp= " \
                      "QMI_WDS_SRVC_START_NW_ASYNC_RSP_MSG");

        /* If a Dual-IP call is already connected with only one valid address,
           send a DSI_EVT_NET_PARTIAL_CONN event */
        if (DSI_IP_VERSION_4_6 == st->priv.ip_version &&
            DSI_STATE_CALL_CONNECTED == st->priv.call_state &&
            (!DSI_IS_ADDR_VALID(st->priv.dsi_iface_id, DSI_IP_FAMILY_V4, iface_addr) ||
             !DSI_IS_ADDR_VALID(st->priv.dsi_iface_id, DSI_IP_FAMILY_V6, iface_addr)))
        {
          DSI_LOG_INFO( "Dual-IP call connected with one valid address, sending DSI_EVT_NET_PARTIAL_CONN on iface=%d",
                        st->priv.dsi_iface_id);
          event = DSI_EVT_NET_PARTIAL_CONN;
          notify_user = DSI_TRUE;
        }
        /* wait for pkt_srvc_ind now */
      }
    }
    else if(QMI_SERVICE_ERR == sys_err_code)
    {
      reti = DSI_SUCCESS;
      switch(qmi_err_code)
      {
      case QMI_SERVICE_ERR_NONE:
      /* some other wds client had already
         started this interface on modem */
        DSI_LOG_DEBUG("dsi interface [%d] already open " \
                      "by some other wds client than [%d]",
                      i, DSI_GET_WDS_HNDL(i));

        /* Query the interface addresses and increment the interface ref_count
           if a valid address is found */
        event = dsi_query_iface_addr_and_incr_ref_count(st, i,ip_family);

        if (DSI_EVT_INVALID != event)
        {
          DSI_LOG_DEBUG("notifying user of event=%d",
                        event);
          notify_user = DSI_TRUE;
        }
        break;

      case QMI_SERVICE_ERR_NO_EFFECT:
        /* some other wds client had already
           started this interface on modem */
        DSI_LOG_DEBUG("dsi interface [%d] already open " \
                      "by same  wds client [%d]",
                      i, DSI_GET_WDS_HNDL(i));

        /* Query the interface addresses and increment the interface ref_count
           if a valid address is found */
        event = dsi_query_iface_addr_and_incr_ref_count(st, i,ip_family);

        if (DSI_EVT_INVALID != event)
        {
          DSI_LOG_DEBUG("notifying client of event=%d",
                        event);
          notify_user = DSI_TRUE;
        }
        break;

      default:
        DSI_LOG_ERROR("received unknown service err [%d][%d]",
                      sys_err_code, qmi_err_code);
        reti = DSI_ERROR;
        event = DSI_EVT_NET_NO_NET;
        notify_user = DSI_TRUE;

        /* For a Dual-IP call, if we already have at least one valid address
           on the interface and this is a partial retry attempt then don't
           detach from the iface in this case */
        if (DSI_IP_VERSION_4_6 == st->priv.ip_version &&
            TRUE == st->priv.partial_retry &&
            (DSI_IS_ADDR_VALID(st->priv.dsi_iface_id, DSI_IP_FAMILY_V4, iface_addr) ||
             DSI_IS_ADDR_VALID(st->priv.dsi_iface_id, DSI_IP_FAMILY_V6, iface_addr)))
        {
          DSI_LOG_INFO( "Dual-IP partial retry failure, not detaching from iface=%d",
                        st->priv.dsi_iface_id);
          event = DSI_EVT_NET_PARTIAL_CONN;
          DSI_UPDATE_CALL_STATE(st, DSI_STATE_CALL_CONNECTED);
        }
        else
        {
          DSI_LOG_INFO( "Detaching from iface=%d", st->priv.dsi_iface_id );
          DSI_UPDATE_CALL_STATE(st, DSI_STATE_CALL_IDLE);

          /* detach interface from call store handle */
          DSI_LOCK_MUTEX(&(st->priv.mutex));
          dsi_detach_dsi_iface(st);
          DSI_UNLOCK_MUTEX(&(st->priv.mutex));
        }
        break;
      }
      if (DSI_ERROR == reti)
      {
        break;
      }
    }
    else
    {
      /* Error condition reported */
      DSI_LOG_ERROR( "qmi txn err: sys=%d qmi=%d rsp=%d",
                       sys_err_code, qmi_err_code,
                     QMI_WDS_SRVC_START_NW_ASYNC_RSP_MSG);
      /* notify user that net is down */
      notify_user = DSI_TRUE;
      event = DSI_EVT_NET_NO_NET;

      /* For a Dual-IP call, if we already have at least one valid address
         on the interface and this is a partial retry attempt then don't
         detach from the iface in this case */
      if (DSI_IP_VERSION_4_6 == st->priv.ip_version &&
          TRUE == st->priv.partial_retry &&
          (DSI_IS_ADDR_VALID(st->priv.dsi_iface_id, DSI_IP_FAMILY_V4, iface_addr) ||
           DSI_IS_ADDR_VALID(st->priv.dsi_iface_id, DSI_IP_FAMILY_V6, iface_addr)))
      {
        DSI_LOG_INFO( "Dual-IP partial retry failure, not detaching from iface=%d",
                      st->priv.dsi_iface_id);
        event = DSI_EVT_NET_PARTIAL_CONN;
        DSI_UPDATE_CALL_STATE(st, DSI_STATE_CALL_CONNECTED);
      }
      else
      {
        DSI_LOG_INFO( "Detaching from iface=%d", st->priv.dsi_iface_id );
        DSI_UPDATE_CALL_STATE(st, DSI_STATE_CALL_IDLE);

        /* detach interface from call store handle */
        DSI_LOCK_MUTEX(&(st->priv.mutex));
        dsi_detach_dsi_iface(st);
        DSI_UNLOCK_MUTEX(&(st->priv.mutex));
      }
    }

    ret = DSI_SUCCESS;
  } while (0);

  if (DSI_TRUE == notify_user)
  {
    /* notify user that net is connected as
       there won't be any pkt_srvc_ind */
    DSI_LOG_DEBUG( "notify users on interface [%d] of event [%d]",
                   i, event );
    if (NULL != st->net_ev_cb)
    {
      st->net_ev_cb((dsi_hndl_t *)st,
                    st->user_data,
                    event,
                    NULL);

   /* Release the IFACE clients only if this store does not have any handle sharing the IFACE */
      if(event == DSI_EVT_NET_NO_NET)
      {
        for (j = 0; j < DSI_MAX_DATA_CALLS; j++)
        {
          dsi_store_t *hndl = (dsi_store_t *)dsi_store_table[j].dsi_store_ptr;
          if ((hndl != NULL && i == hndl->priv.dsi_iface_id) )
          {
            handle_found = TRUE;
            break;
          }
        }
        if(!handle_found)
        {

/* Since DUAL IP call has two txn's, mni_client needs to be released only
   after async response for both families is complete.  */

          if( (DSI_IP_VERSION_4_6 != st->priv.ip_version) ||
              ( (DSI_GET_WDS_V4_TXN(i) == DSI_INVALID_WDS_TXN) &&
                (DSI_GET_WDS_V6_TXN(i) == DSI_INVALID_WDS_TXN) ) )
          {
            dsi_mni_release_client(i);
          }
        }
      }
    }
    else
    {
      DSI_LOG_ERROR("callback on store pointer [%p] is NULL",
                    st);
    }
  }

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_DEBUG( "%s", "dsi_process_start_nw_async_rsp: " \
                   "EXIT with suc" );
  }
  else
  {
    DSI_LOG_DEBUG( "%s", "dsi_process_start_nw_async_rsp: " \
                   "EXIT with err" );
  }

  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_process_async_wds_rsp
===========================================================================*/
/*!
    @brief
    callback function registered for asynchronous qmi wds responses
    currently used for
    start_nw_if
    stop_nw_if


    @return
    none
*/
/*=========================================================================*/
void dsi_process_async_wds_rsp
(
  int                           user_handle,     /* QMI Msg Lib client ID  */
  qmi_service_id_type           service_id,      /* QMI service ID         */
  int                           sys_err_code,    /* QMI Msg Lib error      */
  int                           qmi_err_code,    /* QMI error              */
  void                         *user_data,       /* Callback context       */
  qmi_wds_async_rsp_id_type     rsp_id,          /* QMI Msg Lib txn ID     */
  qdi_wds_async_rsp_data_type  *rsp_data         /* QMI Msg Lib txn data   */
)
{
  dsi_store_t * st = (dsi_store_t *)user_data;
  int i = 0;
  int ret = DSI_ERROR;
  int reti = DSI_SUCCESS;

  DSI_LOG_VERBOSE("%s", "dsi_process_async_wds_rsp: ENTRY");
  DSI_LOG_DEBUG("qmi wds cmd_cb: rsp_id=%d", rsp_id);
  (void)service_id;

  DSI_GLOBAL_LOCK;

  do
  {
    /* verify we received valid rsp_data */
    if (NULL == rsp_data)
    {
      DSI_LOG_ERROR("%s",
                    "dsi_process_async_wds_rsp: null resp data pointer passed");
      break;
    }

    /* verify we received valid user data */
    if (!DSI_IS_HNDL_VALID(st))
    {
      DSI_LOG_ERROR("dsi_process_async_wds_rsp: received invalid user data [%p]",
                    (unsigned int*)user_data);
      break;
    }

    i = st->priv.dsi_iface_id;

    /* Verify user data (iface_id) */
    if (!DSI_IS_ID_VALID(i))
    {
      DSI_LOG_ERROR("dsi_process_async_wds_rsp received invalid iface_id [%d]",
                    i);
      break;
    }

    /* verify wds handle */
    if(DSI_GET_QDI_HNDL(i) != user_handle)
    {
      DSI_LOG_ERROR("dsi_process_async_wds_rsp rcvd invalid wds hndl [%d]",
                    user_handle);
      break;
    }

    reti = DSI_SUCCESS;
    switch(rsp_id)
    {
    case QMI_WDS_SRVC_START_NW_ASYNC_RSP_MSG:
      DSI_LOG_VERBOSE("%s", "rcvd QMI_WDS_SRVC_START_NW_ASYNC_RSP_MSG");

      reti = dsi_process_start_nw_async_rsp(st,
                                            i,
                                            sys_err_code,
                                            qmi_err_code,
                                            rsp_data);

      break;
    case QMI_WDS_SRVC_STOP_NW_ASYNC_RSP_MSG:
      DSI_LOG_VERBOSE("%s", "rcvd QMI_WDS_SRVC_STOP_NW_ASYNC_RSP_MSG");
      reti = dsi_process_stop_nw_async_rsp(st,
                                           i,
                                           sys_err_code,
                                           qmi_err_code,
                                            rsp_data);
      break;
    default:
      DSI_LOG_ERROR("dsi_process_async_wds_rsp received rsp_id [%d] that" \
                    "we do not handle", rsp_id);
      reti = DSI_ERROR;
      break;
    }
    if (DSI_ERROR == reti)
    {
      break;
    }

    ret = DSI_SUCCESS;
  } while (0);

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_VERBOSE( "%s", "dsi_process_async_wds_rsp: EXIT with suc" );
  }
  else
  {
    DSI_LOG_VERBOSE( "%s", "dsi_process_async_wds_rsp: EXIT with err" );
  }

  DSI_GLOBAL_UNLOCK;
}

/*===========================================================================
  FUNCTION:  dsi_is_valid_client_found_for_iface
===========================================================================*/
/*!
    @brief
    Determines if a valid client is present for a given iface

    @return
    TRUE if a valid client is found for the given iface
    FALSE otherwise
*/
/*=========================================================================*/
static boolean dsi_is_valid_client_found_for_iface
(
  int  iface,
  int  ip_version
)
{
  boolean      client_found = FALSE;
  dsi_store_t  *st = NULL;
  int          i;


  if (!DSI_IS_ID_VALID(iface))
  {
    DSI_LOG_ERROR("%s", "dsi_is_valid_client_found_for_iface: invalid dsi iface");
    goto bail;
  }

  /* Check for any valid client for the iface */
  for (i = 0; i < DSI_MAX_DATA_CALLS; i++)
  {
    st = dsi_store_table[i].dsi_store_ptr;

    if ((st != NULL && iface == st->priv.dsi_iface_id) &&
        ((st->priv.ip_version == DSI_IP_VERSION_4_6) ||
         ((st->priv.ip_version == DSI_IP_VERSION_4) && (ip_version == DSI_IP_VERSION_4)) ||
         ((st->priv.ip_version == DSI_IP_VERSION_6) && (ip_version == DSI_IP_VERSION_6))  )
       )
    {
      client_found = TRUE;
      break;
    }
  }

  if (TRUE == client_found)
  {
    DSI_LOG_VERBOSE("dsi_is_valid_client_found_for_iface: valid client=0x%p found at "
                    "store index=%d", st, i);
  }
  else
  {
    DSI_LOG_VERBOSE("%s", "dsi_is_valid_client_found_for_iface: valid client not found");
  }

bail:
  return client_found;
}

/*===========================================================================
  FUNCTION:  dsi_update_client_ref_count
===========================================================================*/
/*!
    @brief
    Updates the ref_count associated with the client's store handle

    @return
    None
*/
/*=========================================================================*/
static void dsi_update_client_ref_count
(
  int  iface,
  qmi_ip_family_pref_type       ip_family
)
{
  boolean      client_found = FALSE;
  dsi_store_t  *st = NULL;
  int          i;


  if (!DSI_IS_ID_VALID(iface))
  {
    DSI_LOG_ERROR("dsi_update_client_ref_count: invalid dsi iface=%d",
                  iface);
    return;
  }

  /* Update the ref_count for all valid clients for this iface */
  for (i = 0; i < DSI_MAX_DATA_CALLS; i++)
  {
    st = dsi_store_table[i].dsi_store_ptr;

    if ((st != NULL && iface == st->priv.dsi_iface_id ) &&
         (((st->priv.ip_version == DSI_IP_VERSION_4) && (ip_family == QMI_IP_FAMILY_PREF_IPV4))||
         ((st->priv.ip_version == DSI_IP_VERSION_6) && (ip_family == QMI_IP_FAMILY_PREF_IPV6)) ||
         (st->priv.ip_version == DSI_IP_VERSION_4_6) ))
    {
      DSI_LOCK_MUTEX(&(st->priv.mutex));
      ++st->priv.ref_count;
      DSI_UNLOCK_MUTEX(&(st->priv.mutex));

      DSI_LOG_VERBOSE("dsi_update_client_ref_count: client=0x%p updated ref_count=%d",
                      st,
                      st->priv.ref_count);
    }
  }
}

/*===========================================================================
  FUNCTION:  dsi_decrement_client_ref_count
===========================================================================*/
/*!
    @brief
    Decrements the ref_count associated with the client's store handle

    @return
    None
*/
/*=========================================================================*/
static void dsi_decrement_client_ref_count
(
  int  iface,
  qmi_ip_family_pref_type       ip_family
)
{
  boolean      client_found = FALSE;
  dsi_store_t  *st = NULL;
  int          i;


  if (!DSI_IS_ID_VALID(iface))
  {
    DSI_LOG_ERROR("dsi_decrement_client_ref_count: invalid dsi iface=%d",
                  iface);
    return;
  }

  /* Update the ref_count for all valid clients for this iface */
  for (i = 0; i < DSI_MAX_DATA_CALLS; i++)
  {
    st = dsi_store_table[i].dsi_store_ptr;

    if ((st != NULL && iface == st->priv.dsi_iface_id ) &&
         (((st->priv.ip_version == DSI_IP_VERSION_4) && (ip_family == QMI_IP_FAMILY_PREF_IPV4))||
         ((st->priv.ip_version == DSI_IP_VERSION_6) && (ip_family == QMI_IP_FAMILY_PREF_IPV6)) ||
         (st->priv.ip_version == DSI_IP_VERSION_4_6) ))
    {
      DSI_LOCK_MUTEX(&(st->priv.mutex));
      if (st->priv.ref_count > 0)
      {
        --st->priv.ref_count;
      }
      else
      {
        DSI_LOG_ERROR("dsi_decrement_client_ref_count: ref_count of client=0x%p is already zero!\n",
                      st);
      }

      DSI_UNLOCK_MUTEX(&(st->priv.mutex));

      DSI_LOG_VERBOSE("dsi_decrement_client_ref_count: client=0x%p ref_count=%d",
                      st,
                      st->priv.ref_count);
    }
  }
}

/*===========================================================================
  FUNCTION:  dsi_handle_pkt_srvc_ind
===========================================================================*/
/*!
    @brief
    handles packet service indications

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
static int dsi_handle_pkt_srvc_ind
(
  int wds_hndl,
  qmi_service_id_type sid,
  void * user_data,
  qmi_wds_indication_data_type * ind_data
)
{
  int ret = DSI_ERROR;
  int reti = DSI_SUCCESS;
  int i = (int)(intptr_t)user_data;
  int ip_version = DSI_IP_VERSION_4;
  dsi_ce_reason_t ce_reason;
  dsi_ip_family_t ipf = DSI_IP_FAMILY_V4;
  dsi_net_evt_t      event;
  dsi_evt_payload_t  payload;
  int                count;
  dsi_store_t *st = NULL;

  DSI_LOG_VERBOSE("%s", "dsi_handle_pkt_srvc_ind: ENTRY");

  do
  {
    DSI_LOG_DEBUG("received pkt_srvc_ind with wds_hndl [0x%x] "
                  "sid [%d] user_data [%p] ind_data [%p]",
                  wds_hndl, sid, user_data, ind_data);

    if (NULL == ind_data)
    {
      DSI_LOG_ERROR("%s", "NULL ind data received in wds_ind_cb");
      break;
    }

    if (!DSI_IS_ID_VALID(i))
    {
      DSI_LOG_ERROR("invalid user data (dsi iface id) "\
                    "%d received in wds_ind_cb", i);
      break;
    }

    reti = DSI_SUCCESS;
    switch(ind_data->pkt_srvc_status.link_status)
    {
    case QMI_WDS_PACKET_DATA_DISCONNECTED:
      {
        ip_version = (ind_data->pkt_srvc_status.ip_family == QMI_IP_FAMILY_PREF_IPV6)?
                   (DSI_IP_VERSION_6):(DSI_IP_VERSION_4);
        DSI_LOG_DEBUG("packet data call connected on iface [%d]", i);

        /* Since the pkt_srvc indication is a broadcast indication, we could get
           a spurious indication when we haven't initiated a call in a multi-PD scenario.
           We need to make sure to increment the ref_count only if the call
           was originated using this instance of the library */
        if (!dsi_is_valid_client_found_for_iface(i,ip_version))
        {
          DSI_LOG_ERROR("received spurious pkt_srvc ind on iface=%d "
                        "without any clients", i);
        }
        else
        {
          /* Client notification happens in dsi_netmgr_post_event() as
           * this ensures kernel interface is down and platform is ready
           * to support next call setup.  Here simply capture the call end
           * reason in local store. */
          DSI_LOCK_MUTEX(DSI_GET_MUTEX(i));

          if (ind_data->pkt_srvc_status.param_mask & QMI_WDS_PKT_SRVC_IND_IP_FAMILY)
          {
            if (QMI_WDS_IP_FAMILY_PREF_IPV4 == ind_data->pkt_srvc_status.ip_family)
            {
              ipf = DSI_IP_FAMILY_V4;
            }
            else if (QMI_WDS_IP_FAMILY_PREF_IPV6 == ind_data->pkt_srvc_status.ip_family)
            {
              ipf = DSI_IP_FAMILY_V6;
            }
          }

          dsi_update_store_table
            (
              i,
              ind_data->pkt_srvc_status.call_end_reason.
                call_end_reason_verbose.verbose_reason_type,
              ind_data->pkt_srvc_status.call_end_reason.
                call_end_reason_verbose.verbose_reason,
              ipf
             );

          /* Set the iface ref count to zero for the given family
           * In case of dual-IP call if one of the families goes down
           * We need to ensure that the iface ref count of that family
           * is reset to zero. When partial-retry is kickstarted for
           * that IP family it will incremented correctly from zero */
          if (DSI_IP_VERSION_6 == ip_version)
          {
            DSI_SET_V6_COUNT(i,0);
            DSI_LOG_DEBUG("iface [%d] v6 ref count reset to [%d]",
                          i, dsi_iface_tbl[i].v6_ref_count);
          }
          else if (DSI_IP_VERSION_4 == ip_version)
          {
            DSI_SET_V4_COUNT(i,0);
            DSI_LOG_DEBUG("iface [%d] v4 ref count reset to [%d]",
                          i, dsi_iface_tbl[i].v4_ref_count);
          }

          DSI_UNLOCK_MUTEX(DSI_GET_MUTEX(i));

          /* Decrement the client ref_count for the given ip family */
          dsi_decrement_client_ref_count(i,ind_data->pkt_srvc_status.ip_family);

          DSI_LOG_DEBUG("packet data call disconnected on iface [%d] "
                        "call end reason [%d/%d]", i,
                        ind_data->pkt_srvc_status.call_end_reason.
                        call_end_reason_verbose.verbose_reason_type,
                        ind_data->pkt_srvc_status.call_end_reason.
                        call_end_reason_verbose.verbose_reason);

          /* we don't notify the user until we get final event from
           * platform (NetMgr) */
        }
        break;
      }

    case QMI_WDS_PACKET_DATA_CONNECTED:
      if (ind_data->pkt_srvc_status.reconfig_required != TRUE)
      {
        ip_version = (ind_data->pkt_srvc_status.ip_family == QMI_IP_FAMILY_PREF_IPV6)?
                   (DSI_IP_VERSION_6):(DSI_IP_VERSION_4);
        DSI_LOG_DEBUG("packet data call connected on iface [%d]", i);

        /* Since the pkt_srvc indication is a broadcast indication, we could get
           a spurious indication when we haven't initiated a call in a multi-PD scenario.
           We need to make sure to increment the ref_count only if the call
           was originated using this instance of the library */
        if (!dsi_is_valid_client_found_for_iface(i,ip_version))
        {
          DSI_LOG_ERROR("received spurious pkt_srvc ind on iface=%d "
                        "without any clients", i);
        }
        else
        {
          /* increment ref count and cache the call technology */
          DSI_LOCK_MUTEX(DSI_GET_MUTEX(i));
          DSI_INCR_COUNT(i,ip_version);
          if (ind_data->pkt_srvc_status.param_mask & QMI_WDS_PKT_SRVC_IND_TECH_NAME)
          {
            dsi_update_call_tech
              (
                i,
                ind_data->pkt_srvc_status.tech_name,ip_version
              );
          }
          DSI_UNLOCK_MUTEX(DSI_GET_MUTEX(i));

          dsi_update_client_ref_count(i,ind_data->pkt_srvc_status.ip_family);
        }

        memset(&payload, 0, sizeof(payload));
        /* Filling payload with IP type, which is the only
         element valid for DSI_EVT_WDS_CONNECTED event
        */
        payload.ip_type = ipf;
        event  = DSI_EVT_WDS_CONNECTED;

        for(count = 0; count < DSI_MAX_DATA_CALLS; count++)
        {
          st = (dsi_store_t *)dsi_store_table[count].dsi_store_ptr;
          /* Checking IP type also to select right store element in case of
            companion IP call
        */
          if (st != NULL &&
              st->priv.dsi_iface_id == i &&
              ((int) ind_data->pkt_srvc_status.ip_family == st->priv.ip_version ||
               st->priv.ip_version == DSI_IP_VERSION_4_6))
          {
            if (NULL != st->net_ev_cb)
            {
              DSI_LOG_DEBUG("notifying event [%d] on handle [%p]",
                            event, st);
              st->net_ev_cb((dsi_hndl_t *)st,
                            st->user_data,
                            event,
                            &payload  );
            }
            else
            {
              DSI_LOG_ERROR("NULL callback found on store pointer" \
                            "[%p]", st);
            }
          }
        }
      }
      else
      {
        DSI_LOG_DEBUG("packet data call reconfigured on iface [%d]", i);
      }
      /* we don't notify the user until we get final event from
       * platform (NetMgr) except for DSI_EVT_WDS_CONNECTED event*/
      break;
    default:
      DSI_LOG_VERBOSE("not processing pkt_srvc_ind [%d]",
                      ind_data->pkt_srvc_status.link_status);
      reti = DSI_ERROR;
      break;
    }
    if(reti == DSI_ERROR)
    {
      break;
    }
    ret = DSI_SUCCESS;
  } while (0);

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_VERBOSE("%s", "dsi_handle_pkt_srvc_ind: EXIT with suc");
  }
  else
  {
    DSI_LOG_VERBOSE("%s", "dsi_handle_pkt_srvc_ind: EXIT with err");
  }
  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_handle_embms_tmgi_activate_status_ind
===========================================================================*/
/*!
    @brief
    handles embms tmgi activate status indications

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
static int dsi_handle_embms_tmgi_activate_status_ind
(
  int                            wds_hndl,
  qmi_service_id_type            sid,
  void                          *user_data,
  qmi_wds_indication_data_type  *ind_data
)
{
  int                ret          = DSI_ERROR;
  int                reti         = DSI_SUCCESS;
  int                i            = (int)(intptr_t)user_data;
  dsi_net_evt_t      event;
  dsi_store_t       *st           = NULL;
  dsi_evt_payload_t  payload;
  int                count;
  boolean            handle_found = FALSE;

  DSI_LOG_VERBOSE("%s", "dsi_handle_embms_tmgi_activate_status_ind: ENTRY");

  memset(&payload, 0, sizeof(payload));

  do
  {
    DSI_LOG_DEBUG("received tmgi_activate_status_ind with wds_hndl [0x%x] "
                  "sid [%d] user_data [%p] ind_data [%p]",
                  wds_hndl, sid, user_data, ind_data);

    if (NULL == ind_data)
    {
      DSI_LOG_ERROR("%s", "NULL ind data received in tmgi_activate_status_ind");
      break;
    }

    if (!DSI_IS_ID_VALID(i))
    {
      DSI_LOG_ERROR("invalid user data (dsi iface id) "\
                    "%d received in tmgi_activate_status_ind", i);
      break;
    }

    for(count = 0; count < DSI_MAX_DATA_CALLS; count++)
    {
      st = (dsi_store_t *)dsi_store_table[count].dsi_store_ptr;
      if (st != NULL && st->priv.dsi_iface_id == i)
      {
        handle_found = TRUE;
        break;
      }
    }
    if(FALSE == handle_found)
    {
      DSI_LOG_ERROR("No call state for index [%d]", i);
      break;
    }

    if( NULL == st->net_ev_cb )
    {
      DSI_LOG_ERROR("No event callback for index [%d]", i);
      break;
    }

    if(ind_data->embms_activate_status.param_mask &
       QMI_WDS_EMBMS_TMGI_ACTIVATE_STATUS_IND_TRANX_ID_PARAM)
    {
      payload.embms_tmgi_info.embms_tmgi_actdeact_params.dbg_trace_id =
        ind_data->embms_activate_status.dbg_trace_id;
      DSI_LOG_DEBUG("activate_status_ind dbg_trace_id:[%d]",
                    ind_data->embms_activate_status.dbg_trace_id);
    }

    switch(ind_data->embms_activate_status.activate_status)
    {
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_SUCCESS:
        {
          event = DSI_NET_TMGI_ACTIVATED;
          break;
        }
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_SUCCESS_DUP_ACTIVATE:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_SUCCESS_IDLE_RADIO_TUNE:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_SUCCESS_CONN_RADIO_TUNE:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_RADIO_CONFIG:       /* fall through */
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_CHANNEL_UNAVAILABLE:/* fall through */
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_EMBMS_NOT_ENABLED:  /* fall through */
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_OUT_OF_COVERAGE:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_UNKNOWN:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_NOT_ALLOWED:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_MISSING_CTL_INFO:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_MISSING_TMGI:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_MCAST_OOS:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_UNICAST_OOS:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_CAMP_ON_OTHER_FREQ:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_SAI_MISMATCH:
      case QMI_WDS_EMBMS_TMGI_ACTIVATION_FAIL_MAX_TMGI_ALREADY_ACTIVE:
        {
          event = DSI_NET_TMGI_DEACTIVATED;
          break;
        }
      default:
        {
          DSI_LOG_VERBOSE("unknown activate_status[%d], drop indication",
                           ind_data->embms_activate_status.activate_status);
          reti = DSI_ERROR;
          break;
        }
    }
    if(DSI_SUCCESS == reti)
    {
      payload.embms_tmgi_info.embms_tmgi_actdeact_params.activate_status =
        ind_data->embms_activate_status.activate_status;

      /* activate status indication should have ONLY 1 TMGI list */
      if(DSI_EMBMS_TMGI_STATUS_IND_TMGI_LIST_LEN !=
         (payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list_len =
          ind_data->embms_activate_status.activation_tmgi.tmgi_list_len))
      {
        DSI_LOG_ERROR("TMGI list length incorrect: [%d]\n",
                      ind_data->embms_activate_status.activation_tmgi.tmgi_list_len);
        break;
      }
      if(DSI_SUCCESS != dsi_netctrl_copy_tmgi_list(
        &(payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list),
        ind_data->embms_activate_status.activation_tmgi.tmgi_list_ptr,
        ind_data->embms_activate_status.activation_tmgi.tmgi_list_len))
      {
        DSI_LOG_ERROR("%s", "Failed on dsi_netctrl_copy_tmgi_list");
        break;
      }

      for(count = 0; count < DSI_MAX_DATA_CALLS; count++)
      {
        st = (dsi_store_t *)dsi_store_table[count].dsi_store_ptr;
        if (st != NULL && st->priv.dsi_iface_id == i)
        {
          if (NULL != st->net_ev_cb)
          {
            DSI_LOG_DEBUG("notifying event [%d] on handle [%p]",
                          event, st);
            st->net_ev_cb( (dsi_hndl_t *)st,
                           st->user_data,
                           event,
                           &payload  );
          }
          else
          {
            DSI_LOG_ERROR("NULL callback found on store pointer" \
                          "[%p]", st);
          }
        }
      }
      ret = DSI_SUCCESS;
    }
    else
    {
      break;
    }
  } while (0);

  /* make sure memory is released */
  if(ind_data && ind_data->embms_activate_status.activation_tmgi.tmgi_list_ptr)
  {
    DSI_LOG_DEBUG("%s", "free tmgi list from indication");
    free(ind_data->embms_activate_status.activation_tmgi.tmgi_list_ptr);
    ind_data->embms_activate_status.activation_tmgi.tmgi_list_ptr = NULL;
  }

  if(NULL != payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list)
  {
    DSI_LOG_DEBUG("%s", " free tmgi_list in payload");
    free(payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list);
    payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list = NULL;
  }
  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_VERBOSE("%s", "dsi_handle_embms_tmgi_activate_status_ind: "
                    "EXIT with suc");
  }
  else
  {
    DSI_LOG_VERBOSE("%s", "dsi_handle_embms_tmgi_activate_status_ind: "
                    "EXIT with err");
  }
  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_handle_embms_tmgi_deactivate_status_ind
===========================================================================*/
/*!
    @brief
    handles embms tmgi deactivate status indications

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
static int dsi_handle_embms_tmgi_deactivate_status_ind
(
  int                            wds_hndl,
  qmi_service_id_type            sid,
  void                          *user_data,
  qmi_wds_indication_data_type  *ind_data
)
{
  int                ret          = DSI_ERROR;
  int                reti         = DSI_SUCCESS;
  int                i            = (int)(intptr_t)user_data;
  dsi_net_evt_t      event;
  dsi_store_t       *st           = NULL;
  dsi_evt_payload_t  payload;
  int                count;
  boolean            handle_found = FALSE;

  DSI_LOG_VERBOSE("%s", "dsi_handle_embms_tmgi_deactivate_status_ind: ENTRY");

  memset(&payload, 0, sizeof(payload));

  do
  {
    DSI_LOG_DEBUG("received tmgi_deactivate_status_ind with wds_hndl [0x%x] "
                  "sid [%d] user_data [%p] ind_data [%p]",
                  wds_hndl, sid, user_data, ind_data);

    if (NULL == ind_data)
    {
      DSI_LOG_ERROR("%s", "NULL ind data received in tmgi_dactivate_status_ind");
      break;
    }

    if (!DSI_IS_ID_VALID(i))
    {
      DSI_LOG_ERROR("invalid user data (dsi iface id) "\
                    "%d received in tmgi_deactivate_status_ind", i);
      break;
    }

    for(count = 0; count < DSI_MAX_DATA_CALLS; count++)
    {
      st = (dsi_store_t *)dsi_store_table[count].dsi_store_ptr;
      if (st != NULL && st->priv.dsi_iface_id == i)
      {
        handle_found = TRUE;
        break;
      }
    }
    if(FALSE == handle_found)
    {
      DSI_LOG_ERROR("No call state for index [%d]", i);
      break;
    }

    if( NULL == st->net_ev_cb )
    {
      DSI_LOG_ERROR("No event callback for index [%d]", i);
      break;
    }

    if(ind_data->embms_deactivate_status.param_mask &
       QMI_WDS_EMBMS_TMGI_DEACTIVATE_STATUS_IND_TRANX_ID_PARAM)
    {
      payload.embms_tmgi_info.embms_tmgi_actdeact_params.dbg_trace_id =
        ind_data->embms_activate_status.dbg_trace_id;
      DSI_LOG_DEBUG("deactivate_status_ind dbg_trace_id:[%d]",
                    ind_data->embms_deactivate_status.dbg_trace_id);
    }
    switch(ind_data->embms_deactivate_status.deactivate_status)
    {
      case QMI_WDS_EMBMS_TMGI_DEACTIVATE_SUCCESS:
        {
          event = DSI_NET_TMGI_DEACTIVATED;

          payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_status =
            ind_data->embms_deactivate_status.deactivate_status;

          /* deactivate status indication should have ONLY 1 TMGI list */
          if(DSI_EMBMS_TMGI_STATUS_IND_TMGI_LIST_LEN !=
             (payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list_len =
              ind_data->embms_deactivate_status.deactivation_tmgi.tmgi_list_len))
          {
            DSI_LOG_ERROR("TMGI list length incorrect: [%d]\n",
                          ind_data->embms_deactivate_status.deactivation_tmgi.tmgi_list_len);
            reti = DSI_ERROR;
            break;
          }

          if(DSI_SUCCESS != dsi_netctrl_copy_tmgi_list(
            &(payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list),
            ind_data->embms_deactivate_status.deactivation_tmgi.tmgi_list_ptr,
            ind_data->embms_deactivate_status.deactivation_tmgi.tmgi_list_len))
          {
            DSI_LOG_ERROR("%s", "Failed on dsi_netctrl_copy_tmgi_list ");
            reti = DSI_ERROR;
            break;
          }
        }
        break;

      default:
        {
          DSI_LOG_ERROR("unknown deactivate_status [%d] drop indication",
                           ind_data->embms_deactivate_status.deactivate_status);
          reti = DSI_ERROR;
        }
        break;
    }/* switch */

    if(DSI_SUCCESS == reti)
    {
      for(count = 0; count < DSI_MAX_DATA_CALLS; count++)
      {
        st = (dsi_store_t *)dsi_store_table[count].dsi_store_ptr;
        if (st != NULL && st->priv.dsi_iface_id == i)
        {
          if (NULL != st->net_ev_cb)
          {
            DSI_LOG_DEBUG("notifying event [%d] on handle [%p]",
                          event, st);
            st->net_ev_cb( (dsi_hndl_t *)st,
                           st->user_data,
                           event,
                           &payload  );
          }
          else
          {
            DSI_LOG_ERROR("NULL callback found on store pointer" \
                          "[%p]", st);
          }
        }
      }
      ret = DSI_SUCCESS;
    }
    else
    {
      break;
    }

  } while (0);

  if(ind_data && ind_data->embms_deactivate_status.deactivation_tmgi.tmgi_list_ptr)
  {
    DSI_LOG_DEBUG("%s", "free deactivate tmgi list from indication");
    free(ind_data->embms_deactivate_status.deactivation_tmgi.tmgi_list_ptr);
    ind_data->embms_deactivate_status.deactivation_tmgi.tmgi_list_ptr = NULL;
  }

  if(NULL != payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list)
  {
    DSI_LOG_DEBUG("%s", " free deactivate tmgi_list in payload");
    free(payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list);
    payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list = NULL;
  }
  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_VERBOSE("%s", "dsi_handle_embms_tmgi_deactivate_status_ind: "
                    "EXIT with suc");
  }
  else
  {
    DSI_LOG_VERBOSE("%s", "dsi_handle_embms_tmgi_deactivate_status_ind: "
                    "EXIT with err");
  }
  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_handle_embms_tmgi_act_deactivate_status_ind
===========================================================================*/
/*!
    @brief
    handles embms tmgi activate_deactivate status indications

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
static int dsi_handle_embms_tmgi_act_deactivate_status_ind
(
  int                            wds_hndl,
  qmi_service_id_type            sid,
  void                          *user_data,
  qmi_wds_indication_data_type  *ind_data
)
{
  int                ret          = DSI_ERROR;
  int                reti         = DSI_SUCCESS;
  int                i            = (int)(intptr_t)user_data;
  dsi_net_evt_t      event;
  dsi_store_t       *st           = NULL;
  dsi_evt_payload_t  payload;
  int                count;
  boolean            handle_found = FALSE;

  DSI_LOG_VERBOSE("%s", "dsi_handle_embms_tmgi_act_deactivate_status_ind: ENTRY");

  memset(&payload, 0, sizeof(payload));

  do
  {
    DSI_LOG_DEBUG("received tmgi_act_deactivate_status_ind with wds_hndl [0x%x] "
                  "sid [%d] user_data [%p] ind_data [%p]",
                  wds_hndl, sid, user_data, ind_data);

    if (NULL == ind_data)
    {
      DSI_LOG_ERROR("%s", "NULL ind data received in tmgi_act_deactivate_status_ind");
      break;
    }

    if (!DSI_IS_ID_VALID(i))
    {
      DSI_LOG_ERROR("invalid user data (dsi iface id) "\
                    "%d received in tmgi_act_deactivate_status_ind", i);
      break;
    }

    for(count = 0; count < DSI_MAX_DATA_CALLS; count++)
    {
      st = (dsi_store_t *)dsi_store_table[count].dsi_store_ptr;
      if (st != NULL && st->priv.dsi_iface_id == i)
      {
        handle_found = TRUE;
        break;
      }
    }
    if(FALSE == handle_found)
    {
      DSI_LOG_ERROR("No call state for index [%d]", i);
      break;
    }

    if( NULL == st->net_ev_cb )
    {
      DSI_LOG_ERROR("No event callback for index [%d]", i);
      break;
    }

    if(ind_data->embms_act_deact_status.param_mask &
       QMI_WDS_EMBMS_TMGI_ACT_DEACT_STATUS_IND_TRANX_ID_PARAM)
    {
      payload.embms_tmgi_info.embms_tmgi_actdeact_params.dbg_trace_id =
        ind_data->embms_act_deact_status.dbg_trace_id;
      DSI_LOG_DEBUG("act_deactivate_status_ind debug_trace_id:[%d]",
                  ind_data->embms_act_deact_status.dbg_trace_id);
    }

    event = DSI_NET_TMGI_ACTIVATED_DEACTIVATED;

    switch(ind_data->embms_act_deact_status.activate_status)
    {
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_SUCCESS:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_SUCCESS_DUP_ACTIVATE:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_SUCCESS_IDLE_RADIO_TUNE:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_SUCCESS_CONN_RADIO_TUNE:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_RADIO_CONFIG:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_CHANNEL_UNAVAILABLE:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_EMBMS_NOT_ENABLED:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_OUT_OF_COVERAGE:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_UNKNOWN:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_NOT_ALLOWED:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_MISSING_CTL_INFO:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_MISSING_TMGI:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_MCAST_OOS:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_UNICAST_OOS:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_CAMP_ON_OTHER_FREQ:
      case QMI_WDS_EMBMS_TMGI_ACTIVATE_FAIL_SAI_MISMATCH:
      case QMI_WDS_EMBMS_TMGI_ACTIVATION_FAIL_MAX_TMGI_ALREADY_ACTIVE:
        break;
      default:
        reti = DSI_ERROR;
        break;
    }
    switch(ind_data->embms_act_deact_status.deactivate_status)
    {
      case QMI_WDS_EMBMS_TMGI_DEACTIVATE_SUCCESS:
        break;
      default:
        reti = DSI_ERROR;
        break;
    }

    if(DSI_SUCCESS != reti)
    {
      DSI_LOG_ERROR("unknown act_deact_status act:[%d] deact:[%d] drop indication",
                    ind_data->embms_act_deact_status.activate_status,
                    ind_data->embms_act_deact_status.deactivate_status);
      break;
    }

    payload.embms_tmgi_info.embms_tmgi_actdeact_params.activate_status =
      ind_data->embms_act_deact_status.activate_status;
    payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_status =
      ind_data->embms_act_deact_status.deactivate_status;

    /* act_deact status indication should have tmgi list length as 1 */
    if((DSI_EMBMS_TMGI_STATUS_IND_TMGI_LIST_LEN !=
        ind_data->embms_act_deact_status.activation_tmgi.tmgi_list_len) ||
       (DSI_EMBMS_TMGI_STATUS_IND_TMGI_LIST_LEN !=
        ind_data->embms_act_deact_status.deactivation_tmgi.tmgi_list_len)
       )
    {
      DSI_LOG_ERROR("TMGI list length wrong act:[%d] deact:[%d]",
                    ind_data->embms_act_deact_status.activation_tmgi.tmgi_list_len,
                    ind_data->embms_act_deact_status.deactivation_tmgi.tmgi_list_len);
      break;
    }
    payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list_len =
      DSI_EMBMS_TMGI_STATUS_IND_TMGI_LIST_LEN;

    if(DSI_SUCCESS != dsi_netctrl_copy_tmgi_list(
      &(payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list),
      ind_data->embms_act_deact_status.activation_tmgi.tmgi_list_ptr,
      ind_data->embms_act_deact_status.activation_tmgi.tmgi_list_len))
    {
      DSI_LOG_ERROR("%s", "Failed on dsi_netctrl_copy_tmgi_list");
      break;
    }
    if(DSI_SUCCESS != dsi_netctrl_copy_tmgi_list(
      &(payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list),
      ind_data->embms_act_deact_status.deactivation_tmgi.tmgi_list_ptr,
      ind_data->embms_act_deact_status.deactivation_tmgi.tmgi_list_len))
    {
      DSI_LOG_ERROR("%s", "Failed on dsi_netctrl_copy_tmgi_list ");
      break;
    }

    for(count = 0; count < DSI_MAX_DATA_CALLS; count++)
    {
      st = (dsi_store_t *)dsi_store_table[count].dsi_store_ptr;
      if (st != NULL && st->priv.dsi_iface_id == i)
      {
        if (NULL != st->net_ev_cb)
        {
          DSI_LOG_DEBUG("notifying event [%d] on handle [%p]",
                        event, st);
          st->net_ev_cb( (dsi_hndl_t *)st,
                         st->user_data,
                         event,
                         &payload  );
        }
        else
        {
          DSI_LOG_ERROR("NULL callback found on store pointer" \
                        "[%p]", st);
        }
      }
      ret = DSI_SUCCESS;
    }

  } while (0);


  if(ind_data && ind_data->embms_act_deact_status.activation_tmgi.tmgi_list_ptr)
  {
    DSI_LOG_DEBUG("%s", "free activate tmgi list from indication");
    free(ind_data->embms_act_deact_status.activation_tmgi.tmgi_list_ptr);
    ind_data->embms_act_deact_status.activation_tmgi.tmgi_list_ptr = NULL;
  }

  if(ind_data && ind_data->embms_act_deact_status.deactivation_tmgi.tmgi_list_ptr)
  {
    DSI_LOG_DEBUG("%s", "free deactivate_tmgi_list from indication");
    free(ind_data->embms_act_deact_status.deactivation_tmgi.tmgi_list_ptr);
    ind_data->embms_act_deact_status.deactivation_tmgi.tmgi_list_ptr = NULL;
  }

  if(NULL != payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list)
  {
    DSI_LOG_DEBUG("%s", " free tmgi_list in payload");
    free(payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list);
    payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list = NULL;
  }

  if(NULL != payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list)
  {
    DSI_LOG_DEBUG("%s", "free deactiate_tmgi_list in payload");
    free(payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list);
    payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list = NULL;
  }
  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_VERBOSE("%s", "dsi_handle_embms_tmgi_act_deactivate_status_ind: "
                    "EXIT with suc");
  }
  else
  {
    DSI_LOG_VERBOSE("%s", "dsi_handle_embms_tmgi_act_deactivate_status_ind: "
                    "EXIT with err");
  }
  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_handle_embms_tmgi_list_ind
===========================================================================*/
/*!
    @brief
    handles embms tmgi list indications

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
static int dsi_handle_embms_tmgi_list_ind
(
  int                            wds_hndl,
  qmi_service_id_type            sid,
  void                          *user_data,
  qmi_wds_indication_data_type  *ind_data
)
{
  int                ret          = DSI_ERROR;
  int                reti         = DSI_SUCCESS;
  int                i            = (int)(intptr_t)user_data;
  dsi_net_evt_t      event;
  dsi_store_t       *st           = NULL;
  dsi_evt_payload_t  payload;
  int                count;
  boolean            handle_found = FALSE;

  DSI_LOG_VERBOSE("%s", "dsi_handle_embms_tmgi_list_ind: ENTRY");

  memset(&payload, 0, sizeof(payload));

  do
  {
    DSI_LOG_DEBUG("received tmgi_list_ind with wds_hndl [0x%x] "
                  "sid [%d] user_data [%p] ind_data [%p]",
                  wds_hndl, sid, user_data, ind_data);

    if (NULL == ind_data)
    {
      DSI_LOG_ERROR("%s", "NULL ind data received in tmgi_list_ind");
      break;
    }

    if (!DSI_IS_ID_VALID(i))
    {
      DSI_LOG_ERROR("invalid user data (dsi iface id) "\
                    "%d received in tmgi_list_ind", i);
      break;
    }

    for(count = 0; count < DSI_MAX_DATA_CALLS; count++)
    {
      st = (dsi_store_t *)dsi_store_table[count].dsi_store_ptr;
      if (st != NULL && st->priv.dsi_iface_id == i)
      {
        handle_found = TRUE;
        break;
      }
    }
    if(FALSE == handle_found)
    {
      DSI_LOG_ERROR("No call state for index [%d]", count);
      break;
    }
    if( NULL == st->net_ev_cb )
    {
      DSI_LOG_ERROR("No event callback for index [%d]", count);
      break;
    }

    payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list = NULL;
    payload.embms_tmgi_info.embms_tmgi_actdeact_params.deactivate_tmgi_list = NULL;

    if(ind_data->embms_list.param_mask & QMI_WDS_EMBMS_LIST_IND_TRANX_ID_PARAM_MASK)
    {
      DSI_LOG_DEBUG("received tmgi_list_ind, dbg_trace_id [%d] ",
                     ind_data->embms_list.dbg_trace_id);
      payload.embms_tmgi_info.embms_tmgi_actdeact_params.dbg_trace_id =
        ind_data->embms_list.dbg_trace_id;
    }

    if(ind_data->embms_list.param_mask & QMI_WDS_EMBMS_LIST_IND_TMGI_LIST_PARAM_MASK)
    {
      switch(ind_data->embms_list.list_type)
      {
        case QMI_WDS_EMBMS_TMGI_LIST_AVAILABLE:
        case QMI_WDS_EMBMS_TMGI_LIST_OOS_WARNING:
        case QMI_WDS_EMBMS_TMGI_LIST_ACTIVE:
          {
            event = DSI_NET_TMGI_LIST_CHANGED;

            DSI_LOG_DEBUG("received tmgi_list_ind, list ind type [%d]",
                          ind_data->embms_list.dbg_trace_id);

            payload.embms_tmgi_info.list_type = ind_data->embms_list.list_type;
            payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list_len =
              ind_data->embms_list.tmgi_list.tmgi_list_len;

            if(0 != ind_data->embms_list.tmgi_list.tmgi_list_len)
            {
              if(DSI_SUCCESS != dsi_netctrl_copy_tmgi_list(
                &(payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list),
                ind_data->embms_list.tmgi_list.tmgi_list_ptr,
                ind_data->embms_list.tmgi_list.tmgi_list_len))
              {
                DSI_LOG_ERROR("%s", "Failed on dsi_netctrl_copy_tmgi_list ");
                reti = DSI_ERROR;
                break;
              }
            }
            /*
            *  OOS warning will only be valid in the case of
            *  QMI_WDS_EMBMS_TMGI_OOS_WARNING_LIST indication type.
            */
            if (ind_data->embms_list.param_mask &
                  QMI_WDS_EMBMS_LIST_IND_OOS_WARNING_PARAM_MASK)
            {
              payload.embms_tmgi_info.oos_warning =
                  ind_data->embms_list.oos_warning_reason;
            }

            for(count = 0; count < DSI_MAX_DATA_CALLS; count++)
            {
              st = (dsi_store_t *)dsi_store_table[count].dsi_store_ptr;
              if (st != NULL && st->priv.dsi_iface_id == i)
              {
                if (NULL != st->net_ev_cb)
                {
                  DSI_LOG_DEBUG("notifying event [%d] on handle [%p]",
                                event, st);
                  st->net_ev_cb( (dsi_hndl_t *)st,
                                 st->user_data,
                                 event,
                                 &payload  );
                }
                else
                {
                  DSI_LOG_ERROR("NULL callback found on store pointer" \
                                "[%p]", st);
                }
              }
            }
          }
          break;

        default:
          {
            DSI_LOG_VERBOSE("not processing embms_list_ind [%d]",
                             ind_data->embms_list.list_type);
            reti = DSI_ERROR;
          }
          break;
      }/* switch */
    }

    if(reti == DSI_ERROR)
    {
      break;
    }
    ret = DSI_SUCCESS;
  } while (0);

  if(ind_data && ind_data->embms_list.tmgi_list.tmgi_list_ptr)
  {
    DSI_LOG_DEBUG("%s", " free tmgi_list from indication");
    free(ind_data->embms_list.tmgi_list.tmgi_list_ptr);
    ind_data->embms_list.tmgi_list.tmgi_list_ptr = NULL;
  }

  if(NULL != payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list)
  {
    DSI_LOG_DEBUG("%s", " free tmgi_list in payload");
    free(payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list);
    payload.embms_tmgi_info.embms_tmgi_actdeact_params.tmgi_list = NULL;
  }
  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_VERBOSE("%s", "dsi_handle_embms_tmgi_list_ind: EXIT with suc");
  }
  else
  {
    DSI_LOG_VERBOSE("%s", "dsi_handle_embms_tmgi_list_ind: EXIT with err");
  }
  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_handle_embms_sai_list_ind
===========================================================================*/
/*!
    @brief
    handles embms sai list indications

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
static int dsi_handle_embms_sai_list_ind
(
  int                            wds_hndl,
  qmi_service_id_type            sid,
  void                          *user_data,
  qmi_wds_indication_data_type  *ind_data
)
{
  int                ret          = DSI_ERROR;
  int                i            = (int)(intptr_t)user_data;
  dsi_net_evt_t      event;
  dsi_store_t       *st           = NULL;
  dsi_evt_payload_t  payload;
  int                count;
  boolean            handle_found = FALSE;
  int temp_index;
  DSI_LOG_VERBOSE("%s", "dsi_handle_embms_sai_list_ind: ENTRY");

  memset(&payload, 0, sizeof(payload));

  do
  {
    DSI_LOG_DEBUG("received sai_list_ind with wds_hndl [0x%x] "
                  "sid [%d] user_data [%p] ind_data [%p]",
                  wds_hndl, sid, user_data, ind_data);

    if (NULL == ind_data)
    {
      DSI_LOG_ERROR("%s", "NULL ind data received in sai_list_ind");
      break;
    }

    if (!DSI_IS_ID_VALID(i))
    {
      DSI_LOG_ERROR("invalid user data (dsi iface id) "\
                    "%d received in sai_list_ind", i);
      break;
    }

    for(count = 0; count < DSI_MAX_DATA_CALLS; count++)
    {
      st = (dsi_store_t *)dsi_store_table[count].dsi_store_ptr;
      if (st != NULL && st->priv.dsi_iface_id == i)
      {
        handle_found = TRUE;
        break;
      }
    }
    if(FALSE == handle_found)
    {
      DSI_LOG_ERROR("No call state for index [%d]", count);
      break;
    }
    if( NULL == st->net_ev_cb )
    {
      DSI_LOG_ERROR("No event callback for index [%d]", count);
      break;
    }

    if(ind_data->sai_list.param_mask & QMI_WDS_EMBMS_LIST_IND_SAI_LIST_TRANX_ID_PARAM_MASK)
    {

      DSI_LOG_DEBUG("received sai_list_ind, dbg_trace_id [%d] ",
                     ind_data->sai_list.dbg_trace_id);
      payload.embms_sai_info.dbg_trace_id = ind_data->sai_list.dbg_trace_id;
    }
    if (ind_data->sai_list.param_mask & QMI_WDS_EMBMS_LIST_IND_SAI_LIST_PARAM_MASK)
    {
      event = DSI_NET_SAI_LIST_CHANGED;

      payload.embms_sai_info.available_sai_list_len =
         ind_data->sai_list.available_sai_list_len;

      if ( 0 != ind_data->sai_list.available_sai_list_len )
      {
        payload.embms_sai_info.available_sai_list =
           malloc(ind_data->sai_list.available_sai_list_len * sizeof(unsigned int));
        if ( NULL == payload.embms_sai_info.available_sai_list)
        {
          DSI_LOG_ERROR("%s", "Failed on allocate available_sai_list");
          break;
        }
        memcpy(payload.embms_sai_info.available_sai_list,
               ind_data->sai_list.available_sai_list,
               ind_data->sai_list.available_sai_list_len * sizeof(unsigned int));
      }

      payload.embms_sai_info.camped_sai_list_len =
         ind_data->sai_list.camped_sai_list_len;

      if ( 0 != ind_data->sai_list.camped_sai_list_len)
      {
        payload.embms_sai_info.camped_sai_list =
           malloc(ind_data->sai_list.camped_sai_list_len * sizeof(unsigned int));
        if ( NULL == payload.embms_sai_info.camped_sai_list)
        {
          DSI_LOG_ERROR("%s", "Failed on allocate camped_sai_list");
          break;
        }
        memcpy(payload.embms_sai_info.camped_sai_list,
               ind_data->sai_list.camped_sai_list,
               ind_data->sai_list.camped_sai_list_len * sizeof(unsigned int));
      }

      payload.embms_sai_info.num_sai_per_group_len =
         ind_data->sai_list.num_sai_per_group_len;

      if ( 0 != ind_data->sai_list.num_sai_per_group_len)
      {
        payload.embms_sai_info.num_sai_per_group =
           malloc(ind_data->sai_list.num_sai_per_group_len * sizeof(unsigned short));
        if ( NULL == payload.embms_sai_info.num_sai_per_group)
        {
          DSI_LOG_ERROR("%s", "Failed on allocate sai_per_group");
          break;
        }
        memcpy(payload.embms_sai_info.num_sai_per_group,
               ind_data->sai_list.num_sai_per_group,
               ind_data->sai_list.num_sai_per_group_len * sizeof(unsigned short));
      }

      for (temp_index = 0; temp_index < (ind_data->sai_list.available_sai_list_len); temp_index ++)
      {
         DSI_LOG_DEBUG("available_sai_list:%d", ind_data->sai_list.available_sai_list[temp_index] );
      }
      for (temp_index = 0; temp_index < (ind_data->sai_list.camped_sai_list_len); temp_index ++)
      {
         DSI_LOG_DEBUG("camped_sai_list:%d", ind_data->sai_list.camped_sai_list[temp_index]);
      }
      for (temp_index = 0; temp_index < (ind_data->sai_list.num_sai_per_group_len); temp_index ++)
      {
         DSI_LOG_DEBUG("num_sai_per_group:%d", ind_data->sai_list.num_sai_per_group[temp_index]);
      }

      for (temp_index = 0; temp_index < payload.embms_sai_info.available_sai_list_len; temp_index ++)
      {
         DSI_LOG_DEBUG("payload available_sai_list:%d", payload.embms_sai_info.available_sai_list[temp_index] );
      }
      for (temp_index = 0; temp_index < payload.embms_sai_info.camped_sai_list_len; temp_index ++)
      {
         DSI_LOG_DEBUG("payload camped_sai_list:%d", payload.embms_sai_info.camped_sai_list[temp_index]);
      }
      for (temp_index = 0; temp_index < payload.embms_sai_info.num_sai_per_group_len; temp_index ++)
      {
         DSI_LOG_DEBUG("payload num_sai_per_group:%d", payload.embms_sai_info.num_sai_per_group[temp_index]);
      }
      for(count = 0; count < DSI_MAX_DATA_CALLS; count++)
      {
        st = (dsi_store_t *)dsi_store_table[count].dsi_store_ptr;
        if (st != NULL && st->priv.dsi_iface_id == i)
        {
          if (NULL != st->net_ev_cb)
          {
            DSI_LOG_DEBUG("notifying event [%d] on handle [%p]",
                          event, st);
            st->net_ev_cb( (dsi_hndl_t *)st,
                          st->user_data,
                          event,
                          &payload  );
          }
          else
          {
            DSI_LOG_ERROR("NULL callback found on store pointer" \
                          "[%p]", st);
          }
        }
      }
    }

    ret = DSI_SUCCESS;
  } while (0);

  /* release memory */
  if(ind_data)
  {
    if( NULL != ind_data->sai_list.available_sai_list)
    {
      DSI_LOG_DEBUG("%s", " free available_sai_list from indication");
      free(ind_data->sai_list.available_sai_list);
      ind_data->sai_list.available_sai_list = NULL;
    }
    if ( NULL != ind_data->sai_list.camped_sai_list)
    {
      DSI_LOG_DEBUG("%s", " free camped_sai_list from indication");
      free(ind_data->sai_list.camped_sai_list);
      ind_data->sai_list.camped_sai_list = NULL;
    }
    if ( NULL != ind_data->sai_list.num_sai_per_group)
    {
      DSI_LOG_DEBUG("%s", " free num_sai_per_group from indication");
      free(ind_data->sai_list.num_sai_per_group);
      ind_data->sai_list.num_sai_per_group = NULL;
    }
  }

  if(NULL != payload.embms_sai_info.available_sai_list)
  {
    DSI_LOG_DEBUG("%s", " free available_sai_list in payload");
    free(payload.embms_sai_info.available_sai_list);
    payload.embms_sai_info.available_sai_list = NULL;
  }
  if (NULL != payload.embms_sai_info.camped_sai_list)
  {
    DSI_LOG_DEBUG("%s", " free camped_sai_list in payload");
    free(payload.embms_sai_info.camped_sai_list);
    payload.embms_sai_info.camped_sai_list = NULL;
  }
  if ( NULL != payload.embms_sai_info.num_sai_per_group)
  {
    DSI_LOG_DEBUG("%s", " free num_sai_per_group");
    free(payload.embms_sai_info.num_sai_per_group);
    payload.embms_sai_info.num_sai_per_group = NULL;
  }

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_VERBOSE("%s", "dsi_handle_embms_sai_list_ind: EXIT with suc");
  }
  else
  {
    DSI_LOG_VERBOSE("%s", "dsi_handle_embms_sai_list_ind: EXIT with err");
  }
  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_handle_embms_content_desc_control_ind
===========================================================================*/
/*!
    @brief
    handles embms content desc control indications

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
static int dsi_handle_embms_content_desc_control_ind
(
  int                            wds_hndl,
  qmi_service_id_type            sid,
  void                          *user_data,
  qmi_wds_indication_data_type  *ind_data
)
{
  int                ret          = DSI_ERROR;
  int                i            = (int)(intptr_t)user_data;
  dsi_net_evt_t      event;
  dsi_store_t       *st           = NULL;
  dsi_evt_payload_t  payload;
  int                count;
  boolean            handle_found = FALSE;
  DSI_LOG_VERBOSE("%s", "dsi_handle_embms_content_desc_control_ind: ENTRY");

  memset(&payload, 0, sizeof(payload));

  do
  {
    event = DSI_NET_CONTENT_DESC_CONTROL;

    DSI_LOG_DEBUG("received content_desc_control_ind with wds_hndl [0x%x] "
                  "sid [%d] user_data [%p] ind_data [%p]",
                  wds_hndl, sid, user_data, ind_data);

    if (NULL == ind_data)
    {
      DSI_LOG_ERROR("%s", "NULL ind data received in content_desc_control_ind");
      break;
    }

    if (!DSI_IS_ID_VALID(i))
    {
      DSI_LOG_ERROR("invalid user data (dsi iface id) "\
                    "%d received in content_desc_control_ind", i);
      break;
    }

    for(count = 0; count < DSI_MAX_DATA_CALLS; count++)
    {
      st = (dsi_store_t *)dsi_store_table[count].dsi_store_ptr;
      if (st != NULL && st->priv.dsi_iface_id == i)
      {
        handle_found = TRUE;
        break;
      }
    }
    if(FALSE == handle_found)
    {
      DSI_LOG_ERROR("No call state for index [%d]", count);
      break;
    }
    if( NULL == st->net_ev_cb )
    {
      DSI_LOG_ERROR("No event callback for index [%d]", count);
      break;
    }

    /* activate status indication should have ONLY 1 TMGI list */
    if(DSI_EMBMS_TMGI_STATUS_IND_TMGI_LIST_LEN !=
       (payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_len =
        ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_len))
    {
      DSI_LOG_ERROR("TMGI list length incorrect: [%d]\n",
                    ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_len);
      break;
    }
    if(DSI_SUCCESS != dsi_netctrl_copy_tmgi_list(
      &(payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_ptr),
      ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_ptr,
      ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_len))
    {
      DSI_LOG_ERROR("%s", "Failed on dsi_netctrl_copy_tmgi_list");
      break;
    }
    DSI_LOG_DEBUG("received content_desc_control_ind, TMGI %d:[%X,%X,%X,%X,%X,%X] ",
      payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_len,
      payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_ptr->tmgi[0],
      payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_ptr->tmgi[1],
      payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_ptr->tmgi[2],
      payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_ptr->tmgi[3],
      payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_ptr->tmgi[4],
      payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_ptr->tmgi[5]);

    if(ind_data->embms_content_desc_control.param_mask
        & QMI_WDS_EMBMS_CONTENT_DESC_CTRL_IND_TRANX_ID_PARAM_MASK)
    {

      DSI_LOG_DEBUG("received content_desc_control_ind, dbg_trace_id [%d] ",
                     ind_data->embms_content_desc_control.dbg_trace_id);
      payload.embms_content_desc_info.dbg_trace_id =
          ind_data->embms_content_desc_control.dbg_trace_id;
    }

    if(ind_data->embms_content_desc_control.param_mask
        & QMI_WDS_EMBMS_CONTENT_DESC_CTRL_IND_CONTENT_CTRL_PARAM_MASK)
    {

      DSI_LOG_DEBUG("received content_desc_control_ind, content_control [%d] ",
                     ind_data->embms_content_desc_control.content_control);
      payload.embms_content_desc_info.param_mask |=
          QMI_WDS_EMBMS_CONTENT_DESC_CTRL_IND_CONTENT_CTRL_PARAM_MASK;
      payload.embms_content_desc_info.content_control =
          ind_data->embms_content_desc_control.content_control;
    }

    if(ind_data->embms_content_desc_control.param_mask
        & QMI_WDS_EMBMS_CONTENT_DESC_CTRL_IND_STATUS_CTRL_PARAM_MASK)
    {

      DSI_LOG_DEBUG("received content_desc_control_ind, status_control [%d] ",
                     ind_data->embms_content_desc_control.status_control);
      payload.embms_content_desc_info.param_mask |=
          QMI_WDS_EMBMS_CONTENT_DESC_CTRL_IND_STATUS_CTRL_PARAM_MASK;
      payload.embms_content_desc_info.status_control =
          ind_data->embms_content_desc_control.status_control;
    }

    for(count = 0; count < DSI_MAX_DATA_CALLS; count++)
    {
      st = (dsi_store_t *)dsi_store_table[count].dsi_store_ptr;
      if (st != NULL && st->priv.dsi_iface_id == i)
      {
        if (NULL != st->net_ev_cb)
        {
          DSI_LOG_DEBUG("notifying event [%d] on handle [%p]",
                        event, st);
          st->net_ev_cb( (dsi_hndl_t *)st,
                         st->user_data,
                         event,
                         &payload  );
        }
        else
        {
          DSI_LOG_ERROR("NULL callback found on store pointer" \
                        "[%p]", st);
        }
      }
    }
    ret = DSI_SUCCESS;
  } while (0);

  /* make sure memory is released */
  if(ind_data && ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_ptr)
  {
    DSI_LOG_DEBUG("%s", "free tmgi list from indication");
    free(ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_ptr);
    ind_data->embms_content_desc_control.content_desc_tmgi.tmgi_list_ptr = NULL;
  }

  if(NULL != payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_ptr)
  {
    DSI_LOG_DEBUG("%s", " free tmgi_list in payload");
    free(payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_ptr);
    payload.embms_content_desc_info.content_desc_tmgi.tmgi_list_ptr = NULL;
  }

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_VERBOSE("%s", "dsi_handle_embms_content_desc_control_ind: EXIT with suc");
  }
  else
  {
    DSI_LOG_VERBOSE("%s", "dsi_handle_embms_content_desc_control_ind: EXIT with err");
  }
  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_handle_handoff_ind
===========================================================================*/
/*!
    @brief
    Handles WDS handoff indications

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
static int dsi_handle_handoff_ind
(
  int                          wds_hndl,
  qmi_service_id_type          sid,
  void                         * user_data,
  qmi_wds_indication_data_type * ind_data
)
{
  int                ret          = DSI_ERROR;
  int                i            = (int)(intptr_t)user_data;
  dsi_net_evt_t      event;
  dsi_store_t        *st          = NULL;
  dsi_evt_payload_t  payload;
  int                count;
  int                qmi_wds_hndl_v4, qmi_wds_hndl_v6;

  DSI_LOG_VERBOSE("%s", "dsi_handle_handoff_ind: ENTRY");

  memset(&payload, 0, sizeof(payload));

  do
  {
    DSI_LOG_DEBUG("received handoff_ind with wds_hndl [0x%x] "
                  "sid [%d] user_data [%p] ind_data [%p]",
                  wds_hndl, sid, user_data, ind_data);

    if (NULL == ind_data)
    {
      DSI_LOG_ERROR("%s", "NULL ind_data received handoff_ind");
      break;
    }

    if (!DSI_IS_ID_VALID(i))
    {
      DSI_LOG_ERROR("invalid user data (dsi iface id) "
                    "%d received in handoff_ind", i);
      break;
    }

    event = DSI_EVT_NET_HANDOFF;
    payload.handoff_info.handoff_type = ind_data->handoff_info.handoff_info_status;
    payload.handoff_info.source_rat = ind_data->handoff_info.source_rat;
    payload.handoff_info.target_rat = ind_data->handoff_info.target_rat;

    /* Determine the IP family for which the handoff was received */
    qmi_wds_hndl_v4 = qdi_get_qmi_wds_handle_for_ip(DSI_GET_QDI_HNDL(i),
                                                    QMI_IP_FAMILY_PREF_IPV4);
    qmi_wds_hndl_v6 = qdi_get_qmi_wds_handle_for_ip(DSI_GET_QDI_HNDL(i),
                                                    QMI_IP_FAMILY_PREF_IPV6);
    if (wds_hndl == qmi_wds_hndl_v4)
    {
      if ( 0 == dsi_iface_tbl[i].v4_ref_count)
      {
        DSI_LOG_ERROR("%s: V4 WDS_HANDOFF_IND got on handle with no V4 call. v4_ref_cnt[%d]",
                      __func__, dsi_iface_tbl[i].v4_ref_count);
        return ret;
      }
      DSI_LOG_DEBUG("%s:Finding dsi_hndl for V4 HANDOFF_IND", __func__ );
      payload.handoff_info.ip_type = QMI_IP_FAMILY_PREF_IPV4;
    }
    else if (wds_hndl == qmi_wds_hndl_v6 )
    {
      if ( 0 == dsi_iface_tbl[i].v6_ref_count)
      {
        DSI_LOG_ERROR("%s: V6 WDS_HANDOFF_IND got on handle with no V6 call. v6_ref_cnt[%d]",
                      __func__, dsi_iface_tbl[i].v6_ref_count);
        return ret;
      }
      DSI_LOG_DEBUG("%s:Finding dsi_hndl for V6 HANDOFF_IND", __func__ );
      payload.handoff_info.ip_type = QMI_IP_FAMILY_PREF_IPV6;
    }

    for (count = 0; count < DSI_MAX_DATA_CALLS; count++)
    {
      st = (dsi_store_t *) dsi_store_table[count].dsi_store_ptr;
      if (st != NULL && st->priv.dsi_iface_id == i)
      {
        if (NULL == st->net_ev_cb)
        {
          DSI_LOG_ERROR("No event callback for index [%d]", i);

          /* Continue with the loop as multiple clients may have
           * registered for handoff events */
          continue;
        }

        /* Post event on callback */
        DSI_LOG_DEBUG("notifying event [%d] on handle [%p]",
                      event, st);
        st->net_ev_cb((dsi_hndl_t * )st,
                      st->user_data,
                      event,
                      &payload );
      }
    }

    ret = DSI_SUCCESS;
  } while (0);

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_VERBOSE("%s", "dsi_handle_handoff_ind: EXIT with suc");
  }
  else
  {
    DSI_LOG_VERBOSE("%s", "dsi_handle_handoff_ind: EXIT with err");
  }
  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_process_wds_ind
===========================================================================*/
/*!
    @brief
    callback function registered for wds indications

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
void dsi_process_wds_ind
(
  int wds_hndl,
  qmi_service_id_type sid,
  void * user_data,
  qmi_wds_indication_id_type ind_id,
  qmi_wds_indication_data_type * ind_data
)
{
  int ret = DSI_ERROR;
  int reti = DSI_SUCCESS;

  DSI_LOG_VERBOSE("%s", "dsi_process_wds_ind: ENTRY");

  DSI_GLOBAL_LOCK;

  do
  {
    DSI_LOG_VERBOSE("received ind_id [%d] with wds_hndl [0x%x] "
                    "sid [%d] user_data [%p] ind_data [%p]",
                    ind_id, wds_hndl, sid, user_data, ind_data);

    reti = DSI_SUCCESS;
    switch(ind_id)
    {
    case QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG:
      reti = dsi_handle_pkt_srvc_ind(wds_hndl,
                                     sid,
                                     user_data,
                                     ind_data);
      break;
    case QMI_WDS_SRVC_EMBMS_TMGI_ACTIVATE_IND_MSG:
      reti = dsi_handle_embms_tmgi_activate_status_ind(wds_hndl,
                                                       sid,
                                                       user_data,
                                                       ind_data);
      break;
    case QMI_WDS_SRVC_EMBMS_TMGI_DEACTIVATE_IND_MSG:
      reti = dsi_handle_embms_tmgi_deactivate_status_ind(wds_hndl,
                                                         sid,
                                                         user_data,
                                                         ind_data);
      break;
    case QMI_WDS_SRVC_EMBMS_TMGI_ACT_DEACT_IND_MSG:
      reti = dsi_handle_embms_tmgi_act_deactivate_status_ind(wds_hndl,
                                                             sid,
                                                             user_data,
                                                             ind_data);
      break;
    case QMI_WDS_SRVC_EMBMS_TMGI_LIST_IND_MSG:
      reti = dsi_handle_embms_tmgi_list_ind(wds_hndl,
                                            sid,
                                            user_data,
                                            ind_data);
      break;
    case QMI_WDS_SRVC_EMBMS_SAI_LIST_IND_MSG:
      reti = dsi_handle_embms_sai_list_ind(wds_hndl,
                                           sid,
                                           user_data,
                                           ind_data);
      break;
    case QMI_WDS_SRVC_EMBMS_CONTENT_DESC_CONTROL_IND_MSG:
      reti = dsi_handle_embms_content_desc_control_ind(wds_hndl,
                                                       sid,
                                                       user_data,
                                                       ind_data);
      break;
    case QMI_WDS_SRVC_HANDOFF_INFORMATION_IND_MSG:
      reti = dsi_handle_handoff_ind(wds_hndl,
                                    sid,
                                    user_data,
                                    ind_data);
      break;

    default:
      DSI_LOG_VERBOSE("dsi_process_wds_ind: IGNORE: do not handle [%d]"
                      " indication as of now", ind_id);
      /* this is not an error, we just ignore this indication*/
      break;
    }
    if (DSI_ERROR == reti)
    {
      break;
    }
    ret = DSI_SUCCESS;
  } while (0);

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_VERBOSE("%s", "dsi_process_wds_ind: EXIT with suc");
  }
  else
  {
    DSI_LOG_VERBOSE("%s", "dsi_process_wds_ind: EXIT with err");
  }

  DSI_GLOBAL_UNLOCK;
}

/*===========================================================================
  FUNCTION:  dsi_process_qmi_sys_ind
===========================================================================*/
/*!
    @brief
    Processes QMI systerm indications in cb thrd context

    @return
    none
*/
/*=========================================================================*/
void dsi_process_qmi_sys_ind
(
  qmi_sys_event_type event_id,
  qmi_sys_event_info_type * event_info
)
{
  int modem_id, iface_id;
  boolean iface_found = FALSE;

 if ( dsi_config.single_qmux_ch_enabled == FALSE)
 {
  /* Try to find a match for dev_id in the SSR payload with one of the ifaces */
  for (modem_id = 0; modem_id < DSI_MAX_MODEMS; modem_id++)
  {
    for (iface_id=DSI_MODEM_GET_IFACE_START(modem_id);
         iface_id<=DSI_MODEM_GET_IFACE_END(modem_id);
         iface_id++)
    {
      const char *cid = NULL;

      if (NULL != (cid = DSI_GET_WDS_STR(iface_id)) &&
          0 == strcmp(cid, event_info->qmi_modem_service_ind.dev_id))
      {
        iface_found = TRUE;
        break;
      }
      else
      {
        DSI_LOG_VERBOSE("dsi_process_qmi_sys_ind: iface=%d string=%s didn't match with dev_id=%s",
                        iface_id,
                        DSI_GET_WDS_STR(iface_id),
                        event_info->qmi_modem_service_ind.dev_id);
      }
    }
  }

  if (TRUE != iface_found)
  {
    DSI_LOG_DEBUG("dsi_process_qmi_sys_ind: sys_evt=%d valid iface not found "
                  "for conn_id=%d dev_id=%s\n",
                  event_id,
                  event_info->qmi_modem_service_ind.conn_id,
                  event_info->qmi_modem_service_ind.dev_id);
    return;
  }

  switch(event_id)
  {
  case QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND:
    DSI_LOG_DEBUG("dsi_process_qmi_sys_ind: rcvd QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND on "
                  "conn_id=%d dev_id=%s iface=%d\n",
                  event_info->qmi_modem_service_ind.conn_id,
                  event_info->qmi_modem_service_ind.dev_id,
                  iface_id);
    dsi_modem_out_of_service_hdlr(iface_id);
    break;
  case QMI_SYS_EVENT_MODEM_IN_SERVICE_IND:
    DSI_LOG_DEBUG("dsi_process_qmi_sys_ind: rcvd QMI_SYS_EVENT_MODEM_IN_SERVICE_IND on "
                  "conn_id=%d dev_id=%s iface=%d\n",
                  event_info->qmi_modem_service_ind.conn_id,
                  event_info->qmi_modem_service_ind.dev_id,
                  iface_id);
    dsi_modem_in_service_hdlr(iface_id);
    break;
  default:
    DSI_LOG_DEBUG("do not handle: [%d] sys ind rcvd", event_id);
    break;
  }
 }
 else
 {
  int iface_id;
  const char* dev_id;
  ds_target_t target = ds_get_target();

  /* If the last event indication is the same as the newly received indication
       we don't process the event. As it has been processed for all ifaces already */
  if(dsi_config.modem_ssr_state == (int)event_id )
  {
    DSI_LOG_DEBUG("dsi_process_qmi_sys_ind: already processed event_id[%d], skipping",(int)event_id);
    return;
  }

  dev_id = event_info->qmi_modem_service_ind.dev_id;
  if (DS_TARGET_LE_MDM9X35 == target
      || DS_TARGET_LE_MDM9X25 == target
      || DS_TARGET_LE_MDM9X15 == target
      || DS_TARGET_LE_LEGACY == target)
  {
    DSI_LOG_DEBUG("Ignore SSR event on phys port: %s", dev_id);
    return;
  }
  /* Received a different event indication. Store new event indication
     to prevent duplicate event handling. */
  dsi_config.modem_ssr_state = event_id;

  switch(event_id)
  {
  case QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND:
    DSI_LOG_DEBUG("%s","dsi_process_qmi_sys_ind: rcvd QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND");
    for(iface_id = 0; iface_id < DSI_MAX_IFACES; ++iface_id )
    {
        DSI_LOG_DEBUG("dsi_process_qmi_sys_ind: calling dsi_modem_out_of_service_hdlr"
                      "for iface_id[%d]",iface_id);
        dsi_modem_out_of_service_hdlr(iface_id);
    }
    break;
  case QMI_SYS_EVENT_MODEM_IN_SERVICE_IND:
    DSI_LOG_DEBUG("%s","dsi_process_qmi_sys_ind: rcvd QMI_SYS_EVENT_MODEM_IN_SERVICE_IND");
    for(iface_id = 0; iface_id < DSI_MAX_IFACES; ++iface_id)
    {
      DSI_LOG_DEBUG("dsi_process_qmi_sys_ind: calling dsi_modem_in_service_hdlr"
                    "for iface_id[%d]",iface_id);
      dsi_modem_in_service_hdlr(iface_id);
    }
    break;
  default:
    DSI_LOG_DEBUG("do not handle: [%d] sys ind rcvd", event_id);
    break;
  }
 }
}
