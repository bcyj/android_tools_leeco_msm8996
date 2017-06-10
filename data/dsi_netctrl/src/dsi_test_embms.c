/*!
  @file
  dsi_test_embms.c

  @brief
  This is a test case for dsi_netctrl_embms.c

*/

/*===========================================================================

  Copyright (c) 2011-2013 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary and Confidential.

===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header:  $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
10/17/11   sy      Initial Version
===========================================================================*/
#include <stdio.h>
#include "dsi_test_defs.h"
#include "dsi_netctrl.h" /* DSI_NETCTRL API */
#include "dsi_netctrl_cb_thrd.h" /* SIMULATE CALLBACKS */
#include <sys/socket.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int test_cb_event = DSI_EVT_INVALID;
static boolean evt_rcvd = FALSE;
/* cb function registered with dsi_netctrl */
void dsi_test_embms_cb(dsi_hndl_t hndl,
                   void * user_data,
                   dsi_net_evt_t evt)
{
  DSI_TEST_FUNC_ENTRY;

  pthread_mutex_lock(&mutex);
  test_cb_event = (int)evt;
  evt_rcvd = TRUE;
  switch(evt)
  {
  case DSI_NET_TMGI_ACTIVATED:
    DSI_TEST_REPORT_LOG("%s","dsi_test_embms_cb: evt DSI_NET_TMGI_ACTIVATED recvd");
    break;
  case DSI_NET_TMGI_DEACTIVATED:
    DSI_TEST_REPORT_LOG("%s", "dsi_test_embms_cb: evt DSI_NET_TMGI_DEACTIVATED recvd");
    break;
  case DSI_NET_TMGI_ACTIVATED_DEACTIVATED:
    DSI_TEST_REPORT_LOG("%s", "dsi_test_embms_cb: evt DSI_NET_TMGI_ACTIVATED_DEACTIVATED recvd");
    break;
  case DSI_NET_TMGI_LIST_CHANGED:
    DSI_TEST_REPORT_LOG("%s", "dsi_test_embms_cb: evt DSI_NET_TMGI_LIST_CHANGED");
    break;
  case DSI_NET_SAI_LIST_CHANGED:
    DSI_TEST_REPORT_LOG("%s", "dsi_test_embms_cb: evt DSI_NET_SAI_LIST_CHANGED");
    break;
  default:
    DSI_TEST_REPORT_LOG("dsi_test_embms_cb: evt [%d] rcvd", evt);
    break;
  }
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);

  DSI_TEST_FUNC_EXIT;
}

/* QMI behavior */
static unsigned long um_hndl = 0x01;
static qmi_wds_iface_name_type ps_tech = 0x8004;
static int qmi_sni_txn = 0xffff;
static int qmi_wds_hndl = 0x01;
static int sni_wds_hndl = 0x01;
static char sni_dev_id[100] = "rmnet_sdio0";
static void * qmi_user_data;
static struct sockaddr_storage sock;

qmi_client_handle_type
dsi_test_embms_wds_init
(
  const char                    *dev_id,
  qmi_wds_indication_hdlr_type  user_ind_msg_hdlr,
  void                          *user_ind_msg_hdlr_user_data,
  int                           *qmi_err_code
)
{
  DSI_TEST_FUNC_ENTRY;
  DSI_TEST_FUNC_EXIT;
  qmi_user_data = user_ind_msg_hdlr_user_data;
  return dsi_test_util_get_wds_hndl(dev_id);
}

int dsi_test_embms_rl
(
  int                               user_handle,
  qmi_wds_route_look_up_params_type *params,
  qmi_wds_route_look_up_rsp_type    *rsp_data,
  int                               *qmi_err_code
)
{
  DSI_TEST_FUNC_ENTRY;
  if (rsp_data)
  {
    rsp_data->iface_handle = um_hndl;
    rsp_data->priority = 1;
    rsp_data->tech_name = ps_tech;
    rsp_data->qmi_inst_is_valid = 0;
    rsp_data->qmi_inst = 0;
    DSI_TEST_FUNC_EXIT;
    return QMI_NO_ERR;
  }
  DSI_TEST_FUNC_EXIT;
  return QMI_INTERNAL_ERR;
}

int dsi_test_embms_sni
(
  int                             user_handle,
  qmi_wds_start_nw_if_params_type  *params,
  qmi_wds_user_async_cb_type       user_cb,
  void                            *user_data,
  qmi_wds_call_end_reason_type    *call_end_reason_resp,
  int                             *qmi_err_code
)
{
  DSI_TEST_FUNC_ENTRY;
  if (qmi_err_code)
    qmi_err_code = QMI_NO_ERR;
  sni_wds_hndl = user_handle;
  DSI_TEST_LOG("user called SNI on wds_hndl [0x%x]", sni_wds_hndl);
  dsi_test_util_get_dev_id(sni_wds_hndl, sni_dev_id);
  DSI_TEST_LOG("user is using device [%s]", sni_dev_id);
  return qmi_sni_txn;
  DSI_TEST_FUNC_EXIT;
}

static dsi_test_qmi_vtbl_t qmi_vtbl =
{
  .wds_init_fptr = dsi_test_embms_wds_init,
  .rl_fptr = dsi_test_embms_rl,
  .sni_fptr = dsi_test_embms_sni
};

/* test logic */
int test_embms_init(dsi_test_obj_t * obj)
{
  DSI_TEST_FUNC_ENTRY;
  DSI_TEST_FUNC_EXIT;
}

int test_embms_execute(dsi_test_obj_t * obj)
{
  dsi_hndl_t hndl;

  DSI_TEST_FUNC_ENTRY;

  do
  {  
    dsi_inited = TRUE;
    /* get hndl */
    hndl = dsi_get_data_srvc_hndl(dsi_test_embms_cb, (void *)obj);
    if (NULL == hndl)
    {
      printf("couldnt' get hndl \n");
      break;
    }

    /* start EMBMS data call */
    dsi_embms_enable(hndl);

    /* generate SNI async rsp */
    dsi_test_sni_rsp(hndl, sni_wds_hndl);

    /* generate pkt srvc ind */
    dsi_test_pkt_data_connected(sni_wds_hndl, sni_dev_id);

    /* generate NET PLATFORM UP EV */
    dsi_test_platform_up(sni_dev_id);

    sleep(1);

    dsi_test_embms_activate(hndl);
    sleep(1);

    if(TRUE == evt_rcvd)
    {
      if(test_cb_event != DSI_NET_TMGI_ACTIVATED)
      {
        DSI_TEST_ERR("rcvd evt [%d] instead of NET_TMGI_ACTIVATED", test_cb_event);
        break;
      }
      else
      {
        DSI_TEST_REPORT_LOG("%s", "verified NET_TMGI_ACTIVATED rcvd");
      }
    }
    else
    {
      DSI_TEST_ERR("%s", "NET_TMGI_ACTIVATED not rcvd");
      break;
    }
    evt_rcvd = FALSE;

    dsi_test_embms_deactivate(hndl);
    sleep(1);
    if(TRUE == evt_rcvd)
    {
      if(test_cb_event != DSI_NET_TMGI_DEACTIVATED)
      {
        DSI_TEST_ERR("rcvd evt [%d] instead of NET_TMGI_DEACTIVATED", test_cb_event);
        break;
      }
      else
      {
        DSI_TEST_REPORT_LOG("%s", "verified NET_TMGI_DEACTIVATED rcvd");
      }
    }
    else
    {
      DSI_TEST_ERR("%s", "NET_TMGI_DEACTIVATED not rcvd");
      break;
    }
    evt_rcvd = FALSE;

    dsi_test_embms_act_deactivate(hndl);
    sleep(1);
    if(TRUE == evt_rcvd)
    {
      if(test_cb_event != DSI_NET_TMGI_ACTIVATED_DEACTIVATED)
      {
        DSI_TEST_ERR("rcvd evt [%d] instead of DSI_NET_TMGI_ACTIVATED_DEACTIVATED", test_cb_event);
        break;
      }
      else
      {
        DSI_TEST_REPORT_LOG("%s", "verified DSI_NET_TMGI_ACTIVATED_DEACTIVATED rcvd");
      }
    }
    else
    {
      DSI_TEST_ERR("%s", "NET_TMGI_ACTIVATED not rcvd");
      break;
    }
 
    evt_rcvd = FALSE;

    dsi_test_embms_tmgi_list_ind(sni_wds_hndl, sni_dev_id);
    sleep(1);
    if(TRUE == evt_rcvd)
    {
      if(test_cb_event != DSI_NET_TMGI_LIST_CHANGED)
      {
        DSI_TEST_ERR("rcvd evt [%d] instead of DSI_NET_TMGI_LIST_CHANGED", test_cb_event);
        break;
      }
      else
      {
        DSI_TEST_REPORT_LOG("%s", "verified DSI_NET_TMGI_LIST_CHANGED rcvd");
      }
    }
    else
    {
      DSI_TEST_ERR("%s", "DSI_NET_TMGI_LIST_CHANGED not rcvd");
      break;
    }
    evt_rcvd = FALSE;

    dsi_test_embms_sai_list_ind(sni_wds_hndl, sni_dev_id);
    sleep(1);
    if (TRUE == evt_rcvd)
    {
       if (test_cb_event != DSI_NET_SAI_LIST_CHANGED)
       {
         DSI_TEST_ERR("rcvd evt [%d] instead of DSI_NET_SAI_LIST_CHANGED", test_cb_event);
         break;
       }
       else
       {
         DSI_TEST_REPORT_LOG("%s", "verified DSI_NET_SAI_LIST_CHANGED rcvd");
       }
    }
    /* if condition is signalled, test passes */
    DSI_TEST_REPORT_LOG("[%s] PASS", obj->name);
    DSI_TEST_FUNC_EXIT;
    return;
  } while (0);

  DSI_TEST_REPORT_LOG("[%s] FAIL", obj->name);
  DSI_TEST_FUNC_EXIT;
}

dsi_test_embms_activate(dsi_hndl_t hndl)
{
   char *activate_tmgi = "ACTIVA";
   unsigned char earfcn_size = 2;
   unsigned int earfcnlist[2];
   unsigned long priority = 5;
   unsigned char sai_size = 2;
   unsigned int sai[2];
   int debug_trace_id = 33;

   earfcnlist[0]=11;
   earfcnlist[1]=22;

   sai[0] = 1;
   sai[1] = 2;

   unsigned char activate_session_id = 4;
   dsi_embms_tmgi_session_id_state_t activate_session_id_state = DSI_EMBMS_TMGI_SESSION_ID_NOT_VALID;
   dsi_embms_tmgi_activate(hndl, activate_tmgi, &(earfcnlist[0]),earfcn_size, priority, &(sai[0]), sai_size, debug_trace_id);
   dsi_test_embms_tmgi_activation_ind( sni_wds_hndl, sni_dev_id);
}

dsi_test_embms_deactivate(dsi_hndl_t hndl)
{
   char *deactivate_tmgi = "DEACTI";
   unsigned char deactivate_session_id = 4;
   dsi_embms_tmgi_session_id_state_t deactivate_session_id_state = DSI_EMBMS_TMGI_SESSION_ID_VALID;
   int dbg_trace_id = 33;

   dsi_embms_tmgi_deactivate(hndl, deactivate_tmgi, dbg_trace_id);
   dsi_test_embms_tmgi_deactivation_ind(sni_wds_hndl, sni_dev_id);
}

dsi_test_embms_act_deactivate(dsi_hndl_t hndl)
{
   char *actdeact_act_tmgi = "ACTIVA";
   unsigned char actdeact_earfcn_size = 2;
   unsigned int actdeact_earfcnlist[2];
   int actdeact_priority = 5;
   unsigned char sai_size = 2;
   unsigned int sai[2];
   int actdeact_debug_trace_id = 33;

   actdeact_earfcnlist[0]=11;
   actdeact_earfcnlist[1]=22;

   sai[0] = 1;
   sai[1] = 2;

   char *actdeact_deact_tmgi = "DEACTI";

   dsi_embms_tmgi_activate_deactivate(hndl,
                                      actdeact_act_tmgi,
                                      &(actdeact_earfcnlist[0]),
                                      actdeact_earfcn_size,
                                      actdeact_priority,
                                      actdeact_deact_tmgi,
                                      &(sai[0]),
                                      sai_size,
                                      actdeact_debug_trace_id);

   dsi_test_embms_tmgi_actdeact_ind( sni_wds_hndl, sni_dev_id);
}

int test_embms_destroy(dsi_test_obj_t * obj)
{
  DSI_TEST_FUNC_ENTRY;
  DSI_TEST_FUNC_EXIT;
}

static dsi_test_logic_vtbl_t logic_vtbl =
{
  .init_fptr = test_embms_init,
  .execute_fptr = test_embms_execute,
  .destroy_fptr = test_embms_destroy
};

dsi_test_obj_t test_embms_obj =
{
  .name = "dsi_test_embms",
  .mutex = PTHREAD_MUTEX_INITIALIZER,
  .cond = PTHREAD_COND_INITIALIZER,
  .state = DSI_TEST_STATE_MIN,
  .status = DSI_TEST_FAIL,
  .qmi_vtbl = &qmi_vtbl,
  .logic_vtbl = &logic_vtbl
};
