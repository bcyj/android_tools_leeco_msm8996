/*!
  @file
  qdp_test_06.c

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
08/06/10   js      created file

===========================================================================*/
#include <stdlib.h>
#include <stdio.h>

#include "qdp.h"
#include "qdp_platform.h"
#include "qdp_test.h"


int qdp_test_06(void)
{
  int ret = QDP_FAILURE;
  int i=0, rc;
  char * params[QDP_RIL_PARAM_MAX];
  unsigned int profile_id_3gpp = 0, profile_id_3gpp2 = 0;
  qdp_profile_pdn_type profile_3gpp_pdn, profile_3gpp2_pdn;
  qdp_error_info_t error_info;

  QDP_TEST_LOG("qdp_test_06: 3gpp/3gpp2 profile look up with ip6 nai [%s] ENTRY",
                QDP_TEST_NAI);

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

    params[QDP_RIL_NAI] = (char *)malloc((strlen(QDP_TEST_NAI)+1)*sizeof(char));
    if (NULL == params[QDP_RIL_NAI])
    {
      QDP_LOG_ERROR("%s","memory error");
      break;
    }
    QDP_TEST_LOG("malloc'ed [%p]", params[QDP_RIL_NAI]);
    strlcpy(params[QDP_RIL_NAI], QDP_TEST_NAI, strlen(QDP_TEST_NAI)+1);

    params[QDP_RIL_IP_FAMILY] = (char *)malloc(sizeof(char)*2);
    if (NULL == params[QDP_RIL_IP_FAMILY])
    {
      QDP_LOG_ERROR("%s","memory error");
      break;
    }
    QDP_TEST_LOG("malloc'ed [%p]", params[QDP_RIL_IP_FAMILY]);
    strlcpy(params[QDP_RIL_IP_FAMILY], QDP_RIL_IP_6, strlen(QDP_RIL_IP_6)+1);

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
  if (QDP_SUCCESS == qdp_test_06())
  {
    QDP_TEST_LOG("%s","QDP_TEST_06 PASS");
  }
  else
  {
    QDP_TEST_LOG("%s","QDP_TEST_06 FAIL");
  }

  return 0;
}
