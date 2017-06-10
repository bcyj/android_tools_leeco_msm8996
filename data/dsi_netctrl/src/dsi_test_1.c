/*!
  @file
  dsi_test_1.c

  @brief
  This is a test case for dsi_netctrl.c

*/

/*===========================================================================

  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved

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
#include <stdio.h>
#include "dsi_test_defs.h"
#include "dsi_netctrl.h" /* DSI_NETCTRL API */
#include "dsi_netctrl_cb_thrd.h" /* SIMULATE CALLBACKS */
#include <sys/socket.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int test_cb_event = DSI_EVT_INVALID;

/* cb function registered with dsi_netctrl */
void dsi_test_1_cb(dsi_hndl_t hndl,
                   void * user_data,
                   dsi_net_evt_t evt)
{
  DSI_TEST_FUNC_ENTRY;

  pthread_mutex_lock(&mutex);
  test_cb_event = (int)evt;
  switch(evt)
  {
  case DSI_EVT_NET_IS_CONN:
    DSI_TEST_REPORT_LOG("%s","dsi_test_1_cb: evt DSI_EVT_NET_IS_CONN rcvd");
    break;
  case DSI_EVT_NET_NO_NET:
    DSI_TEST_REPORT_LOG("%s","dsi_test_1_cb: evt DSI_EVT_NET_NO_NET rcvd");
    break;
  default:
    DSI_TEST_REPORT_LOG("dsi_test_1_cb: evt [%d] rcvd", evt);
    break;
  }
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);

  DSI_TEST_FUNC_EXIT;
}

/* QMI behavior */
unsigned long um_hndl = 0x01;
qmi_wds_technology_type ps_tech = 0x8004;
static int qmi_sni_txn = 0xffff;
static int qmi_wds_hndl = 0x01;
static int sni_wds_hndl = 0x01;
static char sni_dev_id[100] = "rmnet_sdio0";
static void * qmi_user_data;
static struct sockaddr_storage sock;

qmi_client_handle_type
dsi_test_1_wds_init
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

int dsi_test_1_rl
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

int dsi_test_1_sni
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
  .wds_init_fptr = dsi_test_1_wds_init,
  .rl_fptr = dsi_test_1_rl,
  .sni_fptr = dsi_test_1_sni
};

/* test logic */
int test_1_init(dsi_test_obj_t * obj)
{
  DSI_TEST_FUNC_ENTRY;
  dsi_init(DSI_MODE_GENERAL);
  DSI_TEST_FUNC_EXIT;
}

int test_1_execute(dsi_test_obj_t * obj)
{
  dsi_hndl_t hndl;

  DSI_TEST_FUNC_ENTRY;

  do
  {
    dsi_inited = TRUE;
    /* get hndl */
    hndl = dsi_get_data_srvc_hndl(dsi_test_1_cb, (void *)obj);
    if (NULL == hndl)
    {
      printf("couldnt' get hndl \n");
      break;
    }

    /* statt data call */
    dsi_start_data_call(hndl);

    /* generate SNI async rsp */
    dsi_test_sni_rsp(hndl, sni_wds_hndl);

    /* generate pkt srvc ind */
    dsi_test_pkt_data_connected(sni_wds_hndl, sni_dev_id);

    /* generate NET PLATFORM UP EV */
    dsi_test_platform_up(sni_dev_id);
        
    /* wait for cond variable */
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    if (test_cb_event != DSI_EVT_NET_IS_CONN)
    {
      DSI_TEST_ERR("rcvd evt [%d] instead of NET_IS_CONN", test_cb_event);
      break;
    }
    pthread_mutex_unlock(&mutex);
    DSI_TEST_REPORT_LOG("%s","verified NET_IS_CONN rcvd");

    /* modem oos */
    qmi_sys_event_info_type sys_event_info;
    dsi_qmi_sys_cb(QMI_SYS_EVENT_MODEM_OUT_OF_SERVICE_IND,
                   &sys_event_info,
                   NULL);


    /* wait for cond variable */
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    if (test_cb_event != DSI_EVT_NET_NO_NET)
    {
      DSI_TEST_ERR("rcvd evt [%d] instead of NET_NO_NET", test_cb_event);
      break;
    }
    pthread_mutex_unlock(&mutex);
    
    DSI_TEST_REPORT_LOG("%s","verified NET_NO_NET rcvd in modem oos");

    /* get hndl */
    hndl = dsi_get_data_srvc_hndl(dsi_test_1_cb, (void *)obj);
    if (NULL != hndl)
    {
      DSI_TEST_ERR("able to get [%d] hndl when modem oos", hndl);
      break;
    }
    else
    {
      DSI_TEST_REPORT_LOG("%s","verified can't get hndl when modem oos");
    }    

    /* modem is */
    dsi_qmi_sys_cb(QMI_SYS_EVENT_MODEM_IN_SERVICE_IND,
                   &sys_event_info,
                   NULL);

    /* let dsi_netctrl lib be re-initialized again */
    sleep(1);

    /* get hndl */
    hndl = dsi_get_data_srvc_hndl(dsi_test_1_cb, (void *)obj);
    if (NULL == hndl)
    {
      DSI_TEST_ERR("%s","couldnt' get hndl \n");
      break;
    }

    /* statt data call */
    dsi_start_data_call(hndl);

    /* generate SNI async rsp */
    dsi_test_sni_rsp(hndl, sni_wds_hndl);

    /* generate pkt srvc ind */
    dsi_test_pkt_data_connected(sni_wds_hndl, sni_dev_id);

    /* generate NET PLATFORM UP EV */
    dsi_test_platform_up(sni_dev_id);
        
    /* wait for cond variable */
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    if (test_cb_event != DSI_EVT_NET_IS_CONN)
    {
      DSI_TEST_ERR("rcvd evt [%d] instead of NET_IS_CONN", test_cb_event);
      break;
    }
    pthread_mutex_unlock(&mutex);
    DSI_TEST_REPORT_LOG("%s","verified NET_IS_CONN rcvd after modem is");

    /* generate NET PLATFORM DOWN EV */
    dsi_test_platform_down(sni_dev_id);

    /* wait for cond variable */
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    if (test_cb_event != DSI_EVT_NET_NO_NET)
    {
      DSI_TEST_ERR("rcvd evt [%d] instead of NET_NO_NET", test_cb_event);
      break;
    }
    pthread_mutex_unlock(&mutex);
    DSI_TEST_REPORT_LOG("%s","verified NET_NO_NET rcvd after test posted platform down");

    /* if condition is signalled, test passes */
    DSI_TEST_REPORT_LOG("[%s] PASS", obj->name);
    DSI_TEST_FUNC_EXIT;
    return;
  } while (0);

  DSI_TEST_REPORT_LOG("[%s] FAIL", obj->name);
  DSI_TEST_FUNC_EXIT;
}

int test_1_destroy(dsi_test_obj_t * obj)
{
  DSI_TEST_FUNC_ENTRY;
  DSI_TEST_FUNC_EXIT;
}

static dsi_test_logic_vtbl_t logic_vtbl =
{
  .init_fptr = test_1_init,
  .execute_fptr = test_1_execute,
  .destroy_fptr = test_1_destroy
};

dsi_test_obj_t test_1_obj =
{
  .name = "dsi_test_1",
  .mutex = PTHREAD_MUTEX_INITIALIZER,
  .cond = PTHREAD_COND_INITIALIZER,
  .state = DSI_TEST_STATE_MIN,
  .status = DSI_TEST_FAIL,
  .qmi_vtbl = &qmi_vtbl,
  .logic_vtbl = &logic_vtbl
};
