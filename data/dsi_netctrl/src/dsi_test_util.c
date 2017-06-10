/*!
  @file
  dsi_test_util.c

  @brief
  This file contains utility functions shared by dsi offtarget test framework

*/

/*===========================================================================

  Copyright (c) 2011-2013 Qualcomm Technologies, Inc. All Rights Reserved

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
03/29/11   js      Created
===========================================================================*/
#include "qmi_wds_srvc.h"
#include "qmi_platform_config.h"
#include "netmgr.h"
#include <string.h>
#include "dsi_test_defs.h"
#include "dsi_netctrl_cb_thrd.h" /* SIMULATE DSI CALLBACKS */

typedef struct dsi_test_link_struct_s
{
  int link;
  char * dev_id;
  int qmi_wds_hndl;
  int netmgr_link;
} dsi_test_link_struct_t;

dsi_test_link_struct_t dsi_test_links[16] =
{ {0, QMI_PORT_RMNET_0, 0x00, NETMGR_LINK_RMNET_0},
  {1, QMI_PORT_RMNET_1, 0x01, NETMGR_LINK_RMNET_1},
  {2, QMI_PORT_RMNET_2, 0x02, NETMGR_LINK_RMNET_2},
  {3, QMI_PORT_RMNET_3, 0x03, NETMGR_LINK_RMNET_3},
  {4, QMI_PORT_RMNET_4, 0x04, NETMGR_LINK_RMNET_4},
  {5, QMI_PORT_RMNET_5, 0x05, NETMGR_LINK_RMNET_5},
  {6, QMI_PORT_RMNET_6, 0x06, NETMGR_LINK_RMNET_6},
  {7, QMI_PORT_RMNET_7, 0x07, NETMGR_LINK_RMNET_7},

  {8, QMI_PORT_RMNET_SDIO_0, 0x08, NETMGR_LINK_RMNET_8},
  {9, QMI_PORT_RMNET_SDIO_1, 0x09, NETMGR_LINK_RMNET_9},
  {10, QMI_PORT_RMNET_SDIO_2, 0x0A, NETMGR_LINK_RMNET_10},
  {11, QMI_PORT_RMNET_SDIO_3, 0x0B, NETMGR_LINK_RMNET_11},
  {12, QMI_PORT_RMNET_SDIO_4, 0x0C, NETMGR_LINK_RMNET_12},
  {13, QMI_PORT_RMNET_SDIO_5, 0x0D, NETMGR_LINK_RMNET_13},
  {14, QMI_PORT_RMNET_SDIO_6, 0x0E, NETMGR_LINK_RMNET_14},
  {15, QMI_PORT_RMNET_SDIO_7, 0x0F, NETMGR_LINK_RMNET_15}
};

int dsi_test_util_get_wds_hndl(char * dev_id)
{
  int i;
  for(i=0; i<15; i++)
  {
    if (!strcmp(dsi_test_links[i].dev_id,dev_id))
    {
      return dsi_test_links[i].qmi_wds_hndl;
    }
  }

  return -1;
}

int dsi_test_util_get_link(char * dev_id)
{
  int i;
  for(i=0; i<15; i++)
  {
    if (!strcmp(dsi_test_links[i].dev_id,dev_id))
    {
      return dsi_test_links[i].link;
    }
  }

  return -1;
}

void dsi_test_util_get_dev_id(int qmi_wds_hndl, char * dest)
{
  int i;
  for(i=0; i<15; i++)
  {
    if (dsi_test_links[i].qmi_wds_hndl == qmi_wds_hndl)
    {
      strlcpy(dest, dsi_test_links[i].dev_id, sizeof(dest));
      return;
    }
  }

  DSI_TEST_ERR("dsi_test_util: couldn't find dev_id for wds_hndl [0x%x]",qmi_wds_hndl);
  return;
}

int dsi_test_util_get_netmgr_link(int dsi_link)
{
  int i;
  for(i=0; i<15; i++)
  {
    if (dsi_test_links[i].link == dsi_link)
    {
      return dsi_test_links[i].netmgr_link;
    }
  }

  DSI_TEST_ERR("dsi_test_util: couldn't find netmgr_link for dsi_link [%d]", dsi_link);
  return -1;
}

/* QMI SIMULATION */

inline void dsi_test_sni_rsp(int hndl, int sni_wds_hndl)
{
  qmi_wds_async_rsp_data_type rsp_data;
  rsp_data.start_nw_rsp.pkt_data_handle = 0xffff;
  rsp_data.start_nw_rsp.call_end_reason.
    call_end_reason_verbose.verbose_reason_type = QMI_WDS_CE_TYPE_INVALID;
  rsp_data.start_nw_rsp.call_end_reason.
    call_end_reason_verbose.verbose_reason = QMI_WDS_VERBOSE_CE_INVALID;
  dsi_qmi_wds_cmd_cb(
    sni_wds_hndl,
    QMI_WDS_SERVICE,
    QMI_NO_ERR, /* service */
    QMI_NO_ERR, /* qmi err */
    hndl,
    QMI_WDS_SRVC_START_NW_ASYNC_RSP_MSG,
    &rsp_data);
}

inline void dsi_test_pkt_data_connected(int sni_wds_hndl, char * sni_dev_id)
{
  qmi_wds_indication_data_type ind_data;
  ind_data.pkt_srvc_status.link_status = QMI_WDS_PACKET_DATA_CONNECTED;
  ind_data.pkt_srvc_status.reconfig_required = FALSE;
  ind_data.pkt_srvc_status.call_end_reason.call_end_reason_verbose.
    verbose_reason_type = QMI_WDS_CE_TYPE_INVALID;
  ind_data.pkt_srvc_status.call_end_reason.call_end_reason_verbose.
    verbose_reason = QMI_WDS_VERBOSE_CE_INVALID;
  dsi_qmi_wds_ind_cb(
    sni_wds_hndl,
    QMI_WDS_SERVICE,
    dsi_test_util_get_link(sni_dev_id),
    QMI_WDS_SRVC_PKT_SRVC_STATUS_IND_MSG,
    &ind_data);
}

/* NETMGR SIMULATION */

inline void dsi_test_platform_up(char * sni_dev_id)
{
  netmgr_nl_event_info_t netmgr_info;
  struct sockaddr_storage sock;

  memset(&netmgr_info, 0, sizeof(netmgr_info));
  memset(&sock, 0, sizeof(sock));

  strlcpy(sock.__ss_padding, "1234", sizeof(sock.__ss_padding));
  netmgr_info.link = dsi_test_util_get_netmgr_link(
    dsi_test_util_get_link(sni_dev_id));
  strlcpy(netmgr_info.addr_info.addr.ip_addr.__ss_padding, "1234",
          sizeof(netmgr_info.addr_info.addr.ip_addr.__ss_padding));

  /* IP family related assignment */
  netmgr_info.param_mask |= NETMGR_EVT_PARAM_IPADDR;
  netmgr_info.addr_info.addr.ip_addr.ss_family = AF_INET;

  dsi_netmgr_cb(
    NET_PLATFORM_UP_EV,
    &netmgr_info,
    NULL);
}

inline void dsi_test_platform_down(char * sni_dev_id)
{
  netmgr_nl_event_info_t netmgr_info;

  /* generate NET PLATFORM DOWN EV */
  netmgr_info.link = dsi_test_util_get_netmgr_link(
    dsi_test_util_get_link(sni_dev_id));
  strlcpy(netmgr_info.addr_info.addr.ip_addr.__ss_padding, "1234",
          sizeof(netmgr_info.addr_info.addr.ip_addr.__ss_padding));
  dsi_netmgr_cb(
    NET_PLATFORM_DOWN_EV,
    &netmgr_info,
    NULL);
}

inline void dsi_test_embms_tmgi_activation_ind(int sni_wds_hndl,char * sni_dev_id)
{
  qmi_wds_indication_data_type ind_data;  
  ind_data.embms_activate_status.activate_status = QMI_WDS_EMBMS_TMGI_ACTIVATE_SUCCESS;
  ind_data.embms_activate_status.activation_tmgi.tmgi_list_len = 1;
  if(NULL == (ind_data.embms_activate_status.activation_tmgi.tmgi_list_ptr = 
              malloc(sizeof (qmi_wds_embms_tmgi_type))))
  {
    DSI_TEST_ERR("%s", "can not allocate memory");
    return;
  }
  char *tmgi_ptr = "ACTIVA";
  memcpy(&(ind_data.embms_activate_status.activation_tmgi.tmgi_list_ptr->tmgi[0]), 
         tmgi_ptr, 
         QMI_WDS_EMBMS_TMGI_SIZE * sizeof(char) );
  ind_data.embms_activate_status.activation_tmgi.tmgi_list_ptr->session_id = 4;
  ind_data.embms_activate_status.activation_tmgi.tmgi_list_ptr->session_id_valid = DSI_EMBMS_TMGI_SESSION_ID_NOT_VALID;
  ind_data.embms_activate_status.dbg_trace_id = 5;
  ind_data.embms_activate_status.param_mask = 0;
  ind_data.embms_activate_status.param_mask |= QMI_WDS_EMBMS_TMGI_ACTIVATE_STATUS_IND_TRANX_ID_PARAM;
  
  dsi_qmi_wds_ind_cb(
    sni_wds_hndl,
    QMI_WDS_SERVICE,
    dsi_test_util_get_link(sni_dev_id),
    QMI_WDS_SRVC_EMBMS_TMGI_ACTIVATE_IND_MSG,
    &ind_data);
}

inline void dsi_test_embms_tmgi_deactivation_ind(int sni_wds_hndl,char * sni_dev_id)
{
  qmi_wds_indication_data_type ind_data;
  ind_data.embms_deactivate_status.deactivate_status = QMI_WDS_EMBMS_TMGI_DEACTIVATE_SUCCESS;
  ind_data.embms_deactivate_status.deactivation_tmgi.tmgi_list_len = 1;
  if(NULL == (ind_data.embms_deactivate_status.deactivation_tmgi.tmgi_list_ptr = 
              malloc(sizeof (qmi_wds_embms_tmgi_type))))
  {
    DSI_TEST_ERR("%s", "can not allocate memory");
    return;
  }
  char *de_tmgi_ptr = "DEACTI";
  memcpy(&(ind_data.embms_deactivate_status.deactivation_tmgi.tmgi_list_ptr->tmgi[0]), 
         de_tmgi_ptr, 
         QMI_WDS_EMBMS_TMGI_SIZE * sizeof(char));
  ind_data.embms_deactivate_status.deactivation_tmgi.tmgi_list_ptr->session_id = 1;
  ind_data.embms_deactivate_status.deactivation_tmgi.tmgi_list_ptr->session_id_valid = DSI_EMBMS_TMGI_SESSION_ID_VALID;

  ind_data.embms_deactivate_status.dbg_trace_id = 6;
  ind_data.embms_deactivate_status.param_mask = 0;
  ind_data.embms_deactivate_status.param_mask |= QMI_WDS_EMBMS_TMGI_DEACTIVATE_STATUS_IND_TRANX_ID_PARAM;

  dsi_qmi_wds_ind_cb(
    sni_wds_hndl,
    QMI_WDS_SERVICE,
    dsi_test_util_get_link(sni_dev_id),
    QMI_WDS_SRVC_EMBMS_TMGI_DEACTIVATE_IND_MSG,
    &ind_data);
}
inline void dsi_test_embms_tmgi_actdeact_ind(int sni_wds_hndl,char * sni_dev_id)
{
  char *tmgi_ptr = "ACTIVA";
  char *de_tmgi_ptr = "DEACTI";

  qmi_wds_indication_data_type ind_data;
  ind_data.embms_act_deact_status.activate_status = QMI_WDS_EMBMS_TMGI_ACTIVATE_SUCCESS;
  ind_data.embms_act_deact_status.activation_tmgi.tmgi_list_len = 1;
  if(NULL == (ind_data.embms_act_deact_status.activation_tmgi.tmgi_list_ptr =
              malloc(sizeof(qmi_wds_embms_tmgi_type))))
  {
    DSI_TEST_ERR("%s", "can not allocate memory");
    return;
  }
  memcpy(&(ind_data.embms_act_deact_status.activation_tmgi.tmgi_list_ptr->tmgi[0]),
         tmgi_ptr,
         QMI_WDS_EMBMS_TMGI_SIZE);

  ind_data.embms_act_deact_status.deactivate_status = QMI_WDS_EMBMS_TMGI_DEACTIVATE_SUCCESS;
  ind_data.embms_act_deact_status.deactivation_tmgi.tmgi_list_len = 1;
  if(NULL == (ind_data.embms_act_deact_status.deactivation_tmgi.tmgi_list_ptr = 
              malloc(sizeof (qmi_wds_embms_tmgi_type))))
  {
    DSI_TEST_ERR("%s", "can not allocate memory");
    return;
  }
  
  memcpy(&(ind_data.embms_act_deact_status.deactivation_tmgi.tmgi_list_ptr->tmgi[0]), 
         de_tmgi_ptr, 
         QMI_WDS_EMBMS_TMGI_SIZE * sizeof(char));

  ind_data.embms_act_deact_status.dbg_trace_id = 7;
  ind_data.embms_act_deact_status.param_mask = 0;
  ind_data.embms_act_deact_status.param_mask |= QMI_WDS_EMBMS_TMGI_ACT_DEACT_STATUS_IND_TRANX_ID_PARAM;

  dsi_qmi_wds_ind_cb(
    sni_wds_hndl,
    QMI_WDS_SERVICE,
    dsi_test_util_get_link(sni_dev_id),
    QMI_WDS_SRVC_EMBMS_TMGI_ACT_DEACT_IND_MSG,
    &ind_data);
}

inline void dsi_test_embms_tmgi_list_ind(int sni_wds_hndl, char *sni_dev_id)
{
  qmi_wds_indication_data_type ind_data;
  memset(&ind_data, 0, sizeof (ind_data));

  ind_data.embms_list.list_type = QMI_WDS_EMBMS_TMGI_LIST_ACTIVE;
  ind_data.embms_list.tmgi_list.tmgi_list_len = 2;
  char *tmgi_list[2] = { "TMGI1", "TMGI2" };

  if(NULL == (ind_data.embms_list.tmgi_list.tmgi_list_ptr = 
              malloc(ind_data.embms_list.tmgi_list.tmgi_list_len * sizeof (qmi_wds_embms_tmgi_type))))
  {
    DSI_TEST_ERR("%s", "can not allocate memory");
    return;
  }
  ind_data.embms_list.tmgi_list.tmgi_list_ptr[0].session_id = 1;
  ind_data.embms_list.tmgi_list.tmgi_list_ptr[0].session_id_valid = DSI_EMBMS_TMGI_SESSION_ID_VALID;
  memcpy(&(ind_data.embms_list.tmgi_list.tmgi_list_ptr[0].tmgi[0]), 
         tmgi_list[0], 
         QMI_WDS_EMBMS_TMGI_SIZE * sizeof(char));

  ind_data.embms_list.tmgi_list.tmgi_list_ptr[1].session_id = 2;
  ind_data.embms_list.tmgi_list.tmgi_list_ptr[1].session_id_valid = DSI_EMBMS_TMGI_SESSION_ID_VALID;
  memcpy(&(ind_data.embms_list.tmgi_list.tmgi_list_ptr[1].tmgi[0]), tmgi_list[1], QMI_WDS_EMBMS_TMGI_SIZE * sizeof(char));

  ind_data.embms_list.dbg_trace_id = 7;
  ind_data.embms_list.param_mask = 0;
  ind_data.embms_list.param_mask |= QMI_WDS_EMBMS_LIST_IND_TMGI_LIST_PARAM_MASK;
  ind_data.embms_list.param_mask |= QMI_WDS_EMBMS_LIST_IND_TRANX_ID_PARAM_MASK;

  dsi_qmi_wds_ind_cb(
    sni_wds_hndl,
    QMI_WDS_SERVICE,
    dsi_test_util_get_link(sni_dev_id),
    QMI_WDS_SRVC_EMBMS_TMGI_LIST_IND_MSG,
    &ind_data);
}

inline void dsi_test_embms_sai_list_ind(int sni_wds_hndl, char *sni_dev_id)
{
  qmi_wds_indication_data_type ind_data;
  memset(&ind_data, 0, sizeof (ind_data));

  ind_data.sai_list.param_mask = 0;
  ind_data.sai_list.param_mask |= QMI_WDS_EMBMS_LIST_IND_TMGI_LIST_PARAM_MASK;

  ind_data.sai_list.available_sai_list_len = 4;
  unsigned int available_sai_list[4] ={ 1, 2, 3, 4};

  ind_data.sai_list.camped_sai_list_len = 2;
  unsigned int camped_sai_list[2] ={ 2, 3};

  ind_data.sai_list.num_sai_per_group_len = 3;
  unsigned int num_sai_per_group[4] ={ 1, 2, 1};

  if (NULL == (ind_data.sai_list.available_sai_list =
               malloc(ind_data.sai_list.available_sai_list_len * sizeof(unsigned int))))
  {
    DSI_TEST_ERR("%s", "can not allocate memory");
    return;
  }
  memcpy(ind_data.sai_list.available_sai_list,
         available_sai_list,
         ind_data.sai_list.available_sai_list_len * sizeof(unsigned int));

  if (NULL == (ind_data.sai_list.camped_sai_list =
               malloc(ind_data.sai_list.camped_sai_list_len * sizeof(unsigned int))))
  {
    DSI_TEST_ERR("%s", "can not allocate memory");
    return;
  }
  memcpy(ind_data.sai_list.camped_sai_list,
         camped_sai_list,
         ind_data.sai_list.camped_sai_list_len * sizeof(unsigned short));

  if (NULL == (ind_data.sai_list.num_sai_per_group =
               malloc(ind_data.sai_list.num_sai_per_group_len * sizeof(unsigned short))))
  {
    DSI_TEST_ERR("%s", "can not allocate memory");
    return;
  }
  memcpy(ind_data.sai_list.num_sai_per_group,
         num_sai_per_group,
         ind_data.sai_list.num_sai_per_group_len * sizeof(unsigned short));

  dsi_qmi_wds_ind_cb(
    sni_wds_hndl,
    QMI_WDS_SERVICE,
    dsi_test_util_get_link(sni_dev_id),
    QMI_WDS_SRVC_EMBMS_SAI_LIST_IND_MSG,
    &ind_data);
}
