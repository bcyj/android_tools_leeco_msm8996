/*!
  @file
  qdp_test_create_profile.c

  @brief
  test app for qdp

*/

/*===========================================================================

  Copyright (c) 2011,2013 Qualcomm Technologies, Inc. All Rights Reserved

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

when       who     what, where, why
--------   ---     ----------------------------------------------------------
04/20/11   js      created file

===========================================================================*/
#include <stdlib.h>
#include <stdio.h>

#include "qdp.h"
#include "qdp_platform.h"
#include "qdp_test.h"
#include "qmi_wds_srvc.h"


int qdp_test_create_profile(void)
{
  int ret = QDP_FAILURE;
  int i=0, rc, global_qmi_wds_hndl;
  qmi_wds_profile_id_type profile_id;
  qmi_wds_profile_params_type profile_params;
  char * global_qmi_port = NULL;
  int qmi_err_code;

  QDP_TEST_LOG("qdp_test_create_profile: create 3gpp profile with ip6 family and apn [%s]ENTRY",
               QDP_TEST_APN_NAME);

  do
  {
#ifdef FEATURE_QDP_FUSION
    global_qmi_port = QMI_PORT_RMNET_SDIO_0;
    QDP_TEST_LOG("%s","INIT QMI on RMNET_SIO_0 for Fusion");
#else
    global_qmi_port = QMI_PORT_RMNET_0;
    QDP_TEST_LOG("%s","INIT QMI on RMNET0 for non-Fusion");
#endif

    qmi_init(NULL,NULL);
    global_qmi_wds_hndl = qmi_wds_srvc_init_client(global_qmi_port,
                                                   NULL,
                                                   NULL,
                                                   &qmi_err_code);
    if (global_qmi_wds_hndl < 0)
    {
      QDP_TEST_LOG("invalid qmi_wds_hndl [0x%x] returned. "
                   "qmi_err_code is set to [%d]", global_qmi_wds_hndl,
                   qmi_err_code);
      break;
    }

    /* start with clean set of params */
    memset(&profile_params, 0, sizeof(profile_params));
    memset(&profile_id, 0, sizeof(profile_id));
    profile_params.umts_profile_params.param_mask |=
      QMI_WDS_UMTS_PROFILE_APN_NAME_PARAM_MASK;
    strlcpy(profile_params.umts_profile_params.apn_name,
                QDP_TEST_APN_NAME,
                strlen(QDP_TEST_APN_NAME)+1);
    profile_params.umts_profile_params.param_mask |=
      QMI_WDS_UMTS_PROFILE_PDP_TYPE_PARAM_MASK;
    profile_params.umts_profile_params.pdp_type =
      QMI_WDS_PDP_TYPE_IPV6;
    rc = qmi_wds_create_profile(
      global_qmi_wds_hndl,
      &profile_id,
      &profile_params,
      &qmi_err_code);
    if (QMI_NO_ERR != rc)
    {
      QDP_TEST_LOG("could not create wds profile. "
                   "qmi returned [%d] qmi_err_code [%d]",
                   rc,qmi_err_code);
      break;
    }
    QDP_TEST_LOG("IPv6 profile [%lu] created",
                 profile_id.profile_index);

    /* start with clean set of params */
    memset(&profile_params, 0, sizeof(profile_params));
    memset(&profile_id, 0, sizeof(profile_id));
    profile_params.umts_profile_params.param_mask |=
      QMI_WDS_UMTS_PROFILE_APN_NAME_PARAM_MASK;
    strlcpy(profile_params.umts_profile_params.apn_name,
                QDP_TEST_APN_NAME,
                strlen(QDP_TEST_APN_NAME)+1);
    profile_params.umts_profile_params.param_mask |=
      QMI_WDS_UMTS_PROFILE_PDP_TYPE_PARAM_MASK;
    profile_params.umts_profile_params.pdp_type =
      QMI_WDS_PDP_TYPE_IPV4;
    rc = qmi_wds_create_profile(
      global_qmi_wds_hndl,
      &profile_id,
      &profile_params,
      &qmi_err_code);
    if (QMI_NO_ERR != rc)
    {
      QDP_TEST_LOG("could not create wds profile. "
                   "qmi returned [%d] qmi_err_code [%d]",
                   rc,qmi_err_code);
      break;
    }
    QDP_TEST_LOG("IPv4 profile [%lu] created",
                 profile_id.profile_index);

    rc = qmi_wds_srvc_release_client(global_qmi_wds_hndl, &qmi_err_code);
    if (rc < 0)
    {
      QDP_TEST_LOG("wds srvc release failed. qmi_ret=[%d],qmi_err=[%d]",
                   rc,qmi_err_code);
      break;
    }

    ret = QDP_SUCCESS;
  } while (0);
  
  return ret;
}

int main(void)
{
  if (QDP_SUCCESS == qdp_test_create_profile())
  {
    QDP_TEST_LOG("%s","QDP_TEST_CREATE_PROFILE PASS");
  }
  else
  {
    QDP_TEST_LOG("%s","QDP_TEST_CREATE_PROFILE FAIL");
  }

  return 0;
}
