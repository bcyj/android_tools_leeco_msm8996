#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ds_string.h"
#include "qmi.h"
#include "qmi_wds_utils.h"
#include "qmi_wds_srvc.h"
#include "qmi_qos_srvc.h"

#define PROFILE_LIST_SIZE 6

#define TEST_PROFILE_APN_2 "dss_test2"
#define TEST_PROFILE_APN_3 "dss_test3"

#define DS_INVALID_PROFILE   5

/*qmi message library handle*/
int qmi_handle = QMI_INVALID_CLIENT_HANDLE; 

static int
set_profile_params
(
  int  qmi_wds_client,
  int  profile_index,
  char *profile_name,
  char *profile_apn
)
{
  qmi_wds_profile_id_type       profile_id;
  qmi_wds_profile_params_type   profile_params;
  int                           rc, qmi_err_code = 0;
  int                           create;
  profile_id.technology = QMI_WDS_PROFILE_TECH_3GPP;
  profile_id.profile_index = (long unsigned int)profile_index;

  memset(&profile_params, 0, sizeof(qmi_wds_profile_params_type));
  
  /* Query the profile... make sure it exists */
  if ((rc = qmi_wds_query_profile (qmi_wds_client,&profile_id,&profile_params,&qmi_err_code)) < 0)
  {
    /*retaining rc = QMI_SERVICE_ERR to make this test case compatible with non 2H09 modem data packages*/
    if ((rc == QMI_SERVICE_ERR  && qmi_err_code == QMI_SERVICE_ERR_INVALID_PROFILE) 
                               || (rc == QMI_EXTENDED_ERR && qmi_err_code == DS_INVALID_PROFILE))
    {
      fprintf (stdout," rc = %d, qmi_err_code = %d\n", rc, qmi_err_code);
      create = 1;
    }
    else
    {
      fprintf (stderr, "Unable to query state of profile %d, rc = %d, qmi_err_code =%x\n", (int) profile_id.profile_index, rc, qmi_err_code);
      return -1;
    }
  }
  else
  {
    fprintf (stdout,"Create = 0, modify = 1 rc = %d, qmi_err_code = %d\n", rc, qmi_err_code);
    create = 0;
  }

  profile_params.umts_profile_params.param_mask = (QMI_WDS_UMTS_PROFILE_NAME_PARAM_MASK | QMI_WDS_UMTS_PROFILE_APN_NAME_PARAM_MASK);

  strlcpy (profile_params.umts_profile_params.apn_name, profile_apn,
               strlen(profile_params.umts_profile_params.apn_name)+1);
  strlcpy (profile_params.umts_profile_params.profile_name, profile_name,
               strlen(profile_params.umts_profile_params.profile_name)+1);

  if (create)
  {
    if ((rc = qmi_wds_create_profile (qmi_wds_client,&profile_id,&profile_params,&qmi_err_code)) < 0)
    {
      fprintf (stderr,"Unable to create profile %d, %s, rc = %d, errcode = %x\n",profile_index, profile_apn, rc, qmi_err_code);
      return -1;
    }
    else
    {
      fprintf (stderr, "Created profile %d\n",(int)profile_id.profile_index);
    }
  }
  else
  {
    if ((rc = qmi_wds_modify_profile (qmi_wds_client,&profile_id,&profile_params,&qmi_err_code)) < 0)
    {
      fprintf (stderr,"Unable to modify profile %d, %s, rc = %d,errcode = %x\n",(int)profile_index, profile_apn, rc, qmi_err_code);
      return -1;
    }
    else
    {
      fprintf (stderr, "Modified profile %d\n",(int)profile_id.profile_index);
    }
  }
  
  return 0;
}  
    
int main(int argc, char *argv[])
{
  qmi_wds_profile_id_type       profile_id;
  qmi_wds_profile_params_type   default_profile_params;
  int rc, main_ret, i, qmi_err_code;
  int qmi_wds_client;
  qmi_wds_profile_list_type profile_list [PROFILE_LIST_SIZE];
  int profile_list_size;
  static char *profile_names[] =
  {
    "dss_test2",
    "dss_test3",
    "dss_test5",
    "dss_test6"
  };

  static char *profile_apns[] =
  {
    TEST_PROFILE_APN_2,
    TEST_PROFILE_APN_3,
    TEST_PROFILE_APN_2,
    TEST_PROFILE_APN_3
  };
  (void)argc; (void)argv;
   
  qmi_handle = qmi_init (NULL,NULL);

  if (qmi_handle < 0)
  {
    fprintf (stderr,"Unable to acquire qmi handle \n");
    fprintf (stderr, "Test failed!!\n");
    return -1;
  }

  /* Initialize connection to first QMI control connection (corresponds to SMD_DATA_5 for data transfer */
  if ((rc=qmi_connection_init (QMI_PORT_RMNET_1, &qmi_err_code)) < 0)
  {
    fprintf (stderr,"Unable to open QMI RMNET_1 port rc=%d, qmi_err_code=%x\n",rc,qmi_err_code);
    return -1;
  }

  fprintf (stdout,"Starting WDS service.... \n");

  /* Bring up WDS service for second port */
  if ((qmi_wds_client = qmi_wds_srvc_init_client (QMI_PORT_RMNET_1,NULL,NULL,&qmi_err_code)) < 0)
  {
    fprintf (stderr,"Unable to start WDS service rc = %d, qmi_err_code=%x\n",qmi_wds_client,qmi_err_code);
    return -1;
  }
  else
  {
    fprintf (stderr,"Open client handle rc = %d, handle= %x \n",rc,qmi_wds_client);
  }

  /* Initialize main return code */
  main_ret = 0;


  profile_id.technology = QMI_WDS_PROFILE_TECH_3GPP;

  /* Delete profiles #2 through #6 */
  for (i=2;i<=6;i++)
  {
    profile_id.profile_index = (long unsigned int)i;
    qmi_wds_delete_profile (qmi_wds_client,&profile_id,&qmi_err_code);
  }


  /* Query default profile */
  profile_id.profile_index = 1;

  if ((rc = qmi_wds_query_profile (qmi_wds_client,&profile_id,&default_profile_params,&qmi_err_code)) < 0)
  {
    fprintf (stderr, "Unable to query state of default profile, rc = %d, qmi_err_code =%x\n", rc, qmi_err_code);
    main_ret = -1;
    goto finish;
  }

  fprintf (stderr, "Default profile: profile_name = %s, apn=%s\n",
                         default_profile_params.umts_profile_params.profile_name,
                         default_profile_params.umts_profile_params.apn_name);

  /* Now we want to create profiles 2 & 3 */
  for (i=0; i<=1; i++)
  {
    if (set_profile_params (qmi_wds_client,i+2,profile_names[i],profile_apns[i]) < 0)
    {
      main_ret = -1;
      goto finish;
    }
  }

  /* create profile 4*/
  
  if (set_profile_params (qmi_wds_client,
                          (int)4,
                          "profile_4",
                          "dss_test_4") < 0)
  {
    main_ret = -1;
    goto finish;
  }
  
  /* Now we want to create profiles 5 & 6 */
  for (i=2; i<4; i++)
  {
    if (set_profile_params (qmi_wds_client,i+3,profile_names[i],profile_apns[i]) < 0)
    {
      main_ret = -1;
      goto finish;
    }
  }
  
  /* Query profiles to make sure everything looks good */
  profile_list_size = PROFILE_LIST_SIZE;

  if ((rc = qmi_wds_get_profile_list (qmi_wds_client,profile_list,&profile_list_size,&qmi_err_code)) < 0)
  {
    fprintf (stderr,"Unable to get profile list rc = %d, qmi_err_code=%d\n",qmi_wds_client,qmi_err_code);
    main_ret = -1;
    goto finish;
  }
  else
  {
    int i;
    for (i=0; i<profile_list_size; i++)
    {
      profile_id.technology = QMI_WDS_PROFILE_TECH_3GPP;
      profile_id.profile_index = profile_list[i].profile_index;
      if ((rc = qmi_wds_query_profile (qmi_wds_client,&profile_id,&default_profile_params,&qmi_err_code)) < 0)
      {
        fprintf (stderr, "Unable to query state of profile %d, rc = %d, qmi_err_code =%x\n", (int)profile_id.profile_index, rc, qmi_err_code);
        main_ret = -1;
        goto finish;
      }
      fprintf (stdout,"profile_index = %d, profile_name = %s, apn=%s\n",
                                                (int)profile_list[i].profile_index, 
                                                default_profile_params.umts_profile_params.profile_name,
                                                default_profile_params.umts_profile_params.apn_name );
    }
  }

finish:
  if (  (rc =qmi_wds_srvc_release_client (qmi_wds_client,&qmi_err_code)) < 0)
  {
    fprintf  (stderr,"Unable to release WDS handle, rc=%d, qmi_err_code=%x\n",rc,qmi_err_code);
  }
  else
  {
    fprintf (stdout,"released WDS client\n");
  }

  if (main_ret == 0)
  {
    fprintf (stdout, "Test ran sucessfully!!\n");
  }
  else
  {
    fprintf (stderr, "Test failed!!\n");
  }
  qmi_release(qmi_handle);
  return main_ret;
  
}




  
