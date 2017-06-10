/*!
  @file
  dsi_test_engine.c

  @brief
  This file drives various test scenarios by going through the array
  of test objects. 

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

static int qmi_wds_client = 0x01;

extern test_template_obj;
extern test_1_obj;
extern test_embms_obj;

dsi_test_obj_t * dsi_test_obj_arr[] = { &test_template_obj, &test_1_obj, &test_embms_obj };

dsi_test_obj_t * global_test_obj;
int main()
{
  int i=0;

  for(i=0;i<(sizeof(dsi_test_obj_arr)/sizeof(dsi_test_obj_t *));i++)
  {
    printf("\n\n<<<running test [%d]>>>\n\n", i);
    global_test_obj = dsi_test_obj_arr[i];
    dsi_test_obj_arr[i]->logic_vtbl->init_fptr(dsi_test_obj_arr[i]);
    dsi_test_obj_arr[i]->logic_vtbl->execute_fptr(dsi_test_obj_arr[i]);
    dsi_test_obj_arr[i]->logic_vtbl->destroy_fptr(dsi_test_obj_arr[i]);
  }

  return 0;
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
  DSI_TEST_FUNC_ENTRY;
  if (global_test_obj->qmi_vtbl->wds_init_fptr)
  {
    DSI_TEST_FUNC_EXIT;
    return global_test_obj->qmi_vtbl->wds_init_fptr(dev_id,
                                                    user_ind_msg_hdlr,
                                                    user_ind_msg_hdlr_user_data,
                                                    qmi_err_code);
  }
  else
  {
    DSI_TEST_FUNC_EXIT;
    return qmi_wds_client++;
  }

  DSI_TEST_FUNC_EXIT;
}

int
qmi_wds_route_look_up
(
  int                               user_handle,
  qmi_wds_route_look_up_params_type *params,
  qmi_wds_route_look_up_rsp_type    *rsp_data,
  int                               *qmi_err_code
)
{
  int qmi_ret = QMI_NO_ERR;
  DSI_TEST_FUNC_ENTRY;

  if (global_test_obj->qmi_vtbl->rl_fptr)
  {
    /* custom behavior */
    DSI_TEST_FUNC_EXIT;
    return global_test_obj->qmi_vtbl->rl_fptr(user_handle,
                                              params,
                                              rsp_data,
                                              qmi_err_code);
  }
  else
  {
    /* standard behavior */
    if (rsp_data)
    {
      rsp_data->tech_name = 0x8004;
      rsp_data->qmi_inst_is_valid = FALSE;
      rsp_data->iface_handle = 0x01;
      rsp_data->priority = 0;
    }
    else
      qmi_ret = QMI_INTERNAL_ERR;
  }

  DSI_TEST_FUNC_EXIT;
  return qmi_ret;

}

int
qmi_wds_start_nw_if
(
  int                               user_handle,
  qmi_wds_start_nw_if_params_type  *params,
  qmi_wds_user_async_cb_type       user_cb,
  void                            *user_data,
  qmi_wds_call_end_reason_type    *call_end_reason_resp,
  int                             *qmi_err_code
)
{
  DSI_TEST_FUNC_ENTRY;
  if (global_test_obj->qmi_vtbl->sni_fptr)
  {
    DSI_TEST_FUNC_EXIT;
    /* custom behavior */
    return global_test_obj->qmi_vtbl->sni_fptr(user_handle,
                                               params,
                                               user_cb,
                                               user_data,
                                               call_end_reason_resp,
                                               qmi_err_code);
  }
  else
  {
    /* return positive txn handle */
    DSI_TEST_FUNC_EXIT;
    return 0x01;
  }
}

