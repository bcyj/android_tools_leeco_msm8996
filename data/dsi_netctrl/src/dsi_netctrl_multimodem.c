/*!
  @file
  dsi_netctrl_multimodem.c

  @brief
  This file implements multi modem specific routines for
  dsi_netctrli.c

*/

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

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //depot/restricted/linux/android/ril/qcril_dsi.c $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
04/07/10   js      created file

===========================================================================*/

#include "dsi_netctrl.h"
#include "dsi_netctrli.h"
#include "qmi_nas_srvc.h"

typedef struct dsi_mm_route_rsp_s
{
  qmi_wds_route_look_up_rsp_type qmi_rsp_data;
  int modem;
} dsi_mm_route_rsp;

/* following table assists in priting more readable
 * debug msgs */
char * dsi_modem_id_tbl[DSI_MAX_MODEMS] =
{
  "DSI_RADIO_MODEM_2", /* index 0 */
  "DSI_RADIO_MODEM_1"  /* index 1 */
};

/*===========================================================================
  FUNCTION:  dsi_mm_get_pref_tech
===========================================================================*/
/*!
    @brief
    demo purpose only
    following function will determine based on
    NAS query as what modem to use for
    call bring up

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mm_get_pref_modem()
{
  int qmi_err_code;
  int ret;

  qmi_nas_serving_system_info_type serving_system_info;

  if ((ret = qmi_nas_get_serving_system(nas_clnt_id,&serving_system_info,&qmi_err_code)) < 0)
  {
    DSI_LOG_ERROR("qmi_nas_get_serving_system() returned error [%d], qmi_err [%d]",
                  ret, qmi_err_code);
    DSI_LOG_DEBUG("assuming 3GMODEM [%d] is not available", DSI_RADIO_3GMODEM);
    return DSI_RADIO_4GMODEM;
  }

  DSI_LOG_DEBUG("dsi_mm_get_pref_tech: NAS reg state is [%d]", serving_system_info.reg_state);
  DSI_LOG_DEBUG("dsi_mm_get_pref_tech: NAS num_radio_interfaces are [%d]",
                serving_system_info.num_radio_interfaces);
  DSI_LOG_DEBUG("dsi_mm_get_pref_tech: NAS network_type is [%d]",
                serving_system_info.network_type);
  DSI_LOG_DEBUG("dsi_mm_get_pref_tech: NAS cs_attach_state is [%d]",
                serving_system_info.cs_attach_state);
  DSI_LOG_DEBUG("dsi_mm_get_pref_tech: NAS ps_attach_state is [%d]",
                serving_system_info.ps_attach_state);

  if(serving_system_info.num_radio_interfaces > 0 &&
     serving_system_info.reg_state == QMI_NAS_REGISTERED &&
     serving_system_info.ps_attach_state == QMI_NAS_PS_ATTACHED)
  {
    DSI_LOG_DEBUG("%s","using 3G MODEM");
    return DSI_RADIO_3GMODEM;
  }
  else
  {
    DSI_LOG_DEBUG("%s","using 4G MODEM");
    return DSI_RADIO_4GMODEM;
  }
}

/*===========================================================================
  FUNCTION:  dsi_mm_iface_route_look_up
===========================================================================*/
/*!
    @brief

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mm_iface_route_look_up
(
  qmi_wds_route_look_up_params_type * pparams,
  dsi_mm_route_rsp *rsp_data
)
{
  int selected_modem = DSI_INVALID_MODEM;
  int qmi_ret, qmi_err_code;
  int iface, modem;
  int priority = 0;

    /* issue route look up on each modem */
    for (modem=0; modem<DSI_MAX_MODEMS; modem++)
    {
      /* tag this response with the modem id */
      rsp_data[modem].modem = modem;

      iface = DSI_MODEM_GET_DEFAULT_IFACE(modem);

      if(DSI_MODEM_GET_DEFAULT_WDS_HNDL(modem) == DSI_INVALID_WDS_HNDL)
      {
        continue;
      }

      if (QMI_NO_ERR != qmi_wds_bind_subscription ( DSI_MODEM_GET_DEFAULT_WDS_HNDL(modem),
                                      (qmi_wds_bind_subscription_type)dsi_get_modem_subs_id(),
                                      &qmi_err_code))
      {
        DSI_LOG_DEBUG("%s","subs binding failed");
      }

      /* store the response in rsp_data array */
      qmi_ret = qmi_wds_route_look_up(DSI_MODEM_GET_DEFAULT_WDS_HNDL(modem),
                                      pparams,
                                      &(rsp_data[modem].qmi_rsp_data),
                                      &qmi_err_code);

      /* bail out if an error occured */
      if (QMI_NO_ERR != qmi_ret)
      {
        DSI_LOG_ERROR("qmi_wds_route_look_up failed on iface [%d] " \
                      "with err [%d][%d]",
                      iface, qmi_ret,qmi_err_code);
        DSI_LOG_DEBUG("%s","continue doing route_look_up further");
        continue;
      }

      DSI_LOG_DEBUG("modem [%s] returned an interface [0x%lud]" \
                    " with priority set to [%lud] tech set to" \
                    " [0x%x] on qmi inst [%d]",
                    dsi_modem_id_tbl[modem],
                    rsp_data[modem].qmi_rsp_data.iface_handle,
                    rsp_data[modem].qmi_rsp_data.priority,
                    rsp_data[modem].qmi_rsp_data.tech_name,
                    rsp_data[modem].qmi_rsp_data.qmi_inst_is_valid?
                      rsp_data[modem].qmi_rsp_data.qmi_inst :
                      DSI_INVALID_QMI_INST );
      /* update the selected modem to current modem
       * if the interface on it has higher priority */
      if ((int)(rsp_data[modem].qmi_rsp_data.priority) > (int)(priority))
      {
        DSI_LOG_DEBUG("current priority is [%d]", priority);
        priority = (int)rsp_data[modem].qmi_rsp_data.priority;
        DSI_LOG_DEBUG("new high priority is [%d]", priority);
        selected_modem = modem;
      }
    }

  return selected_modem;

}

/*===========================================================================
  FUNCTION:  dsi_mm_iface_look_up
===========================================================================*/
/*!
    @brief
    following function will determine based on
    multi modem route look up, as which dsi
    interface can be used for SNI (start network
    interface). the interface is returned in piface.
    the selected modem is returned in pmodem.
    the selected technology is returned in ptech.

    @return
    DSI_ERROR
    DSI_SUCCESS
*/
/*=========================================================================*/
int dsi_mm_iface_look_up
(
  dsi_store_t * st_hndl,
  int *piface,
  int *pmodem,
  int *ptech
)
{
  int ret = DSI_ERROR;
  dsi_mm_route_rsp rsp_data[DSI_MAX_MODEMS];
  qmi_wds_route_look_up_params_type route_lookup_params;
  int qmi_ret, qmi_err_code;
  int iface, modem;
  int selected_modem = DSI_INVALID_MODEM;
  int other_ip_selected_modem = DSI_INVALID_MODEM;
  boolean companion_ip_rl_up = DSI_FALSE;
  int requested_qmi_family;

  DSI_LOG_DEBUG("%s","dsi_mm_iface_look_up: entry");

  do
  {
    if (!(DSI_IS_HNDL_VALID(st_hndl)) ||
        NULL == piface ||
        NULL == pmodem)
    {
      DSI_LOG_ERROR("%s","invalid params received");
      break;
    }

    *piface = DSI_INVALID_IFACE;

    /* prepare route look up parameters */
    if (DSI_ERROR ==
        dsi_prepare_route_lookup_params(st_hndl,
                                        &route_lookup_params))
    {
      DSI_LOG_ERROR("%s","dsi_prepare_route_lookup_params failed");
      break;
    }

    switch(route_lookup_params.ip_family_pref)
    {
    case QMI_IP_FAMILY_PREF_ANY:
    case QMI_IP_FAMILY_PREF_IPV4:
      companion_ip_rl_up = DSI_TRUE;
      requested_qmi_family = QMI_IP_FAMILY_PREF_IPV4;
      break;

    case QMI_IP_FAMILY_PREF_IPV6:
      companion_ip_rl_up = DSI_TRUE;
      requested_qmi_family = QMI_IP_FAMILY_PREF_IPV6;
      break;

    case QMI_IP_FAMILY_PREF_UNSPECIFIED:
      companion_ip_rl_up = DSI_FALSE;
      requested_qmi_family = QMI_IP_FAMILY_PREF_UNSPECIFIED;
      break;

    default:
      companion_ip_rl_up = DSI_FALSE;
      requested_qmi_family = QMI_IP_FAMILY_PREF_UNSPECIFIED;
      break;
    }

    selected_modem = dsi_mm_iface_route_look_up(&route_lookup_params, &rsp_data[0]);

    if (selected_modem == DSI_INVALID_MODEM ||
        selected_modem < 0 ||
        selected_modem >= DSI_MAX_MODEMS)
    {
      DSI_LOG_ERROR("%s","multi modem route look up could not select " \
                    "a valid modem");
      break;
    }

    DSI_LOG_DEBUG("multimodem route look up found [%s] modem " \
                  "to have the highest priority interface available",
                  dsi_modem_id_tbl[selected_modem]);


    if (rsp_data[selected_modem].qmi_rsp_data.qmi_inst_is_valid == 0)
    {
      DSI_LOG_DEBUG("route_look_up returned Um Handle [%ld]",
                    rsp_data[selected_modem].qmi_rsp_data.iface_handle);

      if(companion_ip_rl_up == DSI_TRUE)
      {
         DSI_LOG_DEBUG("%s","Performing Other IP type route_lookup for an existing call");

         if(route_lookup_params.ip_family_pref == QMI_IP_FAMILY_PREF_IPV6)
         {
           route_lookup_params.ip_family_pref = QMI_IP_FAMILY_PREF_IPV4;
         }
         else
         {
           route_lookup_params.ip_family_pref = QMI_IP_FAMILY_PREF_IPV6;
         }
         other_ip_selected_modem = dsi_mm_iface_route_look_up(&route_lookup_params, &rsp_data[0]);
      }

      if ((other_ip_selected_modem == DSI_INVALID_MODEM) ||
          ( selected_modem != other_ip_selected_modem ) ||
          (rsp_data[other_ip_selected_modem].qmi_rsp_data.qmi_inst_is_valid == 0))
      {

        st_hndl->priv.rl_qmi_inst = DSI_INVALID_QMI_INST;
        /* find an available interface on this modem */
        *piface = dsi_find_available_iface_on_modem(selected_modem, st_hndl->priv.app_type);
      }
      else
      {
        st_hndl->priv.companion_ip_call = TRUE;
        DSI_LOG_DEBUG("%s","Found a valid Companion IP call");
      }
    }

    if (rsp_data[selected_modem].qmi_rsp_data.qmi_inst_is_valid)
    {
      /* call is already UP */
      *piface = dsi_find_dsi_id_by_qmi(rsp_data[selected_modem].
                                       qmi_rsp_data.qmi_inst,
                                       DSI_MODEM_GET_DEFAULT_IFACE(selected_modem));
      DSI_LOG_DEBUG("route_look_up returned RmNet QMI instance [%d] maps to iface[%d]",
                    rsp_data[selected_modem].qmi_rsp_data.qmi_inst, *piface);
      /* Preserve QMI instance to know later if Modem interface was already UP */

        st_hndl->priv.rl_qmi_inst =
         (int)rsp_data[selected_modem].qmi_rsp_data.qmi_inst;

      DSI_LOG_ERROR("rl_qmi_inst = [%d]",st_hndl->priv.rl_qmi_inst);
    }



    if (!DSI_IS_ID_VALID(*piface))
    {
      DSI_LOG_ERROR("couldn't find a dsi id on modem [%s]",
                    dsi_modem_id_tbl[selected_modem]);
      break;
    }

    /* return the selected modem to the caller */
    *pmodem = selected_modem;
    *ptech = rsp_data[selected_modem].qmi_rsp_data.tech_name;

    /* store the iface_handle only if the qmi_inst is valid */
    if (rsp_data[selected_modem].qmi_rsp_data.qmi_inst_is_valid)
    {
      /* store the Um handle on SNI */
      st_hndl->priv.start_nw_params.params_mask |=
        QMI_WDS_START_NW_IFACE_HNDL_PARAM;
      st_hndl->priv.start_nw_params.iface_handle =
        rsp_data[selected_modem].qmi_rsp_data.iface_handle;
    }

    ret = DSI_SUCCESS;
  } while (0);

  if (DSI_SUCCESS == ret)
  {
    DSI_LOG_DEBUG("%s","dsi_mm_iface_look_up: exit with success");
  }
  else
  {
    DSI_LOG_ERROR("%s","dsi_mm_iface_look_up: exit with error");
  }

  return ret;
}



