/*!
  @file
  qdp_test.c

  @brief
  test app for qdp

*/

/*===========================================================================
Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
===========================================================================*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
08/04/10   nd      created file

===========================================================================*/
#include <stdlib.h>
#include <stdio.h>

#include "qdp.h"
#include "qdp_platform.h"
#include "qmi_wds_srvc.h"
#include "qdp_test.h"

#define NON_FUSION_PORT "rmnet1"
#define FUSION_PORT "rmnet_usb1"

/*
Test case description:

1.) Get wds handle
2.) Create a profile with specific parameters set in modem
3.) Querry the same profile info
4.) Validate whether created profile is created properly

*/

int qdp_test_10(void)
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
  qmi_wds_addr_alloc_pref addr_pref = DHCP_USED;
  qmi_wds_lte_qos_params_type lte_qos_params;
  unsigned long bit_rate = 0x11;
  lte_qos_params.g_dl_bit_rate = bit_rate;
  lte_qos_params.max_ul_bit_rate = bit_rate;


  memset( &profile_id, 0x0, sizeof(profile_id));

  QDP_TEST_LOG("%s","qdp_test_10: prereq: use QMI to create IPv6 3GPP profile at beginning");
  QDP_TEST_LOG("qdp_test_10: 3gpp/3gpp2 profiles look up for APN [%s]ENTRY",
                QDP_TEST_APN_NAME);

  do
  {
#ifdef FEATURE_QDP_FUSION
    if (qdp_init(FUSION_PORT) == QDP_SUCCESS)
    {
      QDP_TEST_LOG("INIT QDP success on %s for Fusion", FUSION_PORT);
    }
    else
    {
      QDP_TEST_LOG("INIT QDP failed on %s for Fusion", FUSION_PORT);
    }
#else
    if (qdp_init(NON_FUSION_PORT) == QDP_SUCCESS)
    {
      QDP_TEST_LOG("INIT QDP success on %s for non-Fusion", NON_FUSION_PORT);
    }
    else
    {
      QDP_TEST_LOG("INIT QDP failed on %s for non-Fusion", NON_FUSION_PORT);
    }
#endif
    memset(params, 0, sizeof(char *)*QDP_RIL_PARAM_MAX);

    /* create 3GPP profile with "qdp_test_apn" as APN */
#ifdef FEATURE_QDP_FUSION
    qmi_wds_hndl = qmi_wds_srvc_init_client(FUSION_PORT,
                                            NULL,
                                            NULL,
                                            &qmi_err_code);
    QDP_TEST_LOG("INIT qmi_wds_srvc_init_client on %s for Fusion", FUSION_PORT);
#else
    qmi_wds_hndl = qmi_wds_srvc_init_client(NON_FUSION_PORT,
                                            NULL,
                                            NULL,
                                            &qmi_err_code);
    QDP_TEST_LOG("INIT qmi_wds_srvc_init_client on %s for Non-Fusion", NON_FUSION_PORT);
#endif

    if (qmi_wds_hndl < 0)
    {
      QDP_LOG_ERROR("can not init wds hndl err=[%d]",
                    qmi_err_code);
    }

    memset(&profile_params, 0, sizeof(profile_params));
    profile_params.umts_profile_params.param_mask |=
      QMI_WDS_UMTS_PROFILE_PDP_TYPE_PARAM_MASK;

    profile_params.umts_profile_params.pdp_type =
      QMI_WDS_PDP_TYPE_IPV6;

    profile_params.umts_profile_params.param_mask |=
      QMI_WDS_UMTS_PROFILE_APN_NAME_PARAM_MASK;

    std_strlcpy(profile_params.umts_profile_params.apn_name,
                "qdp-cr_nandan",
                strlen(QDP_TEST_APN_NAME)+1);

    QDP_TEST_LOG("profile setting addr_alloc_pref to %d", addr_pref);

    profile_params.umts_profile_params.param_mask |=
      QMI_WDS_UMTS_PROFILE_ADDR_ALLOC_PREF_PARAM_MASK;

    profile_params.umts_profile_params.addr_alloc_pref=
      addr_pref;

    QDP_TEST_LOG("profile setting lte qos g_dl_bit_rate to %lud", lte_qos_params.g_dl_bit_rate);
    QDP_TEST_LOG("profile setting lte qos max_ul_bit_rate to %lud", lte_qos_params.max_ul_bit_rate);

    profile_params.umts_profile_params.param_mask |=
      QMI_WDS_UMTS_PROFILE_LTE_QOS_PARAMS_PARAM_MASK;
    profile_params.umts_profile_params.lte_qos_params=
      lte_qos_params;

    profile_params.umts_profile_params.param_mask |=
      QMI_WDS_UMTS_PROFILE_APN_DISABLED_FLAG_PARAM_MASK;

    profile_params.umts_profile_params.apn_disabled_flag = 1;
    QDP_TEST_LOG("profile setting apn_disabled_flag to %d",
      profile_params.umts_profile_params.apn_disabled_flag);

    profile_params.umts_profile_params.param_mask |=
      QMI_WDS_UMTS_PROFILE_PDN_INACTIVITY_TIMEOUT_PARAM_MASK;
    profile_params.umts_profile_params.pdn_inactivity_timeout= 12;

    QDP_TEST_LOG("profile setting pdn_inactivity_timeout to %lud",
      profile_params.umts_profile_params.pdn_inactivity_timeout);

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
    }

    QDP_TEST_LOG("profile [%d] created",(int)profile_id.profile_index);
    profile_created = TRUE;

    QDP_TEST_LOG("%s", "Profile created. Enter any key to move forward\n");
    getchar();

    params[QDP_RIL_APN] = (char *)malloc((strlen(QDP_TEST_APN_NAME)+1)*sizeof(char));
    if (NULL == params[QDP_RIL_APN])
    {
      QDP_LOG_ERROR("%s","memory error");
      break;
    }
    std_strlcpy(params[QDP_RIL_APN],QDP_TEST_APN_NAME, strlen(QDP_TEST_APN_NAME)+1);

    params[QDP_RIL_IP_FAMILY] = (char *)malloc((strlen(QDP_RIL_IP_6)+1)*sizeof(char));;
    if (NULL == params[QDP_RIL_IP_FAMILY])
    {
      QDP_LOG_ERROR("%s","memory error");
      break;
    }
    std_strlcpy(params[QDP_RIL_IP_FAMILY],QDP_RIL_IP_6, strlen(QDP_RIL_IP_6)+1);

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
    /* Query the profile info and match */
    memset(&profile_params, 0, sizeof(profile_params));

    rc = qmi_wds_query_profile
    (
      qmi_wds_hndl,
      &profile_id,
      &profile_params,
      &qmi_err_code
    );
    /* Verify the parameters */
    QDP_TEST_LOG("profile addr_alloc_pref is %d",
    profile_params.umts_profile_params.addr_alloc_pref );
    QDP_TEST_LOG("profile  lte qos g_dl_bit_rate to %lud",
      profile_params.umts_profile_params.lte_qos_params.g_dl_bit_rate);
    QDP_TEST_LOG("profile  lte qos max_ul_bit_rate to %lud",
      profile_params.umts_profile_params.lte_qos_params.max_ul_bit_rate);
    QDP_TEST_LOG("profile  apn_disabled_flag to %d",
      profile_params.umts_profile_params.apn_disabled_flag);
    QDP_TEST_LOG("profile  pdn_inactivity_timeout  to %lud",
      profile_params.umts_profile_params.pdn_inactivity_timeout);


    QDP_TEST_LOG("%s", "Enter any key to quit\n");
    getchar();

    qdp_deinit();

    if (QMI_NO_ERR != rc)
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
  if (QDP_SUCCESS == qdp_test_10())
  {
    QDP_TEST_LOG("%s","QDP_TEST PASS");
  }
  else
  {
    QDP_TEST_LOG("%s","QDP_TEST FAIL");
  }

  return 0;
}
