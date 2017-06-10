/*!
  @file
  omh_profile_test.c

  @brief
  test app for QCRIL OHM profile lookup

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

when       who     what, where, why
--------   ---     ----------------------------------------------------------
08/04/10   ar      created file

===========================================================================*/
#include <stdlib.h>
#include <stdio.h>

#include "qmi_platform_config.h"
#include "qmi_wds_srvc.h"
#include "qmi_wds_utils.h"

#define SUCCESS (0)
#define FAILURE (-1)
#define HNDL_INVALID (-1)
#define NUM_UMTS_PROFILES_EXPECTED (8)
#define NUM_OMH_PROFILES_EXPECTED  (6)
#define TEST_PROFILES (4)

int omh_profile_test(void)
{
  int ret = FAILURE;
  int i=0, j=0, rc, qmi_err_code;
  unsigned int profile_id_3gpp2[TEST_PROFILES] = {1,2,32,64};
  int qmi_wds_omh_hndl = HNDL_INVALID;
  int num_elements_expected = NUM_OMH_PROFILES_EXPECTED;
  qmi_wds_profile_list_type result_list[NUM_OMH_PROFILES_EXPECTED];
  int qmi_handle = QMI_INVALID_CLIENT_HANDLE; 
    
  fprintf(stderr, "%s: ENTRY\n", __func__ );
  
  qmi_handle = qmi_init( NULL, NULL );

  /*get qmi wds client handle*/
  qmi_wds_omh_hndl = qmi_wds_srvc_init_client(QMI_PORT_RMNET_1,
					      NULL,
					      NULL,
					      &qmi_err_code);

  if (qmi_wds_omh_hndl < 0)
  {
    fprintf(stderr, "%s: invalid qmi_wds_omh_hndl [0x%08x] returned. "
	    "qmi_err_code is set to [%d]\n",
	    __func__, qmi_wds_omh_hndl, qmi_err_code);
    qmi_wds_omh_hndl = HNDL_INVALID;
    return ret;
  }
#if 0
  fprintf(stderr, "%s: qmi_wds_omh_hndl [0x%08x] initialized. "
	  "qmi_err_code is set to [%d]\n",
	  __func__, qmi_wds_omh_hndl, qmi_err_code);
#endif
    
  do
  {
    fprintf(stderr, "%s: query 3GPP2 app type [%d]\n",
	    __func__,(int)profile_id_3gpp2[i] );

    num_elements_expected = NUM_OMH_PROFILES_EXPECTED;

    /* query cdma profile id. */
    memset(&result_list, 0, sizeof(result_list) );
    rc = qmi_wds_utils_get_profile_list2( qmi_wds_omh_hndl,
					  QMI_WDS_PROFILE_TECH_3GPP2,
					  QMI_WDS_CDMA_PROFILE_APP_TYPE_PARAM_MASK,
					  (void *)&profile_id_3gpp2[i],
					  result_list,
					  &num_elements_expected,
					  &qmi_err_code );

    if (rc < 0)
    {
      fprintf(stderr, "%s: qmi error [%d] returned, "
	      "qmi_err_code [%d]\n",
	      __func__, rc, qmi_err_code);
      qmi_wds_omh_hndl = HNDL_INVALID;
      goto bail;
    }
    
    fprintf(stderr, "%s: profile query results - num[%d]\n",
	    __func__, num_elements_expected);
    for( j=0; j<num_elements_expected; j++ )
    {
      fprintf(stderr, "  profile[%d]: index[%lu] type[0x%x] name[%s]\n", j,
	      result_list[j].profile_index,
	      result_list[j].profile_type,
	      result_list[j].profile_name );
    }
   
    
  } while (++i < TEST_PROFILES);

  ret = SUCCESS;
  
 bail:  
  rc = qmi_wds_srvc_release_client(qmi_wds_omh_hndl,
				   &qmi_err_code);

  if (rc < 0)
  {
    fprintf(stderr, "%s: failed on qmi_wds_srvc_release_client, "
	    "qmi_err_code is set to [%d/%d]\n",
	    __func__, rc, qmi_err_code);
    qmi_wds_omh_hndl = HNDL_INVALID;
    return ret;
  }

  if( SUCCESS != ret )
  {
    fprintf(stderr, "%s EXIT failed\n", __func__);
  }
  else
  {
    fprintf(stderr, "%s EXIT success\n", __func__);
  }
  return ret;
}

int main(void)
{
  if (SUCCESS == omh_profile_test())
  {
    fprintf(stderr, "TEST PASSED\n");
  }
  else
  {
    fprintf(stderr, "TEST FAILED\n");
  }

  return 0;
}
