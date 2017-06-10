/*!
  @file
  dsi_netctrl_mni.c

  @brief
  implements dsi_netctrl modem network interface functions

*/

/*===========================================================================

  Copyright (c) 2008-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
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
#include <string.h>
#include <stdlib.h>
#include "dsi_netctrli.h"
#include "qmi_wds_srvc.h"
#include "qmi_nas_srvc.h"
#include "qmi_qos_srvc.h"
#include "dsi_netctrl_cb_thrd.h"
#include "dsi_netctrl_multimodem.h"
#include "qmi_nas_srvc.h"
#include "qdi.h"
#include <stdint.h>
#include <linux/if.h>
#include <linux/rmnet_data.h>
#include <linux/msm_rmnet.h>

/* Default EPIDs for different transports  */
#define DSI_DEFAULT_EPID_MHI 4
#define DSI_DEFAULT_EPID_IPA 1

/*===========================================================================
  GLOBAL
===========================================================================*/
/*qmi message library handle*/
int qmi_handle = QMI_INVALID_CLIENT_HANDLE;

/*===========================================================================
  FUNCTION:  dsi_mni_start
===========================================================================*/
/*!
    @brief
    starts modem network interface
    i is dsi iface id
    st_hndl is store handle
    ce is a place holder for call end reason (OUT)

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mni_start
(
  int i,
  dsi_store_t * st_hndl
)
{
  int qmi_ret, qmi_err_code;
  qmi_wds_call_end_reason_type call_end_reason;
  int ret = DSI_ERROR;

  DSI_LOG_DEBUG( "%s", "dsi_mni_start: ENTRY" );
  DSI_L2S_ENTRY_PARAMS("dsi_hndl=0x%p iface_id:%d",
                       st_hndl, i);

  /* this do..while loop decides the overall return value
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    ret = DSI_ERROR;

    if (!DSI_IS_ID_VALID(i) ||
        !DSI_IS_HNDL_VALID(st_hndl))
    {
      DSI_LOG_ERROR("%s", "**programming err* invalid params received");
      break;
    }

    memset( &call_end_reason, 0x0, sizeof(call_end_reason) );

    /* Allocate a QDI call handle if it doesn't exist */
    if (QDI_INVALID_CALL_HANDLE == st_hndl->priv.qdi_call_hndl)
    {
      st_hndl->priv.qdi_call_hndl = qdi_alloc_call_handle(DSI_GET_QDI_HNDL(i));

      /* If the allocation failed, return failure */
      if (QDI_INVALID_CALL_HANDLE == st_hndl->priv.qdi_call_hndl)
      {
        DSI_LOG_ERROR("Failed to allocate QDI call handle for iface=%d",
                      i);
        break;
      }
    }

    /* reset call tech in the store store table. If the call was successful
       and packet service indication is later received with call connected
       status, a fresh call tech will be stored in the store table.
    */
    st_hndl->priv.call_tech = QMI_WDS_IFACE_NAME_NOT_REPORTED;

    qmi_ret = qdi_wds_start_nw_if(DSI_GET_QDI_HNDL(i),
                                  st_hndl->priv.qdi_call_hndl,
                                  &(st_hndl->priv.start_nw_params),
                                  st_hndl->priv.partial_retry,
                                  dsi_qmi_wds_cmd_cb,
                                  (void *)st_hndl,
                                  st_hndl->priv.rl_qmi_inst,
                                  &call_end_reason,
                                  &qmi_err_code);

    if(qmi_ret < 0)
    {
      dsi_ip_family_t ipf;

      /* For a Dual-IP call update both V4 and V6 call end reasons */
      if (DSI_IP_VERSION_4_6 == st_hndl->priv.ip_version)
      {
        for (ipf = DSI_IP_FAMILY_V4; ipf < DSI_NUM_IP_FAMILIES; ++ipf)
        {
          st_hndl->priv.ce_reason[ipf].reason_code =
            call_end_reason.call_end_reason_verbose.verbose_reason;
          st_hndl->priv.ce_reason[ipf].reason_type =
            dsi_map_qmi_to_dsi_ce_reason_type(
            call_end_reason.call_end_reason_verbose.verbose_reason_type);
        }

        DSI_LOG_ERROR("qdi_wds_start_nw_if failed on [%d] iface with"     \
                      "[%d] qmi err, dsi reason code/type [%d/%d], IPv4v6 call",
                      i,qmi_err_code,
                      st_hndl->priv.ce_reason[DSI_IP_FAMILY_V4].reason_code,
                      st_hndl->priv.ce_reason[DSI_IP_FAMILY_V4].reason_type);
      }
      else
      {
        ipf = (DSI_IP_VERSION_6 == st_hndl->priv.ip_version) ?
              DSI_IP_FAMILY_V6 : DSI_IP_FAMILY_V4;

        st_hndl->priv.ce_reason[ipf].reason_code =
          call_end_reason.call_end_reason_verbose.verbose_reason;
        st_hndl->priv.ce_reason[ipf].reason_type =
          dsi_map_qmi_to_dsi_ce_reason_type(
          call_end_reason.call_end_reason_verbose.verbose_reason_type);

        DSI_LOG_ERROR("qdi_wds_start_nw_if failed on [%d] iface with"     \
                      "[%d] qmi err, dsi reason code/type [%d/%d], ipf [%d]",
                      i,qmi_err_code,
                      st_hndl->priv.ce_reason[ipf].reason_code,
                      st_hndl->priv.ce_reason[ipf].reason_type,
                      ipf);
      }

      break;
    }

    /*
       store wds txn id for later abort if needed
       by virtue of the asynchronouse execution, the wds txn
       id and ref count by client are mutually exclusive.
       i.e. a client would either have a pending start wds
       txn, but when start is successfully executed, wds txn
       is finished, and client would acquire a ref count
    */
    DSI_LOCK_MUTEX(DSI_GET_MUTEX(i));
    if(st_hndl->priv.ip_version == DSI_IP_VERSION_4_6)
    {
      DSI_SET_WDS_V4_TXN(i, qmi_ret);
      DSI_SET_WDS_V6_TXN(i, qmi_ret);
    }
    else if(st_hndl->priv.ip_version == DSI_IP_VERSION_4)
    {
      DSI_SET_WDS_V4_TXN(i, qmi_ret);
    }
    else if(st_hndl->priv.ip_version == DSI_IP_VERSION_6)
    {
      DSI_SET_WDS_V6_TXN(i, qmi_ret);
    }
    DSI_UNLOCK_MUTEX(DSI_GET_MUTEX(i));

    ret = DSI_SUCCESS;
  } while (0);


  if (ret == DSI_SUCCESS)
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_start: EXIT with suc" );
  }
  else
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_start: EXIT with err" );
    /* delete ipv6 profile if one was ever created */
    dsi_delete_ipv6_profile(i);
  }

  DSI_L2S_EXIT_WITH_STATUS("dsi_hndl=0x%p", st_hndl);
  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_mni_stop
===========================================================================*/
/*!
    @brief
    stops modem network interface
    i is dsi iface id
    st_hndl is the store handle pointer

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mni_stop(int i, dsi_store_t * st_hndl)
{
  int ret = DSI_ERROR;
  int qmi_ret, qmi_err_code;
  qdi_mode_t stop_ip_mode = QDI_MODE_NOT_SET;

  DSI_LOG_DEBUG( "%s", "dsi_mni_stop: ENTRY" );

  /* this do..while loop decides the overall return value
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    ret = DSI_ERROR;

    if (NULL == st_hndl || !DSI_IS_ID_VALID(i))
    {
      DSI_LOG_ERROR("%s", "invalid params received");
      break;
    }

    if(DSI_IP_VERSION_4_6 == st_hndl->priv.ip_version )
    {
      if(0 == DSI_GET_V4_COUNT(i) &&
         0 == DSI_GET_V6_COUNT(i))
      {
        stop_ip_mode = QDI_MODE_V4V6;
      }
      else if ( 0 == DSI_GET_V4_COUNT(i))
      {
        stop_ip_mode = QDI_MODE_V4;
      }
      else if ( 0 == DSI_GET_V6_COUNT(i))
      {
        stop_ip_mode = QDI_MODE_V6;
      }
    }

    qmi_ret = qdi_wds_stop_nw_if(DSI_GET_QDI_HNDL(i),
                                 st_hndl->priv.qdi_call_hndl,
                                 dsi_qmi_wds_cmd_cb,
                                 stop_ip_mode,
                                 (void *)st_hndl,
                                 &qmi_err_code);
    if( 0 > qmi_ret )
    {
      DSI_LOG_ERROR("qmi stop nw if failed: iface=[%d] err=[%d]",
                    i,qmi_err_code);
      break;
    }

    /* we're done with this modem interface,
     * delete ipv6 profile if one ever existed */
    dsi_delete_ipv6_profile(i);

    ret = DSI_SUCCESS;
  } while (0);

  if (ret == DSI_ERROR)
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_stop: EXIT with err");
  }
  else
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_stop: EXIT with suc");
  }

  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_mni_abort_start
===========================================================================*/
/*!
    @brief
    aborts previous start request
    i is dsi iface id
    st_hndl is the store handle pointer

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mni_abort_start(int i, dsi_store_t * st_hndl)
{
  int ret = DSI_ERROR;
  int qmi_ret,qmi_ret6,qmi_err_code,qmi_err6_code;

  DSI_LOG_DEBUG( "%s", "dsi_mni_abort_start: ENTRY" );

  /* this do..while loop decides the overall return value
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    ret = DSI_ERROR;

    if (NULL == st_hndl || !DSI_IS_ID_VALID(i))
    {
      DSI_LOG_ERROR("%s", "invalid params received");
      break;
    }

    switch(st_hndl->priv.ip_version)
    {
      case DSI_IP_VERSION_4:
              qmi_ret = qdi_wds_abort(DSI_GET_QDI_HNDL(i),
                                       st_hndl->priv.qdi_call_hndl,
                                       DSI_GET_WDS_V4_TXN(i),
                                       NULL,
                                       NULL,
                                       &qmi_err_code );
      break;
      case DSI_IP_VERSION_6:
              qmi_ret = qdi_wds_abort(DSI_GET_QDI_HNDL(i),
                                       st_hndl->priv.qdi_call_hndl,
                                       DSI_GET_WDS_V6_TXN(i),
                                       NULL,
                                       NULL,
                                       &qmi_err_code );
      break;
      case DSI_IP_VERSION_4_6:
              qmi_ret = qdi_wds_abort(DSI_GET_QDI_HNDL(i),
                                       st_hndl->priv.qdi_call_hndl,
                                       DSI_GET_WDS_V4_TXN(i),
                                       NULL,
                                       NULL,
                                       &qmi_err_code );
              qmi_ret6 = qdi_wds_abort(DSI_GET_QDI_HNDL(i),
                                       st_hndl->priv.qdi_call_hndl,
                                       DSI_GET_WDS_V6_TXN(i),
                                       NULL,
                                       NULL,
                                       &qmi_err6_code );
              qmi_ret = qmi_ret | qmi_ret6;
      break;
    }

    /* check if txn was completed successfully */
    if(QMI_NO_ERR == qmi_ret)
    {
      /* For a Dual-IP call with one valid address, wait for the NET_PLATFORM_DOWN event */
      if (DSI_IP_VERSION_4_6 == st_hndl->priv.ip_version &&
          DSI_IS_ID_VALID(st_hndl->priv.dsi_iface_id) &&
          (DSI_IS_ADDR_VALID(st_hndl->priv.dsi_iface_id, DSI_IP_FAMILY_V4, iface_addr) ||
           DSI_IS_ADDR_VALID(st_hndl->priv.dsi_iface_id, DSI_IP_FAMILY_V6, iface_addr)))
      {
        DSI_LOG_DEBUG("%s", "successfully aborted START (Dual-IP), waiting for NET_PLATFORM_DOWN");
      }
      else
      {
        DSI_LOG_DEBUG("%s", "successfully aborted START, sending NET_NO_NET");

        /* detach interface from the call store handle */
        DSI_LOCK_MUTEX(&(st_hndl->priv.mutex));
        dsi_detach_dsi_iface(st_hndl);
        DSI_UNLOCK_MUTEX(&(st_hndl->priv.mutex));
        /* notify user that net is down */
        if (NULL != st_hndl->net_ev_cb)
        {
          st_hndl->net_ev_cb((dsi_store_t *)st_hndl,
                             st_hndl->user_data,
                             DSI_EVT_NET_NO_NET,
                             NULL);
        }
        else
        {
          DSI_LOG_ERROR("no callback set on st pointer [%p]", st_hndl);
        }
      }
    }
    else
    {
      DSI_LOG_ERROR("qmi abort failed: iface=[%d] qmi_ret=[%d]" \
                    "qmi_err_code=[%d]",i,qmi_ret,qmi_err_code);
      DSI_LOG_DEBUG("dsi_netctrl will try to stop the " \
                    "interface [%d] now", i);
      /* user had tried to stop the interface, hence
      * if abort failed above, try to stop it now */
      if (DSI_SUCCESS != dsi_mni_stop(i, st_hndl))
      {
        DSI_LOG_DEBUG("couldn't stop interface [%d]",i);
        break;
      }
      break;
    }

    ret = DSI_SUCCESS;
  } while (0);

  if (ret == DSI_SUCCESS)
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_abort_start: EXIT with suc" );
  }
  else
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_abort_start: EXIT with err" );
  }

  return ret;
}

#ifndef FEATURE_QCRIL_USE_QDP
/*===========================================================================
  FUNCTION:  dsi_mni_ipv6_profile
===========================================================================*/
/*!
    @brief
    this function creates an ipv6 profile
    with the start_nw_params on the modem
    side. It also modifies the start_nw_params
    with the newly created profile id
    i is a dsi interface id
    profile_id_ptr is a place holder

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
static int dsi_mni_ipv6_profile
(
  int i,
  qmi_wds_start_nw_if_params_type * start_nw_params,
  int dsi_tech,
  int * profile_id_ptr
)
{
  qmi_wds_profile_id_type profile_id;
  qmi_wds_profile_tech_type qmi_wds_profile_tech;
  qmi_wds_profile_params_type profile_params;
  int ret = DSI_ERROR;
  int qmi_err_code;

  do
  {

    memset(&profile_params, 0, sizeof(profile_params));
    memset(&profile_id, 0, sizeof(profile_id));

    if (NULL == start_nw_params)
    {
      DSI_LOG_ERROR("%s","NULL start_nw_params received");
      break;
    }

    if (start_nw_params->profile_index != 0)
    {
      DSI_LOG_DEBUG("3GPP profile [%d] is already provided by the client" \
                    "This profile will be used for IPv6 route look up",
                    start_nw_params->profile_index);

      /* deliberatly return SUCCESS here, so caller can continue */
      return DSI_SUCCESS;
    }

    /* set pdp_type to IPv6 family */
    profile_params.umts_profile_params.param_mask |=
      QMI_WDS_UMTS_PROFILE_PDP_TYPE_PARAM_MASK;
    /*
    profile_params.umts_profile_params.pdp_type =
      QMI_WDS_PDP_TYPE_IPV6;
    */
    /* use explicit value 0x02 until confirmed of
     * it being added to QMI message lib */
    profile_params.umts_profile_params.pdp_type =
      0x02;

    if (dsi_tech & DSI_RADIO_TECH_UMTS ||
        dsi_tech & DSI_RADIO_TECH_LTE  ||
        dsi_tech & DSI_RADIO_TECH_TDSCDMA)
    {
      qmi_wds_profile_tech = QMI_WDS_PROFILE_TECH_3GPP;
    }
    else
    {
      DSI_LOG_ERROR("%s","QMI does not support creating 3GPP2 profiles yet");
      /* deliberatly return SUCCESS here, so caller can continue with
       * parameter override for 3GPP2 */
      return DSI_SUCCESS;
    }

    profile_id.technology = qmi_wds_profile_tech;

    if (QMI_NO_ERR != qmi_wds_create_profile(
          DSI_GET_WDS_HNDL(i),
          &profile_id,
          &profile_params,
          &qmi_err_code))
    {
      DSI_LOG_ERROR("could not create wds profile. " \
                    "qmi returned [%d] qmi_err_code",
                    qmi_err_code);
      break;
    }
    DSI_LOG_DEBUG("IPv6 profile [%lud] created",
                  profile_id.profile_index);

    start_nw_params->params_mask |=
      QMI_WDS_START_NW_PROFILE_IDX_PARAM;
    start_nw_params->profile_index = profile_id.profile_index;

    *profile_id_ptr = profile_id.profile_index;

    ret = DSI_SUCCESS;
  } while (0);

  return ret;
}
#endif /* FEATURE_QCRIL_USE_QDP */

/*===========================================================================
  FUNCTION:  dsi_mni_look_up
===========================================================================*/
/*!
    @brief
    returns dsi_iface_id that matches
    with the parameters set on the store handle
    st_hndl is the store handle pointer
    pi is a place holder for dsi iface id (OUT)

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mni_look_up(dsi_store_t * st_hndl, int * pi)
{
  int ret = DSI_ERROR;
  int reti = DSI_SUCCESS;
  int technology = DSI_RADIO_TECH_UNKNOWN;
  int i = DSI_INVALID_IFACE;
  int route_look_up_modem;
  boolean iface_found = FALSE;
  int dsd_technology = DSI_RADIO_TECH_UNKNOWN;
  /* this local variable stores ipv6 profile id */
  int ipv6_profile = DSI_INVALID_PROFILE_INDEX;
  int ipv6_iface = DSI_INVALID_IFACE;
#ifndef FEATURE_DSI_MULTIMODEM_ROUTELOOKUP
  qmi_wds_route_look_up_params_type route_lookup_params;
  qmi_wds_route_look_up_rsp_type rsp_data;
  int qmi_ret, qmi_err_code, count;
  int modem_id;
#endif /* FEATURE_DSI_MULTIMODEM_ROUTELOOKUP */

  DSI_LOG_DEBUG( "%s", "dsi_mni_look_up: ENTRY" );

  if (NULL == pi || NULL == st_hndl)
  {
    DSI_LOG_ERROR("%s","invalid parameters");
    return DSI_ERROR;
  }
  /* this do..while loop decides the overall return value
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    ret = DSI_ERROR;

    iface_found = FALSE;
    /* map qmi tech to dsi tech if one provided */
    if (st_hndl->priv.start_nw_params.params_mask &
        QMI_WDS_START_NW_TECH_PREF_PARAM)
    {
      DSI_LOG_DEBUG("start_nw_params has tech_pref set to %d",
                    st_hndl->priv.start_nw_params.tech_pref);
      technology = dsi_map_qmi_to_dsi_tech(st_hndl->priv.
                                           start_nw_params.tech_pref);
      if (-1 == technology)
      {
        DSI_LOG_ERROR("qmi tech [%d] is currently not supported"
                      " by dsi_netctrl",
                      st_hndl->priv.start_nw_params.tech_pref);
        break;
      }
    }
    else if(DSI_RADIO_TECH_UNKNOWN != st_hndl->dsi_tech)
    {
      DSI_LOG_DEBUG("st_hndl has dsi_tech set to [%d]", st_hndl->dsi_tech);
      technology = st_hndl->dsi_tech;
    }
#ifdef FEATURE_DSI_MULTIMODEM_ROUTELOOKUP
#ifdef FEATURE_QCRIL_USE_QDP
    DSI_LOG_VERBOSE("%s","QCRIL is using QDP. No need to create IPv6 profiles");
#endif

    /* perform multi-modem route look up*/
    if (DSI_ERROR ==
        dsi_mm_iface_look_up(st_hndl,
                             &i,
                             &route_look_up_modem,
                             &dsd_technology))
    {
      DSI_LOG_ERROR("%s","dsi_mm_iface_look_up failed");
      break;
    }

    /* make sure multi-modem route look up technology
     * matches with user preferred technology */
    reti = DSI_SUCCESS;
    switch(technology)
    {
    case DSI_RADIO_TECH_UNKNOWN:
      DSI_LOG_DEBUG("user did not provide a technology preference " \
                    "by default dsd tech [0x%x] would be used",
                    dsd_technology);
      break;
    case DSI_RADIO_TECH_UMTS:
      /* EPC(if handoff is supported) or UMTS will be returned from MODEM */
      if (!(dsd_technology == DSI_PS_UMTS_IFACE ||
            dsd_technology == DSI_PS_EPC_IFACE))
      {
        DSI_LOG_ERROR("DSD preferred technology [0x%x] " \
                      "does not match with user preferred " \
                      "DSI_RADIO_TECH_UMTS", dsd_technology);
        reti = DSI_ERROR;
      }
      break;
    case DSI_RADIO_TECH_CDMA:
      /* EPC(if handoff is supported) or CDMA will be returned from MODEM */
      if (!(dsd_technology == DSI_PS_CDMA_SN_IFACE ||
            dsd_technology == DSI_PS_FMC_IFACE     ||
            dsd_technology == DSI_PS_EPC_IFACE))
      {
        DSI_LOG_ERROR("DSD preferred technology [0x%x] " \
                      "does not match with user preferred " \
                      "DSI_RADIO_TECH_CDMA", dsd_technology);
        reti = DSI_ERROR;
      }
      break;
    case DSI_RADIO_TECH_1X:
      if (!(
            (dsd_technology == DSI_PS_CDMA_SN_IFACE && route_look_up_modem == DSI_RADIO_3GMODEM) ||
            (dsd_technology == DSI_PS_FMC_IFACE)
            )
          )
      {
        DSI_LOG_ERROR("DSD preferred technology [0x%x] " \
                      "on modem [%d] does not match with " \
                      "user preferred DSI_RADIO_TECH_1X",
                      dsd_technology, route_look_up_modem);
        reti = DSI_ERROR;
      }
      break;
    case DSI_RADIO_TECH_DO:
      if (!(
            (dsd_technology == DSI_PS_CDMA_SN_IFACE && route_look_up_modem == DSI_RADIO_4GMODEM) ||
            (dsd_technology == DSI_PS_FMC_IFACE)
            )
          )
      {
        DSI_LOG_ERROR("DSD preferred technology [0x%x] " \
                      "on modem [%d] does not match with " \
                      "user preferred DSI_RADIO_TECH_DO",
                      dsd_technology, route_look_up_modem);
        reti = DSI_ERROR;
      }
      break;
    case DSI_RADIO_TECH_LTE:
      /* EPC will be returned from MODEM */
      if (dsd_technology != DSI_PS_EPC_IFACE)
      {
        DSI_LOG_ERROR("DSD preferred technology [0x%x] " \
                      "does not match with user preferred " \
                      "DSI_RADIO_TECH_LTE", dsd_technology);
        reti = DSI_ERROR;
      }
      break;
    case DSI_RADIO_TECH_TDSCDMA:
      /* EPC or UMTS will be returned from MODEM */
      if (!(dsd_technology == DSI_PS_EPC_IFACE ||
            dsd_technology == DSI_PS_UMTS_IFACE))
      {
        DSI_LOG_ERROR("DSD preferred technology [0x%x] " \
                      "does not match with user preferred " \
                      "DSI_RADIO_TECH_TDSCDMA", dsd_technology);
        reti = DSI_ERROR;
      }
      break;
    default:
      DSI_LOG_ERROR("we should never have reached here " \
                    "technology local variable stores [0x%x] " \
                    "that is not recognized", technology);
      reti = DSI_ERROR;
      break;
    }
    /* if switch loop broke with error, break from do..while */
    if(DSI_ERROR == reti)
    {
      break;
    }
#else
    else /* call multimodem module to find out preferred tech */
    {
      modem_id = dsi_mm_get_pref_modem();
      DSI_LOG_DEBUG("dsi_mm_get_pref_modem returned modem [%d]",
                    modem_id);
      i = dsi_find_any_valid_iface_on_modem(modem_id);
      if (-1 == i)
      {
        DSI_LOG_ERROR("no valid interface exists on modem "\
                      "[%d]", modem_id);
        break;
      }
      technology = DSI_MODEM_GET_TECH(modem_id);
      iface_found = TRUE;
    }

    /* find out which QMI port should be used for this tech */
    if (iface_found == FALSE)
    {
      i = dsi_find_dsi_id_by_tech(technology,DSI_FALSE);
      if (-1 == i)
      {
        DSI_LOG_ERROR("no interface exist that supports " \
                      "dsi tech [%d]", technology);
        break;
      }
    }

    /* store the modem we are going to issue route look up on
    * for future reference. this is the same modem we should issue
    * start network later on */
    route_look_up_modem = DSI_GET_MODEM(i);

    DSI_LOG_DEBUG("selected technology is [%d]", technology);
    DSI_LOG_DEBUG("dsi iface [%d] that is on modem [%d] " \
                  "will be used for route look up",
                  i, DSI_GET_MODEM(i));

#ifdef FEATURE_QCRIL_USE_QDP
    DSI_LOG_DEBUG("%s","QCRIL is using QDP. No need to create IPv6 profiles");
#else
    /* for IPv6, create a profile on the modem */
    if (st_hndl->priv.start_nw_params.ip_family_pref ==
        QMI_WDS_IP_FAMILY_PREF_IPV6)
    {
      /* create a UMTS IPv6 profile on the modem and
      * store the profile id in the start_nw_params */
      if (DSI_SUCCESS != dsi_mni_ipv6_profile(
            i,
            &(st_hndl->priv.start_nw_params),
            technology,
            &ipv6_profile))
      {
        DSI_LOG_ERROR("%s","could not create IPv6 profile");
        break;
      }
      else
      {
        /* store the iface that was used to create ipv6
         * profile into a temp local variable */
        ipv6_iface = i;
      }
    }
#endif
    if (DSI_ERROR ==
        dsi_prepare_route_lookup_params(st_hndl,
                                        &route_lookup_params))
    {
      DSI_LOG_ERROR("%s","dsi_prepare_route_lookup_params failed");
      break;
    }

    /* look up the interface via qmi */
    qmi_ret = qmi_wds_route_look_up(DSI_GET_WDS_HNDL(i),
                                    &route_lookup_params,
                                    &rsp_data,
                                    &qmi_err_code);

    if (QMI_NO_ERR != qmi_ret)
    {
      DSI_LOG_ERROR("qmi_wds_route_look_up failed on iface [%d] "\
                    "with err [%d][%d]",
                    i,qmi_ret, qmi_err_code);
      break;
    }

    /* figure out tech pref returned by modem */
    reti = DSI_SUCCESS;
    switch(rsp_data.tech_name)
    {
    case DSI_PS_CDMA_SN_IFACE:
    case DSI_PS_CDMA_AN_IFACE:
      technology = DSI_RADIO_TECH_CDMA;
      break;
    case DSI_PS_UMTS_IFACE:
      technology = DSI_RADIO_TECH_UMTS;
      break;
    case DSI_PS_EPC_IFACE:
      technology = (DSI_RADIO_TECH_CDMA == technology)? DSI_RADIO_TECH_DO : DSI_RADIO_TECH_LTE;
      break;
    default:
      DSI_LOG_ERROR("qmi_wds_route_lookup() returned tech [0x%x]" \
                    " that we don't support in dsi_netctrli", rsp_data.tech_name);
      reti = DSI_ERROR;
      break;
    }

    /* if switch loop broke with error, break from do..while */
    if(DSI_ERROR == reti)
    {
      break;
    }

    if(0 == (DSI_MODEM_GET_TECH(DSI_GET_MODEM(i)) & technology))
    {
      DSI_LOG_ERROR("route look up on modem [%d] returned technology [0x%x]" \
                    ", but modem [%d] isn't configured with technolgoy [%d]" \
                    " in dsi_netctrl_init code. Please change modem to technology" \
                    " mapping", route_look_up_modem, technology, route_look_up_modem,
                    technology);
      break;
    }

    /* bail out if Rm handle doesn't match the tech */
    if (rsp_data.qmi_inst_is_valid)
    {
      DSI_LOG_DEBUG("route_look_up returned RmNet QMI instance [%d]",
                    rsp_data.qmi_inst);
      i = dsi_find_dsi_id_by_qmi(rsp_data.qmi_inst, i);
      if (!DSI_IS_ID_VALID(i))
      {
        DSI_LOG_ERROR("couldn't find a dsi id that can"       \
                        " match with the qmi_inst [%d]",
                        rsp_data.qmi_inst);
        break;
      }
      DSI_LOG_DEBUG("route_look_up returned RmNet QMI instance [%d] maps to iface[%d]",
                    rsp_data.qmi_inst, i);
      if(0 == (DSI_MODEM_GET_TECH(DSI_GET_MODEM(i)) & technology))
      {
        DSI_LOG_ERROR("qmi_wds_route_look_up response: "           \
                        "technology [0x%x] and Rm handle (qmi_inst)"   \
                        " [%d] does not match",
                        rsp_data.tech_name, (int)rsp_data.qmi_inst);
        break;
      }
      st_hndl->priv.start_nw_params.iface_handle = rsp_data.iface_handle;
      /* Preserve QMI instance to know later if Modem interface was already UP */
      st_hndl->priv.rl_qmi_inst = (int)rsp_data.qmi_inst;
    }
    else if(rsp_data.iface_handle)
    {
      DSI_LOG_DEBUG("route_look_up returned only Um instance [0x%x]",
                    (int)rsp_data.iface_handle);

      /* try find an available interface on the modem that
      * we had used to look up the route on */
      iface_found = FALSE;

      if (st_hndl->priv.app_type == DSI_CALL_PARAM_APP_TYPE_IMS)
      {

       /* Sometimes two processes may try to do SNI over the same iface
        * due to a race condition in querying for available ifaces. This
        * issue is currently being seen with the IMS application where
        * both IMS and RIL processes see iface 0 as the valid iface for
        * SNI. To prevent we start iface search from last iface to the
        * first iface in case of IMS */
        for (count=DSI_MODEM_GET_IFACE_END(route_look_up_modem);
             count>=DSI_MODEM_GET_IFACE_START(route_look_up_modem);
             count--)
        {
          DSI_LOG_DEBUG("iface [%d] has wds hndl [%d] that has " \
                        "reserved bit set to [%d]",
                        count, DSI_GET_WDS_HNDL(count),
                        DSI_RESERVED(count));
          if (DSI_GET_WDS_HNDL(count) != DSI_INVALID_WDS_HNDL &&
              !DSI_RESERVED(count))
          {
            iface_found = TRUE;
            i = count;
            break;
          }
        }
      }
      else
      {
        /* For normal cases we do iface search from the first iface on
         * on the given modem */
        for (count=DSI_MODEM_GET_IFACE_START(route_look_up_modem);
             count<=DSI_MODEM_GET_IFACE_END(route_look_up_modem);
             count++)
        {
          DSI_LOG_DEBUG("iface [%d] has wds hndl [%d] that has " \
                        "reserved bit set to [%d]",
                        count, DSI_GET_WDS_HNDL(count),
                        DSI_RESERVED(count));
          if (DSI_GET_WDS_HNDL(count) != DSI_INVALID_WDS_HNDL &&
              !DSI_RESERVED(count))
          {
            iface_found = TRUE;
            i = count;
            break;
          }
        }
      }

      if (iface_found == FALSE)
      {
        DSI_LOG_ERROR("couldn't find an available dsi id " \
                      "that belongs to modem [%d], and has " \
                      "technology [%d] ", route_look_up_modem,
                      technology);
        break;
      }

      /* find a QMI port that is not-in-use and also supports
         the preferred technology */
      /*
      i = dsi_find_dsi_id_by_tech(technology,DSI_TRUE);
      if (!DSI_IS_ID_VALID(i))
      {
        DSI_LOG_ERROR("couldn't find a dsi id that can"       \
                        " match with the qmi_inst [%d]",
                        rsp_data.qmi_inst);
        reti = DSI_ERROR;
        break;
      }
      */
    }
    else
    {
      DSI_LOG_ERROR("%s",
                    "qmi_wds_route_look_up() neither returned " \
                    "Rm nor Um");
      break;
    }
#endif
    /* set ipv6 profile on the selected interface at the end
     * (i.e. only after everything is successful)*/
    DSI_SET_IPV6_PROFILE(i, ipv6_profile);

    ret = DSI_SUCCESS;
  } while (0);

  if (DSI_SUCCESS == ret)
  {
    *pi = i;
    DSI_LOG_DEBUG( "%s", "dsi_mni_look_up: EXIT with suc" );
  }
  else
  {
    *pi = DSI_INVALID_IFACE;
    DSI_LOG_DEBUG( "%s", "dsi_mni_look_up: EXIT with err" );
    /* following function would delete an ipv6 profile
     * if one was ever created on the given ipv6_iface */
    if (ipv6_profile != DSI_INVALID_PROFILE_INDEX)
    {
      /* we must have stored an ifafe with valid wds hndl
       * in ipv6_iface, hence use it*/
      dsi_delete_ipv6_profile(ipv6_iface);
    }
  }

  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_mni_release_client
===========================================================================*/
/*!
    @brief
    This function releases the modem network interface handles

    @return
    none
*/
/*=========================================================================*/
void
dsi_mni_release_client(int conn_id)
{
  int qmi_err = QMI_SERVICE_ERR_NONE;
  int qmi_ret;

  DSI_LOG_DEBUG("%s","dsi_mni_release_client: ENTRY");
  if (qmi_handle < 0)
  {
    DSI_LOG_ERROR("%s","QMI message library was never initialized. "
                                                  "invalid qmi handle. ");
    return;
  }

  if( DSI_IS_WDS_HNDL_VALID(conn_id) )
  {
    DSI_LOG_DEBUG("releasing qdi hndl [%x]", DSI_GET_QDI_HNDL(conn_id));
    qmi_ret = qdi_wds_srvc_release_client(DSI_GET_QDI_HNDL(conn_id), &qmi_err);
    if (qmi_ret < 0)
    {
      DSI_LOG_ERROR("QDI srvc release failed. qmi_ret=[%d],qmi_err=[%d]",
                    qmi_ret,qmi_err);
    }

    DSI_SET_QDI_HNDL(conn_id, QDI_INVALID_CLIENT_HANDLE);
    DSI_SET_WDS_HNDL(conn_id, QMI_INVALID_CLIENT_HANDLE);
  }

  if( DSI_IS_QOS_HNDL_VALID(conn_id) )
  {
    DSI_LOG_DEBUG("releasing qos hndl [%#08x]", DSI_GET_QOS_HNDL(conn_id));
    qmi_ret = qmi_qos_srvc_release_client(DSI_GET_QOS_HNDL(conn_id), &qmi_err);
    if (qmi_ret < 0)
    {
      DSI_LOG_ERROR("qos srvc release failed. qmi_ret=[%d],qmi_err=[%d]",
                    qmi_ret,qmi_err);
    }

    DSI_SET_QOS_HNDL(conn_id, QMI_INVALID_CLIENT_HANDLE);
  }

  DSI_LOG_DEBUG("%s","dsi_mni_release_client: EXIT");
}
/*===========================================================================
  FUNCTION:  dsi_mni_cleanup
===========================================================================*/
/*!
    @brief
    This function releases the QMI WDS service and associated clients.

    @return
    none
*/
/*=========================================================================*/
void
dsi_mni_cleanup(void)
{

  int i = 0;
  int qmi_err;
  DSI_LOG_DEBUG("%s","dsi_mni_cleanup: ENTRY");

  for(i=0; i<DSI_MAX_IFACES; i++)
  {
    dsi_mni_release_client(i);
  }

  for(i=0; i<DSI_MAX_MODEMS; i++)
  {
    if(DSI_MODEM_GET_DEFAULT_WDS_HNDL(i) != DSI_INVALID_WDS_HNDL)
    {
      qmi_wds_srvc_release_client(DSI_MODEM_GET_DEFAULT_WDS_HNDL(i), &qmi_err);
      DSI_MODEM_SET_DEFAULT_WDS_HNDL(i, DSI_INVALID_WDS_HNDL);
    }
  }

  for (i = 0; i < DSI_MAX_DATA_CALLS; i++)
  {
    /* clear store table (call) entry */
    dsi_cleanup_store_tbl(i);
  }

  /* De-initialize QDI */
  if ( QDI_SUCCESS != qdi_release())
  {
    DSI_LOG_ERROR("%s", "QDI release failed");
  }
  qmi_release(qmi_handle);

#ifndef DSI_NETCTRL_OFFTARGET
  Diag_LSM_DeInit();
#endif

  DSI_LOG_DEBUG("%s","dsi_mni_cleanup: EXIT");
}

/*===========================================================================
  FUNCTION:  dsi_at_exit_hdlr
===========================================================================*/
/*!
    @brief
    This function releases the resources when main thread is exiting.

    @return
    none
*/
/*=========================================================================*/
static void
dsi_at_exit_hdlr(void)
{
  /* release QMI resources */
  dsi_mni_cleanup();
  /* kill and clean up callback thread */
  dsi_netctrl_cb_deinit();
}

/*===========================================================================
  FUNCTION:  dsi_get_epid
===========================================================================*/
/*!
    @brief
    This function returns the EPID for the physical network device provided.

    @params[in] net_dev: Physical network device
    @params[out] ep_type: The peripheral endpoint type
    @params[out] epid: End point ID obtained through IOCTL.

    @return
    none
*/
/*=========================================================================*/
static void dsi_get_epid
(
  char *net_dev,
  qmi_wds_per_ep_type *ep_type,
  long unsigned int *epid
)
{
  int fd;
  int rval = NETMGR_FAILURE;
  struct ifreq ifr;
  struct rmnet_ioctl_extended_s *ext_ioctl_arg;

  if (NULL == net_dev || NULL == ep_type || NULL == epid)
  {
    DSI_LOG_ERROR("%s", "dsi_get_epid(): Invalid args");
    return;
  }

  DSI_LOG_DEBUG("dsi_get_epid on net dev %s", net_dev);

  /* EP Type is dependent upon underlying rmnet transport */
  if (!strcmp(DSI_PHYS_NET_DEV_RMNET_MHI0, net_dev))
  {
    *ep_type = QMI_WDS_PER_EP_TYPE_PCIE;
  }
  else if (!strcmp(DSI_PHYS_NET_DEV_RMNET_IPA0, net_dev))
  {
    *ep_type = QMI_WDS_PER_EP_TYPE_EMBEDDED;
  }
  else if (!strcmp(DSI_PHYS_NET_DEV_RMNET_BAM0, net_dev))
  {
    /* There is no epid associated for bam transport */
    *ep_type = QMI_WDS_PER_EP_TYPE_BAM_DMUX;
  }

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    DSI_LOG_ERROR("%s", "get_epid: socket failed");
    goto bail;
  }

  /* Allocate the extended IOCTL argument */
  ext_ioctl_arg = (void *) malloc (sizeof(struct rmnet_ioctl_extended_s));
  if (NULL == ext_ioctl_arg) {
    DSI_LOG_ERROR("%s", "get_epid: malloc for ifru_data failed:");
    close(fd);
    goto bail;
  }

  /* Initialize the ioctl req struct */
  memset(&ifr, 0, sizeof(ifr));
  (void)strlcpy(ifr.ifr_name, net_dev, sizeof(ifr.ifr_name));
  ifr.ifr_ifru.ifru_data = ext_ioctl_arg;
  memset(ext_ioctl_arg, 0, sizeof(struct rmnet_ioctl_extended_s));
  ext_ioctl_arg->extended_ioctl = RMNET_IOCTL_GET_EPID;

  /* Get current if flags for the device */
  if (ioctl(fd, RMNET_IOCTL_EXTENDED, &ifr) < 0) {
    DSI_LOG_ERROR("%s", "get_epid: ioctl RMNET_IOCTL_GET_EPID failed");
    close(fd);
    free(ext_ioctl_arg);
    goto bail;
  }

  *epid = ext_ioctl_arg->u.data;
  DSI_LOG_INFO("get_epid: ioctl RMNET_IOCTL_GET_EPID as %d",ext_ioctl_arg->u.data);

  /* close temporary socket */
  close(fd);
  free(ext_ioctl_arg);
  return;

bail:
  DSI_LOG_ERROR("%s", "get_epid failed, using defaults");
  if (!strcmp(DSI_PHYS_NET_DEV_RMNET_MHI0, net_dev))
  {
    *ep_type = QMI_WDS_PER_EP_TYPE_PCIE;
    *epid = DSI_DEFAULT_EPID_MHI;
  }
  else if(!strcmp(DSI_PHYS_NET_DEV_RMNET_IPA0, net_dev))
  {
    *ep_type = QMI_WDS_PER_EP_TYPE_EMBEDDED;
    *epid = DSI_DEFAULT_EPID_IPA;
  }
  else
  {
    *ep_type = 0;
    *epid = 0;
  }
  return;

}


/*===========================================================================
  FUNCTION:  dsi_mni_init_client
===========================================================================*/
/*!
    @brief
    This function initializes the modem network interface clients

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mni_init_client(int conn_id)
{
  int i = 0;
  int ret = DSI_ERROR;
  int reti = DSI_ERROR;
  int qmi_err_code = QMI_NO_ERR;
  int qmi_ret;
  qmi_wds_bind_mux_data_port_params_type *bind_params = NULL;
  int subs_id;
  DSI_LOG_DEBUG( "%s", "dsi_mni_init_client: ENTRY" );

  /* this do..while loop decides the overall return value
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    ret = DSI_ERROR;

    /* initialize dsi interfaces for this modem */
    DSI_LOG_DEBUG("dsi_mni_init_client: init iface [%d]: START", conn_id);

    DSI_LOG_DEBUG("dsi_mni_init_client: init qmi connection [%s]",
                  DSI_GET_WDS_STR(conn_id));
    /* initialize qmi connection with the given qmi port name */
    qmi_ret = qmi_connection_init(DSI_GET_WDS_STR(conn_id),
                                  &qmi_err_code);
    if (QMI_NO_ERR != qmi_ret)
    {
      DSI_LOG_ERROR(
        "dsi_mni_init_client: qmi_connection_init failed with error %d",
        qmi_err_code);
      /* do not set reti to DSI_ERROR as we want to ignore
       * when wds client init doesn't succeed */
    }

    DSI_LOG_DEBUG("dsi_mni_init_client: init wds srvc client"
                  "with name [%s]", DSI_GET_WDS_STR(conn_id));
    /* Poplulate bind parameters accordingly for single_qmux_ch_enabled case */
    if(dsi_config.single_qmux_ch_enabled == DSI_TRUE)
    {
      bind_params = (qmi_wds_bind_mux_data_port_params_type*) malloc(sizeof(qmi_wds_bind_mux_data_port_params_type));
      if( NULL == bind_params)
      {
        DSI_LOG_FATAL("%s"," Insufficient memory while allocating mux data port parameter");
      }
      else
      {
        memset(bind_params, 0, sizeof(qmi_wds_bind_mux_data_port_params_type));
        dsi_get_epid(dsi_config.phys_net_dev, &bind_params->ep_id.ep_type, &bind_params->ep_id.iface_id);
        if( bind_params->ep_id.ep_type != QMI_WDS_PER_EP_TYPE_RESERVED )
        {
          /* Got a valid ep_type and ep_id */
          DSI_LOG_DEBUG("Got ep_type %d, EPID %d, from %s",
              bind_params->ep_id.ep_type, bind_params->ep_id.iface_id, dsi_config.phys_net_dev);
          bind_params->params_mask |= QMI_WDS_BIND_MUX_DATA_PORT_PARAMS_EP_ID;
        }
        else
        {
          DSI_LOG_DEBUG("%s","Ignoring epid and ep_type params");
        }

        bind_params->params_mask |= QMI_WDS_BIND_MUX_DATA_PORT_PARAMS_MUX_ID;
        bind_params->mux_id = (uint8_t) (conn_id + 1);
      }
    }

    subs_id = dsi_get_modem_subs_id();

    /* obtain qmi_wds service client for the given qmi port */
    qmi_ret = qdi_wds_srvc_init_client(DSI_GET_WDS_STR(conn_id),
                                       DSI_GET_DEV_STR(conn_id),
                                       dsi_qmi_wds_ind_cb,
                                       (void *)(intptr_t)conn_id,
                                       bind_params,
                                       subs_id,
                                       &qmi_err_code);

    /* Clean up bind parameters */
    if( NULL != bind_params)
    {
      free(bind_params);
    }

    if (qmi_ret < 0)
    {
      DSI_LOG_ERROR(
        "dsi_mni_init_clients: qdi_wds_srvc_init_client failed with error "
        "[%d][%d]",
        qmi_ret, qmi_err_code);
      /* do not set reti to DSI_ERROR as we want to ignore
       * when wds client init doesn't succeed */
    }

    /* cache qdi handle for this dsi interface */
    DSI_SET_QDI_HNDL(conn_id, qmi_ret);
    DSI_LOG_DEBUG("qdi hndl is [0x%08x]",(unsigned int)DSI_GET_QDI_HNDL(conn_id));

    /* retrieve the default qmi handle from QDI */
    DSI_SET_WDS_HNDL(conn_id, qdi_get_qmi_wds_handle(qmi_ret));
    DSI_LOG_DEBUG("wds hndl is [0x%08x]",(unsigned int)DSI_GET_WDS_HNDL(conn_id));


    /* configure QOS if feature enabled */
    if( DSI_TRUE == dsi_config.qos_enable )
    {
      /* obtain qmi_qos service client for the given qmi port.
       * Use the device string to allow for automatic client binding if reqd */
      qmi_ret = qmi_qos_srvc_init_client( DSI_GET_DEV_STR(conn_id),
                                          dsi_qmi_qos_ind_cb,
                                          (void *)(intptr_t)conn_id,
                                          &qmi_err_code );
      if (qmi_ret < 0)
      {
        if( QMI_SERVICE_ERR_INVALID_SERVICE_TYPE == qmi_err_code )
        {
          DSI_LOG_DEBUG( "%s", "dsi_qos_init: QOS service not supported on Modem" );
        }
        else
        {
          DSI_LOG_ERROR(
            "dsi_qos_init: qmi_qos_srvc_init_client failed with error [%d][%d]",
            qmi_ret, qmi_err_code);
        }
        /* Invalidate QOS handle to prevent flow operations */
        DSI_SET_QOS_HNDL(conn_id, DSI_INVALID_QOS_HNDL );
      }
      else
      {
        /* cache wds handle for this dsi interface */
        DSI_SET_QOS_HNDL(conn_id, qmi_ret);
        DSI_LOG_DEBUG("qos hndl is [0x%08x]",(unsigned int)DSI_GET_QOS_HNDL(conn_id));

        if (QMI_NO_ERR != qmi_qos_bind_subscription ( DSI_GET_QOS_HNDL(conn_id),
                                                      subs_id,
                                                      &qmi_err_code))
        {
          DSI_LOG_ERROR("%s","dsi_qos_init: subs_id binding failed");
        }


      }
    }
    else
    {
      DSI_SET_QOS_HNDL(conn_id, DSI_INVALID_QOS_HNDL );
    }

    DSI_LOG_DEBUG("dsi_mni_init_client: init iface [%d]: END", conn_id);

#ifndef FEATURE_DSI_MULTIMODEM_ROUTELOOKUP
    /* init nas service only after we have called qmi_connection_init */
    nas_clnt_id = qmi_nas_srvc_init_client(
      DSI_GET_WDS_DEV_STR(DSI_NAS_IFACE_ID),
      NULL, NULL, &qmi_err_code);
    if (nas_clnt_id < 0)
    {
      DSI_LOG_ERROR("%s","dsi_mni_init_clients: couldn't init nas clnt id");
      break;
    }
#endif
    ret = DSI_SUCCESS;
  } while (0);

  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_mni_init
===========================================================================*/
/*!
    @brief
    This function initializes the QMI WDS service and corresponding
    wds handles

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mni_init()
{
  int ret = DSI_ERROR;
  int reti = DSI_ERROR;
  int modem, default_iface;
  int qmi_ret, qmi_err_code;
  int wds_hndl;

  DSI_LOG_DEBUG( "%s", "dsi_mni_init: ENTRY" );

  /* this do..while loop decides the overall return value
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    /* Store the opaque qmi handle in the global variable,
     * which will later be used by qmi_release() in
     * clean up routine
     */
    qmi_handle = qmi_init(dsi_qmi_sys_cb,NULL);

    if(qmi_handle < 0)
    {
      DSI_LOG_ERROR( "%s",
                     "dsi_init: qmi_init failed" );
      /* deliberately returning instead of breaking out of
      * do..while loop. We do not want to call dsi_mni_cleanup
      * that attempts to qmi_release() */
      return ret;
    }

    if (QDI_SUCCESS != qdi_init())
    {
      DSI_LOG_ERROR("%s","qdi_init failed");
      break;
    }

    for (modem=0; modem < DSI_MAX_MODEMS; modem++)
    {
      default_iface = DSI_MODEM_GET_DEFAULT_IFACE(modem);

      qmi_ret = qmi_connection_init(DSI_GET_WDS_STR(default_iface),
                                    &qmi_err_code);
      if (QMI_NO_ERR != qmi_ret)
      {
        DSI_LOG_ERROR("qmi_connection_init failed with error [%d][%d]",
                      qmi_ret, qmi_err_code);
        continue;
      }

      wds_hndl = qmi_wds_srvc_init_client(DSI_GET_WDS_STR(default_iface),
                                                     NULL,
                                                     NULL,
                                                     &qmi_err_code);

      if (wds_hndl < 0)
      {
        DSI_LOG_ERROR("qmi_wds_srvc_init_client failed with error [%d]",qmi_err_code);
        continue;
      }


      DSI_MODEM_SET_DEFAULT_WDS_HNDL(modem,wds_hndl);

      DSI_LOG_DEBUG( "wds_srvc_init for %d modem: %d wds_hndl",modem, wds_hndl);

    }

    if((DSI_MODEM_GET_DEFAULT_WDS_HNDL(DSI_RADIO_4GMODEM) == DSI_INVALID_WDS_HNDL ) &&
        (DSI_MODEM_GET_DEFAULT_WDS_HNDL(DSI_RADIO_3GMODEM)) == DSI_INVALID_WDS_HNDL)
    {
      ret = DSI_ERROR;
    }
    else
    {
      ret = DSI_SUCCESS;
    }


  } while (0);

  /* print appropriate status before exiting */
  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_init: EXIT with success" );
    atexit(dsi_at_exit_hdlr);
  }
  else
  {
    /* release any non-NULL wds handles and release qmi */
    dsi_mni_cleanup();
    DSI_LOG_DEBUG( "%s", "dsi_mni_init: EXIT with error" );
  }

  return ret;
}


/*===========================================================================
  FUNCTION: dsi_mni_reg_unreg_handoff_ind
===========================================================================*/
/*!
  @brief
  This function is used to register/unregister for Handoff indications

  @return
  DSI_ERROR
  DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mni_reg_unreg_handoff_ind
(
  int           i,
  dsi_store_t * st_hndl,
  boolean       reg_unreg
)
{
  int                              ret = DSI_ERROR;
  int                              reti = DSI_SUCCESS;
  int                              qmi_wds_hndl_v4, qmi_wds_hndl_v6, qmi_ret;
  int                              qmi_err_code_v4, qmi_err_code_v6;
  int                              ip_type = st_hndl->priv.start_nw_params.ip_family_pref;
  qmi_wds_indication_reg_req_type  ind_type;

  DSI_LOG_DEBUG("%s", "dsi_mni_reg_unreg_handoff_ind: ENTRY");

  do
  {
    if (!DSI_IS_ID_VALID(i) ||
        !DSI_IS_HNDL_VALID(st_hndl))
    {
      DSI_LOG_ERROR("%s", "**programming err* invalid params received");
      break;
    }

    qmi_wds_hndl_v4 = qdi_get_qmi_wds_handle_for_ip(DSI_GET_QDI_HNDL(i),
                                                    QMI_IP_FAMILY_PREF_IPV4);
    qmi_wds_hndl_v6 = qdi_get_qmi_wds_handle_for_ip(DSI_GET_QDI_HNDL(i),
                                                    QMI_IP_FAMILY_PREF_IPV6);

    /* Mark the indications to register for */
    ind_type.param_mask = 0;
    ind_type.param_mask |= QMI_WDS_HANDOFF_INDICATION_REG_PARAM_MASK;
    ind_type.handoff_pref = (TRUE == reg_unreg) ?
      QMI_WDS_HANDOFF_IND_REPORT
      : QMI_WDS_HANDOFF_IND_NO_REPORT;

    /* Register for handoff events */
    switch (ip_type)
    {
    case QMI_WDS_IP_FAMILY_PREF_IPV4:
      {
        if (DSI_INVALID_WDS_HNDL == qmi_wds_hndl_v4)
        {
          DSI_LOG_ERROR("invalid WDS handle for iface=%d", i);
          reti = DSI_ERROR;
          break;
        }

        /* V4 Indication */
        qmi_ret = qmi_wds_indication_register(qmi_wds_hndl_v4,
                                              ind_type,
                                              &qmi_err_code_v4);
        if(qmi_ret < 0)
        {
          DSI_LOG_ERROR("qmi_wds_indication_register for V4 failed: iface=[%d] err=[%d]",
                      i, qmi_err_code_v4);
          reti = DSI_ERROR;
          break;
        }
      }
      break;
    case QMI_WDS_IP_FAMILY_PREF_IPV6:
      {
        if (DSI_INVALID_WDS_HNDL == qmi_wds_hndl_v6)
        {
          DSI_LOG_ERROR("invalid WDS handle for iface=%d", i);
          reti = DSI_ERROR;
          break;
        }

        /* V6 Indication */
        qmi_ret = qmi_wds_indication_register(qmi_wds_hndl_v6,
                                            ind_type,
                                            &qmi_err_code_v6);
        if(qmi_ret < 0)
        {
          DSI_LOG_ERROR("qmi_wds_indication_register for V6 failed: iface=[%d] err=[%d]",
                      i, qmi_err_code_v6);
          reti = DSI_ERROR;
          break;
        }
      }
      break;
    case QMI_WDS_IP_FAMILY_PREF_UNSPECIFIED:
      {
        if (DSI_INVALID_WDS_HNDL == qmi_wds_hndl_v4
            || DSI_INVALID_WDS_HNDL == qmi_wds_hndl_v6)
        {
          DSI_LOG_ERROR("invalid WDS handle for iface=%d", i);
          reti = DSI_ERROR;
          break;
        }

        qmi_ret = qmi_wds_indication_register(qmi_wds_hndl_v4,
                                              ind_type,
                                              &qmi_err_code_v4);

        if(qmi_ret < 0)
        {
          DSI_LOG_ERROR("qmi_wds_indication_register for V4/V6 failed: iface=[%d] err=[%d]",
                      i, qmi_err_code_v4);
          reti = DSI_ERROR;
          break;
        }

        qmi_ret = qmi_wds_indication_register(qmi_wds_hndl_v6,
                                              ind_type,
                                              &qmi_err_code_v6);

        if(qmi_ret < 0)
        {
          DSI_LOG_ERROR("qmi_wds_indication_register for V4/V6 failed: iface=[%d] err=[%d]",
                      i, qmi_err_code_v6);
          reti = DSI_ERROR;
          break;
        }
      }
      break;

    default:
      {
        DSI_LOG_ERROR("%s", "qmi_wds_indication_register: Unknown IP type!");
        reti = DSI_ERROR;
      }
      break;
    }

    if (DSI_SUCCESS != reti)
    {
      break;
    }

    ret = DSI_SUCCESS;
  } while (0);

  if (ret == DSI_ERROR)
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_reg_unreg_handoff_ind: EXIT with err");
  }
  else
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_reg_unreg_handoff_ind: EXIT with suc");
  }

  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_mni_register_embms_ind
===========================================================================*/
/*!
    @brief
    Register different WDS indication

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mni_register_embms_ind(int i, dsi_store_t * st_hndl)
{
  int                               ret = DSI_ERROR;
  int                               qmi_ret, qmi_err_code;
  qmi_wds_indication_reg_req_type   ind_type;

  DSI_LOG_DEBUG( "%s", "dsi_mni_register_embms_ind: ENTRY" );

  /* this do..while loop decides the overall return value
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    if (!DSI_IS_ID_VALID(i) ||
        !DSI_IS_HNDL_VALID(st_hndl))
    {
      DSI_LOG_ERROR("%s", "**programming err* invalid params received");
      break;
    }

    /* register tmgi list indication */
    ind_type.param_mask       = 0;
    ind_type.param_mask      |= QMI_WDS_EMBMS_TMGI_INDICATION_REG_LIST_PARAM_MASK;

    ind_type.tmgi_list_pref   = QMI_WDS_EMBMS_TMGI_IND_REPORT;

    /* register sai list indication */
    ind_type.param_mask      |= QMI_WDS_EMBMS_SAI_INDICATION_REG_LIST_PARAM_MASK;
    ind_type.sai_list_pref = QMI_WDS_EMBMS_SAI_IND_REPORT;

    /* register control desc control indication */
    ind_type.param_mask      |= QMI_WDS_EMBMS_CONT_DESC_CTRL_INDICATION_REG_LIST_PARAM_MASK;
    ind_type.cont_desc_ctrl_pref = QMI_WDS_EMBMS_CONT_DESC_CTRL_IND_REPORT;

    qmi_ret = qmi_wds_indication_register(DSI_GET_WDS_HNDL(i),
                                          ind_type,
                                          &qmi_err_code);
    if(qmi_ret < 0)
    {
      DSI_LOG_ERROR("qmi_wds_indication_register failed: iface=[%d] err=[%d]",
                    i, qmi_err_code);
      break;
    }
    ret = DSI_SUCCESS;
  } while (0);

  if (ret == DSI_ERROR)
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_register_embms_ind: EXIT with err");
  }
  else
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_register_embms_ind: EXIT with suc");
  }

  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_mni_embms_tmgi_activate
===========================================================================*/
/*!
    @brief
    This function activate TMGI on EMBMS data call.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mni_embms_tmgi_activate(int i, dsi_store_t * st_hndl)
{
  int ret = DSI_ERROR;
  int qmi_ret, qmi_err_code;

  DSI_LOG_DEBUG( "%s", "dsi_mni_embms_tmgi_activate: ENTRY" );

  /* this do..while loop decides the overall return value
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    if (!DSI_IS_ID_VALID(i) ||
        !DSI_IS_HNDL_VALID(st_hndl))
    {
      DSI_LOG_ERROR("%s", "**programming err* invalid params received");
      break;
    }

    DSI_LOG_DEBUG("dsi_mni_embms_tmgi_activate dbg_trace_id:[%d]",
                  st_hndl->priv.embms_tmgi_list_info.embms_tmgi_actdeact_params.dbg_trace_id);

    qmi_ret = qmi_wds_embms_tmgi_activate(DSI_GET_WDS_HNDL(i),
                                          &(st_hndl->priv.embms_tmgi_list_info.embms_tmgi_actdeact_params),
                                          &qmi_err_code);
    if(qmi_ret < 0)
    {
      DSI_LOG_ERROR("qmi_wds_embms_tmgi_activate failed: iface=[%d] err=[%d]",
                    i, qmi_err_code);
      st_hndl->priv.embms_tmgi_list_info.embms_tmgi_actdeact_params.activate_status = qmi_err_code;
      break;
    }

    ret = DSI_SUCCESS;
  } while (0);

  if (ret == DSI_ERROR)
    {
    DSI_LOG_DEBUG( "%s", "dsi_mni_embms_tmgi_activate: EXIT with err");
    }
    else
    {
    DSI_LOG_DEBUG( "%s", "dsi_mni_embms_tmgi_activate: EXIT with suc");
  }

  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_mni_embms_tmgi_deactivate
===========================================================================*/
/*!
    @brief
    This function deactivate TMGI on EMBMS data call.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mni_embms_tmgi_deactivate(int i, dsi_store_t * st_hndl)
{
  int ret = DSI_ERROR;
  int qmi_ret, qmi_err_code;

  DSI_LOG_DEBUG( "%s", "dsi_mni_embms_tmgi_deactivate: ENTRY" );

  /* this do..while loop decides the overall return value
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    if (!DSI_IS_ID_VALID(i) ||
        !DSI_IS_HNDL_VALID(st_hndl))
    {
      DSI_LOG_ERROR("%s", "**programming err* invalid params received");
      break;
    }

    DSI_LOG_DEBUG("dsi_mni_embms_tmgi_deactivate dbg_trace_id:[%d]",
                  st_hndl->priv.embms_tmgi_list_info.embms_tmgi_actdeact_params.dbg_trace_id);

    qmi_ret = qmi_wds_embms_tmgi_deactivate(DSI_GET_WDS_HNDL(i),
                                            &(st_hndl->priv.embms_tmgi_list_info.embms_tmgi_actdeact_params),
                                            &qmi_err_code);
    if(qmi_ret < 0)
    {
      DSI_LOG_ERROR("qmi_wds_embms_tmgi_deactivate failed: iface=[%d] err=[%d]",
                    i, qmi_err_code);
      st_hndl->priv.embms_tmgi_list_info.embms_tmgi_actdeact_params.deactivate_status = qmi_err_code;
      break;
    }

    ret = DSI_SUCCESS;
  } while (0);

  if (ret == DSI_ERROR)
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_embms_tmgi_deactivate: EXIT with err");
  }
  else
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_embms_tmgi_deactivate: EXIT with suc");
  }

  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_mni_embms_tmgi_activate_deactivate
===========================================================================*/
/*!
    @brief
    This function activate and deactivate TMGI on EMBMS data call.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mni_embms_tmgi_activate_deactivate(int i, dsi_store_t * st_hndl)
{
  int ret = DSI_ERROR;
  int qmi_ret, qmi_err_code;

  DSI_LOG_DEBUG( "%s", "dsi_mni_embms_tmgi_activate_deactivate: ENTRY" );

  /* this do..while loop decides the overall return value
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    if (!DSI_IS_ID_VALID(i) ||
        !DSI_IS_HNDL_VALID(st_hndl))
    {
      DSI_LOG_ERROR("%s", "**programming err* invalid params received");
      break;
    }

    DSI_LOG_DEBUG("dsi_mni_embms_tmgi_activate_deactivate dbg_trace_id:[%d]",
                  st_hndl->priv.embms_tmgi_list_info.embms_tmgi_actdeact_params.dbg_trace_id);

    qmi_ret = qmi_wds_embms_tmgi_activate_deactivate(DSI_GET_WDS_HNDL(i),
                                          &(st_hndl->priv.embms_tmgi_list_info.embms_tmgi_actdeact_params),
                                          &qmi_err_code);
    if(qmi_ret < 0)
    {
      DSI_LOG_ERROR("dsi_mni_embms_tmgi_activate_deactivate failed: iface=[%d] err=[%d]",
                    i, qmi_err_code);
      st_hndl->priv.embms_tmgi_list_info.embms_tmgi_actdeact_params.deactivate_status = qmi_err_code;
      break;
    }

    ret = DSI_SUCCESS;
  } while (0);

  if (ret == DSI_ERROR)
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_embms_tmgi_activate_deactivate: EXIT with err");
  }
  else
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_embms_tmgi_activate_deactivate: EXIT with suc");
  }

  return ret;
}
/*===========================================================================
  FUNCTION:  dsi_mni_embms_tmgi_list_query
===========================================================================*/
/*!
    @brief
    This function queries for active or available TMGI list on EMBMS data call.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mni_embms_tmgi_list_query(int i, dsi_store_t * st_hndl)
{
  int ret = DSI_ERROR;
  int qmi_ret, qmi_err_code;
  int j = 0;

  qmi_wds_embms_tmgi_list_ind_type list_query;

  DSI_LOG_DEBUG( "%s", "dsi_mni_embms_tmgi_list_query: ENTRY" );

  memset(&list_query, 0, sizeof(qmi_wds_embms_tmgi_list_ind_type));

  /* this do..while loop decides the overall return value
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    if (!DSI_IS_ID_VALID(i) ||
        !DSI_IS_HNDL_VALID(st_hndl))
    {
      DSI_LOG_ERROR("%s", "**programming err* invalid params received");
      break;
    }

    DSI_LOG_DEBUG("dsi_mni_embms_tmgi_list_query dbg_trace_id:[%d]",
                  st_hndl->priv.embms_tmgi_list_info.embms_tmgi_actdeact_params.dbg_trace_id);

    qmi_ret = qmi_wds_embms_tmgi_list_query(DSI_GET_WDS_HNDL(i),
                                            st_hndl->priv.embms_tmgi_list_info.list_type,
                                            &list_query,
                                            &qmi_err_code);
    if(qmi_ret < 0)
    {
      DSI_LOG_ERROR("dsi_mni_embms_tmgi_list_query failed: iface=[%d] err=[%d]",
                    i, qmi_err_code);
      break;
    }
    else
    {
      if(list_query.param_mask == QMI_WDS_EMBMS_LIST_QUERY_RESP_TMGI_LIST_PARAM_MASK )
      {
        st_hndl->priv.embms_tmgi_list_info.list_type = list_query.list_type;

        /* copy param_mask */
        st_hndl->priv.embms_tmgi_list_info.embms_tmgi_actdeact_params.param_mask =
          QMI_WDS_EMBMS_LIST_QUERY_RESP_TMGI_LIST_PARAM_MASK;

        /* validate tmgi list length */
        st_hndl->priv.embms_tmgi_list_info.embms_tmgi_actdeact_params.tmgi_list_len =
          (unsigned char)((list_query.tmgi_list.tmgi_list_len > DSI_MAX_NUM_OF_TMGI_LIST)?
          DSI_MAX_NUM_OF_TMGI_LIST : list_query.tmgi_list.tmgi_list_len);

        if(NULL == (st_hndl->priv.embms_tmgi_list_info.embms_tmgi_actdeact_params.tmgi_list =
                    malloc(st_hndl->priv.embms_tmgi_list_info.embms_tmgi_actdeact_params.tmgi_list_len * sizeof (qmi_wds_embms_tmgi_type))))
        {
          DSI_LOG_ERROR("%s", "dsi_mni_embms_tmgi_list_query: allocate memory failure");
          break;
        }

        /* zero out memeory */
        memset(st_hndl->priv.embms_tmgi_list_info.embms_tmgi_actdeact_params.tmgi_list,
               0,
               st_hndl->priv.embms_tmgi_list_info.embms_tmgi_actdeact_params.tmgi_list_len * sizeof(qmi_wds_embms_tmgi_type));

        for( j = 0; j < st_hndl->priv.embms_tmgi_list_info.embms_tmgi_actdeact_params.tmgi_list_len; j++ )
        {
          /* session information is not needed from EMBMS service layer, skip the copy */
          memcpy(&(st_hndl->priv.embms_tmgi_list_info.embms_tmgi_actdeact_params.tmgi_list[j].tmgi[0]),
                 &(list_query.tmgi_list.tmgi_list_ptr[j].tmgi[0]),
                 DSI_SIZE_OF_TMGI);
        }
      }/* list_query.param_mask == QMI_WDS_EMBMS_LIST_QUERY_RESP_TMGI_LIST_PARAM_MASK*/
    }
    /* release memory */
    if(NULL != list_query.tmgi_list.tmgi_list_ptr)
    {
      free(list_query.tmgi_list.tmgi_list_ptr);
      list_query.tmgi_list.tmgi_list_ptr = NULL;
    }

    ret = DSI_SUCCESS;
  } while (0);

  if (ret == DSI_ERROR)
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_embms_tmgi_list_query: EXIT with err");
  }
  else
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_embms_tmgi_list_query: EXIT with suc");
  }

  return ret;
}

/*===========================================================================
  FUNCTION:  dsi_mni_embms_tmgi_content_desc_update
===========================================================================*/
/*!
    @brief
    This function updates content desc on EMBMS data call.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mni_embms_tmgi_content_desc_update(int i, dsi_store_t * st_hndl)
{
  int ret = DSI_ERROR;
  int qmi_ret, qmi_err_code;

  DSI_LOG_DEBUG( "%s", "dsi_mni_embms_tmgi_content_desc_update: ENTRY" );

  /* this do..while loop decides the overall return value
     set ret to ERROR at the beginning. set ret to SUCCESS
     at the end. If there was an error in the middle, we break out*/
  do
  {
    if (!DSI_IS_ID_VALID(i) ||
        !DSI_IS_HNDL_VALID(st_hndl))
    {
      DSI_LOG_ERROR("%s", "**programming err* invalid params received");
      break;
    }

    DSI_LOG_DEBUG("dsi_mni_embms_tmgi_content_desc_update dbg_trace_id:[%d]",
                  st_hndl->priv.embms_content_desc_update_info.dbg_trace_id);

    qmi_ret = qmi_wds_embms_content_desc_update(
                DSI_GET_WDS_HNDL(i),
                &(st_hndl->priv.embms_content_desc_update_info),
                &qmi_err_code);

    if(qmi_ret < 0)
    {
      DSI_LOG_ERROR("dsi_mni_embms_tmgi_content_desc_update failed: iface=[%d] err=[%d]",
                    i, qmi_err_code);
      break;
    }

    ret = DSI_SUCCESS;
  } while (0);

  if (ret == DSI_ERROR)
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_embms_tmgi_content_desc_update: EXIT with err");
  }
  else
  {
    DSI_LOG_DEBUG( "%s", "dsi_mni_embms_tmgi_content_desc_update: EXIT with suc");
  }

  return ret;
}

