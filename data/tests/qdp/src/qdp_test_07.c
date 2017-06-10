/*!
  @file
  qdp_test.c

  @brief
  test app for qdp

*/

/*===========================================================================

  Copyright (c) 2010-2013 Qualcomm Technologies, Inc. All Rights Reserved

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
08/04/10   js      created file

===========================================================================*/
#include <stdlib.h>
#include <stdio.h>

#include "qdp.h"
#include "qdp_platform.h"
#include "qmi_wds_srvc.h"
#include "qdp_test.h"


int qdp_test_07(void)
{
  int ret = QDP_FAILURE;
  int i=0, rc, qmi_err_code;
  char * params[QDP_RIL_PARAM_MAX];
  unsigned int profile_id_3gpp = 0, profile_id_3gpp2 = 0;
  qdp_profile_pdn_type profile_3gpp_pdn, profile_3gpp2_pdn;
  int qmi_wds_hndl = -1;
  qmi_wds_profile_id_type profile_id;
  qmi_wds_profile_params_type profile_params;
  boolean profile_created = FALSE;
  qdp_error_info_t error_info;

  QDP_TEST_LOG("%s","qdp_test_07: prereq: use QMI to create 3GPP profile at beginning");
  QDP_TEST_LOG("qdp_test_07: 3gpp/3gpp2 profiles look up for APN [%s] ENTRY",
                QDP_TEST_APN_NAME);

  do
  {
#ifdef FEATURE_QDP_FUSION
    qdp_init("rmnet3");
    QDP_TEST_LOG("%s","INIT QDP on RMNET3 for Fusion");
#else
    qdp_init("rmnet0");
    QDP_TEST_LOG("%s","INIT QDP on RMNET0 for non-Fusion");
#endif

    memset(params, 0, sizeof(char *)*QDP_RIL_PARAM_MAX);

    /* create 3GPP profile with "qdp_test_apn" as APN */
    qmi_wds_hndl = qmi_wds_srvc_init_client("rmnet3",
                                            NULL,
                                            NULL,
                                            &qmi_err_code);
    if (qmi_wds_hndl < 0)
    {
      QDP_LOG_ERROR("can not init wds hndl  qmi_wds_hndl=[%d], err=[%d]",
                    qmi_wds_hndl, qmi_err_code);
    }

    memset(&profile_params, 0, sizeof(profile_params));
    profile_params.umts_profile_params.param_mask |=
      QMI_WDS_UMTS_PROFILE_APN_NAME_PARAM_MASK;
    strlcpy(profile_params.umts_profile_params.apn_name,
                QDP_TEST_APN_NAME,
                strlen(QDP_TEST_APN_NAME)+1);
    profile_id.technology = QMI_WDS_PROFILE_TECH_3GPP;

    rc = qmi_wds_create_profile(
      qmi_wds_hndl,
      &profile_id,
      &profile_params,
      &qmi_err_code);
    if (QMI_NO_ERR != rc)
    {
      QDP_LOG_ERROR("can not create prereq 3gpp profile rc=[%d], err=[%d]",
                    rc, qmi_err_code);
      break;
    }

    QDP_TEST_LOG("profile [%d] created",(int)profile_id.profile_index);
    profile_created = TRUE;

    printf("Profile created. Enter any key to move forward\n");
    getchar();

    params[QDP_RIL_APN] = (char *)malloc((strlen(QDP_TEST_APN_NAME)+1)*sizeof(char));
    if (NULL == params[QDP_RIL_APN])
    {
      QDP_LOG_ERROR("%s","memory error");
      break;
    }
    QDP_TEST_LOG("malloc'ed [%p]", params[QDP_RIL_APN]);
    strlcpy(params[QDP_RIL_APN],QDP_TEST_APN_NAME, strlen(QDP_TEST_APN_NAME)+1);

    rc = qdp_profile_look_up( (const char **)params,
                              &profile_id_3gpp,
                              &profile_3gpp_pdn,
                              &profile_id_3gpp2,
                              &profile_3gpp2_pdn,
                              &error_info );

    QDP_TEST_LOG("profile id [%d] 3gpp found/created", profile_id_3gpp);
    QDP_TEST_LOG("profile id [%d] 3gpp2 found/created", profile_id_3gpp2);
    QDP_TEST_LOG("error code[%d] tech[%d]", error_info.error, error_info.tech);

    if (0 != profile_id_3gpp)
    {
      qdp_profile_release(profile_id_3gpp);
      QDP_TEST_LOG("profile id [%d] 3gpp released", profile_id_3gpp);
    }
    if (0 != profile_id_3gpp2)
    {
      qdp_profile_release(profile_id_3gpp2);
      QDP_TEST_LOG("profile id [%d] 3gpp2 released", profile_id_3gpp2);
    }

    printf("Enter any key to quit\n");
    getchar();

    qdp_deinit();

    if (QDP_FAILURE == rc)
    {
      QDP_LOG_ERROR("qdp_profile_look_up failed err [%d]",
                    rc);
      break;
    }


    ret = QDP_SUCCESS;
  } while (0);

  if (profile_created)
  {
    rc = qmi_wds_delete_profile(qmi_wds_hndl,
                                &profile_id,
                                &qmi_err_code);
    if (QMI_NO_ERR != rc)
    {
      QDP_TEST_LOG("profile [%d] deleted",(int)profile_id.profile_index);
    }
    else
    {
      QDP_LOG_ERROR("could not delete profile [%d]. rc=[%d],qmi_err=[%d]",profile_id.profile_index,
                    rc,qmi_err_code);
    }
  }

  if (qmi_wds_hndl >= 0)
  {
    rc = qmi_wds_srvc_release_client(qmi_wds_hndl, &qmi_err_code);
    if (QMI_NO_ERR != rc)
    {
      QDP_LOG_ERROR("can not release wds hndl  rc=[%d], err=[%d]",
                    rc, qmi_err_code);
    }
  }

  for(i=0; i<QDP_RIL_PARAM_MAX; i++)
  {
    if (NULL != params[i])
    {
      QDP_TEST_LOG("freeing [%p]", params[i]);
      free(params[i]);
    }
  }

  if (QDP_SUCCESS != ret)
  {
    QDP_LOG_ERROR("%s","qdp_init EXIT failed");
  }
  else
  {
    QDP_TEST_LOG("%s","qdp_init EXIT success");
  }

  return ret;
}

int main(void)
{
  if (QDP_SUCCESS == qdp_test_07())
  {
    QDP_TEST_LOG("%s","QDP_TEST PASS");
  }
  else
  {
    QDP_TEST_LOG("%s","QDP_TEST FAIL");
  }

  return 0;
}
