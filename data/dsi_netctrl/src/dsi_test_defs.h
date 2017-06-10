/*!
  @file
  dsi_test_defs.h

  @brief
  This file contains definitions used by dsi offtarget test framework

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
#ifndef __DSI_TEST_DEFS_H
#define __DSI_TEST_DEFS_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "qmi_wds_srvc.h"
#include "comdef.h"
#include "dsi_netctrli.h"

#define DSI_TEST_FUNC_ENTRY printf("** %s ENTRY ** \n", __FUNCTION__)
#define DSI_TEST_FUNC_EXIT printf("** %s EXIT ** \n", __FUNCTION__)
#define DSI_TEST_LOG(fmt, ...) \
  printf("%s:%d ",__FILE__, __LINE__); \
  printf(fmt, __VA_ARGS__); \
  printf("\n")
#define DSI_TEST_ERR(fmt, ...) \
  printf("%s:%d ",__FILE__, __LINE__); \
  printf("\n**ERR** "); \
  printf(fmt, __VA_ARGS__); \
  printf("\n")
#define DSI_TEST_REPORT_LOG(fmt, ...) printf("\n## "); \
  printf(fmt, __VA_ARGS__); \
  printf(" ##\n")

typedef enum dsi_test_state_e
{
  DSI_TEST_STATE_MIN,
  DSI_TEST_STATE_MAX
} dsi_test_state_t;

typedef enum dsi_test_status_e
{
  DSI_TEST_PASS,
  DSI_TEST_FAIL
} dsi_test_status_t;

typedef int
(* qmi_wds_start_nw_if_fptr)
(
  int                             user_handle,
  qmi_wds_start_nw_if_params_type  *params,
  qmi_wds_user_async_cb_type       user_cb,
  void                            *user_data,
  qmi_wds_call_end_reason_type    *call_end_reason_resp,
  int                             *qmi_err_code
);

typedef int
(* qmi_wds_route_look_up_fptr)
(
  int                               user_handle,
  qmi_wds_route_look_up_params_type *params,
  qmi_wds_route_look_up_rsp_type    *rsp_data,
  int                               *qmi_err_code
);

typedef qmi_client_handle_type
(* qmi_wds_srvc_init_client_fptr)
(
  const char                    *dev_id,
  qmi_wds_indication_hdlr_type  user_ind_msg_hdlr,
  void                          *user_ind_msg_hdlr_user_data,
  int                           *qmi_err_code
);

/* per test case custom QMI behavior */
typedef struct dsi_test_qmi_vtbl_s
{
  qmi_wds_srvc_init_client_fptr wds_init_fptr;
  qmi_wds_route_look_up_fptr rl_fptr;
  qmi_wds_start_nw_if_fptr sni_fptr;
} dsi_test_qmi_vtbl_t;

/* forward decl */
struct dsi_test_obj_s;

typedef int
(* dsi_test_fptr)(struct dsi_test_obj_s * obj);

/* test logic */
typedef struct dsi_test_logic_s
{
  dsi_test_fptr init_fptr; /* init stuff */
  dsi_test_fptr execute_fptr; /* calls API, CB */
  dsi_test_fptr destroy_fptr; /* release stuff */
} dsi_test_logic_vtbl_t;

typedef struct dsi_test_obj_s
{
  char name[100];
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  dsi_test_state_t state;
  dsi_test_status_t status;
  dsi_test_qmi_vtbl_t * qmi_vtbl;
  dsi_test_logic_vtbl_t * logic_vtbl;
} dsi_test_obj_t;

#endif
