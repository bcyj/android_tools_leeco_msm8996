/*!
  @file
  dsi_test_template.c

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
#include "dsi_netctrl.h"

/* QMI behavior */

int dsi_test_template_rl
(
  int                               user_handle,
  qmi_wds_route_look_up_params_type *params,
  qmi_wds_route_look_up_rsp_type    *rsp_data,
  int                               *qmi_err_code
)
{
  if (rsp_data)
  {
    rsp_data->iface_handle = 0x01;
    rsp_data->priority = 0;
    rsp_data->tech_name = 0x8004;
    rsp_data->qmi_inst_is_valid = 0;
    rsp_data->qmi_inst = 0;
    return QMI_NO_ERR;
  }
  
  return QMI_INTERNAL_ERR;
}

int dsi_test_template_sni
(
  int                             user_handle,
  qmi_wds_start_nw_if_params_type  *params,
  qmi_wds_user_async_cb_type       user_cb,
  void                            *user_data,
  qmi_wds_call_end_reason_type    *call_end_reason_resp,
  int                             *qmi_err_code
)
{
  if (qmi_err_code)
    qmi_err_code = QMI_SERVICE_ERR_NONE;
  return QMI_NO_ERR;
}

static dsi_test_qmi_vtbl_t qmi_vtbl =
{
  .wds_init_fptr = NULL,
  .rl_fptr = dsi_test_template_rl,
  .sni_fptr = dsi_test_template_sni
};

/* test logic */
int test_template_init(dsi_test_obj_t * obj)
{
  printf("\n**test_template_init called**\n");
  dsi_init(DSI_MODE_GENERAL);
}

int test_template_execute(dsi_test_obj_t * obj)
{
  printf("\n**test_template_execute called**\n");
}

int test_template_destroy(dsi_test_obj_t * obj)
{
  printf("\n**test_template_destroy called**\n");
}

static dsi_test_logic_vtbl_t logic_vtbl =
{
  .init_fptr = test_template_init,
  .execute_fptr = test_template_execute,
  .destroy_fptr = test_template_destroy
};

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

dsi_test_obj_t test_template_obj =
{
  .name = "test_template",
  .mutex = PTHREAD_MUTEX_INITIALIZER,
  .cond = PTHREAD_COND_INITIALIZER,
  .state = DSI_TEST_STATE_MIN,
  .status = DSI_TEST_FAIL,
  .qmi_vtbl = &qmi_vtbl,
  .logic_vtbl = &logic_vtbl
};
